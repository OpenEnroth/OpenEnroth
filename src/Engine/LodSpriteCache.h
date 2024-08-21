#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Engine/Graphics/Sprites.h"

#include "Library/Image/Image.h"
#include "Library/Lod/LodReader.h"

class LodReader;

struct LODSprite {
    void Release();

    std::string name;
    GrayscaleImage bitmap;
};

class LodSpriteCache {
 public:
    LodSpriteCache();
    ~LodSpriteCache();

    bool open(Blob blob);

    void reserveLoadedSprites();
    void releaseUnreserved();

    Sprite *loadSprite(std::string_view pContainerName);

 private:
    bool LoadSpriteFromFile(LODSprite *pSpriteHeader, std::string_view pContainer);

 private:
    LodReader _reader;
    int _reservedCount = 0;
    std::unordered_map<std::string, Sprite> _spriteByName;
    std::vector<std::string> _spritesInOrder;
};

extern LodSpriteCache *pSprites_LOD;
extern LodSpriteCache *pSprites_LOD_mm6;
extern LodSpriteCache *pSprites_LOD_mm8;
