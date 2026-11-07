#pragma once

#include <vector>

#include "Library/Font/Font.h"
#include "Library/Image/Image.h"

/**
 * Assembles a `Font` glyph by glyph.
 */
class FontBuilder {
 public:
    /**
     * @param height                    Font height in pixels. Must be positive.
     */
    explicit FontBuilder(int height);

    [[nodiscard]] int height() const {
        return _height;
    }

    [[nodiscard]] bool supports(char32_t c) const {
        return _indexByCharacter.value(c) != -1;
    }

    /**
     * Adds a glyph for the given character, copying the image. If the character already has a glyph, the old glyph
     * is replaced, keeping its glyph index.
     *
     * @param c                         Character to add the glyph for, must be non-zero.
     * @param metrics                   Glyph metrics. Width in the metrics must match the image width.
     * @param image                     Glyph image, byte per pixel, of the same height as the font.
     */
    void add(char32_t c, const GlyphMetrics &metrics, GrayscaleImageView image);

    /**
     * Adds all glyphs of another font of the same height, replacing the ones already added.
     *
     * @param font                      Font to copy glyphs from.
     */
    void add(const Font &font);

    /**
     * @return                          Assembled font, with glyph indices in the order the characters were first
     *                                  added.
     */
    [[nodiscard]] Font build() &&;

 private:
    int _height = 0;
    detail::UnicodeMap<int> _indexByCharacter{-1};
    std::vector<char32_t> _characters;
    std::vector<GlyphMetrics> _metrics;
    std::vector<GrayscaleImage> _images;
};
