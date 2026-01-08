#include "../include/memory_allocator.h"
#include "../include/buddy_allocator.h"
#include "../include/cache.h"
#include "../include/virtual_memory.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

class MemorySimulator
{
private:
    MemoryAllocator *allocator;
    BuddyAllocator *buddy_allocator;
    CacheHierarchy *cache_hierarchy;
    VirtualMemory *virtual_memory;

    bool memory_initialized;
    bool cache_initialized;
    bool vm_initialized;
    bool using_buddy;

    void printHelp()
    {
        cout << "\n=== Memory Management Simulator ===" << endl;
        cout << "\nMemory Allocation Commands:" << endl;
        cout << "  init memory <size>              - Initialize physical memory" << endl;
        cout << "  set allocator <type>            - Set allocator (first_fit/best_fit/worst_fit/buddy)" << endl;
        cout << "  malloc <size>                   - Allocate memory block" << endl;
        cout << "  free <block_id>                 - Free memory block" << endl;
        cout << "  dump memory                     - Show memory layout" << endl;
        cout << "  stats                           - Show memory statistics" << endl;

        cout << "\nCache Commands:" << endl;
        cout << "  init cache <mem_size>           - Initialize cache hierarchy and memory" << endl;
        cout << "  add cache <size> <block> <assoc> <policy>" << endl;
        cout << "                                  - Add cache level (policy: fifo/lru/lfu)" << endl;
        cout << "  cache read <address>            - Read from address through cache" << endl;
        cout << "  cache write <address> <data>    - Write to address through cache" << endl;
        cout << "  cache stats                     - Show cache statistics" << endl;

        cout << "\nVirtual Memory Commands:" << endl;
        cout << "  init vm <virt_bits> <phys_bits> <page_size> <policy>" << endl;
        cout << "                                  - Initialize virtual memory (policy: fifo/lru/clock)" << endl;
        cout << "  vm access <address>             - Access virtual address" << endl;
        cout << "  vm stats                        - Show VM statistics" << endl;
        cout << "  vm pagetable                    - Show page table" << endl;

        cout << "\nIntegrated VM+Cache Commands:" << endl;
        cout << "  init integrated <virt_bits> <phys_bits> <page_size> <policy>" << endl;
        cout << "                                  - Initialize integrated VM+Cache system" << endl;
        cout << "  integrated read <virt_addr>     - Read via VM → Cache → Memory" << endl;
        cout << "  integrated write <virt_addr> <data> - Write via VM → Cache → Memory" << endl;
        cout << "  integrated stats                - Show combined statistics" << endl;

        cout << "\nGeneral Commands:" << endl;
        cout << "  help                            - Show this help message" << endl;
        cout << "  exit                            - Exit simulator" << endl;
        cout << endl;
    }

    void handleInitMemory(istringstream &iss)
    {
        size_t size;
        if (!(iss >> size))
        {
            cerr << "Usage: init memory <size>" << endl;
            return;
        }

        if (allocator)
            delete allocator;
        if (buddy_allocator)
            delete buddy_allocator;

        allocator = new MemoryAllocator(size);
        buddy_allocator = nullptr;
        memory_initialized = true;
        using_buddy = false;

        cout << "Memory initialized: " << size << " bytes" << endl;
    }

    void handleSetAllocator(istringstream &iss)
    {
        string type;
        if (!(iss >> type))
        {
            cerr << "Usage: set allocator <type>" << endl;
            return;
        }

        transform(type.begin(), type.end(), type.begin(), ::tolower);

        if (type == "buddy")
        {
            if (!memory_initialized || !allocator)
            {
                cerr << "Error: Initialize memory first" << endl;
                return;
            }

            size_t total_mem = allocator->getTotalMemory();
            delete allocator;
            allocator = nullptr;

            buddy_allocator = new BuddyAllocator(total_mem);
            using_buddy = true;
            cout << "Switched to buddy allocator" << endl;
        }
        else
        {
            if (using_buddy && buddy_allocator)
            {
                size_t total_mem = buddy_allocator->getTotalMemory();
                delete buddy_allocator;
                buddy_allocator = nullptr;

                allocator = new MemoryAllocator(total_mem);
                using_buddy = false;
            }

            if (!allocator)
            {
                cerr << "Error: Initialize memory first" << endl;
                return;
            }

            if (type == "first_fit")
            {
                allocator->setStrategy(AllocationStrategy::FIRST_FIT);
                cout << "Allocator set to: First Fit" << endl;
            }
            else if (type == "best_fit")
            {
                allocator->setStrategy(AllocationStrategy::BEST_FIT);
                cout << "Allocator set to: Best Fit" << endl;
            }
            else if (type == "worst_fit")
            {
                allocator->setStrategy(AllocationStrategy::WORST_FIT);
                cout << "Allocator set to: Worst Fit" << endl;
            }
            else
            {
                cerr << "Unknown allocator type: " << type << endl;
                cerr << "Options: first_fit, best_fit, worst_fit, buddy" << endl;
            }
        }
    }

