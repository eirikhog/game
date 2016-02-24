#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float real32;
typedef float real64;

typedef uint32 bool32;

typedef struct {
    real32 r;
    real32 g;
    real32 b;
    real32 a;
} Color;

template<typename T>
struct Rect2D {
    T x;
    T y;
    T width;
    T height;

    Rect2D(T _x, T _y, T _width, T _height) : x(_x), y(_y), width(_width), height(_height) { }
};

typedef Rect2D<int32> Rect2Di;

inline char *mprintf(const char *format, ...) {
    const int bufferSize = 256;
    va_list ap;
    char buffer[bufferSize];
    va_start(ap, format);
    uint32 length = vsnprintf(buffer, bufferSize, format, ap);
    va_end(ap);

    char *result = (char*)malloc(length + 1);
    memcpy(result, buffer, length + 1);
    return result;
}

#define _CONCAT( x, y ) x##y
#define CONCAT( x, y ) _CONCAT( x, y )
#define _SCOPE_FREE(x) ScopeFree CONCAT(_scope_free_, __COUNTER__)##(x)
#define SCOPE_FREE(x) _SCOPE_FREE(x)

struct ScopeFree {
    ScopeFree(void *ptr) : mPtr(ptr) {};
    ~ScopeFree() { free(mPtr); }
    void *mPtr;
};

#endif

