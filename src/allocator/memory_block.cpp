#include "../../include/memory_block.h"

using namespace std;

MemoryBlock::MemoryBlock(size_t addr, size_t sz, bool free, int id)
    : address(addr), size(sz), is_free(free), block_id(id), next(nullptr), prev(nullptr)
{
}
