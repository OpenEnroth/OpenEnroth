#pragma once

#include <cmath>
#include <algorithm>

[[nodiscard]] inline bool fuzzyIsNull(float value, float eps = 0.00001f) {
    return std::abs(value) < eps;
}

[[nodiscard]] inline bool fuzzyIsNull(double value, double eps = 0.000000000001) {
    return std::abs(value) < eps;
}

[[nodiscard]] inline bool fuzzyEquals(float l, float r, float eps = 0.00001f) {
    return std::abs(l - r) <= std::min(std::abs(l), std::abs(r)) * eps;
}

[[nodiscard]] inline bool fuzzyEquals(double l, double r, double eps = 0.000000000001) {
    return std::abs(l - r) <= std::min(std::abs(l), std::abs(r)) * eps;
}
