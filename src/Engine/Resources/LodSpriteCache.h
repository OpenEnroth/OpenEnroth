#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Engine/Graphics/Sprites.h"

#include "Library/Image/Image.h"
#include "Library/Lod/LodReader.h"

class LodReader;

class LodSpriteCache {
 public:
    LodSpriteCache();
    ~LodSpriteCache();

    bool open(Blob blob);

    void reserveLoadedSprites();
    void releaseUnreserved();

    /**
     * @param pContainerName            Sprite name in the LOD.
     * @return                          Cached sprite, or `nullptr` if there's no such sprite.
     */
    Sprite *loadSprite(std::string_view pContainerName);

    Blob read(std::string_view pContainer) const; // TODO(captainurist): drop this, the texture loader shouldn't read through the sprite cache.

 private:
    LodReader _reader;
    int _reservedCount = 0;
    std::unordered_map<std::string, Sprite> _spriteByName;
    std::vector<std::string> _spritesInOrder;
};

extern LodSpriteCache *pSprites_LOD;
extern LodSpriteCache *pSprites_LOD_mm6;
extern LodSpriteCache *pSprites_LOD_mm8;
