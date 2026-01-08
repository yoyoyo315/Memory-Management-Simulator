#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include "memory_block.h"
#include <string>
#include <map>

using namespace std;

enum class AllocationStrategy
{
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
};

class MemoryAllocator
{
private:
    MemoryBlock *head;
    size_t total_memory;
    size_t used_memory;
    AllocationStrategy strategy;
    int next_block_id;
    map<int, MemoryBlock *> allocated_blocks;
    map<int, size_t> requested_sizes;

    MemoryBlock *findFreeBlock(size_t size);
    void splitBlock(MemoryBlock *block, size_t size);
    void coalesceBlocks();
    double calculateExternalFragmentation();
    double calculateInternalFragmentation();

public:
    MemoryAllocator(size_t memory_size);
    ~MemoryAllocator();

    void setStrategy(AllocationStrategy strat);
    int malloc(size_t size);
    bool free(int block_id);
    void dumpMemory();
    void printStats();

    size_t getTotalMemory() const { return total_memory; }
    size_t getUsedMemory() const { return used_memory; }
};

#endif
