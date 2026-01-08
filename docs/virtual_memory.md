# Virtual Memory

## Overview

Simulates virtual memory with page tables and page replacement algorithms.

## Parameters

- **Virtual Address Bits**: Size of virtual address space (2^bits bytes)
- **Physical Address Bits**: Size of physical memory (2^bits bytes)
- **Page Size**: Size of each page/frame in bytes
- **Policy**: Page replacement policy (FIFO, LRU, Clock)

## Page Replacement Policies

**FIFO**: Replaces oldest page in memory

- Simple to implement
- May evict frequently used pages

**LRU**: Replaces least recently used page

- Good performance
- More complex

**Clock**: FIFO with second chance

- Approximates LRU
- Less overhead

## Commands

```bash
init vm <virt_bits> <phys_bits> <page_size> <policy>  # Initialize VM
vm access <address>             # Access virtual address
vm stats                        # Show statistics
vm pagetable                    # Show page table
```

## Example

```bash
init vm 20 18 1024 lru          # 1MB virtual, 256KB physical, 1KB pages
vm access 0                     # Access page 0
vm access 1024                  # Access page 1
vm access 5120                  # Access page 5
vm stats
vm pagetable
```

## Statistics

- **Page Hits**: Accesses to pages already in memory
- **Page Faults**: Accesses requiring page load from disk
- **Hit Ratio**: Hits / Total Accesses

Higher hit ratio means fewer disk accesses and better performance.

## Page Table Output

```
Page 0: Frame 0, Valid, Clean
Page 1: Frame 1, Valid, Clean
Page 2: Invalid
```

- **Valid**: Page is in physical memory
- **Invalid**: Page not loaded (will cause page fault)
- **Clean**: Not modified since loading
