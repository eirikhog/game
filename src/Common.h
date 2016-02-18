#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>

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

#endif

