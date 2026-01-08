#ifndef CACHE_H
#define CACHE_H

#include <cstddef>
#include <vector>
#include <list>
#include <map>
#include <string>

using namespace std;

enum class ReplacementPolicy
{
    FIFO,
    LRU,
    LFU
};

struct CacheLine
{
    bool valid;
    size_t tag;
    size_t data;
    int frequency;
    int timestamp;

    CacheLine() : valid(false), tag(0), data(0), frequency(0), timestamp(0) {}
};

class CacheLevel
{
private:
    size_t cache_size;
    size_t block_size;
    size_t num_sets;
    size_t associativity;
    ReplacementPolicy policy;

    vector<vector<CacheLine>> cache_sets;
    map<size_t, list<size_t>> lru_lists;

    int current_time;
    size_t hits;
    size_t misses;

    size_t getSetIndex(size_t address);
    size_t getTag(size_t address);
    int findVictim(size_t set_index);
    void updateLRU(size_t set_index, size_t way);

public:
    CacheLevel(size_t size, size_t block_sz, size_t assoc, ReplacementPolicy pol);

    bool access(size_t address, size_t &data);
    void insert(size_t address, size_t data);
    void printStats(const string &level_name);
    double getHitRatio() const;

    size_t getHits() const { return hits; }
    size_t getMisses() const { return misses; }
};

class CacheHierarchy
{
private:
    vector<CacheLevel *> levels;
    size_t *main_memory;
    size_t memory_size;

public:
    CacheHierarchy(size_t mem_size);
    ~CacheHierarchy();

    void addLevel(size_t size, size_t block_size, size_t associativity, ReplacementPolicy policy);
    size_t read(size_t address);
    void write(size_t address, size_t data);
    void printAllStats();
};

#endif
