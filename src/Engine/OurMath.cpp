#include "Engine/OurMath.h"

#include <utility>

//----- (0042EBBE) --------------------------------------------------------
//----- (004453C0) mm6-----------------------------------------------------
//----- (004A1760) mm6_chinese---------------------------------------------
int64_t fixpoint_mul(int a1, int a2) {
    return ((int64_t)a1 * (int64_t)a2) >> 16;
}

// int64_t fixpoint_dot(int x1, int x2, int y1, int y2, int z1, int z2) {
//    return fixpoint_mul(x1, x2) + fixpoint_mul(y1, y2) + fixpoint_mul(z1, z2);
// }

//----- (004A1780) mm6_chinese---------------------------------------------
int64_t fixpoint_div(int a1, int a2) { return ((int64_t)a1 << 16) / a2; }

// int64_t fixpoint_sub_unknown(int a1, int a2) {
//    return (((int64_t)a1 << 16) * a2) >> 16;
// }

////----- (0048B561) --------------------------------------------------------
// int fixpoint_from_float(float val) {
//    //  float X.Yf -> int XXXX YYYY
//    int left = floorf((val - 0.5f) + 0.5f);
//    int right = floorf((val - left) * 65536.0f);
//    return (left << 16) | right;
// }

// int fixpoint_from_int(int lhv, int rhv) { return (lhv << 16) | rhv; }

//----- (00452A9E) --------------------------------------------------------
int integer_sqrt(int val) {
    ///////////////////////////////
    //Получение квадратного корня//
    ///////////////////////////////

    return int(sqrt(val));

    // binary square root algo
    // (int) sqrt(i) probably faster?


    int result;       // eax@2
    unsigned int v2;  // edx@3
    unsigned int v3;  // edi@3
    int v5;           // esi@4

    if (val < 1) return val;

    v2 = 0;
    v3 = val;
    result = 0;
    for (unsigned int i = 0; i < 16; ++i) {
        result *= 2;
        v2 = (v3 >> 30) | 4 * v2;
        v5 = 2 * result + 1;
        v3 *= 4;
        if (v2 >= v5) {
            ++result;
            v2 -= v5;
        }
    }
    if (val - result * result >= (unsigned int)(result - 1)) ++result;
    return result;
}

//----- (00452B2E) --------------------------------------------------------
int GetDiceResult(unsigned int uNumDice, unsigned int uDiceSides) {
    int v3 = 0;
    if (uDiceSides) {
        for (unsigned int i = 0; i < uNumDice; ++i)
            v3 += rand() % uDiceSides + 1;
        return v3;
    }
    return 0;
}

uint32_t int_get_vector_length(int32_t x, int32_t y, int32_t z) { // approx distance
    if (x < y) {
        std::swap(x, y);
    }
    if (x < z) {
        std::swap(x, z);
    }
    if (y < z) {
        std::swap(y, z);
    }

    return x + (11 * y >> 5) + (z >> 2);
}
