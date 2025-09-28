#pragma once

#include <cassert>
#include <utility>

#include "PlatformEnums.h"

inline PlatformKey platformKeyForDigit(int digit) {
    assert(digit >= 0 && digit <= 9);

    return static_cast<PlatformKey>(std::to_underlying(PlatformKey::KEY_DIGIT_0) + digit);
}
