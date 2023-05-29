#pragma once

#include <cstdint>
#include <cstring>


/**
 * Color in A8B8G8R8 format.
 */
struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255): r(r), g(g), b(b), a(a) {}

    static Color fromC16(uint16_t color) {
        // 16-bit zero was used as a marker for 'default color', but in new code that marker is Color(), so we have
        // to special-case the old behavior.
        if (color == 0)
            return Color();

        Color result;
        result.b = (color & 31) * 8;
        result.g = ((color >> 5) & 63) * 4;
        result.r = ((color >> 11) & 31) * 8;
        result.a = 255;
        return result;
    }

    [[nodiscard]] uint32_t c32() const {
        uint32_t result;
        memcpy(&result, this, 4);
        return result;
    }

    // TODO(captainurist): replace with ColorTag & std::format spec?
    [[nodiscard]] uint16_t c16() const {
        return (b >> (8 - 5)) | (0x7E0 & (g << (6 + 5 - 8))) | (0xF800 & (r << (6 + 5 + 5 - 8)));
    }

    friend bool operator==(const Color &l, const Color &r) = default;
};
