#pragma once

#include <cassert>
#include <cstdint>
#include <array>
#include <memory>
#include <vector>

#include "Library/Image/Image.h"
#include "Utility/Memory/MemoryBuffer.h"

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
 * A unicode font assembled glyph by glyph.
 *
 * Glyphs are addressed by a dense glyph index in `[0, size())`, with `index()` translating characters into
 * glyph indices.
 */
class Font {
 public:
    /**
     * Creates an invalid font. It supports no characters, and `add`ing glyphs to it will assert.
     */
    Font() = default;

    /**
     * Creates a valid empty font.
     *
     * @param height                    Font height in pixels. Must be positive.
     */
    explicit Font(int height) : _height(height) {
        assert(height > 0);
    }

    Font(const Font &) = delete;
    Font(Font &&) = default;
    Font &operator=(const Font &) = delete;
    Font &operator=(Font &&) = default;

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
     * @return                          Character this glyph was added for.
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
        return GrayscaleImageView(_images[index], _metrics[index].width, _height);
    }

    /**
     * Adds a glyph for the given character to this font. Copies the glyph image, the passed image doesn't need to
     * stay alive after this call.
     *
     * If the character already has a glyph, the old glyph is replaced, reusing its glyph index.
     *
     * @param c                         Character to add the glyph for.
     * @param metrics                   Glyph metrics. Width in the metrics must match the image width.
     * @param image                     Glyph image, byte per pixel. Must be of the same height as this font.
     * @return                          Glyph index of the added glyph.
     */
    int add(char32_t c, const GlyphMetrics &metrics, GrayscaleImageView image);

 private:
    int _height = 0;
    detail::UnicodeMap<int> _indexByCharacter{-1}; // Character -> glyph index.
    std::vector<char32_t> _characters; // Glyph index -> character.
    std::vector<GlyphMetrics> _metrics; // Glyph index -> metrics.
    std::vector<const uint8_t *> _images; // Glyph index -> image data, pointing into `_data`, byte per pixel.
    MemoryBuffer _data; // Pointer-stable glyph image storage.
};
