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

    // Note that std::round call is important here, otherwise atan2(x, y) + atan2(y, x) != uIntegerHalfPi.
    return static_cast<int>(std::round(angle / M_PI * 1024)) & uDoublePiMask;
}