    void handleMalloc(istringstream &iss)
    {
        size_t size;
        if (!(iss >> size))
        {
            cerr << "Usage: malloc <size>" << endl;
            return;
        }

        if (!memory_initialized)
        {
            cerr << "Error: Initialize memory first" << endl;
            return;
        }

        if (using_buddy)
        {
            buddy_allocator->malloc(size);
        }
        else
        {
            allocator->malloc(size);
        }
    }

    void handleFree(istringstream &iss)
    {
        int block_id;
        if (!(iss >> block_id))
        {
            cerr << "Usage: free <block_id>" << endl;
            return;
        }

        if (!memory_initialized)
        {
            cerr << "Error: Initialize memory first" << endl;
            return;
        }

        if (using_buddy)
        {
            buddy_allocator->free(block_id);
        }
        else
        {
            allocator->free(block_id);
        }
    }

    void handleDumpMemory()
    {
        if (!memory_initialized)
        {
            cerr << "Error: Initialize memory first" << endl;
            return;
        }

        if (using_buddy)
        {
            buddy_allocator->dumpMemory();
        }
        else
        {
            allocator->dumpMemory();
        }
    }

    void handleStats()
    {
        if (!memory_initialized)
        {
            cerr << "Error: Initialize memory first" << endl;
            return;
        }

        if (using_buddy)
        {
            buddy_allocator->printStats();
        }
        else
        {
            allocator->printStats();
        }
    }

    void handleInitCache(istringstream &iss)
    {
        size_t mem_size;
        if (!(iss >> mem_size))
        {
            cerr << "Usage: init cache <memory_size>" << endl;
            return;
        }

        if (cache_hierarchy)
            delete cache_hierarchy;

        cache_hierarchy = new CacheHierarchy(mem_size);
        cache_initialized = true;

        cout << "Cache hierarchy initialized with " << mem_size << " bytes of memory" << endl;
    }

    void handleAddCache(istringstream &iss)
    {
        size_t size, block_size, assoc;
        string policy_str;

        if (!(iss >> size >> block_size >> assoc >> policy_str))
        {
            cerr << "Usage: add cache <size> <block_size> <associativity> <policy>" << endl;
            return;
        }

        if (!cache_initialized)
        {
            cerr << "Error: Initialize cache first" << endl;
            return;
        }

        transform(policy_str.begin(), policy_str.end(), policy_str.begin(), ::tolower);

        ReplacementPolicy policy;
        if (policy_str == "fifo")
        {
            policy = ReplacementPolicy::FIFO;
        }
        else if (policy_str == "lru")
        {
            policy = ReplacementPolicy::LRU;
        }
        else if (policy_str == "lfu")
        {
            policy = ReplacementPolicy::LFU;
        }
        else
        {
            cerr << "Unknown policy: " << policy_str << endl;
            return;
        }

        cache_hierarchy->addLevel(size, block_size, assoc, policy);
        cout << "Added cache level: " << size << " bytes, " << block_size
             << " block size, " << assoc << "-way, " << policy_str << endl;
    }

    void handleCacheRead(istringstream &iss)
    {
        size_t address;
        if (!(iss >> address))
        {
            cerr << "Usage: cache read <address>" << endl;
            return;
        }

        if (!cache_initialized)
        {
            cerr << "Error: Initialize cache first" << endl;
            return;
        }

        size_t data = cache_hierarchy->read(address);
        cout << "Read address " << address << ": data = " << data << endl;
    }

    void handleCacheWrite(istringstream &iss)
    {
        size_t address, data;
        if (!(iss >> address >> data))
        {
            cerr << "Usage: cache write <address> <data>" << endl;
            return;
        }

        if (!cache_initialized)
        {
            cerr << "Error: Initialize cache first" << endl;
            return;
        }

        cache_hierarchy->write(address, data);
        cout << "Wrote " << data << " to address " << address << endl;
    }

