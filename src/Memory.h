#ifndef _MEMORY_H
#define _MEMORY_H

#pragma once

#include "Platform.h"

typedef struct {
    u8 *base;
    u32 size;
    u32 used;
} MemorySegment;

// TODO: Move to implementation file.
inline MemorySegment
AllocMemory(MemorySegment *memory, u32 size) {
    Assert(memory->size - memory->used >= size);
    MemorySegment allocated = {};
    allocated.base = memory->base + memory->used;
    allocated.size = size;
    allocated.used = 0;

    memory->used += size;

    return allocated;
}

#define PUSH_STRUCT(segment, type) (type *)PushStruct_(segment, sizeof(type))

inline void *
PushStruct_(MemorySegment *segment, u32 size) {
    Assert(segment->size - segment->used >= size);

    u8 *result = (u8*)(segment->base + segment->used);
    segment->used += size;

    return (void*)result;
}

inline void
SegmentClear(MemorySegment *segment) {
    segment->used = 0;
}


// This is a naive implementation of a general purpose memory
// allocator. The algorithm is to search the space and find the
// first memory region which fits. The algorithm makes no
// attempt to optimize best-fit.

struct MemoryPoolRegion {
    u32 size; // Size is used to calculate next region
    u32 used;
};

struct MemoryPool {
    u8 *base;
    u32 size;
}; 

MemoryPool InitializeAllocator(MemorySegment *segment) {
    MemoryPool pool;
    pool.base = segment->base;
    pool.size = segment->size;

    MemoryPoolRegion *region = (MemoryPoolRegion*)pool.base;
    region->size = pool.size;
    region->used = 0;

    return pool;
}

void *Allocate(MemoryPool *pool, u32 size) {
    u32 requiredSize = size + sizeof(MemoryPoolRegion);

    MemoryPoolRegion *current = (MemoryPoolRegion*)pool->base;
    for (;;) {
        if (current->used || current->size < requiredSize) {
            current = (MemoryPoolRegion*)((u8*)current + current->size);
        } else {
            // We can use this region
            
            // Make sure to mark the following region as available
            if (current->size > requiredSize + sizeof(MemoryPoolRegion)) {
                MemoryPoolRegion *next = (MemoryPoolRegion*)((u8*)current + requiredSize);
                next->used = 0;
                next->size = current->size - requiredSize;
            }

            current->size = requiredSize;
            current->used = 1;
            return (void*)((u8*)current + sizeof(MemoryPoolRegion));
        }

        if ((i64)current - (i64)pool->base > (i64)pool->size) {
            // No more free memory
            InvalidCodePath(); // TODO: Assert
            return 0;
        }
    }

    //InvalidCodePath();
    //return 0;
}

void Free(MemoryPool *pool, void *ptr) {
    // 1. Find the region
    // 2. Mark the memory as unused
    // 3. Merge the region with unused neighbours.

    if (ptr == NULL) {
        return;
    }

    u8* offset = pool->base;
    if (ptr < offset || ptr > pool->base + pool->size) {
        // This pointer does not belong to the pool
        InvalidCodePath();
    }

    MemoryPoolRegion *before = NULL;
    for (;;) {
        if (offset + sizeof(MemoryPoolRegion) == ptr) {
            MemoryPoolRegion *current = (MemoryPoolRegion*)offset;
            current->used = 0;

            // Find the following region, and see if we should merge...
            MemoryPoolRegion *next = (MemoryPoolRegion*)(offset + current->size);
            if ((u8*)next < pool->size + pool->base) {
                if (!next->used) {
                    current->size += next->size;
                }
            }

            if (before != NULL && !before->used) {
                before->size += current->size;
            }
            return;
        }

        if (offset >= pool->base + pool->size) {
            return;
        }
    }
}

#if 0
// General purpose memory allocator
struct AllocatorRange {
    AllocatorRange *next;
    u32 size;
    bool32 used;
};

struct AllocatorContext {
    MemorySegment segment;
    AllocatorRange *first;
};

AllocatorContext InitializeGenAllocator(MemorySegment memory) {
    AllocatorContext context = {};
    context.segment = memory;
}

bool32 GenAllocate(AllocatorContext *ctx, u32 size) {
    if (!ctx->first) {
        if (size > ctx->segment->size) {
            // Bad allocation
            InvalidCodePath();
            return 0;
        }

        AllocatorRange *range;
        range = (AllocatorRange*)ctx->memory->base;
        range->size = size;
        range->next = 0;
        return 1;
    }

    AllocatorRange *current = ctx->first;
    while (current->next) {
        if (!current->used && size <= current->size) {
            // The allocation fits in this space.
            // TODO: Use best-fit instead?
        }
        current = current->next;
    }

}

bool32 GenFree(void *ptr) {
}

#endif

#endif

