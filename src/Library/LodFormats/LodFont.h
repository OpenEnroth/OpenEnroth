#pragma once

#include <cassert>
#include <array>
#include <utility>

#include "Library/Image/Image.h"
#include "Utility/Memory/Blob.h"

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
    LodFont() = default;
    LodFont(int height, const LodFontAtlas &atlas, Blob pixels)
        : _height(height), _atlas(atlas), _pixels(std::move(pixels)) {}

    [[nodiscard]] int height() const {
        return _height;
    }

    [[nodiscard]] bool supports(char c) const {
        return _atlas.metrics[static_cast<unsigned char>(c)].width != 0;
    }

    /**
     * @param c                         Character to get metrics for, must be in `[0, 255]`.
     * @return                          Font glyph metrics for the character. Will return all zeros for chars that are
     *                                  not supported.
     */
    [[nodiscard]] const LodFontMetrics &metrics(char c) const {
        return _atlas.metrics[static_cast<unsigned char>(c)];
    }

    /**
     * @param c                         Character to get image for, must be in `[0, 255]`.
     * @return                          Character image. Pixels are `0` for background, `1` for shadow, `255` for text.
     *                                  Will return an empty image for chars that are not supported. Note that some of
     *                                  MM7 fonts have blank images for some of the characters, e.g. `book.fnt` has
     *                                  a blank 2-pixel wide image for U+00B9 (Superscript One).
     */
    [[nodiscard]] GrayscaleImageView image(char c) const {
        return GrayscaleImageView(
            static_cast<const uint8_t *>(_pixels.data()) + _atlas.offsets[static_cast<unsigned char>(c)],
            _atlas.metrics[static_cast<unsigned char>(c)].width,
            _height);
    }

 private:
    int _height = 0;
    LodFontAtlas _atlas;
    Blob _pixels;
};
