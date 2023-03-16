#pragma once

#include <cmath>
#include <algorithm>

[[nodiscard]] inline bool fuzzyIsNull(float value) {
    return std::abs(value) < 0.00001f;
}

[[nodiscard]] inline bool fuzzyIsNull(double value) {
    return std::abs(value) < 0.000000000001;
}

[[nodiscard]] inline bool fuzzyEquals(float l, float r) {
    return std::abs(l - r) * 100000.f <= std::min(std::abs(l), std::abs(r));
}

[[nodiscard]] inline bool fuzzyEquals(double l, double r) {
    return std::abs(l - r) * 1000000000000. <= std::min(std::abs(l), std::abs(r));
}
