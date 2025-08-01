#pragma once

#include <cassert>
#include <utility>

// Naming here follows P3724R0, just with the codestyle applied.
// Implementation taken from https://github.com/Eisenwave/cpp-proposals/blob/master/src/intdiv.cpp.

[[nodiscard]] constexpr std::pair<int, int> divRemToNegInf(int x, int y) {
    assert(y > 0); // Negative y makes very little sense so we don't allow it.

    bool quotient_negative = (x ^ y) < 0;
    bool adjust = x % y != 0 && quotient_negative;
    return {
        x / y - static_cast<int>(adjust),
        x % y + static_cast<int>(adjust) * y,
    };
}

/**
 * @param x                             Dividend.
 * @param y                             Divisor, must be positive.
 * @return                              Division result, rounded to negative infinity.
 */
[[nodiscard]] constexpr int divToNegInf(int x, int y) {
    return divRemToNegInf(x, y).first;
}

[[nodiscard]] static int roundToNegInf(int x, int y) {
    return x - divRemToNegInf(x, y).second;
}
