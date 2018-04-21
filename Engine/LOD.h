#pragma once

#include <memory.h>
#include <stdio.h>
#include <cstdint>
#include <vector>

#include "Engine/Strings.h"

#include "Engine/Graphics/Image.h"

class Sprite;

struct IDirectDrawSurface4;

#define MAX_LOD_TEXTURES 1000
#define MAX_LOD_SPRITES 1500

/*  354 */
enum TEXTURE_TYPE {
    TEXTURE_DEFAULT = 0,
    TEXTURE_24BIT_PALETTE = 0x1,
};

namespace LOD {
#pragma pack(push, 1)
struct FileHeader {
    inline FileHeader() {
        memset(pSignature, 0, 4);
        memset(LodVersion, 0, 80);
        memset(LodDescription, 0, 80);
        memset(array_0000B0, 0, 28);
        memset(array_0000CC, 0, 52);

        LODSize = 0;
        dword_0000A8 = 0;
        uNumIndices = 0;
    }

    char pSignature[4];
    char LodVersion[80];
    char LodDescription[80];
    int LODSize;
    int dword_0000A8;
    unsigned int uNumIndices;
    char array_0000B0[28];
    char array_0000CC[52];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Directory {
    Directory();

    char pFilename[15];
    char field_F;
    unsigned int uOfsetFromSubindicesStart;
    unsigned int uDataSize;
    int dword_000018;
    uint16_t uNumSubIndices;
    uint16_t priority;
};
#pragma pack(pop)

struct File {
    File();
    virtual ~File();
    void *LoadRaw(const String &pContainer);
    FILE *FindContainer(const String &filename, size_t *data_size = nullptr);
    bool DoesContainerExist(const String &filename);
    bool LoadHeader(const String &pFilename, bool bWriting);
    int LoadSubIndices(const String &pContainer);
    void AllocSubIndicesAndIO(unsigned int uNumSubIndices,
                              unsigned int uBufferSize);
    void FreeSubIndexAndIO();
    bool AppendDirectory(LOD::Directory *pDir, const void *pData);
    void ResetSubIndices();
    void Close();

    FILE *pFile;
    String pLODName;
    bool isFileOpened;
    uint8_t *pIOBuffer;
    unsigned int uIOBufferSize;
    struct FileHeader header;
    std::vector<Directory> pRoot;
    String pContainerName;
    unsigned int uLODDataSize;
    unsigned int uNumSubDirs;
    struct Directory *pSubIndices;
    unsigned int uOffsetToSubIndex;
    FILE *pOutputFileHandle;
};
};  // namespace LOD

struct LODWriteableFile : public LOD::File {
    bool LoadFile(const String &pFilename, bool bWriting);
    unsigned int Write(const LOD::Directory *pDir, const void *pDirData,
                       int a4);
    void CloseWriteFile();
    int CreateTempFile();
    int FixDirectoryOffsets();
    bool _4621A7();
    int CreateNewLod(LOD::FileHeader *pHeader, LOD::Directory *pDir,
                     const String &Source);
};

struct LODFile_IconsBitmaps : public LOD::File {
    LODFile_IconsBitmaps();
    virtual ~LODFile_IconsBitmaps();
    void SyncLoadedFilesCount();
    unsigned int FindTextureByName(const char *pName);
    bool Load(const String &pFilename, const String &pFolderName);
    void ReleaseAll();
    unsigned int LoadTexture(const char *pContainer,
                             enum TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    struct Texture_MM7 *LoadTexturePtr(
        const char *pContainer,
        enum TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    int LoadTextureFromLOD(struct Texture_MM7 *pOutTex, const char *pContainer,
                           enum TEXTURE_TYPE eTextureType);
    int ReloadTexture(struct Texture_MM7 *pDst, const char *pContainer,
                      int mode);
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

    Texture_MM7 *GetTexture(int idx);

    Texture_MM7 pTextures[MAX_LOD_TEXTURES];
    unsigned int uNumLoadedFiles;
    int dword_11B80;
    int dword_11B84;
    int dword_11B88;
    int uTextureRedBits;
    int uTextureGreenBits;
    int uTextureBlueBits;
    int uNumPrevLoadedFiles;
    int uTexturePacksCount;
    int pFacesLock;
    int _011BA4_debug_paletted_pixels_uncompressed;
    // int can_load_hardware_sprites;
    struct IDirectDrawSurface **pHardwareSurfaces;
    struct IDirect3DTexture2 **pHardwareTextures;
    char *ptr_011BB4;
};

#pragma pack(push, 1)
struct LODSprite {
    inline LODSprite() {
        uHeight = 0;
        uPaletteId = 0;
        word_1A = 0;
        pSpriteLines = nullptr;
        pDecompressedBytes = nullptr;
    }
    ~LODSprite();

    void Release();
    int _4AD2D1_overlays(struct SoftwareBillboard *a2, int a3);

    char pName[12];         // 0
    uint32_t uSpriteSize;        // C
    uint16_t uWidth;         // 10  SW width (as opposed to Sprite::BufferWidth)
    uint16_t uHeight;        // 12  SW height
    uint16_t uPaletteId;     // 14
    uint16_t word_16;        // 16
    uint16_t uTexturePitch;  // 18
    uint16_t word_1A;        // 1a
    uint32_t uDecompressedSize;  // 1c
    struct LODSprite_stru0 *pSpriteLines;  // 20
    void *pDecompressedBytes;              // 24
};
#pragma pack(pop)

struct LODFile_Sprites : public LOD::File {
    LODFile_Sprites();
    virtual ~LODFile_Sprites();

    void DeleteSomeSprites();
    void DeleteSpritesRange(int uStartIndex, int uStopIndex);
    int _461397();
    void DeleteSomeOtherSprites();
    int LoadSpriteFromFile(LODSprite *pSpriteHeader, const String &pContainer);
    bool LoadSprites(const String &pFilename);
    int LoadSprite(const char *pContainerName, unsigned int uPaletteID);
    void ReleaseLostHardwareSprites();
    void ReleaseAll();
    void MoveSpritesToVideoMemory();
    void _inlined_sub0();
    void _inlined_sub1();

    struct LODSprite pSpriteHeaders[MAX_LOD_SPRITES];
    unsigned int uNumLoadedSprites;
    int field_ECA0;
    int field_ECA4;
    int field_ECA8;
    // int can_load_hardware_sprites;
    Sprite *pHardwareSprites;
    int field_ECB4;
};

/*   17 */
#pragma pack(push, 1)
struct LODSprite_stru0 {
    int16_t a1;
    int16_t a2;
    char *pos;
};
#pragma pack(pop)

extern LODFile_IconsBitmaps *pEvents_LOD;

extern LODFile_IconsBitmaps *pIcons_LOD;
extern LODFile_IconsBitmaps *pIcons_LOD_mm6;
extern LODFile_IconsBitmaps *pIcons_LOD_mm8;

extern LODFile_IconsBitmaps *pBitmaps_LOD;
extern LODFile_IconsBitmaps *pBitmaps_LOD_mm6;
extern LODFile_IconsBitmaps *pBitmaps_LOD_mm8;

extern LODFile_Sprites *pSprites_LOD;
extern LODFile_Sprites *pSprites_LOD_mm6;
extern LODFile_Sprites *pSprites_LOD_mm8;

extern LODWriteableFile *pNew_LOD;
extern LODWriteableFile *pGames_LOD;
