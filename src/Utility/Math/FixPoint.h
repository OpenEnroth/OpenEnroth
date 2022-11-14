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

/**
 * @param value                         Fixed-point value.
 * @return                              Corresponding value as a `float`.
 */
inline float fixpoint_to_float(int32_t value) {
    return static_cast<float>(value / 65536.0);
}

// TODO(captainurist): Drop!
/**
 * Takes a non-fixpoint vector and normalizes it, resulting in a fixpoint vector.
 */
inline void normalize_to_fixpoint(int *x, int *y, int *z) {
    int denom = *y * *y + *z * *z + *x * *x;
    int mult = 65536 / (static_cast<int>(std::sqrt(denom)) | 1);
    *x *= mult;
    *y *= mult;
    *z *= mult;
}
