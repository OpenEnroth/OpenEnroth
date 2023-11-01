#include "LodSpriteCache.h"

#include <vector>
#include <utility>

#include "Library/LodFormats/LodFormats.h"

#include "Utility/Streams/BlobInputStream.h"

#include "AssetsManager.h"

LodSpriteCache *pSprites_LOD = nullptr;
LodSpriteCache *pSprites_LOD_mm6 = nullptr;
LodSpriteCache *pSprites_LOD_mm8 = nullptr;

void LODSprite::Release() {
    bitmap.reset();
    name.clear();
}

LodSpriteCache::LodSpriteCache() = default;

LodSpriteCache::~LodSpriteCache() {
    for (size_t i = 0; i < this->_sprites.size(); ++i) {
        this->_sprites[i].Release();
    }
}

bool LodSpriteCache::open(const std::string &pFilename, const std::string &folder) {
    _reader.open(pFilename);
    return true;
}

void LodSpriteCache::reserveLoadedSprites() {  // final init
    _reservedCount = _sprites.size();
}

void LodSpriteCache::releaseUnreserved() {
    for (size_t i = this->_reservedCount; i < this->_sprites.size(); i++) {
        this->_sprites[i].Release();
    }
    this->_sprites.resize(this->_reservedCount);
}

Sprite *LodSpriteCache::loadSprite(const std::string &pContainerName) {
    for (Sprite &pSprite : _sprites) {
        if (pSprite.pName == pContainerName) {
            return &pSprite;
        }
    }

    std::unique_ptr<LODSprite> header = std::make_unique<LODSprite>();
    if (!LoadSpriteFromFile(header.get(), pContainerName))
        return nullptr;

    // if (uNumLoadedSprites == 879) assert(false);

    Sprite &sprite = _sprites.emplace_back();
    sprite.pName = pContainerName;
    sprite.uWidth = header->bitmap.width();
    sprite.uHeight = header->bitmap.height();
    sprite.texture = assets->getSprite(pContainerName); // TODO(captainurist): very weird dependency here.
    sprite.sprite_header = header.release();
    return &sprite;
}

bool LodSpriteCache::LoadSpriteFromFile(LODSprite *pSprite, const std::string &pContainer) {
    if (!_reader.exists(pContainer))
        return false;

    LodSprite sprite = lod::decodeSprite(_reader.read(pContainer));
    pSprite->name = pContainer;
    pSprite->bitmap = std::move(sprite.image);

    return true;
}
