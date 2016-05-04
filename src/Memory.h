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