    void handleCacheStats()
    {
        if (!cache_initialized)
        {
            cerr << "Error: Initialize cache first" << endl;
            return;
        }

        cache_hierarchy->printAllStats();
    }

    void handleInitVM(istringstream &iss)
    {
        size_t virt_bits, phys_bits, page_size;
        string policy_str;

        if (!(iss >> virt_bits >> phys_bits >> page_size >> policy_str))
        {
            cerr << "Usage: init vm <virtual_bits> <physical_bits> <page_size> <policy>" << endl;
            return;
        }

        transform(policy_str.begin(), policy_str.end(), policy_str.begin(), ::tolower);

        PageReplacementPolicy policy;
        if (policy_str == "fifo")
        {
            policy = PageReplacementPolicy::FIFO;
        }
        else if (policy_str == "lru")
        {
            policy = PageReplacementPolicy::LRU;
        }
        else if (policy_str == "clock")
        {
            policy = PageReplacementPolicy::CLOCK;
        }
        else
        {
            cerr << "Unknown policy: " << policy_str << endl;
            return;
        }

        if (virtual_memory)
            delete virtual_memory;

        virtual_memory = new VirtualMemory(virt_bits, phys_bits, page_size, policy);
        vm_initialized = true;
    }

    void handleVMAccess(istringstream &iss)
    {
        size_t address;
        if (!(iss >> address))
        {
            cerr << "Usage: vm access <address>" << endl;
            return;
        }

        if (!vm_initialized)
        {
            cerr << "Error: Initialize virtual memory first" << endl;
            return;
        }

        virtual_memory->accessPage(address);
    }

    void handleVMStats()
    {
        if (!vm_initialized)
        {
            cerr << "Error: Initialize virtual memory first" << endl;
            return;
        }

        virtual_memory->printStats();
    }

    void handleVMPageTable()
    {
        if (!vm_initialized)
        {
            cerr << "Error: Initialize virtual memory first" << endl;
            return;
        }

        virtual_memory->printPageTable();
    }

    // Integrated VM + Cache handlers
    void handleInitIntegrated(istringstream &iss)
    {
        size_t virt_bits, phys_bits, page_size;
        string policy_str;

        if (!(iss >> virt_bits >> phys_bits >> page_size >> policy_str))
        {
            cerr << "Usage: init integrated <virt_bits> <phys_bits> <page_size> <policy>" << endl;
            return;
        }

        transform(policy_str.begin(), policy_str.end(), policy_str.begin(), ::tolower);

        PageReplacementPolicy policy;
        if (policy_str == "fifo")
        {
            policy = PageReplacementPolicy::FIFO;
        }
        else if (policy_str == "lru")
        {
            policy = PageReplacementPolicy::LRU;
        }
        else if (policy_str == "clock")
        {
            policy = PageReplacementPolicy::CLOCK;
        }
        else
        {
            cerr << "Unknown policy: " << policy_str << endl;
            return;
        }

        // Initialize VM
        if (virtual_memory)
            delete virtual_memory;
        virtual_memory = new VirtualMemory(virt_bits, phys_bits, page_size, policy);
        vm_initialized = true;

        // Initialize Cache with physical memory size
        size_t phys_mem_size = (1 << phys_bits);
        if (cache_hierarchy)
            delete cache_hierarchy;
        cache_hierarchy = new CacheHierarchy(phys_mem_size);
        cache_initialized = true;

        // Add default cache levels (L1 and L2)
        cache_hierarchy->addLevel(512, 16, 4, ReplacementPolicy::LRU);
        cache_hierarchy->addLevel(2048, 32, 8, ReplacementPolicy::LRU);

        cout << "\nIntegrated VM+Cache system initialized:" << endl;
        cout << "  L1 Cache: 512B, 16B blocks, 4-way, LRU" << endl;
        cout << "  L2 Cache: 2KB, 32B blocks, 8-way, LRU" << endl;
        cout << "  (Use 'add cache' to customize cache levels)" << endl;
    }

