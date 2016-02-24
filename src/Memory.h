#pragma once

#include "Platform.h"

typedef struct {
    uint8 *base;
    uint32 size;
    uint32 used;
} MemorySegment;

// TODO: Move to implementation file.
inline MemorySegment
AllocMemory(MemorySegment *memory, uint32 size) {
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
PushStruct_(MemorySegment *segment, uint32 size) {
    Assert(segment->size - segment->used >= size);

    uint8 *result = (uint8*)(segment->base + segment->used);
    segment->used += size;

    return (void*)result;
}

inline void
SegmentClear(MemorySegment *segment) {
    segment->used = 0;
}