#ifndef MEMORY_BLOCK_H
#define MEMORY_BLOCK_H

#include <cstddef>

using namespace std;

class MemoryBlock
{
public:
    size_t address;
    size_t size;
    bool is_free;
    int block_id;
    MemoryBlock *next;
    MemoryBlock *prev;

    MemoryBlock(size_t addr, size_t sz, bool free = true, int id = -1);
};

#endif
