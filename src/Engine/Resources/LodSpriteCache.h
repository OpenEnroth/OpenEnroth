#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Engine/Graphics/Sprites.h"

#include "Library/Image/Image.h"

class ResourceManager;

class LodSpriteCache {
 public:
    explicit LodSpriteCache(ResourceManager *resources);
    ~LodSpriteCache();

    void reserveLoadedSprites();
    void releaseUnreserved();

    /**
     * @param pContainerName            Sprite name in the LOD.
     * @return                          Cached sprite, or `nullptr` if there's no such sprite.
     */
    Sprite *loadSprite(std::string_view pContainerName);

 private:
    ResourceManager *_resources = nullptr;
    int _reservedCount = 0;
    std::unordered_map<std::string, Sprite> _spriteByName;
    std::vector<std::string> _spritesInOrder;
};

extern LodSpriteCache *pSprites_LOD;
