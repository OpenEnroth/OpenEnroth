#pragma once

#include <cmath>
#include <algorithm>

[[nodiscard]] inline bool FuzzyIsNull(float value) {
    return std::abs(value) < 0.00001f;
}

[[nodiscard]] inline bool FuzzyIsNull(double value) {
    return std::abs(value) < 0.000000000001;
}

[[nodiscard]] inline bool FuzzyEquals(float l, float r) {
    return std::abs(l - r) * 100000.f <= std::min(std::abs(l), std::abs(r));
}

[[nodiscard]] inline bool FuzzyEquals(double l, double r) {
    return std::abs(l - r) * 1000000000000. <= std::min(std::abs(l), std::abs(r));
}
