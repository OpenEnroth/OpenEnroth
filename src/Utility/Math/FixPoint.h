#pragma once

#include <cmath>
#include <cstdint>

inline int64_t fixpoint_mul(int32_t l, int32_t r) {
    return (static_cast<int64_t>(l) * r) >> 16;
}

inline int64_t fixpoint_div(int32_t l, int32_t r) {
    return (static_cast<int64_t>(l) << 16) / r;
}

// These shouldn't compile:
void fixpoint_mul(float, float) = delete;
void fixpoint_div(float, float) = delete;
void fixpoint_mul(double, double) = delete;
void fixpoint_div(double, double) = delete;

