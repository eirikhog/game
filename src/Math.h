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
};

typedef v2<int32> v2i;
typedef v2<real32> v2f;

typedef struct {
    real32 x;
    real32 y;
    real32 z;
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



#ifdef _MSC_VER

// Visual Studio intinsics

inline real32 square_root(real32 value) {
    return sqrtf(value);
}

#endif
