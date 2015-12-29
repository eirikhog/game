#pragma once

typedef struct {
    float x;
    float y;
} v2;

typedef struct {
    float x;
    float y;
    float z;
} v3;

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