    void handleIntegratedRead(istringstream &iss)
    {
        size_t virt_addr;
        if (!(iss >> virt_addr))
        {
            cerr << "Usage: integrated read <virtual_address>" << endl;
            return;
        }

        if (!vm_initialized || !cache_initialized)
        {
            cerr << "Error: Initialize integrated system first" << endl;
            return;
        }

        cout << "\n[Integrated Access Flow]" << endl;
        cout << "1. Virtual Address: 0x" << hex << virt_addr << dec << endl;

        // Step 1: Translate virtual to physical address
        size_t phys_addr = virtual_memory->translate(virt_addr);
        cout << "2. Page Table Translation: Virtual 0x" << hex << virt_addr
             << " -> Physical 0x" << phys_addr << dec << endl;

        // Step 2: Access through cache hierarchy
        cout << "3. Cache Hierarchy Access:" << endl;
        size_t data = cache_hierarchy->read(phys_addr);
        cout << "4. Data Retrieved: " << data << endl;
        cout << "   [Flow: Virtual Address → Page Table → Physical Address → Cache → Memory]" << endl;
    }

    void handleIntegratedWrite(istringstream &iss)
    {
        size_t virt_addr, data;
        if (!(iss >> virt_addr >> data))
        {
            cerr << "Usage: integrated write <virtual_address> <data>" << endl;
            return;
        }

        if (!vm_initialized || !cache_initialized)
        {
            cerr << "Error: Initialize integrated system first" << endl;
            return;
        }

        cout << "\n[Integrated Write Flow]" << endl;
        cout << "1. Virtual Address: 0x" << hex << virt_addr << dec << endl;

        // Step 1: Translate virtual to physical address
        size_t phys_addr = virtual_memory->translate(virt_addr);
        cout << "2. Page Table Translation: Virtual 0x" << hex << virt_addr
             << " -> Physical 0x" << phys_addr << dec << endl;

        // Step 2: Write through cache hierarchy
        cout << "3. Cache Hierarchy Write:" << endl;
        cache_hierarchy->write(phys_addr, data);
        cout << "4. Data Written: " << data << endl;
        cout << "   [Flow: Virtual Address → Page Table → Physical Address → Cache → Memory]" << endl;
    }

    void handleIntegratedStats()
    {
        if (!vm_initialized || !cache_initialized)
        {
            cerr << "Error: Initialize integrated system first" << endl;
            return;
        }

        cout << "\n=== Integrated System Statistics ===" << endl;
        cout << "\n--- Virtual Memory ---" << endl;
        virtual_memory->printStats();
        cout << "\n--- Cache Hierarchy ---" << endl;
        cache_hierarchy->printAllStats();
    }

public:
    MemorySimulator()
        : allocator(nullptr), buddy_allocator(nullptr),
          cache_hierarchy(nullptr), virtual_memory(nullptr),
          memory_initialized(false), cache_initialized(false),
          vm_initialized(false), using_buddy(false)
    {
    }

    ~MemorySimulator()
    {
        if (allocator)
            delete allocator;
        if (buddy_allocator)
            delete buddy_allocator;
        if (cache_hierarchy)
            delete cache_hierarchy;
        if (virtual_memory)
            delete virtual_memory;
    }

    void run()
    {
        cout << "Memory Management Simulator" << endl;
        cout << "Type 'help' for available commands" << endl;

        string line;
        bool is_interactive = isatty(STDIN_FILENO);

        while (true)
        {
            if (is_interactive)
            {
                cout << "\n> ";
            }
            if (!getline(cin, line))
            {
                break;
            }

            istringstream iss(line);
            string command;
            iss >> command;

            if (command.empty())
            {
                continue;
            }

            // Skip comment lines starting with #
            if (command[0] == '#')
            {
                continue;
            }

            transform(command.begin(), command.end(), command.begin(), ::tolower);

            if (command == "exit" || command == "quit")
            {
                cout << "Exiting simulator..." << endl;
                break;
            }
            else if (command == "help")
            {
                printHelp();
            }
            else if (command == "init")
            {
                string subcommand;
                iss >> subcommand;
                transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);

                if (subcommand == "memory")
                {
                    handleInitMemory(iss);
                }
                else if (subcommand == "cache")
                {
                    handleInitCache(iss);
                }
                else if (subcommand == "vm")
                {
                    handleInitVM(iss);
                }
                else if (subcommand == "integrated")
                {
                    handleInitIntegrated(iss);
                }
                else
                {
                    cerr << "Unknown init command: " << subcommand << endl;
                }
            }
            else if (command == "set")
            {
                string subcommand;
                iss >> subcommand;
                transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);

                if (subcommand == "allocator")
                {
                    handleSetAllocator(iss);
                }
                else
                {
                    cerr << "Unknown set command: " << subcommand << endl;
                }
            }
            else if (command == "malloc")
            {
                handleMalloc(iss);
            }
            else if (command == "free")
            {
                handleFree(iss);
            }
            else if (command == "dump")
            {
                string subcommand;
                iss >> subcommand;
                transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);

