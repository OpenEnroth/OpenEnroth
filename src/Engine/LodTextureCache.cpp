#include "LodTextureCache.h"

#include <utility>

#include "Library/LodFormats/LodFormats.h"

#include "Utility/Streams/BlobInputStream.h"
#include "Utility/String.h"
#include "Utility/MapAccess.h"

LodTextureCache *pIcons_LOD = nullptr;
LodTextureCache *pIcons_LOD_mm6 = nullptr;
LodTextureCache *pIcons_LOD_mm8 = nullptr;

LodTextureCache *pBitmaps_LOD = nullptr;
LodTextureCache *pBitmaps_LOD_mm6 = nullptr;
LodTextureCache *pBitmaps_LOD_mm8 = nullptr;

LodTextureCache::LodTextureCache() = default;

LodTextureCache::~LodTextureCache() {
    for (auto &[_, texture] : _textureByName)
        texture.Release();
}

void LodTextureCache::open(const std::string &pFilename) {
    _reader.open(pFilename);
}

void LodTextureCache::reserveLoadedTextures() {
    _reservedCount = _texturesInOrder.size();
}

void LodTextureCache::releaseUnreserved() {
    while (_texturesInOrder.size() > _reservedCount) {
        const std::string &name = _texturesInOrder.back();
        _textureByName[name].Release();
        _textureByName.erase(name);
        _texturesInOrder.pop_back();
    }
}

Texture_MM7 *LodTextureCache::loadTexture(const std::string &pContainer, bool useDummyOnError) {
    std::string name = toLower(pContainer);

    Texture_MM7 *result = valuePtr(_textureByName, name);
    if (result)
        return result;

    result = &_textureByName[name];
    if (LoadTextureFromLOD(result, name)) {
        _texturesInOrder.push_back(name);
        return result;
    }
    _textureByName.erase(name);

    if (useDummyOnError) {
        return loadTexture("pending", false);
    } else {
        return nullptr;
    }
}

Blob LodTextureCache::LoadCompressedTexture(const std::string &pContainer) {
    return lod::decodeCompressed(_reader.read(pContainer));
}

bool LodTextureCache::LoadTextureFromLOD(Texture_MM7 *pOutTex, const std::string &pContainer) {
    if (!_reader.exists(pContainer))
        return false;

    LodImage image = lod::decodeImage(_reader.read(pContainer));

    pOutTex->name = pContainer;
    pOutTex->indexed = std::move(image.image);
    pOutTex->palette = image.palette;
    pOutTex->zeroIsTransparent = image.zeroIsTransparent;
    return true;
}
