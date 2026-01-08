#include "../../include/virtual_memory.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

using namespace std;

VirtualMemory::VirtualMemory(size_t virtual_bits, size_t physical_bits,
                             size_t page_sz, PageReplacementPolicy pol)
    : virtual_address_bits(virtual_bits), physical_address_bits(physical_bits),
      page_size(page_sz), policy(pol), clock_hand(0), current_time(0),
      page_faults(0), page_hits(0)
{

    num_virtual_pages = (1 << virtual_bits) / page_size;
    num_physical_frames = (1 << physical_bits) / page_size;

    page_table.resize(num_virtual_pages);
    frame_allocation.resize(num_physical_frames, false);
    frame_to_page.resize(num_physical_frames, 0);

    cout << "Virtual Memory initialized:" << endl;
    cout << "  Virtual address space: " << (1 << virtual_bits) << " bytes" << endl;
    cout << "  Physical address space: " << (1 << physical_bits) << " bytes" << endl;
    cout << "  Page size: " << page_size << " bytes" << endl;
    cout << "  Virtual pages: " << num_virtual_pages << endl;
    cout << "  Physical frames: " << num_physical_frames << endl;
}

size_t VirtualMemory::getPageNumber(size_t virtual_address)
{
    return virtual_address / page_size;
}

size_t VirtualMemory::getOffset(size_t virtual_address)
{
    return virtual_address % page_size;
}

int VirtualMemory::findFreeFrame()
{
    for (size_t i = 0; i < num_physical_frames; i++)
    {
        if (!frame_allocation[i])
        {
            return i;
        }
    }
    return -1;
}

int VirtualMemory::selectVictimPage()
{
    switch (policy)
    {
    case PageReplacementPolicy::FIFO:
    {
        if (fifo_queue.empty())
        {
            return -1;
        }
        size_t victim_page = fifo_queue.front();
        fifo_queue.pop();
        return victim_page;
    }

    case PageReplacementPolicy::LRU:
    {
        int lru_page = -1;
        int oldest_time = current_time;

        for (size_t i = 0; i < num_virtual_pages; i++)
        {
            if (page_table[i].valid && page_table[i].timestamp < oldest_time)
            {
                oldest_time = page_table[i].timestamp;
                lru_page = i;
            }
        }
        return lru_page;
    }

    case PageReplacementPolicy::CLOCK:
    {
        while (true)
        {
            size_t frame = clock_hand;
            size_t page = frame_to_page[frame];

            if (page_table[page].valid && page_table[page].frame_number == frame)
            {
                if (!page_table[page].reference)
                {
                    clock_hand = (clock_hand + 1) % num_physical_frames;
                    return page;
                }
                else
                {
                    page_table[page].reference = false;
                }
            }

            clock_hand = (clock_hand + 1) % num_physical_frames;
        }
    }
    }

    return -1;
}

void VirtualMemory::loadPage(size_t page_number, size_t frame_number)
{
    page_table[page_number].valid = true;
    page_table[page_number].frame_number = frame_number;
    page_table[page_number].timestamp = current_time++;
    page_table[page_number].load_time = current_time;
    page_table[page_number].reference = true;

    frame_allocation[frame_number] = true;
    frame_to_page[frame_number] = page_number;

    if (policy == PageReplacementPolicy::FIFO)
    {
        fifo_queue.push(page_number);
    }
}

void VirtualMemory::evictPage(size_t frame_number)
{
    size_t old_page = frame_to_page[frame_number];

    if (page_table[old_page].valid && page_table[old_page].frame_number == frame_number)
    {
        page_table[old_page].valid = false;

        if (page_table[old_page].dirty)
        {
            cout << "  [Write-back] Page " << old_page << " written to disk" << endl;
        }
    }

    frame_allocation[frame_number] = false;
}

size_t VirtualMemory::translate(size_t virtual_address)
{
    size_t page_number = getPageNumber(virtual_address);
    size_t offset = getOffset(virtual_address);

    if (page_number >= num_virtual_pages)
    {
        cerr << "Error: Invalid virtual address " << virtual_address << endl;
        return 0;
    }

    if (!page_table[page_number].valid)
    {
        // Page fault
        cout << "[Page Fault] Virtual address 0x" << hex << virtual_address
             << " (page " << dec << page_number << ")" << endl;
        page_faults++;

        // Find free frame or evict
        int frame = findFreeFrame();

        if (frame == -1)
        {
            // Need to evict
            int victim_page = selectVictimPage();

            if (victim_page >= 0)
            {
                frame = page_table[victim_page].frame_number;
                cout << "  [Eviction] Page " << victim_page
                     << " evicted from frame " << frame << endl;
                evictPage(frame);
            }
            else
            {
                cerr << "Error: Cannot find victim page" << endl;
                return 0;
            }
        }

        // Load page
        cout << "  [Load] Page " << page_number << " loaded into frame "
             << frame << endl;
        loadPage(page_number, frame);
    }
    else
    {
        // Page hit
        page_hits++;
        page_table[page_number].timestamp = current_time++;
        page_table[page_number].reference = true;
    }

    size_t physical_address = page_table[page_number].frame_number * page_size + offset;
    return physical_address;
}

void VirtualMemory::accessPage(size_t virtual_address)
{
    size_t physical_address = translate(virtual_address);
    cout << "Virtual 0x" << hex << virtual_address
         << " -> Physical 0x" << physical_address << dec << endl;
}

void VirtualMemory::printStats()
{
    size_t total_accesses = page_hits + page_faults;
    double hit_ratio = total_accesses > 0 ? (100.0 * page_hits / total_accesses) : 0.0;

    cout << "\n=== Virtual Memory Statistics ===" << endl;
    cout << "Page hits: " << page_hits << endl;
    cout << "Page faults: " << page_faults << endl;
    cout << "Total accesses: " << total_accesses << endl;
    cout << "Hit ratio: " << fixed << setprecision(2)
         << hit_ratio << "%" << endl;

    int valid_pages = 0;
    for (const auto &entry : page_table)
    {
        if (entry.valid)
            valid_pages++;
    }
    cout << "Pages in memory: " << valid_pages << " / " << num_physical_frames << endl;
    cout << endl;
}

void VirtualMemory::printPageTable()
{
    cout << "\n=== Page Table ===" << endl;
    cout << "Page | Valid | Frame | Dirty | Ref | Time" << endl;
    cout << "-----+-------+-------+-------+-----+------" << endl;

    for (size_t i = 0; i < num_virtual_pages; i++)
    {
        if (page_table[i].valid)
        {
            cout << setw(4) << i << " | "
                 << (page_table[i].valid ? "  Y  " : "  N  ") << " | "
                 << setw(5) << page_table[i].frame_number << " | "
                 << (page_table[i].dirty ? "  Y  " : "  N  ") << " | "
                 << (page_table[i].reference ? " Y " : " N ") << " | "
                 << setw(4) << page_table[i].timestamp << endl;
        }
    }
    cout << endl;
}

double VirtualMemory::getHitRatio() const
{
    size_t total = page_hits + page_faults;
    return total > 0 ? (100.0 * page_hits / total) : 0.0;
}
