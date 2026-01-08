#include "../../include/buddy_allocator.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

using namespace std;

BuddyAllocator::BuddyAllocator(size_t memory_size, size_t min_size)
    : min_block_size(min_size), next_block_id(1)
{

    // Ensure memory size is power of 2
    if (!isPowerOf2(memory_size))
    {
        memory_size = roundUpToPowerOf2(memory_size);
        cout << "Warning: Memory size rounded up to " << memory_size
             << " (power of 2)" << endl;
    }

    total_memory = memory_size;

    // Initialize free list with one large block
    BuddyBlock *initial_block = new BuddyBlock(0, total_memory, true);
    free_lists[total_memory].push_back(initial_block);
}

BuddyAllocator::~BuddyAllocator()
{
    // Clean up all blocks
    for (auto &pair : free_lists)
    {
        for (auto block : pair.second)
        {
            delete block;
        }
    }
    for (auto &pair : allocated_blocks)
    {
        delete pair.second;
    }
}

bool BuddyAllocator::isPowerOf2(size_t n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

size_t BuddyAllocator::roundUpToPowerOf2(size_t size)
{
    if (size == 0)
        return 1;

    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size |= size >> 32;
    size++;

    return size;
}

size_t BuddyAllocator::getBuddyAddress(size_t address, size_t size)
{
    return address ^ size;
}

BuddyBlock *BuddyAllocator::splitBlock(size_t size)
{
    // Find smallest available block that's >= size
    for (size_t block_size = size; block_size <= total_memory; block_size *= 2)
    {
        if (!free_lists[block_size].empty())
        {
            // Found a block, split it down to required size
            while (block_size > size)
            {
                BuddyBlock *block = free_lists[block_size].back();
                free_lists[block_size].pop_back();

                size_t half_size = block_size / 2;

                // Create two buddy blocks
                BuddyBlock *left = new BuddyBlock(block->address, half_size, true);
                BuddyBlock *right = new BuddyBlock(block->address + half_size, half_size, true);

                free_lists[half_size].push_back(left);
                free_lists[half_size].push_back(right);

                delete block;
                block_size = half_size;
            }

            // Return the block of required size
            if (!free_lists[size].empty())
            {
                BuddyBlock *result = free_lists[size].back();
                free_lists[size].pop_back();
                return result;
            }
        }
    }

    return nullptr;
}

void BuddyAllocator::mergeBuddies()
{
    bool merged = true;

    while (merged)
    {
        merged = false;

        for (size_t size = min_block_size; size < total_memory; size *= 2)
        {
            vector<BuddyBlock *> &blocks = free_lists[size];

            for (size_t i = 0; i < blocks.size(); i++)
            {
                size_t buddy_addr = getBuddyAddress(blocks[i]->address, size);

                // Find buddy
                for (size_t j = 0; j < blocks.size(); j++)
                {
                    if (i != j && blocks[j]->address == buddy_addr)
                    {
                        // Found buddy, merge them
                        size_t merged_addr = min(blocks[i]->address, blocks[j]->address);
                        BuddyBlock *merged_block = new BuddyBlock(merged_addr, size * 2, true);

                        free_lists[size * 2].push_back(merged_block);

                        // Remove the two buddies
                        BuddyBlock *first = blocks[i];
                        BuddyBlock *second = blocks[j];

                        blocks.erase(blocks.begin() + max(i, j));
                        blocks.erase(blocks.begin() + min(i, j));

                        delete first;
                        delete second;

                        merged = true;
                        break;
                    }
                }

                if (merged)
                    break;
            }

            if (merged)
                break;
        }
    }
}

int BuddyAllocator::malloc(size_t size)
{
    if (size == 0)
    {
        cerr << "Error: Cannot allocate 0 bytes" << endl;
        return -1;
    }

    // Round size up to power of 2, with minimum size
    size_t alloc_size = max(min_block_size, roundUpToPowerOf2(size));

    if (alloc_size > total_memory)
    {
        cerr << "Error: Requested size " << size << " (rounded to "
             << alloc_size << ") exceeds total memory" << endl;
        return -1;
    }

    BuddyBlock *block = splitBlock(alloc_size);

    if (block == nullptr)
    {
        cerr << "Error: No suitable block found for size " << size << endl;
        return -1;
    }

    block->is_free = false;
    block->block_id = next_block_id++;
    allocated_blocks[block->block_id] = block;
    requested_sizes[block->block_id] = size;

    cout << "Allocated block id=" << block->block_id
         << " at address=0x" << hex << setfill('0')
         << setw(4) << block->address << dec
         << " (requested: " << size << ", allocated: " << alloc_size << ")" << endl;

    return block->block_id;
}

bool BuddyAllocator::free(int block_id)
{
    auto it = allocated_blocks.find(block_id);

    if (it == allocated_blocks.end())
    {
        cerr << "Error: Block " << block_id << " not found" << endl;
        return false;
    }

    BuddyBlock *block = it->second;
    block->is_free = true;
    block->block_id = -1;

    // Add to free list
    free_lists[block->size].push_back(block);
    allocated_blocks.erase(it);
    requested_sizes.erase(block_id);

    // Try to merge with buddies
    mergeBuddies();

    cout << "Block " << block_id << " freed and merged with buddies" << endl;
    return true;
}

void BuddyAllocator::dumpMemory()
{
    cout << "\n=== Buddy Allocator Memory Dump ===" << endl;

    // Show allocated blocks
    cout << "Allocated Blocks:" << endl;
    for (const auto &pair : allocated_blocks)
    {
        BuddyBlock *block = pair.second;
        cout << "  [0x" << hex << setfill('0') << setw(4)
             << block->address << " - 0x" << setw(4)
             << (block->address + block->size - 1) << "] " << dec
             << "USED (id=" << block->block_id << ", size=" << block->size << ")" << endl;
    }

    // Show free blocks by size
    cout << "\nFree Blocks by Size:" << endl;
    for (const auto &pair : free_lists)
    {
        if (!pair.second.empty())
        {
            cout << "  Size " << pair.first << ": " << pair.second.size() << " blocks" << endl;
            for (const auto &block : pair.second)
            {
                cout << "    [0x" << hex << setfill('0') << setw(4)
                     << block->address << " - 0x" << setw(4)
                     << (block->address + block->size - 1) << "]" << dec << endl;
            }
        }
    }
    cout << endl;
}

double BuddyAllocator::calculateInternalFragmentation()
{
    if (allocated_blocks.empty())
        return 0.0;

    size_t total_allocated = 0;
    size_t total_wasted = 0;

    for (const auto &pair : allocated_blocks)
    {
        int block_id = pair.first;
        BuddyBlock *block = pair.second;
        size_t requested = requested_sizes[block_id];
        size_t allocated = block->size;

        total_allocated += allocated;
        total_wasted += (allocated - requested);
    }

    if (total_allocated == 0)
        return 0.0;

    return (double)total_wasted / total_allocated * 100.0;
}

void BuddyAllocator::printStats()
{
    size_t used_memory = 0;

    for (const auto &pair : allocated_blocks)
    {
        used_memory += pair.second->size;
    }

    cout << "\n=== Buddy Allocator Statistics ===" << endl;
    cout << "Total memory: " << total_memory << " bytes" << endl;
    cout << "Used memory: " << used_memory << " bytes" << endl;
    cout << "Free memory: " << (total_memory - used_memory) << " bytes" << endl;
    cout << "Memory utilization: " << fixed << setprecision(2) << (100.0 * used_memory / total_memory) << "%" << endl;
    cout << "Internal fragmentation: " << fixed << setprecision(2) << calculateInternalFragmentation() << "%" << endl;
    cout << "Minimum block size: " << min_block_size << " bytes" << endl;
    cout << "Number of allocated blocks: " << allocated_blocks.size() << endl;

    int total_free_blocks = 0;
    for (const auto &pair : free_lists)
    {
        total_free_blocks += pair.second.size();
    }
    cout << "Number of free blocks: " << total_free_blocks << endl;
    cout << endl;
}
