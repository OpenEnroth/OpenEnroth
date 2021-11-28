#pragma once

#include <map>
#include <memory>
#include <numeric>
#include <vector>

#include "Engine/ErrorHandling.h"
#include "Engine/Strings.h"
#include "Engine/Graphics/Image.h"

class Sprite;

#define MAX_LOD_TEXTURES 1000
#define MAX_LOD_SPRITES 1500

enum LOD_VERSION {
    LOD_VERSION_MM6,
    LOD_VERSION_MM7,
    LOD_VERSION_MM8,
};

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
        memset(LodVersion, 0, sizeof(LodVersion));
        memset(LodDescription, 0, sizeof(LodDescription));
        memset(array_0000B0, 0, sizeof(array_0000B0));

        LODSize = 0;
        dword_0000A8 = 0;
        num_directories = 0;
    }

    char pSignature[4];
    char LodVersion[80];
    char LodDescription[80];
    uint32_t LODSize;
    uint32_t dword_0000A8;
    uint32_t num_directories;
    char array_0000B0[80];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Directory_Image_Mm6 {
    inline Directory_Image_Mm6() {
        memset(this, 0, sizeof(this));
    }

    char pFilename[16];
    uint32_t data_offset;
    uint32_t uDataSize;
    uint32_t dword_000018;
    uint16_t num_items;
    uint16_t priority;
};


struct File_Image_Mm6 {
    inline File_Image_Mm6() {
        memset(this, 0, sizeof(this));
    }

    char name[16];
    uint32_t data_offset;
    uint32_t size;
    uint32_t dword_000018;
    uint16_t num_items;
    uint16_t priority;
};

struct File_Image_Mm8 {
    inline File_Image_Mm8() {
        memset(this, 0, sizeof(this));
    }

    char name[16];
    int32_t unk_0;
    int32_t unk_1;
    int32_t unk_2;
    int32_t unk_3;
    int32_t unk_4;
    int32_t unk_5;
    int32_t unk_6;
    int32_t unk_7;
    int32_t unk_8;
    int32_t unk_9;
    int32_t unk_10;
    int32_t unk_11;
    int32_t unk_12;
    int32_t unk_13;
    int32_t unk_14;
};
#pragma pack(pop)

struct File {
    std::string name;
    size_t offset;
    size_t size;
};


struct Directory {
    std::string name;
    std::vector<File> files;
    size_t files_start;

    inline size_t size_in_bytes() const {
        return std::accumulate(
            files.begin(),
            files.end(),
            size_t{ 0 },
            [](const size_t accumulated_size, const File& file) {
                return accumulated_size + file.size;
            }
        );
    }

    inline void recalculate_file_offsets(size_t start_file_offset) {
        for (auto& file : files) {
            file.offset = start_file_offset;
            start_file_offset += file.size;
        }
    }
};


class Container {
 public:
     Container();
    virtual ~Container();

    bool Open(const std::string& pFilename);
    bool OpenFolder(const std::string& folder);
    void Close();

    void *LoadRaw(const std::string& pContainer, size_t *data_size = nullptr);
    void *LoadCompressed2(const std::string& pContainer, size_t *data_size = nullptr);
    void *LoadCompressed(const std::string& pContainer, size_t *data_size = nullptr);
    bool DoesContainerExist(const std::string& filename);

    std::string GetSubNodeName(size_t index) const { return _current_folder->files[index].name; }
    size_t GetSubNodesCount() const { return _current_folder->files.size(); }
    int GetSubNodeIndex(const std::string& name) const;

    LOD_VERSION GetVersion() const {
        static std::map<std::string, LOD_VERSION> version_map = {
            {"MMVI", LOD_VERSION_MM6},
            {"MMVII", LOD_VERSION_MM7},
            {"MMVIII", LOD_VERSION_MM8},
        };

        const char* version = _header.LodVersion;
        auto it = version_map.find(version);
        if (it != version_map.end()) {
            return it->second;
        }
        Error("Unknown LOD version: %s", version);
    }

 protected:
    FILE *FindContainer(const String &filename, size_t *data_size = nullptr);
    virtual bool OpenFile(const String &sFilename);
    bool LoadHeader();
    virtual void ResetSubIndices();

 protected:
    FILE *pFile;
    String pLODName;
    bool isFileOpened;

    struct FileHeader _header;
    std::vector<Directory_Image_Mm6> _index;
    std::shared_ptr<Directory> _current_folder;
    //unsigned int _current_folder_ptr;
    //unsigned int _current_folder_num_items;
    //Directory *_current_folder_items;
};

class WriteableFile : public Container {
 public:
    WriteableFile();
    bool LoadFile(const String &pFilename, bool bWriting);

    bool AppendFileToCurrentDirectory(const std::string& file_name, const void* file_bytes, size_t file_size);
    bool AddFileToCurrentDirectory(const std::string& file_name, const void *file_bytes, size_t file_size, int flags = 0);
    
    void CloseWriteFile();
    int CreateTempFile();
    int FixDirectoryOffsets();
    bool _4621A7();
    int CreateEmptyLod(LOD::FileHeader *pHeader, const String &root_name, const String &Source);

    void AllocSubIndicesAndIO(unsigned int uNumSubIndices, unsigned int uBufferSize);
    void FreeSubIndexAndIO();

    void ClearSubNodes() { _current_folder->files.clear(); }

 protected:
    virtual void ResetSubIndices();

 protected:
    uint8_t * pIOBuffer;
    unsigned int uIOBufferSize;
    FILE *pOutputFileHandle;
    unsigned int uLODDataSize;
};
};  // namespace LOD

class LODFile_IconsBitmaps : public LOD::Container {
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

class LODFile_Sprites : public LOD::Container {
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
extern LOD::Container *pGames_LOD;
