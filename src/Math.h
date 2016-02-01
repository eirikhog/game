#pragma once

#include "Common.h"

#ifdef _MSC_VER
// MSVC: Use the math library, but rely in intrinsics
#include <math.h>
#endif

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

inline v2 operator*(const v2 left, const float scalar) {
    v2 result = { left.x * scalar, left.y * scalar };
    return result;
}

inline v2& operator*=(v2 &left, const float scalar) {
    left = left * scalar;
    return left;
}

inline v2 operator/(const v2 left, const float scalar) {
    v2 result = { left.x / scalar, left.y / scalar };
    return result;
}

inline v2& operator/=(v2 &left, const float scalar) {
    left = left / scalar;
    return left;
}



#ifdef _MSC_VER

// Visual Studio intinsics

inline real32 square_root(real32 value) {
    return sqrtf(value);
}

#endif
