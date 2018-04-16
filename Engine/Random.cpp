#include "Engine/Random.h"

#include "OurMath.h"

struct Random *pRnd = new Random();

//----- (004BE59E) --------------------------------------------------------
void Random::SetRange(int min, int max) {
    this->min = min;
    this->range = max - min;
}

//----- (004BE5B1) --------------------------------------------------------
double Random::GetRandom() {
    Random *v1;  // esi@1
    double v2;   // st7@1
    double v3;   // st7@1

    v1 = this;
    this->field_0[3] = this->field_0[2];
    this->field_0[2] = this->field_0[1];
    v2 = this->field_0[0];
    this->field_0[1] = v2;
    v3 = this->field_0[3] * 1492.0 + this->field_0[2] * 1776.0 +
         this->field_0[3] * 2111111111.0 + v2 * 5115.0 + this->field_0[4];
    v1->field_0[0] = v3 - floor(v3);
    v1->field_0[4] = floor(v3) * 2.328306436538696e-10;
    return v1->field_0[0];
}

//----- (004BE623) --------------------------------------------------------
int Random::GetInRange() {
    float v2;            // ST10_4@1
    int floored_random;  // ecx@1

    v2 = GetRandom() * (double)this->range;
    floored_random = bankersRounding(v2);
    if (floored_random >= this->range) floored_random = this->range;
    return floored_random + this->min;
}

//----- (004BE65D) --------------------------------------------------------
void Random::Initialize(int seed) {
    int v2;           // eax@1
                      //    Random *this_; // esi@1
    signed int v4;    // ecx@3
    unsigned int v5;  // eax@4
    signed int v6;    // edi@5

    v2 = seed;
    if (!seed) v2 = -1;
    v4 = 0;
    do {
        v5 = (((v2 << 13) ^ (unsigned int)v2) >> 17) ^ (v2 << 13) ^ v2;
        v2 = 32 * v5 ^ v5;
        this->field_0[v4++] = (double)(unsigned int)v2 * 2.328306436538696e-10;
    } while (v4 < 5);

    v6 = 19;
    do {
        GetRandom();
        --v6;
    } while (v6);
}
