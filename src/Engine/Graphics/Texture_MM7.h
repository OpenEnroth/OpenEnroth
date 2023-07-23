#pragma once

#include <cstdint>
#include <array>

#include "Library/Binary/MemCopySerialization.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

#pragma pack(push, 1)
struct TextureHeader {
    std::array<char, 16> name; // TODO(captainurist): there's quite a lot of unsafe .data() calls for this field.
    uint32_t size; // Width * height, size of the first mipmap. Can be followed by smaller size bitmaps, down to 16x16.
    uint32_t dataSize; // Size of the pixel data that follows, in bytes.
                       // Can be larger than `size` if mipmaps are present. Can be compressed. Doesn't include palette.
    uint16_t width;
    uint16_t height;
    int16_t widthLn2;  // log2(width) for power of 2 sizes, otherwise 0.
    int16_t heightLn2;  // log2(height) for power of 2 sizes, otherwise 0.
    int16_t widthMinus1; // Seems to be set only for power of 2 sizes. Can be random garbage.
    int16_t heightMinus1; // Seems to be set only for power of 2 sizes. Can be random garbage.
    int16_t paletteId; // Palette id, actually palette is stored with the image, so this field isn't needed.
    int16_t anotherPaletteId; // Seems to always be zero?
    uint32_t decompressedSize; // Size of the decompressed pixel data, 0 if pixel data is not compressed.
    uint32_t flags; // 0x0002 - has mipmaps
                    // 0x0100 - not an image, but a text file (???)
                    // 0x0200 - 0th palette entry is transparent, else colorkey
                    // 0x0400 - don't free buffers (???)
};
#pragma pack(pop)
MM_DECLARE_MEMCOPY_SERIALIZABLE(TextureHeader)

struct Texture_MM7 {
    Texture_MM7();
    void Release();

    TextureHeader header;
    GrayscaleImage paletted_pixels;
    Palette palette;
};
