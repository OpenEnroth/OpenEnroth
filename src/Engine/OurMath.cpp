#include "Engine/OurMath.h"

#include <utility>

//----- (00452A9E) --------------------------------------------------------
int integer_sqrt(int val) {
    ///////////////////////////////
    //Получение квадратного корня//
    ///////////////////////////////

    return int(std::sqrt(val));

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
