#pragma once

struct Colorf;

struct HsvColorf {
    float h = 0.0f; // Hue, in [0, 360).
    float s = 0.0f; // Saturation, in [0, 1].
    float v = 0.0f; // Value, in [0, 1].
    float a = 0.0f;

    constexpr HsvColorf() = default;

    /**
     * @offset 0x0048A643
     *
     * @return                          This color, converted to RGB.
     */
    [[nodiscard]] Colorf toRgb() const;
};

