#include "Engine/OurMath.h"
#include "Engine/Engine.h"

TrigTableLookup* TrigLUT = new TrigTableLookup();

//----- (00452969) --------------------------------------------------------
TrigTableLookup::TrigTableLookup() {
    for (int i = 0; i <= this->uIntegerHalfPi; i++)
        pCosTable[i] = std::cos(i * pi_double / uIntegerPi);
}

//----- (00402CAE) --------------------------------------------------------
float TrigTableLookup::Cos(int angle) const {
    angle &= uDoublePiMask;

    if (angle > uIntegerPi)
        angle = uIntegerDoublePi - angle;
    if (angle >= uIntegerHalfPi)
        return -pCosTable[uIntegerPi - angle];
    else
        return pCosTable[angle];
}

//----- (0042EBDB) --------------------------------------------------------
float TrigTableLookup::Sin(int angle) const {
    return Cos(angle - this->uIntegerHalfPi);
}

//----- (0045281E) --------------------------------------------------------
int TrigTableLookup::Atan2(int x, int y) const {
    double angle = std::atan2(static_cast<double>(y), static_cast<double>(x));

    return static_cast<int>(angle / pi_double * 1024) & uDoublePiMask;
}

//----- (0042EBBE) --------------------------------------------------------
//----- (004453C0) mm6-----------------------------------------------------
//----- (004A1760) mm6_chinese---------------------------------------------
__int64 fixpoint_mul(int a1, int a2) {
    return ((__int64)a1 * (__int64)a2) >> 16;
}

// __int64 fixpoint_dot(int x1, int x2, int y1, int y2, int z1, int z2) {
//    return fixpoint_mul(x1, x2) + fixpoint_mul(y1, y2) + fixpoint_mul(z1, z2);
// }

//----- (004A1780) mm6_chinese---------------------------------------------
__int64 fixpoint_div(int a1, int a2) { return ((__int64)a1 << 16) / a2; }

// __int64 fixpoint_sub_unknown(int a1, int a2) {
//    return (((__int64)a1 << 16) * a2) >> 16;
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
