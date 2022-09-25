#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <array>
#include <limits>

#include "Engine/MM7.h"

#define pi_double 3.14159265358979323846

const float pi = static_cast<float>(M_PI);

__int64 fixpoint_mul(int, int);
__int64 fixpoint_div(int, int);

// These shouldn't compile:
void fixpoint_mul(float, float) = delete;
void fixpoint_div(float, float) = delete;
void fixpoint_mul(double, double) = delete;
void fixpoint_div(double, double) = delete;

/**
 * @param value                         Fixed-point value.
 * @return                              Corresponding value as a `float`.
 */
inline float fixpoint_to_float(int value) {
    return static_cast<float>(value / 65536.0);
}

int integer_sqrt(int val);
int GetDiceResult(unsigned int uNumDice, unsigned int uDiceSides);  // idb
inline int round_to_int(float x) { return (int)floor(x + 0.5f); }

/**
 * Takes a non-fixpoint vector and normalizes it, resulting in a fixpoint vector.
 */
inline void normalize_to_fixpoint(int *x, int *y, int *z) {
    extern int integer_sqrt(int val);
    int denom = *y * *y + *z * *z + *x * *x;
    int mult = 65536 / (integer_sqrt(denom) | 1);
    *x *= mult;
    *y *= mult;
    *z *= mult;
}

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z);

[[nodiscard]] inline bool FuzzyIsNull(float value) {
    return std::abs(value) < 0.00001f;
}

[[nodiscard]] inline bool FuzzyIsNull(double value) {
    return std::abs(value) < 0.000000000001;
}

[[nodiscard]] inline bool FuzzyEquals(float l, float r) {
    return std::abs(l - r) * 100000.f <= std::min(std::abs(l), std::abs(r));
}

[[nodiscard]] inline bool FuzzyEquals(double l, double r) {
    return std::abs(l - r) * 1000000000000. <= std::min(std::abs(l), std::abs(r));
}


// #pragma pack(push, 1)
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
// #pragma pack(pop)

/**
 * Lookup table for trigonometric functions.
 */
class TrigTableLookup {
 public:
    static const int uIntegerPi = 1024;
    static const int uIntegerHalfPi = 512;
    static const int uIntegerDoublePi = 2048;
    static const int uDoublePiMask = 2047;
    static const int uPiMask = 1023;
    static const int uHalfPiMask = 511;

    TrigTableLookup();

    /**
     * @param angle                     Angle in 1/2048ths of a full circle.
     * @return                          Cosine of the provided angle.
     */
    float Cos(int angle) const;

    /**
     * @param angle                     Angle in 1/2048ths of a full circle.
     * @return                          Sine of the provided angle.
     */
    float Sin(int angle) const;

    /**
     * @return                          Angle in 1/2048ths of a full circle. Actual result is in range [0, 2047].
     */
    int Atan2(int x, int y) const;

 private:
    std::array<float, uIntegerHalfPi + 1> pCosTable;
};

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

extern TrigTableLookup *TrigLUT;
