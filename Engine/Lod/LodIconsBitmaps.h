#pragma once

#include "Engine/LodTexture.h"
#include "Engine/Lod/Reader.h"


enum TEXTURE_TYPE {
    TEXTURE_DEFAULT = 0,
    TEXTURE_24BIT_PALETTE = 1,
};



#define MAX_LOD_TEXTURES 1000

class LODFile_IconsBitmaps : public Lod::Reader {
public:
    LODFile_IconsBitmaps();
    virtual ~LODFile_IconsBitmaps();
    void SyncLoadedFilesCount();
    unsigned int FindTextureByName(const char* pName);
    bool Load(const std::string& filename, const std::string& folder);
    void ReleaseAll();
    unsigned int LoadTexture(const std::string& filename, TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    LodTexture* LoadTexturePtr(const std::string& filename, TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    int LoadTextureFromLOD(LodTexture* pOutTex, const std::string& filename, TEXTURE_TYPE eTextureType);
    int ReloadTexture(LodTexture* pDst, const std::string& filename, int mode);
    void ReleaseHardwareTextures();
    void ReleaseLostHardwareTextures();
    // void _410423_move_textures_to_device();
    void SetupPalettes(unsigned int uTargetRBits, unsigned int uTargetGBits,
        unsigned int uTargetBBits);
    void ReleaseAll2();
    void RemoveTexturesPackFromTextureList();
    void RemoveTexturesFromTextureList();
    void _inlined_sub0();
    void _inlined_sub1();
    void _inlined_sub2();

    int LoadDummyTexture();

    LodTexture* GetTexture(int idx);

    LodTexture pTextures[MAX_LOD_TEXTURES];
    unsigned int uNumLoadedFiles;
    int dword_11B80;
    int dword_11B84;  // bitmaps lod reserved
    int dword_11B88;
    int uTextureRedBits;
    int uTextureGreenBits;
    int uTextureBlueBits;
    int uNumPrevLoadedFiles;
    int uTexturePacksCount;
    int pFacesLock;
    // int can_load_hardware_sprites;
    struct IDirectDrawSurface** pHardwareSurfaces;
    struct IDirect3DTexture2** pHardwareTextures;
};

extern LODFile_IconsBitmaps* pEvents_LOD;
extern LODFile_IconsBitmaps* pIcons_LOD;
extern LODFile_IconsBitmaps* pBitmaps_LOD;
