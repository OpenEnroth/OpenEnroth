#pragma once

#include <cstdint>
#include <string>
#include <deque>
#include <memory>

#include "Engine/Graphics/Sprites.h"

#include "Library/Image/Image.h"

class LodReader;

#pragma pack(push, 1)
struct LODSpriteHeader {
    char pName[12] = {};         // 0
    uint32_t uSpriteSize = 0;        // C
    uint16_t uWidth = 0;         // 10  SW width (as opposed to Sprite::BufferWidth)
    uint16_t uHeight = 0;        // 12  SW height
    uint16_t uPaletteId = 0;     // 14
    uint16_t word_16 = 0;        // 16
    uint16_t uTexturePitch = 0;  // 18
    uint16_t word_1A = 0;        // 1a  flags - 1024 delete bitmap
    uint32_t uDecompressedSize = 0;  // 1c
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
    std::unique_ptr<LodReader> _reader;
    int _reservedCount = 0;
    std::deque<Sprite> _sprites;
};

extern LodSpriteCache *pSprites_LOD;
extern LodSpriteCache *pSprites_LOD_mm6;
extern LodSpriteCache *pSprites_LOD_mm8;
