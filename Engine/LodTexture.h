#pragma once

#include <cstdint>
#include <string>


struct LodTextureHeader {
    std::string name;
    uint32_t uSizeOfMaxLevelOfDetail = 0;
    uint32_t uTextureSize = 0;
    uint16_t uTextureWidth = 0;
    uint16_t uTextureHeight = 0;
    int16_t uWidthLn2 = 0;
    int16_t uHeightLn2 = 0;
    int16_t uWidthMinus1 = 0;
    int16_t uHeightMinus1 = 0;
    int16_t palette_id1 = 0;
    int16_t palette_id2 = 0;
    uint32_t uDecompressedSize = 0;
    uint32_t pBits = 0;  // 0x0002 - generate mipmaps
                         // 0x0200 - 0th palette entry is transparent, else colorkey
                         // (7FF)
};


struct LodTexture {
    void Release();

    LodTextureHeader header;
    uint8_t* paletted_pixels = nullptr;
    uint8_t* pLevelOfDetail1 = nullptr;
    uint8_t* pPalette24 = nullptr;
};
