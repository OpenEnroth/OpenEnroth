#include "LodSpriteCache.h"

#include <vector>
#include <utility>
#include <string>
#include <memory>

#include "Library/LodFormats/LodFormats.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"

#include "AssetsManager.h"

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

    std::unique_ptr<LodSprite> header = std::make_unique<LodSprite>();
    if (!LoadSpriteFromFile(header.get(), name))
        return nullptr;

    Sprite &sprite = _spriteByName[name];
    sprite.pName = pContainerName;
    sprite.uWidth = header->image.width();
    sprite.uHeight = header->image.height();
    sprite.texture = assets->getSprite(pContainerName); // TODO(captainurist): very weird dependency here.
    sprite.sprite_header = header.release();
    _spritesInOrder.push_back(name);
    return &sprite;
}

bool LodSpriteCache::LoadSpriteFromFile(LodSprite *pSprite, std::string_view pContainer) {
    if (!_reader.exists(pContainer))
        return false;

    *pSprite = lod::decodeSprite(_reader.read(pContainer));
    return true;
}
