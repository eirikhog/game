#pragma once

#include "Common.h"

typedef struct {
    real32 x;
    real32 y;
} v2;

typedef struct {
    real32 x;
    real32 y;
    real32 z;
} v3;

typedef struct {
    real32 x;
    real32 y;
    real32 width;
    real32 height;
} rect;

inline v2 operator+(const v2 left, const v2 right) {
    v2 result = { left.x + right.x, left.y + right.y };
    return result;
}

inline v2 operator-(const v2 left, const v2 right) {
    v2 result = { left.x - right.x, left.y - right.y };
    return result;
}

inline v2& operator+=(v2 &left, const v2 right) {
    left = left + right;
    return left;
}

inline v2& operator-=(v2 &left, const v2 right) {
    left = left - right;
    return left;
}

