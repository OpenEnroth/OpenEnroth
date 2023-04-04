#pragma once

#include <cstring>
#include <string>
#include <vector>

#include "Engine/Graphics/Image.h"
#include "Utility/Memory/Blob.h"

class Sprite;

#define MAX_LOD_TEXTURES 1000
#define MAX_LOD_SPRITES 1500

/*  354 */
enum class TEXTURE_TYPE {
    TEXTURE_DEFAULT = 0,
    TEXTURE_24BIT_PALETTE = 0x1,
};
using enum TEXTURE_TYPE;

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

    bool Open(const std::string &pFilename);
    void Close();

    Blob LoadRaw(const std::string &pContainer);
    Blob LoadCompressedTexture(const std::string &pContainer);
    Blob LoadCompressed(const std::string &pContainer);
    bool DoesContainerExist(const std::string &filename);

    std::string GetSubNodeName(size_t index) const { return pSubIndices[index].pFilename; }
    size_t GetSubNodesCount() const { return uNumSubDirs; }
    int GetSubNodeIndex(const std::string &name) const;

 protected:
    FILE *FindContainer(const std::string &filename, size_t *data_size = nullptr);
    virtual bool OpenFile(const std::string &filePath);
    bool LoadHeader();
    bool LoadSubIndices(const std::string &sFolder);
    virtual void ResetSubIndices();

 protected:
    FILE *pFile;
    std::string pLODPath;
    bool isFileOpened;

    struct FileHeader header;

    std::vector<Directory> pRoot;
    std::string pContainerName;
    unsigned int uOffsetToSubIndex;

    unsigned int uNumSubDirs;
    struct Directory *pSubIndices;
};

class WriteableFile : public File {
 public:
    WriteableFile();
    bool LoadFile(const std::string &filePath, bool bWriting);
    unsigned int Write(const std::string &file_name, const void *pDirData, size_t size, int a4);
    void CloseWriteFile();
    int CreateTempFile();
    bool FixDirectoryOffsets();
    bool _4621A7();
    int CreateNewLod(LOD::FileHeader *pHeader, const std::string &root_name, const std::string &Source);

    void AllocSubIndicesAndIO(unsigned int uNumSubIndices, unsigned int uBufferSize);
    void FreeSubIndexAndIO();
    bool AppendDirectory(const std::string &file_name, const void *pData, size_t data_size);

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
    unsigned int FindTextureByName(const std::string &pName);
    bool Load(const std::string &pFilename, const std::string &pFolderName);
    void ReleaseAll();
    unsigned int LoadTexture(const std::string &pContainer, TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    struct Texture_MM7 *LoadTexturePtr(const std::string &pContainer, TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    int LoadTextureFromLOD(struct Texture_MM7 *pOutTex, const std::string &pContainer, TEXTURE_TYPE eTextureType);
    int ReloadTexture(struct Texture_MM7 *pDst, const std::string &pContainer,
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
    int dword_11B84;  // bitmaps lod reserved
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

    char pName[12] {};         // 0
    uint32_t uSpriteSize = 0;        // C
    uint16_t uWidth = 0;         // 10  SW width (as opposed to Sprite::BufferWidth)
    uint16_t uHeight = 0;        // 12  SW height
    uint16_t uPaletteId = 0;     // 14
    uint16_t word_16 = 0;        // 16
    uint16_t uTexturePitch = 0;  // 18
    uint16_t word_1A = 0;        // 1a  flags - 1024 delete bitmap
    uint32_t uDecompressedSize = 0;  // 1c
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
    int LoadSpriteFromFile(LODSprite *pSpriteHeader, const std::string &pContainer);
    bool Load(const std::string &pFilename, const std::string &folder);
    int LoadSprite(const char *pContainerName, unsigned int uPaletteID);
    Sprite *GetSprite(std::string_view pContainerName);
    void ReleaseLostHardwareSprites();
    void ReleaseAll();
    void MoveSpritesToVideoMemory();
    void _inlined_sub0();
    void _inlined_sub1();

    unsigned int uNumLoadedSprites;
    int field_ECA0;  // reserved sprites -522
    int field_ECA4;  // 2nd init sprites
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
