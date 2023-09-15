#pragma once

#include <cstring>
#include <string>
#include <vector>
#include <memory>

#include "Library/Lod/LodReader.h"
#include "Library/Lod/LodWriter.h"

#include "Utility/Memory/Blob.h"

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

    Blob LoadRaw(const std::string &pContainer) const;
    Blob LoadCompressedTexture(const std::string &pContainer);
    Blob LoadCompressed(const std::string &pContainer);
    bool DoesContainerExist(const std::string &filename);

    std::string GetSubNodeName(size_t index) const { return pSubIndices[index].pFilename; }
    size_t GetSubNodesCount() const { return uNumSubDirs; }
    int GetSubNodeIndex(const std::string &name) const;

 protected:
    FILE *FindContainer(const std::string &filename, size_t *data_size = nullptr) const;
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
    ~WriteableFile() { FreeSubIndexAndIO(); }
    bool LoadFile(const std::string &filePath, bool bWriting);
    unsigned int Write(const std::string &file_name, const void *pDirData, size_t size, int a4);
    unsigned int Write(const std::string &file_name, const Blob &data);
    void CloseWriteFile();
    int CreateTempFile();
    bool FixDirectoryOffsets();
    int CreateNewLod(LOD::FileHeader *pHeader, const std::string &root_name, const std::string &Source);

    void AllocSubIndicesAndIO(unsigned int uNumSubIndices, unsigned int uBufferSize);
    void FreeSubIndexAndIO();
    bool AppendDirectory(const std::string &file_name, const void *pData, size_t data_size);

    void ClearSubNodes() { uNumSubDirs = 0; }

 protected:
    virtual void ResetSubIndices() override;

 protected:
    uint8_t * pIOBuffer;
    unsigned int uIOBufferSize;
    FILE *pOutputFileHandle;
    unsigned int uLODDataSize;
};
};  // namespace LOD

extern std::unique_ptr<LodReader> pSave_LOD;
extern std::unique_ptr<LodReader> pGames_LOD;