                if (subcommand == "memory")
                {
                    handleDumpMemory();
                }
                else
                {
                    cerr << "Unknown dump command: " << subcommand << endl;
                }
            }
            else if (command == "stats")
            {
                handleStats();
            }
            else if (command == "add")
            {
                string subcommand;
                iss >> subcommand;
                transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);

                if (subcommand == "cache")
                {
                    handleAddCache(iss);
                }
                else
                {
                    cerr << "Unknown add command: " << subcommand << endl;
                }
            }
            else if (command == "cache")
            {
                string subcommand;
                iss >> subcommand;
                transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);

                if (subcommand == "read")
                {
                    handleCacheRead(iss);
                }
                else if (subcommand == "write")
                {
                    handleCacheWrite(iss);
                }
                else if (subcommand == "stats")
                {
                    handleCacheStats();
                }
                else
                {
                    cerr << "Unknown cache command: " << subcommand << endl;
                }
            }
            else if (command == "vm")
            {
                string subcommand;
                iss >> subcommand;
                transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);

                if (subcommand == "access")
                {
                    handleVMAccess(iss);
                }
                else if (subcommand == "stats")
                {
                    handleVMStats();
                }
                else if (subcommand == "pagetable")
                {
                    handleVMPageTable();
                }
                else
                {
                    cerr << "Unknown vm command: " << subcommand << endl;
                }
            }
            else if (command == "integrated")
            {
                string subcommand;
                iss >> subcommand;
                transform(subcommand.begin(), subcommand.end(), subcommand.begin(), ::tolower);

                if (subcommand == "read")
                {
                    handleIntegratedRead(iss);
                }
                else if (subcommand == "write")
                {
                    handleIntegratedWrite(iss);
                }
                else if (subcommand == "stats")
                {
                    handleIntegratedStats();
                }
                else
                {
                    cerr << "Unknown integrated command: " << subcommand << endl;
                }
            }
            else
            {
                cerr << "Unknown command: " << command << endl;
                cerr << "Type 'help' for available commands" << endl;
            }
        }
    }
};

int main(int argc, char *argv[])
{
    // Create output directory if it doesn't exist
    struct stat st = {};
    if (stat("output", &st) == -1)
    {
        mkdir("output", 0755);
    }

    // Check if input is from a pipe or file (not interactive terminal)
    bool is_batch_mode = !isatty(fileno(stdin));

    ofstream output_file;
    streambuf *cout_buffer = nullptr;
    streambuf *cerr_buffer = nullptr;

    if (is_batch_mode)
    {
        // Determine output file name based on command line argument or stdin
        string output_filename = "output/simulation_output.txt";

        if (argc > 1)
        {
            // Use command line argument to determine output name
            string input_file = argv[1];
            size_t last_slash = input_file.find_last_of("/\\");
            string basename = (last_slash != string::npos) ? input_file.substr(last_slash + 1) : input_file;

            // Remove .txt extension if present and add _output.txt
            size_t dot_pos = basename.find_last_of('.');
            if (dot_pos != string::npos)
            {
                basename = basename.substr(0, dot_pos);
            }
            output_filename = "output/" + basename + "_output.txt";
        }

        output_file.open(output_filename);
        if (output_file.is_open())
        {
            cout_buffer = cout.rdbuf();
            cerr_buffer = cerr.rdbuf();
            cout.rdbuf(output_file.rdbuf());
            cerr.rdbuf(output_file.rdbuf());

            // Write header to output file
            cout << "=== Memory Management Simulator - Output Log ===" << endl;
            cout << "Output file: " << output_filename << endl;
            cout << "=================================================" << endl
                 << endl;
        }
    }

    MemorySimulator simulator;
    simulator.run();

    // Restore original buffers if redirected
    if (is_batch_mode && output_file.is_open())
    {
        cout << "\n=== Simulation Complete ===" << endl;
        cout.rdbuf(cout_buffer);
        cerr.rdbuf(cerr_buffer);
        output_file.close();
    }

    return 0;
}
