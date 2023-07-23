#include "LodSpriteCache.h"

#include <vector>

#include "Library/Lod/LodReader.h"
#include "Library/Compression/Compression.h"

#include "Utility/Streams/BlobInputStream.h"

#include "AssetsManager.h"

LodSpriteCache *pSprites_LOD = nullptr;
LodSpriteCache *pSprites_LOD_mm6 = nullptr;
LodSpriteCache *pSprites_LOD_mm8 = nullptr;

#pragma pack(push, 1)
struct LODSpriteLine {
    int16_t begin;
    int16_t end;
    uint32_t offset;
};
#pragma pack(pop)

void LODSprite::Release() {
    this->flags = 0;
    this->bitmap.reset();
    this->name[0] = 0;
    this->unk_0 = 0;
    this->paletteId = 0;
    this->emptyBottomLines = 0;
    this->height = 0;
    this->width = 0;
    this->dataSize = 0;
}

LodSpriteCache::LodSpriteCache() = default;

LodSpriteCache::~LodSpriteCache() {
    for (size_t i = 0; i < this->_sprites.size(); ++i) {
        this->_sprites[i].Release();
    }
}

bool LodSpriteCache::open(const std::string &pFilename, const std::string &folder) {
    _reader = LodReader::open(pFilename);
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

    static_assert(sizeof(LODSpriteHeader) == 32, "Wrong type size");

    std::unique_ptr<LODSprite> header = std::make_unique<LODSprite>();
    if (!LoadSpriteFromFile(header.get(), pContainerName))
        return nullptr;

    // if (uNumLoadedSprites == 879) __debugbreak();

    Sprite &sprite = _sprites.emplace_back();
    sprite.pName = pContainerName;
    sprite.uWidth = header->width;
    sprite.uHeight = header->height;
    sprite.texture = assets->getSprite(pContainerName); // TODO(captainurist): very weird dependency here.
    sprite.sprite_header = header.release();
    return &sprite;
}

bool LodSpriteCache::LoadSpriteFromFile(LODSprite *pSprite, const std::string &pContainer) {
    if (!_reader->exists(pContainer))
        return false;

    BlobInputStream input(_reader->read(pContainer));
    input.readOrFail(pSprite, sizeof(LODSpriteHeader));

    strcpy(pSprite->name.data(), pContainer.c_str());

    std::vector<LODSpriteLine> pSpriteLines;
    pSpriteLines.resize(pSprite->height);
    input.readOrFail(pSpriteLines.data(), sizeof(LODSpriteLine) * pSprite->height);

    Blob pixels = input.readBlobOrFail(pSprite->dataSize);
    if (pSprite->decompressedSize)
        pixels = zlib::Uncompress(pixels, pSprite->decompressedSize);

    pSprite->bitmap = GrayscaleImage::solid(pSprite->width, pSprite->height, 0);
    for (size_t i = 0; i < pSprite->height; i++) {
        if (pSpriteLines[i].begin >= 0) {
            memcpy(pSprite->bitmap[i].data() + pSpriteLines[i].begin,
                   static_cast<const char *>(pixels.data()) + pSpriteLines[i].offset,
                   pSpriteLines[i].end - pSpriteLines[i].begin);
        }
    }

    return true;
}
