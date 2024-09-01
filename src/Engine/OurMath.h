#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

// TODO(captainurist): drop this header

#define pi_double 3.14159265358979323846

const float pi = static_cast<float>(M_PI);

int integer_sqrt(int val);
inline int round_to_int(float x) { return (int)std::floor(x + 0.5f); }

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z);


// struct fixed {  // fixed-point decimal
//    inline fixed() : _internal(0) {}
//    explicit fixed(int _bits) : _internal(_bits) {}
//
//    static fixed FromFloat(float f) {
//        return fixed::Raw(fixpoint_from_float(f));
//    }
//
//    static fixed FromInt(int value) { return fixed::Raw(value << 16); }
//
//    static fixed Raw(int value) { return fixed(value); }
//
//    float GetFloatFraction() const {
//        return (float)((double)((unsigned int)this->_internal & 0xFFFF) /
//                       65530.0);
//    }
//    float GetFloat() const {
//        return (float)this->GetInt() + this->GetFloatFraction();
//    }
//    int GetInt() const { return this->_internal >> 16; }
//    int GetUnsignedInt() const { return (unsigned int)this->_internal >> 16; }
//
//    //----- (0042EBBE) --------------------------------------------------------
//    //----- (004453C0) mm6-----------------------------------------------------
//    //----- (004A1760) mm6_chinese---------------------------------------------
//    inline fixed operator*(const fixed &rhs) {
//        return fixed::Raw(((int64_t)this->_internal * (int64_t)rhs._internal) >> 16);
//    }
//
//    inline fixed operator/(const fixed &rhs) {
//        return fixed::Raw(((int64_t)this->_internal << 16) / rhs._internal);
//    }
//
//    inline fixed operator+(const fixed &rhs) {
//        return fixed::Raw(this->_internal + rhs._internal);
//    }
//    inline fixed operator-(const fixed &rhs) {
//        return fixed::Raw(this->_internal - rhs._internal);
//    }
//    inline bool operator>=(const fixed &rhs) {
//        return this->_internal >= rhs._internal;
//    }
//    inline bool operator<=(const fixed &rhs) {
//        return this->_internal <= rhs._internal;
//    }
//    inline bool operator>(const fixed &rhs) {
//        return this->_internal > rhs._internal;
//    }
//    inline bool operator<(const fixed &rhs) {
//        return this->_internal < rhs._internal;
//    }
//
//    int32_t _internal;
// };

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

