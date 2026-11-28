#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <array>
#include <memory>
#include <vector>

#include "Library/Image/Image.h"

#include "Utility/Memory/Blob.h"

namespace detail {
/**
 * Sparse map from `char32_t` into `T`, implemented as a paged array.
 */
template<class T>
class UnicodeMap {
 public:
    explicit UnicodeMap(T defaultValue = T()) : _default(defaultValue) {}

    void insert(char32_t c, T value) {
        size_t lo = static_cast<uint32_t>(c) & 0xFF;
        size_t hi = static_cast<uint32_t>(c) >> 8;
        if (hi >= _pages.size())
            _pages.resize(hi + 1);
        if (!_pages[hi]) {
            _pages[hi] = std::make_unique<std::array<T, 256>>();
            _pages[hi]->fill(_default);
        }
        (*_pages[hi])[lo] = value;
    }

    [[nodiscard]] T value(char32_t c) const {
        size_t lo = static_cast<uint32_t>(c) & 0xFF;
        size_t hi = static_cast<uint32_t>(c) >> 8;
        return hi < _pages.size() && _pages[hi] ? (*_pages[hi])[lo] : _default;
    }

 private:
    T _default = T();
    std::vector<std::unique_ptr<std::array<T, 256>>> _pages;
};
} // namespace detail

// TODO(captainurist): Next steps:
//                     - Texture size in GUIFont: get # of characters, sqrt() - that's our atlas size,
//                       height+1 / maxwidth+1 - that's atlas cells, +1 so that there is no color bleeding if blending.
//                       Single texture, reusing AtlasLayout.
//                     - Then, AssetsManager does all font loading. From resources.
//                     - MM3 fonts are LOD fonts, so multi-color fonts should work out of the box.

struct GlyphMetrics {
    int leftSpacing = 0; // Spacing in pixels to the left of the glyph. Can be negative.
    int width = 0; // Width of the glyph image.
    int rightSpacing = 0; // Spacing in pixels to the right of the glyph. Can be negative.
};

/**
 * An immutable unicode bitmap font. All glyph images are stored in a single `Blob`. Use `FontBuilder` to assemble
 * a font glyph by glyph, or `fromData` if the data is already laid out.
 *
 * Glyphs are addressed by a dense glyph index in `[0, size())`, with `index()` translating characters into
 * glyph indices.
 */
class Font {
 public:
    /**
     * Creates an invalid font that supports no characters.
     */
    Font() = default;

    Font(const Font &) = delete;
    Font(Font &&) = default;
    Font &operator=(const Font &) = delete;
    Font &operator=(Font &&) = default;

    /**
     * @param height                    Font height in pixels. Must be positive.
     * @param characters                Glyph index -> character. Characters must be non-zero and unique.
     * @param metrics                   Glyph index -> metrics, same size as `characters`.
     * @param offsets                   Glyph index -> offset of the glyph image in `pixels`, same size as
     *                                  `characters`.
     * @param pixels                    Glyph images, byte per pixel, row-major, `metrics[i].width * height` bytes
     *                                  each. Each pixel is a color index: `0` is the transparent background, and
     *                                  `1`-`4` are the font colors.
     * @return                          Font that takes ownership of the provided data.
     */
    [[nodiscard]] static Font fromData(int height, std::vector<char32_t> characters, std::vector<GlyphMetrics> metrics,
                                       std::vector<size_t> offsets, Blob pixels);

    [[nodiscard]] bool operator!() const {
        return _height == 0;
    }

    [[nodiscard]] explicit operator bool() const {
        return _height != 0;
    }

    /**
     * @return                          Font height in pixels. Zero for an invalid font.
     */
    [[nodiscard]] int height() const {
        return _height;
    }

    /**
     * @return                          Total number of glyphs in this font.
     */
    [[nodiscard]] int size() const {
        return _metrics.size();
    }

    /**
     * @param c                         Character to look up.
     * @return                          Glyph index for the character, or `-1` if the character is not supported.
     */
    [[nodiscard]] int index(char32_t c) const {
        return _indexByCharacter.value(c);
    }

    [[nodiscard]] bool supports(char32_t c) const {
        return index(c) != -1;
    }

    /**
     * @param index                     Glyph index, must be in `[0, size())`.
     * @return                          Character this glyph is for.
     */
    [[nodiscard]] char32_t character(int index) const {
        assert(index >= 0 && index < size());
        return _characters[index];
    }

    /**
     * @param index                     Glyph index, must be in `[0, size())`.
     * @return                          Metrics for the glyph.
     */
    [[nodiscard]] const GlyphMetrics &metrics(int index) const {
        assert(index >= 0 && index < size());
        return _metrics[index];
    }

    /**
     * @param index                     Glyph index, must be in `[0, size())`.
     * @return                          Glyph image, byte per pixel. Each pixel is a color index: `0` is the
     *                                  transparent background, and `1`-`4` are the font colors.
     */
    [[nodiscard]] GrayscaleImageView image(int index) const {
        assert(index >= 0 && index < size());
        return GrayscaleImageView(static_cast<const uint8_t *>(_pixels.data()) + _offsets[index], _metrics[index].width,
                                  _height);
    }

 private:
    int _height = 0;
    detail::UnicodeMap<int> _indexByCharacter{-1}; // Character -> glyph index.
    std::vector<char32_t> _characters; // Glyph index -> character.
    std::vector<GlyphMetrics> _metrics; // Glyph index -> metrics.
    std::vector<size_t> _offsets; // Glyph index -> offset of the glyph image in `_pixels`.
    Blob _pixels;
};
