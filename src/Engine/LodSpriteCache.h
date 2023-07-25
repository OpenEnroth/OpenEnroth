#pragma once

#include <cstdint>
#include <string>
#include <deque>
#include <memory>

#include "Engine/Graphics/Sprites.h"

#include "Library/Image/Image.h"
#include "Library/Lod/LodReader.h"

class LodReader;

#pragma pack(push, 1)
// See https://github.com/GrayFace/MMExtension/blob/4d6600f164315f38157591d7f0307a86594c22ef/Src/RSPak/Extra/RSLod.pas#L553.
struct LODSpriteHeader {
    std::array<char, 12> name = {};
    uint32_t dataSize = 0; // Size of the pixel data, in bytes.
    uint16_t width = 0; // SW width.
    uint16_t height = 0; // SW height, also the number of `LODSpriteLine`s that follow.
    uint16_t paletteId = 0; // Palette id, references "palXXX".
    uint16_t unk_0 = 0; // Always 0?
    uint16_t emptyBottomLines = 0; // Number of clear lines at the bottom (tail of sprite lines array).
                                   // They are still set in sprite lines array, so this info is redundant.
    uint16_t flags = 0; // Used at runtime only?
    uint32_t decompressedSize = 0; // Size of the decompressed pixel data, 0 if pixel data is not compressed.
};
#pragma pack(pop)

struct LODSprite : public LODSpriteHeader {
    void Release();

    GrayscaleImage bitmap;
};

class LodSpriteCache {
 public:
    LodSpriteCache();
    ~LodSpriteCache();

    bool open(const std::string &pFilename, const std::string &folder);

    void reserveLoadedSprites();
    void releaseUnreserved();

    Sprite *loadSprite(const std::string &pContainerName);

 private:
    bool LoadSpriteFromFile(LODSprite *pSpriteHeader, const std::string &pContainer);

 private:
    LodReader _reader;
    int _reservedCount = 0;
    std::deque<Sprite> _sprites;
};

extern LodSpriteCache *pSprites_LOD;
extern LodSpriteCache *pSprites_LOD_mm6;
extern LodSpriteCache *pSprites_LOD_mm8;
