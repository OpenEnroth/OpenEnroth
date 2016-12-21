#pragma once
#include <stdio.h>
#include <memory.h>
#include <cstdint>

#include "Engine/Strings.h"

#include "Engine/Graphics/Image.h"

class Sprite;


#define MAX_LOD_TEXTURES 1000
#define MAX_LOD_SPRITES  1500

/*  354 */
enum TEXTURE_TYPE
{
    TEXTURE_DEFAULT = 0,
    TEXTURE_24BIT_PALETTE = 0x1,
    TEXTURE_16BIT_PALETTE = 0x2,
};

namespace LOD
{
#pragma pack(push, 1)
    struct FileHeader
    {
        //----- (004617B6) --------------------------------------------------------
        inline FileHeader()
        {
            memset(pSignature, 0, 4);
            memset(LodVersion, 0, 80);
            memset(LodDescription, 0, 80);
            memset(array_0000B0, 0, 28);
            memset(array_0000CC, 0, 52);

            pSignature[0] = 0;
            LodVersion[0] = 0;
            LodDescription[0] = 0;
            LODSize = 0;
            dword_0000A8 = 0;
            uNumIndices = 0;
        }

        char pSignature[4];
        char LodVersion[80];
        //char field_C[32];
        //char field_2C[40];
        char LodDescription[80];
        int LODSize;
        int dword_0000A8;
        unsigned int uNumIndices;
        char array_0000B0[28];
        char array_0000CC[52];
    };
#pragma pack(pop)


#pragma pack(push, 1)
    struct Directory
    {
        Directory();

        char pFilename[15];
        char field_F;
        unsigned int uOfsetFromSubindicesStart;
        unsigned int uDataSize;
        int dword_000018;
        unsigned __int16 uNumSubIndices;
        __int16 word_00001E;
    };
#pragma pack(pop)


#pragma pack(push, 1)
    struct File
    {
        File();
        virtual ~File();
        void *LoadRaw(const char *pContainer, int a3);
        FILE *FindContainer(const char *pContainerName, bool bLinearSearch);
        FILE *FindContainer(const String &filename, bool bLinearSearch);
        bool DoesContainerExist(const char *pContainer);
        bool DoesContainerExist(const String &filename);
        int CalcIndexFast(int startIndex, int maxIndex, const char *pContainerName);
        bool LoadHeader(const char *pFilename, bool bWriting);
        int LoadSubIndices(const char *pContainer);
        void AllocSubIndicesAndIO(unsigned int uNumSubIndices, unsigned int uBufferSize);
        void FreeSubIndexAndIO();
        bool AppendDirectory(LOD::Directory *pDir, const void *pData);
        void ResetSubIndices();
        void Close();

        FILE *pFile;
        char pLODName[256];
        bool isFileOpened;
        unsigned __int8 *pIOBuffer;
        unsigned int uIOBufferSize;
        struct FileHeader header;
        struct Directory *pRoot;
        char pContainerName[16];
        unsigned int uCurrentIndexDir;
        unsigned int uLODDataSize;
        unsigned int uNumSubDirs;
        struct Directory *pSubIndices;
        unsigned int uOffsetToSubIndex;
        FILE *pOutputFileHandle;
    };
#pragma pack(pop)
};






/*    6 */
#pragma pack(push, 1)
struct LODWriteableFile : public LOD::File
{
    bool LoadFile(const char *pFilename, bool bWriting);
    unsigned int Write(const LOD::Directory *pDir, const void *pDirData, int a4);
    void CloseWriteFile();
    int CreateTempFile();
    int FixDirectoryOffsets();
    bool _4621A7();
    int CreateNewLod(LOD::FileHeader *pHeader, LOD::Directory *pDir, const char *Source);
};
#pragma pack(pop)




