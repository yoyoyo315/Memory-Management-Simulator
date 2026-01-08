#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include <cstddef>
#include <vector>
#include <map>

using namespace std;

struct BuddyBlock
{
    size_t address;
    size_t size;
    bool is_free;
    int block_id;

    BuddyBlock(size_t addr, size_t sz, bool free = true, int id = -1)
        : address(addr), size(sz), is_free(free), block_id(id) {}
};

class BuddyAllocator
{
private:
    size_t total_memory;
    size_t min_block_size;
    map<size_t, vector<BuddyBlock *>> free_lists;
    map<int, BuddyBlock *> allocated_blocks;
    map<int, size_t> requested_sizes;
    int next_block_id;

    size_t roundUpToPowerOf2(size_t size);
    bool isPowerOf2(size_t n);
    size_t getBuddyAddress(size_t address, size_t size);
    BuddyBlock *splitBlock(size_t size);
    void mergeBuddies();
    double calculateInternalFragmentation();

public:
    BuddyAllocator(size_t memory_size, size_t min_size = 64);
    ~BuddyAllocator();

    int malloc(size_t size);
    bool free(int block_id);
    void dumpMemory();
    void printStats();

    size_t getTotalMemory() const { return total_memory; }
};

#endif
