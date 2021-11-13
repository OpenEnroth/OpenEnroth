#pragma once

#include <cassert>
#include <cmath>
#include <limits>
#include <cstdint>

#include "Engine/MM7.h"

#define pi_double 3.14159265358979323846

__int64 fixpoint_mul(int, int);
// __int64 fixpoint_dot(int x1, int x2, int y1, int y2, int z1, int z2);
__int64 fixpoint_div(int, int);
// __int64 fixpoint_sub_unknown(int, int);
// int fixpoint_from_float(float value);
// int fixpoint_from_int(int lhv, int rhv);

int integer_sqrt(int val);
int GetDiceResult(unsigned int uNumDice, unsigned int uDiceSides);  // idb
inline int round_to_int(float x) { return (int)floor(x + 0.5f); }

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

/*  186 */
#pragma pack(push, 1)
struct TrigTableLookup {
    TrigTableLookup();

    int Cos(int angle);
    unsigned int Atan2(int x, int y);
    int Sin(int angle);

    int pTanTable[520];
    int pCosTable[520];
    int pInvCosTable[520];
    static const unsigned int uIntegerPi = 1024;
    static const unsigned int uIntegerHalfPi = 512;
    static const unsigned int uIntegerDoublePi = 2048;
    static const unsigned int uDoublePiMask = 2047;
    static const unsigned int uPiMask = 1023;
    static const unsigned int uHalfPiMask = 511;
};
#pragma pack(pop)

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

#pragma push_macro("max")
#undef max
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
#pragma pop_macro("max")

extern struct TrigTableLookup *TrigLUT;
