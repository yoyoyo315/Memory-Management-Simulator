# Cache Simulation

## Overview

Simulates multi-level cache hierarchy with different replacement policies.

## Cache Parameters

- **Cache Size**: Total capacity in bytes
- **Block Size**: Size of each cache line
- **Associativity**: Number of ways (1=direct-mapped, N=N-way set-associative)
- **Policy**: Replacement policy (FIFO, LRU, LFU)

## Replacement Policies

**FIFO**: Replaces the oldest block

- Simple implementation
- No usage consideration

**LRU**: Replaces least recently used block

- Good for temporal locality
- Common in real processors

**LFU**: Replaces least frequently used block

- Good for frequently accessed data
- Slower to adapt

## Commands

```bash
init cache <memory_size>        # Initialize cache system
add cache <size> <block> <assoc> <policy>  # Add cache level
cache read <address>            # Read from cache
cache write <address> <data>    # Write to cache
cache stats                     # Show statistics
```

## Example

```bash
init cache 8192
add cache 512 16 4 lru          # L1: 512B, 16B blocks, 4-way, LRU
add cache 2048 32 8 lru         # L2: 2KB, 32B blocks, 8-way, LRU
cache read 0
cache read 16
cache stats
```

## Statistics

For each cache level:

- **Hits**: Successful cache accesses
- **Misses**: Cache misses
- **Hit Ratio**: Hits / Total Accesses

Higher hit ratio means better performance.
