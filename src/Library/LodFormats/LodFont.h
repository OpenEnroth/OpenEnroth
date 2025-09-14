#pragma once

#include <cassert>
#include <array>

#include "Library/Image/Image.h"
#include "Utility/Memory/Blob.h"

class LodFont;

namespace lod {
LodFont decodeFont(const Blob &blob);
} // namespace lod

struct LodFontHeader {
    int firstChar = 0; // First char that has an image in this font.
    int lastChar = 0; // Last char that has an image in this font.
    int fontHeight = 0;
};

struct LodFontMetrics {
    int leftSpacing = 0; // Spacing in pixels to the left of the character. Can be negative.
    int width = 0; // Width of the character image.
    int rightSpacing = 0; // Spacing in pixels to the right of the character. Can be negative.
};

struct LodFontAtlas {
    std::array<LodFontMetrics, 256> metrics = {{}};
    std::array<int, 256> offsets = {{}};
};

class LodFont {
 public:
    [[nodiscard]] int height() const {
        return _header.fontHeight;
    }

    [[nodiscard]] bool supports(int c) const {
        return c >= _header.firstChar && c <= _header.lastChar;
    }
    [[nodiscard]] bool supports(char c) const = delete; // Don't call with char as it can be signed.

    /**
     * @param c                         Character to get metrics for, must be in `[0, 255]`.
     * @return                          Font glyph metrics for the character. Will return all zeros for chars that are
     *                                  not supported.
     */
    [[nodiscard]] const LodFontMetrics &metrics(int c) const {
        return _atlas.metrics[c];
    }
    [[nodiscard]] const LodFontMetrics &metrics(char c) const = delete; // Don't call with char as it can be signed.

    /**
     * @param c                         Character to get image for, must be in `[0, 255]`.
     * @return                          Character image. Pixels are `0` for background, `1` for shadow, `255` for text.
     *                                  Will return an empty image for chars that are not supported.
     */
    [[nodiscard]] GrayscaleImageView image(int c) const {
        return GrayscaleImageView(
            static_cast<const uint8_t *>(_pixels.data()) + _atlas.offsets[c],
            _atlas.metrics[c].width,
            _header.fontHeight);
    }
    [[nodiscard]] GrayscaleImageView image(char c) const = delete; // Don't call with char as it can be signed.

    friend LodFont lod::decodeFont(const Blob &blob);

 private:
    LodFontHeader _header;
    LodFontAtlas _atlas;
    Blob _pixels;
};
