#include "../../include/memory_allocator.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

using namespace std;

MemoryAllocator::MemoryAllocator(size_t memory_size)
    : total_memory(memory_size), used_memory(0),
      strategy(AllocationStrategy::FIRST_FIT), next_block_id(1)
{
    head = new MemoryBlock(0, memory_size, true, -1);
}

MemoryAllocator::~MemoryAllocator()
{
    MemoryBlock *current = head;
    while (current != nullptr)
    {
        MemoryBlock *next = current->next;
        delete current;
        current = next;
    }
}

void MemoryAllocator::setStrategy(AllocationStrategy strat)
{
    strategy = strat;
}

MemoryBlock *MemoryAllocator::findFreeBlock(size_t size)
{
    MemoryBlock *current = head;
    MemoryBlock *best_fit = nullptr;
    MemoryBlock *worst_fit = nullptr;

    while (current != nullptr)
    {
        if (current->is_free && current->size >= size)
        {
            switch (strategy)
            {
            case AllocationStrategy::FIRST_FIT:
                return current;

            case AllocationStrategy::BEST_FIT:
                if (best_fit == nullptr || current->size < best_fit->size)
                {
                    best_fit = current;
                }
                break;

            case AllocationStrategy::WORST_FIT:
                if (worst_fit == nullptr || current->size > worst_fit->size)
                {
                    worst_fit = current;
                }
                break;
            }
        }
        current = current->next;
    }

    if (strategy == AllocationStrategy::BEST_FIT)
    {
        return best_fit;
    }
    else if (strategy == AllocationStrategy::WORST_FIT)
    {
        return worst_fit;
    }

    return nullptr;
}

void MemoryAllocator::splitBlock(MemoryBlock *block, size_t size)
{
    if (block->size > size)
    {
        // Create a new free block for the remaining space
        MemoryBlock *new_block = new MemoryBlock(
            block->address + size,
            block->size - size,
            true,
            -1);

        new_block->next = block->next;
        new_block->prev = block;

        if (block->next != nullptr)
        {
            block->next->prev = new_block;
        }

        block->next = new_block;
        block->size = size;
    }
}

void MemoryAllocator::coalesceBlocks()
{
    MemoryBlock *current = head;

    while (current != nullptr && current->next != nullptr)
    {
        if (current->is_free && current->next->is_free)
        {
            // Merge with next block
            MemoryBlock *next = current->next;
            current->size += next->size;
            current->next = next->next;

            if (next->next != nullptr)
            {
                next->next->prev = current;
            }

            delete next;
        }
        else
        {
            current = current->next;
        }
    }
}

int MemoryAllocator::malloc(size_t size)
{
    if (size == 0)
    {
        cerr << "Error: Cannot allocate 0 bytes" << endl;
        return -1;
    }

    MemoryBlock *block = findFreeBlock(size);

    if (block == nullptr)
    {
        cerr << "Error: No suitable block found for size " << size << endl;
        return -1;
    }

    splitBlock(block, size);

    block->is_free = false;
    block->block_id = next_block_id++;
    allocated_blocks[block->block_id] = block;
    requested_sizes[block->block_id] = size;
    used_memory += block->size;

    cout << "Allocated block id=" << block->block_id
         << " at address=0x" << hex << setfill('0')
         << setw(4) << block->address << dec << endl;

    return block->block_id;
}

bool MemoryAllocator::free(int block_id)
{
    auto it = allocated_blocks.find(block_id);

    if (it == allocated_blocks.end())
    {
        cerr << "Error: Block " << block_id << " not found" << endl;
        return false;
    }

    MemoryBlock *block = it->second;
    block->is_free = true;
    block->block_id = -1;
    used_memory -= block->size;
    allocated_blocks.erase(it);
    requested_sizes.erase(block_id);

    coalesceBlocks();

    cout << "Block " << block_id << " freed and merged" << endl;
    return true;
}

void MemoryAllocator::dumpMemory()
{
    MemoryBlock *current = head;

    cout << "\n=== Memory Dump ===" << endl;
    while (current != nullptr)
    {
        cout << "[0x" << hex << setfill('0') << setw(4)
             << current->address << " - 0x" << setw(4)
             << (current->address + current->size - 1) << "] " << dec;

        if (current->is_free)
        {
            cout << "FREE";
        }
        else
        {
            cout << "USED (id=" << current->block_id << ")";
        }
        cout << " [" << current->size << " bytes]" << endl;

        current = current->next;
    }
    cout << endl;
}

double MemoryAllocator::calculateExternalFragmentation()
{
    size_t total_free = 0;
    size_t largest_free = 0;

    MemoryBlock *current = head;
    while (current != nullptr)
    {
        if (current->is_free)
        {
            total_free += current->size;
            largest_free = max(largest_free, current->size);
        }
        current = current->next;
    }

    if (total_free == 0)
        return 0.0;

    return (1.0 - (double)largest_free / total_free) * 100.0;
}

double MemoryAllocator::calculateInternalFragmentation()
{
    // Internal fragmentation occurs when allocated block is larger than requested
    if (allocated_blocks.empty())
        return 0.0;

    size_t total_allocated = 0;
    size_t total_wasted = 0;

    for (const auto &pair : allocated_blocks)
    {
        int block_id = pair.first;
        MemoryBlock *block = pair.second;
        size_t requested = requested_sizes[block_id];
        size_t allocated = block->size;

        total_allocated += allocated;
        total_wasted += (allocated - requested);
    }

    if (total_allocated == 0)
        return 0.0;

    return (double)total_wasted / total_allocated * 100.0;
}

void MemoryAllocator::printStats()
{
    cout << "\n=== Memory Statistics ===" << endl;
    cout << "Total memory: " << total_memory << " bytes" << endl;
    cout << "Used memory: " << used_memory << " bytes" << endl;
    cout << "Free memory: " << (total_memory - used_memory) << " bytes" << endl;
    cout << "Memory utilization: " << fixed << setprecision(2) << (100.0 * used_memory / total_memory) << "%" << endl;
    cout << "External fragmentation: " << fixed << setprecision(2) << calculateExternalFragmentation() << "%" << endl;
    cout << "Internal fragmentation: " << fixed << setprecision(2) << calculateInternalFragmentation() << "%" << endl;

    int free_blocks = 0;
    MemoryBlock *current = head;
    while (current != nullptr)
    {
        if (current->is_free)
            free_blocks++;
        current = current->next;
    }
    cout << "Number of free blocks: " << free_blocks << endl;
    cout << "Number of allocated blocks: " << allocated_blocks.size() << endl;
    cout << endl;
}
