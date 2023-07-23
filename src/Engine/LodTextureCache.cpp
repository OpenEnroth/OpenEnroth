#include "LodTextureCache.h"

#include "Library/Lod/LodReader.h"
#include "Library/Compression/Compression.h"

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

bool LodTextureCache::open(const std::string &pFilename, const std::string &pFolderName) {
    _reader = LodReader::open(pFilename);
    return true;
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
        if (iequals(pContainer.data(), pTexture.header.name.data())) {
            return &pTexture;
        }
    }

    if (LoadTextureFromLOD(&_textures.emplace_back(), pContainer) != -1)
        return &_textures.back();
    _textures.pop_back();

    if (!useDummyOnError)
        return nullptr;

    for (Texture_MM7 &pTexture : _textures) {
        if (iequals(pTexture.header.name.data(), "pending")) {
            return &pTexture;
        }
    }

    if (LoadTextureFromLOD(&_textures.emplace_back(), "pending") != -1)
        return &_textures.back();
    _textures.pop_back();

    return nullptr;
}


Blob LodTextureCache::LoadCompressedTexture(const std::string &pContainer) {
    BlobInputStream input(_reader->read(pContainer));

    TextureHeader DstBuf;
    input.readOrFail(&DstBuf, sizeof(TextureHeader));

    if (DstBuf.decompressedSize) {
        return zlib::Uncompress(input.readBlobOrFail(DstBuf.dataSize), DstBuf.decompressedSize);
    } else {
        return input.readBlobOrFail(DstBuf.dataSize);
    }
}

int LodTextureCache::LoadTextureFromLOD(Texture_MM7 *pOutTex, const std::string &pContainer) {
    if (!_reader->exists(pContainer))
        return -1;

    BlobInputStream input(_reader->read(pContainer));

    TextureHeader *header = &pOutTex->header;
    input.readOrFail(header, sizeof(TextureHeader));

    strncpy(header->name.data(), pContainer.c_str(), 16);

    // ICONS
    if (!header->decompressedSize) {
        pOutTex->paletted_pixels = (uint8_t *)malloc(header->dataSize);
        if (header->dataSize)
            input.readOrFail(pOutTex->paletted_pixels, header->dataSize);
    } else {
        // TODO(captainurist): just store Blob in pOutTex
        Blob pixels = zlib::Uncompress(input.readBlobOrFail(header->dataSize), header->decompressedSize);
        pOutTex->paletted_pixels = (uint8_t *)malloc(pixels.size());
        memcpy(pOutTex->paletted_pixels, pixels.data(), pixels.size());
        header->dataSize = pixels.size();
    }

    pOutTex->pPalette24 = (uint8_t *)malloc(0x300);
    input.readOrFail(pOutTex->pPalette24, 0x300);

    if (header->flags & 2) {
        pOutTex->pLevelOfDetail1 =
            &pOutTex->paletted_pixels[header->size];
        // v8->pLevelOfDetail2 =
        // &v8->pLevelOfDetail1[v8->uSizeOfMaxLevelOfDetail >> 2];
        // v8->pLevelOfDetail3 =
        // &v8->pLevelOfDetail2[v8->uSizeOfMaxLevelOfDetail >> 4];
    } else {
        pOutTex->pLevelOfDetail1 = 0;
        // v8->pLevelOfDetail2 = 0;
        // v8->pLevelOfDetail3 = 0;
    }

    for (int v41 = 1; v41 < 15; ++v41) {
        if (1 << v41 == header->width) header->widthLn2 = v41;
    }
    for (int v42 = 1; v42 < 15; ++v42) {
        if (1 << v42 == header->height) header->heightLn2 = v42;
    }

    header->widthMinus1 = (1 << header->widthLn2) - 1;
    header->heightMinus1 = (1 << header->heightLn2) - 1;

    return 1;
}
