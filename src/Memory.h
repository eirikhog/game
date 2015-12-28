#pragma once

#include "Platform.h"

typedef struct {
    uint8 *base;
    uint32 size;
    uint32 used;
} memory_segment;

// TODO: Move to implementation file.
inline memory_segment
allocate_memory(memory_segment *memory, uint32 size) {
    Assert(memory->size - memory->used >= size);
    memory_segment allocated = {};
    allocated.base = memory->base + memory->used;
    allocated.size = size;
    allocated.used = 0;

    memory->used += size;

    return allocated;
}

#define PUSH_STRUCT(segment, type) push_struct_(segment, sizeof(type))

inline void *
push_struct_(memory_segment *segment, uint32 size) {
    Assert(segment->size - segment->used >= size);

    uint8 *result = (uint8*)(segment->base + segment->used);
    segment->used += size;

    // Zero the memory
    for (uint32 i = 0; i < size; ++i) {
        result[i] = 0;
    }

    return (void*)result;
}

inline void
segment_clear(memory_segment *segment) {
    segment->used = 0;
}

