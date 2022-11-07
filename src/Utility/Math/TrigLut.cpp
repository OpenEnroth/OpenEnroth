#include "TrigLut.h"

#include <cmath>

TrigTableLookup TrigLUT;

TrigTableLookup::TrigTableLookup() {
    for (int i = 0; i <= this->uIntegerHalfPi; i++)
        pCosTable[i] = std::cos(i * M_PI / uIntegerPi);
}

float TrigTableLookup::Cos(int angle) const {
    angle &= uDoublePiMask;

    if (angle > uIntegerPi)
        angle = uIntegerDoublePi - angle;
    if (angle >= uIntegerHalfPi)
        return -pCosTable[uIntegerPi - angle];
    else
        return pCosTable[angle];
}

float TrigTableLookup::Sin(int angle) const {
    return Cos(angle - this->uIntegerHalfPi);
}

int TrigTableLookup::Atan2(int x, int y) const {
    double angle = std::atan2(static_cast<double>(y), static_cast<double>(x));

    return static_cast<int>(angle / M_PI * 1024) & uDoublePiMask;
}
