#pragma once

#include "Common.h"

#ifdef _MSC_VER
// MSVC: Use the math library, but rely in intrinsics
#include <math.h>
#endif

template<typename T>
struct v2 {
    T x;
    T y;
    v2() : x(0), y(0) {}
    v2(T x, T y) : x(x), y(y) {}
};

typedef v2<i32> v2i;
typedef v2<r32> v2f;

typedef struct {
    r32 x;
    r32 y;
    r32 z;
} v3;

template<typename T>
inline v2<T> operator+(const v2<T> left, const v2<T> right) {
    v2<T> result = { left.x + right.x, left.y + right.y };
    return result;
}

template<typename T>
inline v2<T> operator-(const v2<T> left, const v2<T> right) {
    v2<T> result = { left.x - right.x, left.y - right.y };
    return result;
}

template<typename T>
inline v2<T>& operator+=(v2<T> &left, const v2<T> right) {
    left = left + right;
    return left;
}

template<typename T>
inline v2<T>& operator-=(v2<T> &left, const v2<T> right) {
    left = left - right;
    return left;
}

template<typename T>
inline v2<T> operator*(const v2<T> left, const T scalar) {
    v2<T> result = { left.x * scalar, left.y * scalar };
    return result;
}

template<typename T>
inline v2<T>& operator*=(v2<T> &left, const T scalar) {
    left = left * scalar;
    return left;
}

template<typename T>
inline v2<T> operator/(const v2<T> left, const T scalar) {
    v2<T> result = { left.x / scalar, left.y / scalar };
    return result;
}

template<typename T>
inline v2<T>& operator/=(v2<T> &left, const T scalar) {
    left = left / scalar;
    return left;
}

template<typename T>
inline v2<T> operator+(const v2<T> left, const T scalar) {
    v2<T> result = { left.x + scalar, left.y + scalar };
    return result;
}

template<typename T>
inline r32 magnitude(v2<T> vec) {
    r32 result = square_root((r32)(vec.x * vec.x + vec.y * vec.y));
    return result;
}

template<typename T>
inline v2<T> unit(v2<T> vec) {
    r32 mag = magnitude(vec);
    v2<T> result = v2<T>((T)(vec.x / mag), (T)(vec.y / mag));
    return result;
}

#ifdef _MSC_VER

// Visual Studio intinsics

inline r32 square_root(r32 value) {
    return sqrtf(value);
}

#define min(x, y) (((x) < (y)) ? (x) : (y))

template<typename T>
bool32 Intersects(Rect2D<T> a, Rect2D<T> b) {
    if (a.x + a.width < b.x || b.x + b.width < a.x || a.y + a.height < b.y || b.y + b.height < a.y) {
        return 0;
    }
    return 1;
}

template<typename T>
Rect2D<T> Normalize(Rect2D<T> rect) {
    Rect2D<T> result(rect.x, rect.y, rect.width, rect.height);
    if (result.width < 0) {
        result.x += result.width;
        result.width *= -1;
    }

    if (result.height < 0) {
        result.y += result.height;
        result.height *= -1;
    }
    return result;
}

#endif
