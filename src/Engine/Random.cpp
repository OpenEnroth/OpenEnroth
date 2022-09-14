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
    this->field_0[3] = this->field_0[2];
    this->field_0[2] = this->field_0[1];
    this->field_0[1] = this->field_0[0];
    double v3 = this->field_0[3] * 1492.0 +
                this->field_0[2] * 1776.0 +
                this->field_0[3] * 2111111111.0 +
                this->field_0[0] * 5115.0 +
                this->field_0[4];
    this->field_0[0] = v3 - floor(v3);
    this->field_0[4] = floor(v3) * 2.328306436538696e-10;
    return this->field_0[0];
}

int Random::GetInRange() {
    float v2 = GetRandom() * (double)this->range;
    int floored_random = bankersRounding(v2);
    if (floored_random >= this->range) floored_random = this->range;
    return floored_random + this->min;
}

//----- (004BE65D) --------------------------------------------------------
void Random::Initialize(int seed) {
    int v6;    // edi@5

    unsigned int v2 = seed;
    if (!seed) v2 = -1;
    for (int i = 0; i < 5; i++) {
        unsigned int v5 = (((v2 << 13) ^ v2) >> 17) ^ (v2 << 13) ^ v2;
        v2 = 32 * v5 ^ v5;
        this->field_0[i] = (double)v2 * 2.328306436538696e-10;
    }

    v6 = 19;
    do {
        GetRandom();
        --v6;
    } while (v6);
}
