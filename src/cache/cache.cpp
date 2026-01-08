#include "../../include/cache.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

using namespace std;

CacheLevel::CacheLevel(size_t size, size_t block_sz, size_t assoc, ReplacementPolicy pol)
    : cache_size(size), block_size(block_sz), associativity(assoc), policy(pol),
      current_time(0), hits(0), misses(0)
{

    num_sets = cache_size / (block_size * associativity);

    cache_sets.resize(num_sets);
    for (size_t i = 0; i < num_sets; i++)
    {
        cache_sets[i].resize(associativity);
    }
}

size_t CacheLevel::getSetIndex(size_t address)
{
    return (address / block_size) % num_sets;
}

size_t CacheLevel::getTag(size_t address)
{
    return address / (block_size * num_sets);
}

int CacheLevel::findVictim(size_t set_index)
{
    vector<CacheLine> &set = cache_sets[set_index];

    for (size_t i = 0; i < associativity; i++)
    {
        if (!set[i].valid)
        {
            return i;
        }
    }

    switch (policy)
    {
    case ReplacementPolicy::FIFO:
    {
        int oldest_way = 0;
        int oldest_time = set[0].timestamp;
        for (size_t i = 1; i < associativity; i++)
        {
            if (set[i].timestamp < oldest_time)
            {
                oldest_time = set[i].timestamp;
                oldest_way = i;
            }
        }
        return oldest_way;
    }

    case ReplacementPolicy::LRU:
    {
        int lru_way = 0;
        int oldest_time = set[0].timestamp;
        for (size_t i = 1; i < associativity; i++)
        {
            if (set[i].timestamp < oldest_time)
            {
                oldest_time = set[i].timestamp;
                lru_way = i;
            }
        }
        return lru_way;
    }

    case ReplacementPolicy::LFU:
    {
        int lfu_way = 0;
        int min_freq = set[0].frequency;
        for (size_t i = 1; i < associativity; i++)
        {
            if (set[i].frequency < min_freq)
            {
                min_freq = set[i].frequency;
                lfu_way = i;
            }
        }
        return lfu_way;
    }
    }

    return 0;
}

void CacheLevel::updateLRU(size_t set_index, size_t way)
{
    cache_sets[set_index][way].timestamp = current_time++;
    if (policy == ReplacementPolicy::LFU)
    {
        cache_sets[set_index][way].frequency++;
    }
}

bool CacheLevel::access(size_t address, size_t &data)
{
    size_t set_index = getSetIndex(address);
    size_t tag = getTag(address);

    vector<CacheLine> &set = cache_sets[set_index];

    // Check for hit
    for (size_t i = 0; i < associativity; i++)
    {
        if (set[i].valid && set[i].tag == tag)
        {
            // Cache hit
            hits++;
            data = set[i].data;
            updateLRU(set_index, i);
            return true;
        }
    }

    // Cache miss
    misses++;
    return false;
}

void CacheLevel::insert(size_t address, size_t data)
{
    size_t set_index = getSetIndex(address);
    size_t tag = getTag(address);

    int victim_way = findVictim(set_index);

    CacheLine &line = cache_sets[set_index][victim_way];
    line.valid = true;
    line.tag = tag;
    line.data = data;
    line.timestamp = current_time++;
    if (policy == ReplacementPolicy::LFU)
    {
        line.frequency = 1;
    }
}

void CacheLevel::printStats(const string &level_name)
{
    size_t total_accesses = hits + misses;
    double hit_ratio = total_accesses > 0 ? (100.0 * hits / total_accesses) : 0.0;

    cout << level_name << " Cache Statistics:" << endl;
    cout << "  Hits: " << hits << endl;
    cout << "  Misses: " << misses << endl;
    cout << "  Total Accesses: " << total_accesses << endl;
    cout << "  Hit Ratio: " << fixed << setprecision(2)
         << hit_ratio << "%" << endl;
}

double CacheLevel::getHitRatio() const
{
    size_t total = hits + misses;
    return total > 0 ? (100.0 * hits / total) : 0.0;
}

CacheHierarchy::CacheHierarchy(size_t mem_size)
    : memory_size(mem_size)
{
    main_memory = new size_t[memory_size];

    for (size_t i = 0; i < memory_size; i++)
    {
        main_memory[i] = 0;
    }
}

CacheHierarchy::~CacheHierarchy()
{
    for (auto level : levels)
    {
        delete level;
    }
    delete[] main_memory;
}

void CacheHierarchy::addLevel(size_t size, size_t block_size,
                              size_t associativity, ReplacementPolicy policy)
{
    levels.push_back(new CacheLevel(size, block_size, associativity, policy));
}

size_t CacheHierarchy::read(size_t address)
{
    if (address >= memory_size)
    {
        cerr << "Error: Address " << address << " out of bounds" << endl;
        return 0;
    }

    size_t data;

    for (size_t i = 0; i < levels.size(); i++)
    {
        if (levels[i]->access(address, data))
        {
            return data;
        }
    }

    data = main_memory[address];

    for (auto level : levels)
    {
        level->insert(address, data);
    }

    return data;
}

void CacheHierarchy::write(size_t address, size_t data)
{
    if (address >= memory_size)
    {
        cerr << "Error: Address " << address << " out of bounds" << endl;
        return;
    }

    main_memory[address] = data;

    for (auto level : levels)
    {
        size_t dummy;
        if (level->access(address, dummy))
        {
            level->insert(address, data);
        }
    }
}

void CacheHierarchy::printAllStats()
{
    cout << "\n=== Cache Hierarchy Statistics ===" << endl;

    for (size_t i = 0; i < levels.size(); i++)
    {
        string level_name = "L" + to_string(i + 1);
        levels[i]->printStats(level_name);
        cout << endl;
    }
}
