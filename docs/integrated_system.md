# Integrated VM+Cache System

## Overview

The integrated system demonstrates a complete memory hierarchy where virtual memory and cache work together, simulating how real operating systems and processors manage memory.

## Memory Access Flow

When you use the integrated commands, memory accesses follow this exact flow:

```
1. Virtual Address (provided by user)
          ↓
2. Page Table Translation (Virtual Memory)
          ↓
3. Physical Address (result of translation)
          ↓
4. Cache Hierarchy (L1 → L2 → L3)
          ↓
5. Main Memory (if cache miss)
```

This is **exactly** how real systems work!

## How It Works

### Initialization

```bash
init integrated 20 18 1024 lru
```

This command:

- Creates a virtual memory system (1MB virtual, 256KB physical, 1KB pages)
- Creates a cache hierarchy with default L1 and L2 caches
- Links them together so cache accesses use physical addresses

### Reading Data

```bash
integrated read 0
```

**Step-by-step process:**

1. Takes virtual address `0`
2. Translates through page table → gets physical address
3. If page fault occurs, loads page into physical memory
4. Accesses physical address through cache hierarchy
5. If cache miss, fetches from main memory
6. Returns data to user

**Console output shows each step:**

```
[Integrated Access Flow]
1. Virtual Address: 0x0
2. Page Table Translation: Virtual 0x0 -> Physical 0x0
3. Cache Hierarchy Access:
4. Data Retrieved: 0
   [Flow: Virtual Address → Page Table → Physical Address → Cache → Memory]
```

### Writing Data

```bash
integrated write 5120 100
```

Same flow as read, but writes the value through all levels:

1. Virtual → Physical translation
2. Write to cache (all levels updated)
3. Write to main memory

## Commands

### Initialize Integrated System

```bash
init integrated <virt_bits> <phys_bits> <page_size> <policy>
```

Parameters:

- `virt_bits`: Virtual address space size (2^bits bytes)
- `phys_bits`: Physical memory size (2^bits bytes)
- `page_size`: Size of each page in bytes
- `policy`: Page replacement policy (fifo/lru/clock)

Example:

```bash
init integrated 20 18 1024 lru
# Creates: 1MB virtual, 256KB physical, 1KB pages, LRU replacement
```

### Read Through Hierarchy

```bash
integrated read <virtual_address>
```

Example:

```bash
integrated read 0        # Read virtual address 0
integrated read 5120     # Read virtual address 5120
```

### Write Through Hierarchy

```bash
integrated write <virtual_address> <data>
```

Example:

```bash
integrated write 1024 42     # Write 42 to virtual address 1024
integrated write 2048 100    # Write 100 to virtual address 2048
```

### View Statistics

```bash
integrated stats
```

Shows:

- Virtual memory statistics (page hits/faults)
- Cache statistics for all levels (hits/misses)
- Combined performance metrics

## Customizing Cache Levels

After initialization, you can add more cache levels:

```bash
init integrated 20 18 1024 lru
add cache 4096 64 16 lru      # Add L3 cache
integrated read 0              # Now uses L1→L2→L3→Memory
```

## Example Session

```bash
# Initialize integrated system
init integrated 20 18 1024 lru

# Sequential accesses (will cause page faults initially)
integrated read 0
integrated read 1024
integrated read 2048

# Check statistics
integrated stats

# Re-access same addresses (cache hits!)
integrated read 0
integrated read 1024

# See improved performance
integrated stats

# Write operations
integrated write 5120 999
integrated write 6144 888

# Final statistics
integrated stats
```

## Performance Metrics

### Page Fault Rate

- Initial accesses: High page faults (cold start)
- Repeated accesses: Fewer page faults (working set loaded)

### Cache Hit Ratio

- First access to page: Cache miss
- Subsequent accesses: Cache hits (temporal locality)
- Sequential accesses: Better cache performance (spatial locality)

### Example Output

```
=== Integrated System Statistics ===

--- Virtual Memory ---
Page hits: 10
Page faults: 5
Total accesses: 15
Hit ratio: 66.67%

--- Cache Hierarchy ---
L1 Cache Statistics:
  Hits: 5
  Misses: 10
  Total Accesses: 15
  Hit Ratio: 33.33%

L2 Cache Statistics:
  Hits: 7
  Misses: 3
  Total Accesses: 10
  Hit Ratio: 70.00%
```

## Why This Matters

This integration demonstrates:

1. **Real OS Behavior**: How operating systems actually manage memory
2. **Performance Impact**: Why both VM and cache are crucial
3. **Hierarchy Benefits**: How multiple levels work together
4. **Address Translation**: The cost and necessity of page tables
5. **Caching Effectiveness**: How cache improves VM performance

## Comparison: Separate vs Integrated

### Separate Systems

```bash
# VM only - no cache benefits
init vm 20 18 1024 lru
vm access 0

# Cache only - no VM translation
init cache 8192
cache read 0
```

### Integrated System

```bash
# Both working together
init integrated 20 18 1024 lru
integrated read 0   # Uses BOTH VM and Cache
```

The integrated system is closer to how real computers work!
