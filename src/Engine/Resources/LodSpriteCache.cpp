#include "LodSpriteCache.h"

#include <optional>
#include <string>
#include <vector>

#include "Engine/AssetsManager.h" // TODO(captainurist): dependency doesn't belong here

#include "Library/Geometry/Size.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"

#include "ResourceManager.h"

LodSpriteCache *pSprites_LOD = nullptr;

LodSpriteCache::LodSpriteCache(ResourceManager *resources) : _resources(resources) {}

LodSpriteCache::~LodSpriteCache() {
    for (auto &[_, sprite] : _spriteByName)
        sprite.Release();
}

void LodSpriteCache::reserveLoadedSprites() {  // final init
    _reservedCount = _spritesInOrder.size();
}

void LodSpriteCache::releaseUnreserved() {
    while (_spritesInOrder.size() > _reservedCount) {
        const std::string &name = _spritesInOrder.back();
        _spriteByName[name].Release();
        _spriteByName.erase(name);
        _spritesInOrder.pop_back();
    }
}

Sprite *LodSpriteCache::loadSprite(std::string_view pContainerName) {
    std::string name = ascii::toLower(pContainerName);

    Sprite *result = valuePtr(_spriteByName, name);
    if (result)
        return result;

    std::optional<Sizei> size = _resources->spriteSize(name);
    if (!size)
        return nullptr;

    Sprite &sprite = _spriteByName[name];
    sprite.pName = pContainerName;
    sprite.uWidth = size->w;
    sprite.uHeight = size->h;
    sprite.texture = assets->getSprite(pContainerName); // TODO(captainurist): very weird dependency here.
    _spritesInOrder.push_back(name);
    return &sprite;
}
