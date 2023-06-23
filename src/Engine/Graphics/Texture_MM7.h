#pragma once

#include <cstdint>
#include <array>

#include "Library/Binary/MemCopySerialization.h"

#pragma pack(push, 1)
struct TextureHeader {
    std::array<char, 16> pName; // TODO(captainurist): there's quite a lot of unsafe .data() calls for this field.
    uint32_t uSizeOfMaxLevelOfDetail;
    uint32_t uTextureSize;
    uint16_t uTextureWidth;
    uint16_t uTextureHeight;
    int16_t uWidthLn2;  // log2(uTextureWidth)
    int16_t uHeightLn2;  // log2(uTextureHeight)
    int16_t uWidthMinus1;
    int16_t uHeightMinus1;
    int16_t palette_id1;
    int16_t palette_id2;
    uint32_t uDecompressedSize;
    uint32_t pBits;  // 0x0002 - generate mipmaps
    // 0x0200 - 0th palette entry is transparent, else colorkey
    // 0x0400 - don't free buffers (???)
};
#pragma pack(pop)
MM_DECLARE_MEMCOPY_SERIALIZABLE(TextureHeader)

struct Texture_MM7 {
    Texture_MM7();
    void Release();

    TextureHeader header;
    uint8_t *paletted_pixels;
    uint8_t *pLevelOfDetail1;
    uint8_t *pPalette24;
};
