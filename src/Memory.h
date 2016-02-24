#pragma once

#include "Platform.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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

#include <stdio.h>

inline char *mprintf(const char *format, ...) {
    va_list ap;
    char buffer[256];
    va_start(ap, format);
    uint32 length = vsprintf(buffer, format, ap);
    va_end(ap);

    char *result = (char*)malloc(length + 1);
    memcpy(result, buffer, length + 1);
    return result;
}

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define _SCOPE_FREE(x) ScopeFree MACRO_CONCAT(_scope_free_, __COUNTER__)##(x)
#define SCOPE_FREE(x) _SCOPE_FREE(x)

struct ScopeFree {
    ScopeFree(void *ptr) : mPtr(ptr) {};
    ~ScopeFree() { free(mPtr); }
    void *mPtr;
};
