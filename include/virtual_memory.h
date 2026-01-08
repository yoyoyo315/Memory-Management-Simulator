#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include <cstddef>
#include <vector>
#include <queue>
#include <map>

using namespace std;

enum class PageReplacementPolicy
{
    FIFO,
    LRU,
    CLOCK
};

struct PageTableEntry
{
    bool valid;          
    size_t frame_number;
    bool dirty;          
    bool reference;      
    int timestamp;       
    int load_time;       

    PageTableEntry() : valid(false), frame_number(0), dirty(false),
                       reference(false), timestamp(0), load_time(0) {}
};

class VirtualMemory
{
private:
    size_t virtual_address_bits;
    size_t physical_address_bits;
    size_t page_size;
    size_t num_virtual_pages;
    size_t num_physical_frames;

    vector<PageTableEntry> page_table;
    vector<bool> frame_allocation; 
    vector<size_t> frame_to_page; 

    PageReplacementPolicy policy;
    queue<size_t> fifo_queue;
    int clock_hand;
    int current_time;

    size_t page_faults;
    size_t page_hits;

    size_t getPageNumber(size_t virtual_address);
    size_t getOffset(size_t virtual_address);
    int findFreeFrame();
    int selectVictimPage();
    void loadPage(size_t page_number, size_t frame_number);
    void evictPage(size_t frame_number);

public:
    VirtualMemory(size_t virtual_bits, size_t physical_bits,
                  size_t page_sz, PageReplacementPolicy pol);

    size_t translate(size_t virtual_address);
    void accessPage(size_t virtual_address);
    void printStats();
    void printPageTable();

    size_t getPageFaults() const { return page_faults; }
    size_t getPageHits() const { return page_hits; }
    double getHitRatio() const;
};

#endif
