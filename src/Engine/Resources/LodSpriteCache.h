#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Engine/Graphics/Sprites.h"

#include "Library/Image/Image.h"
#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodSprite.h"

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
     * @return                          Cached sprite with its size filled in, or `nullptr` if there's no such
     *                                  sprite. Pixels are not decoded, use `decodeSprite` for that.
     */
    Sprite *loadSprite(std::string_view pContainerName);

    /**
     * @param name                      Sprite name in the LOD.
     * @return                          Sprite pixels, decompressed anew on every call.
     */
    LodSprite decodeSprite(std::string_view name);

 private:
    LodReader _reader;
    int _reservedCount = 0;
    std::unordered_map<std::string, Sprite> _spriteByName;
    std::vector<std::string> _spritesInOrder;
};

extern LodSpriteCache *pSprites_LOD;
extern LodSpriteCache *pSprites_LOD_mm6;
extern LodSpriteCache *pSprites_LOD_mm8;
