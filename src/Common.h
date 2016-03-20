#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float r32;
typedef double r64;

typedef u32 bool32;

struct Color {
    r32 r;
    r32 g;
    r32 b;
    r32 a;
    Color(r32 _r, r32 _g, r32 _b, r32 _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
    Color() : r(0.0f), g(0.0f), b(0.0f), a(0.0f) { }
};

template<typename T>
struct Rect2D {
    T x;
    T y;
    T width;
    T height;

    Rect2D(T _x, T _y, T _width, T _height) : x(_x), y(_y), width(_width), height(_height) { }
};

typedef Rect2D<i32> Rect2Di;

inline char *mprintf(const char *format, ...) {
    const int bufferSize = 256;
    va_list ap;
    char buffer[bufferSize];
    va_start(ap, format);
    u32 length = vsnprintf(buffer, bufferSize, format, ap);
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

