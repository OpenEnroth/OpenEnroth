#pragma once

#include <vector>

#include "Engine/Strings.h"

#include "Engine/Graphics/Image.h"

class Sprite;

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
        memset(array_0000B0, 0, 80);

        LODSize = 0;
        dword_0000A8 = 0;
        uNumIndices = 0;
    }

    char pSignature[4];
    char LodVersion[80];
    char LodDescription[80];
    uint32_t LODSize;
    uint32_t dword_0000A8;
    uint32_t uNumIndices;
    char array_0000B0[80];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Directory {
    Directory();

    char pFilename[15];
    char field_F;
    uint32_t uOfsetFromSubindicesStart;
    uint32_t uDataSize;
    uint32_t dword_000018;
    uint16_t uNumSubIndices;
    uint16_t priority;
};
#pragma pack(pop)

class File {
 public:
    File();
    virtual ~File();

    bool Open(const String &pFilename);
    void Close();

    void *LoadRaw(const String &pContainer, size_t *data_size = nullptr);
    void *LoadCompressedTexture(const String &pContainer, size_t *data_size = nullptr);
    void *LoadCompressed(const String &pContainer, size_t *data_size = nullptr);
    bool DoesContainerExist(const String &filename);

    String GetSubNodeName(size_t index) const { return pSubIndices[index].pFilename; }
    size_t GetSubNodesCount() const { return uNumSubDirs; }
    int GetSubNodeIndex(const String &name) const;

 protected:
    FILE *FindContainer(const String &filename, size_t *data_size = nullptr);
    virtual bool OpenFile(const String &sFilename);
    bool LoadHeader();
    bool LoadSubIndices(const String &sFolder);
    virtual void ResetSubIndices();

 protected:
    FILE *pFile;
    String pLODName;
    bool isFileOpened;

    struct FileHeader header;

    std::vector<Directory> pRoot;
    String pContainerName;
    unsigned int uOffsetToSubIndex;

    unsigned int uNumSubDirs;
    struct Directory *pSubIndices;
};

class WriteableFile : public File {
 public:
    WriteableFile();
    bool LoadFile(const String &pFilename, bool bWriting);
    unsigned int Write(const String &file_name, const void *pDirData, size_t size, int a4);
    void CloseWriteFile();
    int CreateTempFile();
    int FixDirectoryOffsets();
    bool _4621A7();
    int CreateNewLod(LOD::FileHeader *pHeader, const String &root_name, const String &Source);

    void AllocSubIndicesAndIO(unsigned int uNumSubIndices, unsigned int uBufferSize);
    void FreeSubIndexAndIO();
    bool AppendDirectory(const String &file_name, const void *pData, size_t data_size);

    void ClearSubNodes() { uNumSubDirs = 0; }

 protected:
    virtual void ResetSubIndices();

 protected:
    uint8_t * pIOBuffer;
    unsigned int uIOBufferSize;
    FILE *pOutputFileHandle;
    unsigned int uLODDataSize;
};
};  // namespace LOD

class LODFile_IconsBitmaps : public LOD::File {
 public:
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
struct LODSpriteHeader {
    inline LODSpriteHeader() {
        uHeight = 0;
        uPaletteId = 0;
        word_1A = 0;
    }

    char pName[12];         // 0
    uint32_t uSpriteSize;        // C
    uint16_t uWidth;         // 10  SW width (as opposed to Sprite::BufferWidth)
    uint16_t uHeight;        // 12  SW height
    uint16_t uPaletteId;     // 14
    uint16_t word_16;        // 16
    uint16_t uTexturePitch;  // 18
    uint16_t word_1A;        // 1a
    uint32_t uDecompressedSize;  // 1c
};
#pragma pack(pop)

#pragma pack(push, 1)
struct LODSprite : public LODSpriteHeader {
    inline LODSprite() {
        bitmap = nullptr;
    }
    ~LODSprite();

    void Release();
    struct SoftwareBillboard *_4AD2D1_overlays(struct SoftwareBillboard *a2, int a3);

    uint8_t *bitmap;
};
#pragma pack(pop)

class LODFile_Sprites : public LOD::File {
 public:
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

    unsigned int uNumLoadedSprites;
    int field_ECA0;
    int field_ECA4;
    int field_ECA8;
    // int can_load_hardware_sprites;
    Sprite *pHardwareSprites;
    int field_ECB4;
};

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

extern LOD::WriteableFile *pNew_LOD;
extern LOD::File *pGames_LOD;
