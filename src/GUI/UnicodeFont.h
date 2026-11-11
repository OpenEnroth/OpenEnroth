#pragma once

#include <vector>
#include <array>
#include <memory>

#include "Library/LodFormats/LodFont.h"

namespace detail {
template<class T>
class UnicodeMap {
 public:
    void insert(char32_t c, T value) {
        size_t lo = static_cast<uint32_t>(c) & 0xFF;
        size_t hi = static_cast<uint32_t>(c) >> 8;
        if (hi >= _mapping.size())
            _mapping.resize(hi + 1);
        if (!_mapping[hi])
            _mapping[hi] = std::make_unique<std::array<T, 256>>();
        (*_mapping[hi])[lo] = value;
    }

    T value(char32_t c) const {
        size_t lo = static_cast<uint32_t>(c) & 0xFF;
        size_t hi = static_cast<uint32_t>(c) >> 8;
        return hi < _mapping.size() ? (*_mapping[hi])[lo] : T();
    }

 private:
    std::vector<std::unique_ptr<std::array<T, 256>>> _mapping; // char32_t -> Index.
};
} // namespace detail

// TODO(captainurist): BS! We need to rewrite this:
//                     - char -> index mapping.
//                     - index -> metrics vector.
//                     - index -> image data pointers.
//                     - image data buffer - class Buffer? Growing bytes storage with pointer stability.

// class Memory --- view into Blob / anything else. std::span<std::byte>, basically, but sane.
// class WriteableMemory --- same as above but writeable.
//
// MemoryBuffer -- pointer-stable owning Memory storage, use MemoryScratchpad internally I guess?
// - Memory store(Memory mem)
//
// LodFont ->
//
// UnicodeFont:
// - detail::UnicodeMap<int> --- map into index
// - int height
// - std::vector<FontMetrics>
// - std::vector<Memory> images --- byte per pixel.
// - Buffer data
// - size_t size() --- total # of chars
// - add(const std::array<char32_t, 256> &characters, const LodFont &) --- makes a copy.
// - add(const std::array<char32_t, 256> &characters, const MM3Font &) --- wait, MM3 font IS a LOD font??? I think so.
//                                                                         Can just add support out of the box.
//
// Also UnicodeMap should have valueOr() now b/c we need to get -1 from there.
//
// Then I'll be able to use multi-color fonts! E.g. mm3 font.
//
// Now, GUIFont. Needs to use ONE texture. RGBA, so up to 4 colors per font. R-G-B-A set in texture, blending working
// out of the box, passing 4 colors into the shader, then re-blending. Basically R+G+B+A=1 guaranteed b/c of how we
// will construct textures --- this can be done as step one.
//
// Then, texture size in GUIFont.
// - Get # of characters.
// - sqrt() - that's our atlas size.
// - height+1, maxwidth+1 - that's atlas cells, +1 so that there is no color bleeding if blending.
// - DONE.
// Single texture, reusing AtlasLayout.
//
// Then, AssetsManager does all font loading. From resources.

class UnicodeFont {
 public:
    [[nodiscard]] int height() const {
        return _fonts[0].height();
    }

    [[nodiscard]] bool supports(char32_t c) const {
        return _indexByCharacter.value(c).character != 0;
    }

    [[nodiscard]] const LodFontMetrics &metrics(char32_t c) const {
        Index index = _indexByCharacter.value(c);
        return index.character == 0 ? _nullMetrics : _fonts[index.font].metrics(index.character);
    }

    [[nodiscard]] GrayscaleImageView image(char32_t c) const {
        Index index = _indexByCharacter.value(c);
        return index.character == 0 ? GrayscaleImageView() : _fonts[index.font].image(index.character);
    }

    /**
     * @return                          Index for the character, or -1 if not supported.
     */
    [[nodiscard]] int index(char32_t c) const {
        Index idx = _indexByCharacter.value(c);
        return idx.character == 0 ? -1 : idx.font * 256 + idx.character;
    }

    /**
     * @return                          Total number of characters in the font.
     */
    [[nodiscard]] int size() const {
        return _fonts.size() * 256;
    }

    void addChunk(const std::array<char32_t, 256> &characters, LodFont &&font);

 private:
    struct Index {
        Index() = default;
        Index(uint8_t font, uint8_t character) : font(font), character(character) {}

        uint8_t font = 0;
        uint8_t character = 0; // 0 means invalid, no reason looking up the font.
    };

    std::vector<LodFont> _fonts;
    std::vector<char32_t> _characters;
    detail::UnicodeMap<Index> _indexByCharacter;
    LodFontMetrics _nullMetrics;
};
