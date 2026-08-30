#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

// TODO(captainurist): drop this header

#define pi_double 3.14159265358979323846

const float pi = static_cast<float>(M_PI);

int integer_sqrt(int val);

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z);


template <typename FloatType>
inline int bankersRounding(const FloatType &value) {
    assert("Method unsupported for this type" && false);
    return value;
}

template <>
inline int bankersRounding<float>(const float &inValue) {
    union Cast {
        double d;
        long l;
    };
    volatile Cast c;
    c.d = inValue + 6755399441055744.0;
    return c.l;
}

template <>
inline int bankersRounding<double>(const double &inValue) {
    constexpr double maxValue = std::numeric_limits<int>::max();
    assert(maxValue - 6755399441055744.0 >= inValue);
    union Cast {
        double d;
        long l;
    };
    volatile Cast c;
    c.d = inValue + 6755399441055744.0;
    return c.l;
}