#pragma pack(push, 1)
struct LODFile_IconsBitmaps : public LOD::File
{
    LODFile_IconsBitmaps();
    virtual ~LODFile_IconsBitmaps();
    void SyncLoadedFilesCount();
    unsigned int FindTextureByName(const char *pName);
    bool Load(const char *pFilename, const char *pFolderName);
    void ReleaseAll();
    unsigned int LoadTexture(const char *pContainer, enum TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    struct Texture_MM7 *LoadTexturePtr(const char *pContainer, enum TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    int LoadTextureFromLOD(struct Texture_MM7 *pOutTex, const char *pContainer, enum TEXTURE_TYPE eTextureType);
    int ReloadTexture(struct Texture_MM7 *pDst, const char *pContainer, int mode);
    void ReleaseHardwareTextures();
    void ReleaseLostHardwareTextures();
    void _410423_move_textures_to_device();
    int PlacementLoadTexture(struct Texture_MM7 *pDst, const char *pContainer, unsigned int uTextureType);
    void SetupPalettes(unsigned int uTargetRBits, unsigned int uTargetGBits, unsigned int uTargetBBits);
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
    //int can_load_hardware_sprites;
    struct IDirectDrawSurface **pHardwareSurfaces;
    struct IDirect3DTexture2 **pHardwareTextures;
    char *ptr_011BB4;
};
#pragma pack(pop)






#pragma pack(push, 1)
struct LODSprite
{
    //----- (0046244C) --------------------------------------------------------
    inline LODSprite()
    {
        uHeight = 0;
        uPaletteId = 0;
        word_1A = 0;
        pSpriteLines = nullptr;
        pDecompressedBytes = nullptr;
    }
    ~LODSprite();

    void Release();
    int DrawSprite_sw(struct RenderBillboardTransform_local0 *a2, char a3);
    int _4AD2D1(struct RenderBillboardTransform_local0 *a2, int a3);

    char pName[12]; //0
    int uSpriteSize; //C
    __int16 uWidth; //10
    __int16 uHeight; //12
    __int16 uPaletteId; //14
    __int16 word_16;  //16
    __int16 uTexturePitch; //18
    __int16 word_1A; //1a
    int uDecompressedSize;  //1c
    struct LODSprite_stru0 *pSpriteLines; //20
    void *pDecompressedBytes;  //24
};
#pragma pack(pop)

/*   15 */
#pragma pack(push, 1)
struct LODFile_Sprites : public LOD::File
{
    LODFile_Sprites();
    virtual ~LODFile_Sprites();

    void DeleteSomeSprites();
    void DeleteSpritesRange(int uStartIndex, int uStopIndex);
    int _461397();
    void DeleteSomeOtherSprites();
    int LoadSpriteFromFile(LODSprite *pSpriteHeader, const char *pContainer);
    bool LoadSprites(const char *pFilename);
    int LoadSprite(const char *pContainerName, unsigned int uPaletteID);
    void ReleaseLostHardwareSprites();
    void ReleaseAll();
    void MoveSpritesToVideoMemory();
    void _inlined_sub0();
    void _inlined_sub1();


    /*FILE *pFile;
    unsigned __int8 pLODName[256];
    unsigned int isFileOpened;
    unsigned __int8 *pIOBuffer;
    unsigned int uIOBufferSize;
    struct LOD::FileHeader header;
    struct LOD::Directory *pRoot;
    unsigned __int8 pContainerName[16];
    unsigned int uCurrentIndexDir;
    unsigned int uLODDataSize;
    unsigned int uNumSubIndices;
    struct LOD::Directory *pSubIndices;
    unsigned int uOffsetToSubIndex;
    FILE *pOutputFileHandle;*/
    struct LODSprite pSpriteHeaders[MAX_LOD_SPRITES];
    signed int uNumLoadedSprites;
    int field_ECA0;
    int field_ECA4;
    int field_ECA8;
    //int can_load_hardware_sprites;
    Sprite *pHardwareSprites;
    int field_ECB4;
};
#pragma pack(pop)

/*   17 */
#pragma pack(push, 1)
struct LODSprite_stru0
{
    int16_t a1;
    int16_t a2;
    char* pos;
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


extern int _6A0CA4_lod_binary_search;
extern int _6A0CA8_lod_unused;