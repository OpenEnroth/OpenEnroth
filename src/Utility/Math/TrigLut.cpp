#include "TrigLut.h"

#include <cmath>

TrigTableLookup TrigLUT;

TrigTableLookup::TrigTableLookup() {
    for (int i = 0; i <= this->uIntegerHalfPi; i++)
        _cosTable[i] = std::cos(i * M_PI / uIntegerPi);
}

float TrigTableLookup::cos(int angle) const {
    angle &= uDoublePiMask;

    if (angle > uIntegerPi)
        angle = uIntegerDoublePi - angle;
    if (angle >= uIntegerHalfPi)
        return -_cosTable[uIntegerPi - angle];
    else
        return _cosTable[angle];
}

float TrigTableLookup::sin(int angle) const {
    return cos(angle - this->uIntegerHalfPi);
}

int TrigTableLookup::atan2(int x, int y) const {
    double angle = std::atan2(static_cast<double>(y), static_cast<double>(x));

    return static_cast<int>(angle / M_PI * 1024) & uDoublePiMask;
}
