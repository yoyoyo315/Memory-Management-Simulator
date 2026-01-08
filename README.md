# Memory Management Simulator

A comprehensive C++ simulator for learning and testing memory management concepts including memory allocation algorithms, cache hierarchies, and virtual memory with page replacement.
[Demo Video](Video-demo.mp4)


## Features

- **Memory Allocation**: First-fit, best-fit, worst-fit, and buddy system algorithms
- **Cache Simulation**: Multi-level cache hierarchy with FIFO, LRU, and LFU replacement policies
- **Virtual Memory**: Page table management with FIFO, LRU, and Clock page replacement algorithms
- **Integrated VM+Cache**: Full integration of virtual memory with cache hierarchy
- **Fragmentation Analysis**: Internal and external fragmentation statistics
- **Batch Mode**: Run test workloads from files with automatic output file generation
- **Output Files**: All test results automatically saved to `output/` directory

## Architecture

The simulator implements a complete memory hierarchy:

```
Virtual Address
      ↓
  Page Table (VM Translation)
      ↓
Physical Address
      ↓
  Cache Hierarchy (L1 → L2 → L3)
      ↓
  Main Memory
```

## Quick Start

### Build

```bash
make
```

This creates the executable at `bin/memsim`.

### Run Interactive Mode

```bash
./bin/memsim
```

Type `help` to see all available commands.

### Run with Test Workloads

The simulator automatically generates output files when running in batch mode:

```bash
# Run all tests and generate output files (recommended)
make test

# This creates output files in output/ directory:
# - workload_allocator_output.txt
# - workload_cache_output.txt
# - workload_vm_output.txt
# - workload_integrated_output.txt
```

Or run individual tests:

```bash
# Memory allocation test
./bin/memsim tests/workload_allocator.txt < tests/workload_allocator.txt

# Cache hierarchy test
./bin/memsim tests/workload_cache.txt < tests/workload_cache.txt

# Virtual memory test
./bin/memsim tests/workload_vm.txt < tests/workload_vm.txt

# Integrated VM+Cache test
./bin/memsim tests/workload_integrated.txt < tests/workload_integrated.txt
```

**Note**: Output files are automatically created in the `output/` directory when running tests.

## Usage Examples

### Memory Allocation

```bash
init memory 4096              # Initialize 4KB memory
set allocator first_fit       # Choose allocation strategy
malloc 256                    # Allocate 256 bytes
free 1                        # Free block ID 1
dump memory                   # Show memory layout
stats                         # Show statistics
```

### Cache Hierarchy

```bash
init cache 8192               # Initialize cache with 8KB memory
add cache 512 16 4 lru        # Add L1: 512B, 16B blocks, 4-way, LRU
add cache 2048 32 8 lru       # Add L2: 2KB, 32B blocks, 8-way, LRU
cache read 100                # Read address 100
cache write 200 42            # Write value 42 to address 200
cache stats                   # Show cache statistics
```

### Virtual Memory

```bash
init vm 20 18 1024 lru        # 1MB virtual, 256KB physical, 1KB pages, LRU
vm access 0                   # Access virtual address 0
vm access 5120                # Access virtual address 5120
vm stats                      # Show page fault statistics
vm pagetable                  # Display page table
```

### Integrated VM+Cache System

```bash
# Initialize integrated system (VM + Cache working together)
init integrated 20 18 1024 lru

# Read through complete hierarchy: VM → Cache → Memory
integrated read 0
integrated read 1024

# Write through complete hierarchy
integrated write 5120 100

# View combined statistics
integrated stats

# Add additional cache levels
add cache 4096 64 16 lru

# Continue accessing with enhanced cache
integrated read 0
```

## Commands Reference

### Memory Allocation

- `init memory <size>` - Initialize physical memory
- `set allocator <type>` - Set allocator (first_fit/best_fit/worst_fit/buddy)
- `malloc <size>` - Allocate memory block
- `free <block_id>` - Free memory block
- `dump memory` - Show memory layout
- `stats` - Show memory statistics

### Cache

- `init cache <mem_size>` - Initialize cache and memory
- `add cache <size> <block_size> <associativity> <policy>` - Add cache level
- `cache read <address>` - Read from address
- `cache write <address> <data>` - Write to address
- `cache stats` - Show cache statistics

### Virtual Memory

- `init vm <virt_bits> <phys_bits> <page_size> <policy>` - Initialize VM
- `vm access <address>` - Access virtual address
- `vm stats` - Show VM statistics
- `vm pagetable` - Show page table

### Integrated VM+Cache

- `init integrated <virt_bits> <phys_bits> <page_size> <policy>` - Initialize integrated system
- `integrated read <virt_addr>` - Read via VM → Cache → Memory
- `integrated write <virt_addr> <data>` - Write via VM → Cache → Memory
- `integrated stats` - Show combined VM and cache statistics

### General

- `help` - Show help message
- `exit` - Exit simulator

## Build Targets

```bash
make              # Build the project
make clean        # Remove build artifacts
make clean-output # Remove output files
make run          # Build and run interactively
make test         # Run all tests and generate output files
make help         # Show Makefile help
```

## Project Structure

```
├── include/           # Header files
├── src/              # Source files
│   ├── main.cpp
│   ├── allocator/    # Memory allocation
│   ├── buddy/        # Buddy system
│   ├── cache/        # Cache simulation
│   └── virtual_memory/ # VM and paging
├── tests/            # Test workload files
├── output/           # Generated output files (created automatically)
├── docs/             # Documentation
├── Makefile
└── README.md
```

## Documentation

See [docs/](docs/) for detailed documentation:

- [Memory Allocation](docs/memory_allocation.md)
- [Cache Simulation](docs/cache_simulation.md)
- [Virtual Memory](docs/virtual_memory.md)
- [Integrated VM+Cache System](docs/integrated_system.md)

## Requirements

- C++11 or later
- g++ compiler
- Make
