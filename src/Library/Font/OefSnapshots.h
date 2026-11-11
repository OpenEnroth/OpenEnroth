#pragma once

#include <cstdint>
#include <array>

#include "Library/Binary/BinaryFwd.h"

#pragma pack(push, 1)

/**
 * On-disk header of an OpenEnroth font file. Followed by the zlib-compressed payload, which is
 * `OefHeader::glyphCount` `OefGlyph`s, then the glyph pixel data.
 */
struct OefHeader {
    std::array<char, 4> signature; // Always "OEFT".
    uint32_t version; // Format version.
    uint32_t height; // Font height in pixels. All glyph images are of this height.
    uint32_t glyphCount; // Number of `OefGlyph`s at the start of the payload.
    uint32_t decompressedSize; // Size of the payload after decompression.
};
static_assert(sizeof(OefHeader) == 20);
MM_DECLARE_MEMCOPY_SERIALIZABLE(OefHeader)

struct OefGlyph {
    uint32_t codePoint; // Unicode code point this glyph is for.
    int32_t leftSpacing; // Spacing in pixels to the left of the glyph, can be negative.
    int32_t width; // Width of the glyph image.
    int32_t rightSpacing; // Spacing in pixels to the right of the glyph, can be negative.
    uint32_t pixelOffset; // Offset of the glyph image inside the pixel data that follows the glyph table.
                          // The image is `width * OefHeader::height` bytes, byte per pixel, row-major.
                          // Each pixel is a color index: `0` is background, `1`-`4` are font colors.
};
static_assert(sizeof(OefGlyph) == 20);
MM_DECLARE_MEMCOPY_SERIALIZABLE(OefGlyph)

#pragma pack(pop)
