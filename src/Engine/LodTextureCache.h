#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Engine/Graphics/Texture_MM7.h"

#include "Library/Lod/LodReader.h"

#include "Utility/Memory/Blob.h"

class LodReader;

class LodTextureCache {
 public:
    LodTextureCache();
    ~LodTextureCache();

    void open(const std::string &pFilename);

    void reserveLoadedTextures();
    void releaseUnreserved();

    Texture_MM7 *loadTexture(const std::string &pContainer, bool useDummyOnError = true);

    Blob LoadCompressedTexture(const std::string &pContainer); // TODO(captainurist): doesn't belong here.

 private:
    bool LoadTextureFromLOD(struct Texture_MM7 *pOutTex, const std::string &pContainer);

 private:
    LodReader _reader;
    int _reservedCount = 0;
    std::unordered_map<std::string, Texture_MM7> _textureByName;
    std::vector<std::string> _texturesInOrder;
};

extern LodTextureCache *pIcons_LOD;
extern LodTextureCache *pIcons_LOD_mm6;
extern LodTextureCache *pIcons_LOD_mm8;

extern LodTextureCache *pBitmaps_LOD;
extern LodTextureCache *pBitmaps_LOD_mm6;
extern LodTextureCache *pBitmaps_LOD_mm8;
