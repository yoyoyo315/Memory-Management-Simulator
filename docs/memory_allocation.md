# Memory Allocation

## Overview

Simulates dynamic memory management with different allocation strategies.

## Allocation Strategies

**First Fit**: Allocates the first free block that fits

- Fast allocation
- Can cause fragmentation

**Best Fit**: Finds the smallest free block that fits

- Minimizes wasted space
- Slower search

**Worst Fit**: Allocates the largest free block

- Leaves larger fragments
- Can exhaust large blocks

**Buddy System**: Uses power-of-2 sized blocks

- Fast allocation/deallocation
- Internal fragmentation (up to 50%)

## Commands

```bash
init memory <size>              # Initialize memory
set allocator <type>            # first_fit, best_fit, worst_fit, buddy
malloc <size>                   # Allocate memory (returns block ID)
free <block_id>                 # Free memory block
dump memory                     # Show memory layout
stats                           # Show statistics
```

## Example

```bash
init memory 4096
set allocator first_fit
malloc 128                      # Returns ID 1
malloc 256                      # Returns ID 2
dump memory
free 1
stats
```

## Statistics

- **Total Memory**: Total available memory
- **Used Memory**: Currently allocated
- **Free Memory**: Available for allocation
- **External Fragmentation**: Unusable free memory scattered in small blocks
- **Internal Fragmentation**: Wasted space within blocks (buddy system only)
