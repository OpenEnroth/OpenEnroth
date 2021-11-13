#include "Engine/OurMath.h"
#include "Engine/Engine.h"

//----- (00452969) --------------------------------------------------------
TrigTableLookup::TrigTableLookup() {
    // populates tables

    double v3;  // ST18_8@2

    this->pTanTable[0] = 0;
    this->pCosTable[0] = 65536;
    this->pInvCosTable[0] = 65536;
    for (int i = 1; i < (signed int)this->uIntegerHalfPi; i++) {
        v3 = (double)i * pi_double / (double)uIntegerPi;
        pTanTable[i] =
            (signed __int64)(tan(v3) * (double)this->pCosTable[0] + 0.5);
        pCosTable[i] =
            (signed __int64)(cos(v3) * (double)this->pCosTable[0] + 0.5);
        pInvCosTable[i] =
            (signed __int64)(1.0 / cos(v3) * (double)this->pCosTable[0] + 0.5);
    }
    for (int i = this->uIntegerHalfPi; i < 520; i++) {
        this->pTanTable[i] = 0xEFFFFFFFu;
        this->pCosTable[i] = 0;
        this->pInvCosTable[i] = 0xEFFFFFFFu;
    }
}

//----- (00402CAE) --------------------------------------------------------
int TrigTableLookup::Cos(int angle) {
    int v2;  // eax@1

    // a2: (angle - uIntegerHalfPi)    for  sin(angle)
    //    (angle)                     for  cos(angle)

    v2 = uDoublePiMask & angle;

    if (v2 > uIntegerPi) v2 = uIntegerDoublePi - v2;
    if (v2 >= uIntegerHalfPi)
        return -pCosTable[uIntegerPi - v2];
    else
        return pCosTable[v2];
}

//----- (0045281E) --------------------------------------------------------
//    Calculates atan2(y/x)
// return value: angle in integer format (multiplier of Pi/1024)
unsigned int TrigTableLookup::Atan2(int x, int y) {
    signed int quadrant;
    __int64 dividend;
    int quotient;
    int lowIdx;
    int highIdx;
    int angle;

    int X = x;
    int Y = y;

    if (abs(X) < 65536) {
        if ((abs(Y) >> 15) >= abs(X)) X = 0;
    }

    if (!X) {
        if (Y > 0) {
            return uIntegerHalfPi;  // Pi/2
        } else {
            return uIntegerHalfPi + uIntegerPi;  // 3*(Pi/2)
        }
    }

    if (Y) {
        if (X < 0) {
            X = -X;
            if (Y > 0) {
                quadrant = 4;
            } else {
                quadrant = 3;
            }
        } else {
            if (Y > 0) {
                quadrant = 1;
            } else {
                quadrant = 2;
            }
        }

        if (Y < 0) Y = -Y;

        HEXRAYS_LODWORD(dividend) = Y << 16;
        HEXRAYS_HIDWORD(dividend) = Y >> 16;
        quotient = dividend / X;

        // looks like binary search
        {
            int i;
            highIdx = uIntegerHalfPi;
            lowIdx = 0;

            for (i = 0; i < 6; ++i) {
                if (quotient <= pTanTable[(lowIdx + highIdx) / 2])
                    highIdx = (lowIdx + highIdx) / 2;
                else
                    lowIdx = (lowIdx + highIdx) / 2;
            }
        }

        angle = lowIdx + 1;
        while (angle < (highIdx - 1) && quotient >= pTanTable[angle]) ++angle;

        switch (quadrant) {
            case 1:  // X > 0, Y > 0
                return angle;

            case 2:                               // X > 0, Y < 0
                return uIntegerDoublePi - angle;  // 2*Pi - angle

            case 3:                         // X > 0, Y < 0
                return uIntegerPi + angle;  // Pi + angle

            case 4:                         // X < 0, Y > 0
                return uIntegerPi - angle;  // Pi - angle
        }

        // should never get here
        return 0;
    }

    if (X < 0)  // Y == 0, X < 0
        return uIntegerPi;

    return 0;
}

//----- (0042EBDB) --------------------------------------------------------
int TrigTableLookup::Sin(int angle) { return Cos(angle - this->uIntegerHalfPi); }

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
