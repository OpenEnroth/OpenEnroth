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

    // TODO(captainurist): replace all calls with color constants
    static Color fromC16(uint16_t color) {
        if (color == 0)
            return Color(); // TODO(captainurist): code in GUIFont relies on this.

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
