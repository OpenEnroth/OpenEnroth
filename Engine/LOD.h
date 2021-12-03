#pragma once

#include <map>
#include <memory>
#include <numeric>
#include <vector>

#include "Engine/ErrorHandling.h"
#include "Engine/LodTexture.h"


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
};


class Container {
public:
    Container();
    virtual ~Container();

    bool Open(const std::string& pFilename);
    bool OpenFolder(const std::string& folder);
    void Close();

    bool FileExists(const std::string& filename);
    void *LoadRaw(const std::string& pContainer, size_t *data_size = nullptr);
    void *LoadCompressed2(const std::string& filename, size_t *out_file_size = nullptr);
    void *LoadCompressed(const std::string& pContainer, size_t *data_size = nullptr);

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
    FILE *FindFile(const std::string& filename, size_t *out_file_size = nullptr);
    virtual bool OpenFile(const std::string& sFilename);
    bool LoadHeader();
    virtual void ResetSubIndices();

protected:
    std::string _filename;
    FILE *_file = nullptr;

    struct FileHeader _header;
    std::vector<std::shared_ptr<Directory>> _index;
    std::shared_ptr<Directory> _current_folder = nullptr;
};

class WriteableFile : public Container {
 public:
    WriteableFile();
    bool LoadFile(const std::string &filename, bool writing);

    bool AppendFileToCurrentDirectory(const std::string& file_name, const void* file_bytes, size_t file_size);
    bool AddFileToCurrentDirectory(const std::string& file_name, const void *file_bytes, size_t file_size, int flags = 0);
    
    void CloseWriteFile();
    int CreateTempFile();
    int FixDirectoryOffsets();
    bool _4621A7();
    int CreateEmptyLod(LOD::FileHeader *pHeader, const std::string& root_name, const std::string& Source);

    void FreeSubIndexAndIO();

    void ClearSubNodes() { _current_folder->files.clear(); }

 protected:
    virtual void ResetSubIndices();

 protected:
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
    bool Load(const std::string& pFilename, const std::string& pFolderName);
    void ReleaseAll();
    unsigned int LoadTexture(const std::string& pContainer, TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    LodTexture *LoadTexturePtr(const std::string& pContainer, TEXTURE_TYPE uTextureType = TEXTURE_DEFAULT);
    int LoadTextureFromLOD(LodTexture* pOutTex, const std::string& pContainer, TEXTURE_TYPE eTextureType);
    int ReloadTexture(LodTexture* pDst, const std::string& pContainer, int mode);
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
    struct IDirectDrawSurface **pHardwareSurfaces;
    struct IDirect3DTexture2 **pHardwareTextures;
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
    ~LODSprite();

    void Release();
    struct SoftwareBillboard *_4AD2D1_overlays(struct SoftwareBillboard *a2, int a3);

    uint8_t *bitmap = nullptr;
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
    int LoadSpriteFromFile(LODSprite *pSpriteHeader, const std::string& pContainer);
    bool LoadSprites(const std::string& pFilename);
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
extern LODFile_IconsBitmaps *pBitmaps_LOD;
extern LODFile_Sprites *pSprites_LOD;

extern LOD::WriteableFile *pNew_LOD;
extern LOD::Container *pGames_LOD;
