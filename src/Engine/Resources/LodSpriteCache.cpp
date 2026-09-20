#include "LodSpriteCache.h"

#include <vector>
#include <utility>
#include <string>

#include "Engine/AssetsManager.h" // TODO(captainurist): dependency doesn't belong here

#include "Library/LodFormats/LodFormats.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"

LodSpriteCache *pSprites_LOD = nullptr;
LodSpriteCache *pSprites_LOD_mm6 = nullptr;
LodSpriteCache *pSprites_LOD_mm8 = nullptr;

LodSpriteCache::LodSpriteCache() = default;

LodSpriteCache::~LodSpriteCache() {
    for (auto &[_, sprite] : _spriteByName)
        sprite.Release();
}

bool LodSpriteCache::open(Blob blob) {
    _reader.open(std::move(blob));
    return true;
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

    if (!_reader.exists(name))
        return nullptr;

    Sizei size = lod::decodeSpriteSize(_reader.read(name));

    Sprite &sprite = _spriteByName[name];
    sprite.pName = pContainerName;
    sprite.uWidth = size.w;
    sprite.uHeight = size.h;
    sprite.texture = assets->getSprite(pContainerName); // TODO(captainurist): very weird dependency here.
    _spritesInOrder.push_back(name);
    return &sprite;
}

Blob LodSpriteCache::read(std::string_view pContainer) const {
    return _reader.read(pContainer);
}
