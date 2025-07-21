#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Library/Lod/LodReader.h"

#include "Utility/Memory/Blob.h"

struct LodImage;
class LodReader;

class LodTextureCache {
 public:
    LodTextureCache();
    ~LodTextureCache();

    void open(Blob blob);

    void reserveLoadedTextures();
    void releaseUnreserved();

    LodImage *loadTexture(std::string_view pContainer, bool useDummyOnError = true);

    Blob LoadCompressedTexture(std::string_view pContainer); // TODO(captainurist): doesn't belong here.

 private:
    bool LoadTextureFromLOD(LodImage *pOutTex, std::string_view pContainer);

 private:
    LodReader _reader;
    int _reservedCount = 0;
    std::unordered_map<std::string, LodImage> _textureByName;
    std::vector<std::string> _texturesInOrder;
};

extern LodTextureCache *pIcons_LOD;
extern LodTextureCache *pIcons_LOD_mm6;
extern LodTextureCache *pIcons_LOD_mm8;

extern LodTextureCache *pBitmaps_LOD;
extern LodTextureCache *pBitmaps_LOD_mm6;
extern LodTextureCache *pBitmaps_LOD_mm8;
