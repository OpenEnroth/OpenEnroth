#pragma once

struct Color;
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
     * @return                          This color, converted to `Colorf`.
     */
    [[nodiscard]] Colorf toColorf() const;

    /**
     * @return                          This color, converted to `Color`.
     */
    [[nodiscard]] Color toColor() const;

    /**
     * Performs a hue-saturation-value adjustment.
     *
     * @param dh                        Hue delta, must be in [-180, 180].
     * @param xs                        Saturation multiplier, must be non-negative.
     * @param xv                        Value multiplier, must be non-negative
     * @return                          Adjusted `HsvColorf`.
     */
    [[nodiscard]] HsvColorf adjusted(float dh, float xs, float xv) const;
};

