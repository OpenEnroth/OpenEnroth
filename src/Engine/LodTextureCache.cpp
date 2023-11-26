#include "LodTextureCache.h"

#include <utility>

#include "Library/LodFormats/LodFormats.h"

#include "Utility/Streams/BlobInputStream.h"
#include "Utility/String.h"

LodTextureCache *pIcons_LOD = nullptr;
LodTextureCache *pIcons_LOD_mm6 = nullptr;
LodTextureCache *pIcons_LOD_mm8 = nullptr;

LodTextureCache *pBitmaps_LOD = nullptr;
LodTextureCache *pBitmaps_LOD_mm6 = nullptr;
LodTextureCache *pBitmaps_LOD_mm8 = nullptr;

LodTextureCache::LodTextureCache() = default;

LodTextureCache::~LodTextureCache() {
    for (size_t i = 0; i < this->_textures.size(); i++) {
        this->_textures[i].Release();
    }
}

void LodTextureCache::open(const std::string &pFilename) {
    _reader.open(pFilename);
}

void LodTextureCache::reserveLoadedTextures() {
    _reservedCount = _textures.size();
}

void LodTextureCache::releaseUnreserved() {
    for (size_t i = this->_reservedCount; i < this->_textures.size(); i++) {
        this->_textures[i].Release();
    }
    this->_textures.resize(this->_reservedCount);
}

Texture_MM7 *LodTextureCache::loadTexture(const std::string &pContainer, bool useDummyOnError) {
    for (Texture_MM7 &pTexture : _textures) {
        if (iequals(pContainer, pTexture.name)) {
            return &pTexture;
        }
    }

    if (LoadTextureFromLOD(&_textures.emplace_back(), pContainer) != -1)
        return &_textures.back();
    _textures.pop_back();

    if (!useDummyOnError)
        return nullptr;

    for (Texture_MM7 &pTexture : _textures) {
        if (iequals(pTexture.name, "pending")) {
            return &pTexture;
        }
    }

    if (LoadTextureFromLOD(&_textures.emplace_back(), "pending") != -1)
        return &_textures.back();
    _textures.pop_back();

    return nullptr;
}


Blob LodTextureCache::LoadCompressedTexture(const std::string &pContainer) {
    return lod::decodeCompressed(_reader.read(pContainer));
}

int LodTextureCache::LoadTextureFromLOD(Texture_MM7 *pOutTex, const std::string &pContainer) {
    if (!_reader.exists(pContainer))
        return -1;

    LodImage image = lod::decodeImage(_reader.read(pContainer));

    pOutTex->name = pContainer;
    pOutTex->indexed = std::move(image.image);
    pOutTex->palette = image.palette;
    pOutTex->zeroIsTransparent = image.zeroIsTransparent;

    return 1;
}
