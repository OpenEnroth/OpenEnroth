#pragma once

#include <string>
#include <deque>
#include <memory>

#include "Engine/Graphics/Texture_MM7.h"
#include "Library/Lod/LodReader.h"
#include "Utility/Memory/Blob.h"

class LodReader;

class LodTextureCache {
 public:
    LodTextureCache();
    ~LodTextureCache();

    bool open(const std::string &pFilename, const std::string &pFolderName);

    void reserveLoadedTextures();
    void releaseUnreserved();

    Texture_MM7 *loadTexture(const std::string &pContainer, bool useDummyOnError = true);

    Blob LoadCompressedTexture(const std::string &pContainer); // TODO(captainurist): doesn't belong here.

 private:
    int LoadTextureFromLOD(struct Texture_MM7 *pOutTex, const std::string &pContainer);

 private:
    LodReader _reader;
    int _reservedCount = 0;
    std::deque<Texture_MM7> _textures;
};

extern LodTextureCache *pIcons_LOD;
extern LodTextureCache *pIcons_LOD_mm6;
extern LodTextureCache *pIcons_LOD_mm8;

extern LodTextureCache *pBitmaps_LOD;
extern LodTextureCache *pBitmaps_LOD_mm6;
extern LodTextureCache *pBitmaps_LOD_mm8;
