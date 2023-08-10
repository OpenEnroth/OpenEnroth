#include "LOD.h"

#include <filesystem>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/ErrorHandling.h"
#include "Engine/EngineIocContainer.h"

#include "Library/Compression/Compression.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/Logger/Logger.h"

#include "Utility/Memory/FreeDeleter.h"
#include "Utility/String.h"
#include "Utility/DataPath.h"

std::unique_ptr<LodReader> pSave_LOD; // LOD pointing to the savegame file currently being processed
std::unique_ptr<LodReader> pGames_LOD; // LOD pointing to data/games.lod

struct FileCloser {
    void operator()(FILE *file) {
        if (file)
            fclose(file);
    }
};


void LOD::File::Close() {
    if (!isFileOpened) {
        return;
    }

    pContainerName.clear();
    pRoot.clear();
    free(pSubIndices);
    pSubIndices = nullptr;
    fclose(pFile);
    isFileOpened = false;
}

int LOD::WriteableFile::CreateNewLod(LOD::FileHeader *pHeader,
                                     const std::string &root_name, const std::string &lod_name) {
    if (isFileOpened) return 1;
    if (root_name.empty()) {
        return 2;
    }
    strcpy(pHeader->pSignature, "LOD");
    pHeader->LODSize = 100;
    pHeader->uNumIndices = 1;

    LOD::Directory dir;
    strcpy(dir.pFilename, root_name.c_str());
    dir.field_F = 0;
    dir.uDataSize = 0;
    dir.uOfsetFromSubindicesStart = sizeof(LOD::FileHeader) + sizeof(LOD::Directory);
    pLODPath = lod_name;

    pFile = fopen(pLODPath.c_str(), "wb+");
    if (!pFile) return 3;
    fwrite(pHeader, sizeof(LOD::FileHeader), 1, pFile);
    fwrite(&dir, sizeof(LOD::Directory), 1, pFile);
    fclose(pFile);
    pFile = nullptr;
    return 0;
}

void LOD::File::ResetSubIndices() {
    if (!isFileOpened) {
        return;
    }

    pContainerName.clear();
    uOffsetToSubIndex = 0;
    free(pSubIndices);
    pSubIndices = nullptr;
}

void LOD::WriteableFile::ResetSubIndices() {
    LOD::File::ResetSubIndices();
    uLODDataSize = 0;
}

bool LOD::WriteableFile::FixDirectoryOffsets() {
    unsigned int total_size = 0;
    for (int i = 0; i < uNumSubDirs; i++) {
        total_size += pSubIndices[i].uDataSize;
    }

    // fix offsets
    int temp_offset = sizeof(LOD::Directory) * uNumSubDirs;
    for (int i = 0; i < uNumSubDirs; i++) {
        pSubIndices[i].uOfsetFromSubindicesStart = temp_offset;
        temp_offset += pSubIndices[i].uDataSize;
    }

    std::string tempPath = pLODPath + ".tmp";
    std::unique_ptr<FILE, FileCloser> tmp_file(fopen(tempPath.c_str(), "wb+"));
    if (tmp_file == nullptr) {
        return 5;
    }

    fwrite((const void *)&header, sizeof(LOD::FileHeader), 1, tmp_file.get());

    LOD::Directory Lindx;
    strcpy(Lindx.pFilename, "chapter");
    Lindx.uOfsetFromSubindicesStart = uOffsetToSubIndex;  // 10h 16
    Lindx.uDataSize =
        sizeof(LOD::Directory) * uNumSubDirs + total_size;  // 14h 20
    Lindx.dword_000018 = 0;                                 // 18h 24
    Lindx.uNumSubIndices = uNumSubDirs;                     // 1ch 28
    Lindx.priority = 0;                                  // 1Eh 30
    fwrite(&Lindx, sizeof(LOD::Directory), 1, tmp_file.get());
    fwrite(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, tmp_file.get());
    fseek(pOutputFileHandle, 0, 0);
    while (total_size > 0) {
        int write_size = uIOBufferSize;
        if (total_size <= uIOBufferSize) {
            write_size = total_size;
        }
        if (fread(pIOBuffer, write_size, 1, pOutputFileHandle) != 1)
            return false;
        fwrite(pIOBuffer, write_size, 1, tmp_file.get());
        total_size -= write_size;
    }

    tmp_file.reset();
    fclose(pOutputFileHandle);
    pOutputFileHandle = nullptr;
    CloseWriteFile();
    std::string tempLODAppPath = pLODPath + ".app.tmp";
    std::filesystem::remove(tempLODAppPath);
    std::filesystem::remove(pLODPath);
    std::filesystem::rename(tempPath, pLODPath);
    CloseWriteFile();

    return LoadFile(pLODPath, 0);
}

bool LOD::WriteableFile::AppendDirectory(const std::string &file_name, const void *pData, size_t data_size) {
    assert(uNumSubDirs < 299);

    LOD::Directory dir;
    strcpy(dir.pFilename, file_name.c_str());
    dir.uDataSize = data_size;

    pSubIndices[uNumSubDirs++] = dir;
    fwrite(pData, 1, dir.uDataSize, pOutputFileHandle);
    return true;
}

int LOD::WriteableFile::CreateTempFile() {
    if (!isFileOpened) return 1;

    if (pIOBuffer && uIOBufferSize) {
        uNumSubDirs = 0;
        std::string tempLODAppPath = pLODPath + ".app.tmp";
        pOutputFileHandle = fopen(tempLODAppPath.c_str(), "wb+");
        return pOutputFileHandle ? 1 : 7;
    } else {
        return 5;
    }
}

void LOD::WriteableFile::CloseWriteFile() {
    if (isFileOpened) {
        pContainerName[0] = 0;

        isFileOpened = false;
        fflush(pFile);
        fclose(pFile);
        pFile = nullptr;
    }
    // else
    // __debugbreak();
}

unsigned int LOD::WriteableFile::Write(const std::string &file_name, const void *pDirData, size_t size, int a4) {
    LOD::Directory dir;
    strcpy(dir.pFilename, file_name.c_str());
    dir.uDataSize = size;

    // insert new data in sorted index lod file
    bool bRewrite_data = false;
    int insert_index = -1;
    if (!isFileOpened) {  // sometimes gives crash
        return 1;
    }
    if (!pSubIndices) {
        return 2;
    }
    if (!pIOBuffer || !uIOBufferSize) {
        return 3;
    }

    for (size_t i = 0; i < uNumSubDirs; i++) {
        if (iequals(pSubIndices[i].pFilename, dir.pFilename)) {
            insert_index = i;
            if (a4 == 0) {
                bRewrite_data = true;
                break;
            }
            if (a4 == 1) {
                if (pSubIndices[i].uNumSubIndices < dir.uNumSubIndices) {
                    if (pSubIndices[i].priority < dir.priority)
                        return 4;
                } else {
                    bRewrite_data = true;
                }
                break;
            }
            if (a4 == 2) return 4;
        } else if (!iless(pSubIndices[i].pFilename, dir.pFilename)) {
            if (insert_index == -1) {
                insert_index = i;
                break;
            }
        }
    }

    int size_correction = 0;
    std::string tempPath = pLODPath + ".tmp";
    std::unique_ptr<FILE, FileCloser> tmp_file(fopen(tempPath.c_str(), "wb+"));
    if (!tmp_file) return 5;
    if (!bRewrite_data)
        size_correction = 0;
    else
        size_correction = pSubIndices[insert_index].uDataSize;

    // create chapter index
    LOD::Directory Lindx;
    strcpy(Lindx.pFilename, "chapter");
    Lindx.dword_000018 = 0;
    Lindx.priority = 0;
    Lindx.uNumSubIndices = uNumSubDirs;
    Lindx.uOfsetFromSubindicesStart = sizeof(LOD::FileHeader) + sizeof(LOD::Directory);
    int total_data_size = uLODDataSize + dir.uDataSize - size_correction;
    if (!bRewrite_data) {
        total_data_size += sizeof(LOD::Directory);
        Lindx.uNumSubIndices++;
    }

    Lindx.uDataSize = total_data_size;
    uNumSubDirs = Lindx.uNumSubIndices;
    // move indexes +1 after insert point
    if (!bRewrite_data &&
        (insert_index < uNumSubDirs)) {  // перезаписывание файлов для освобождения
                                         // места для нового ф-ла
        for (int i = uNumSubDirs; i > insert_index; --i)
            pSubIndices[i] = pSubIndices[i - 1];  // Uninitialized memory access
    }
    // insert
    pSubIndices[insert_index] = dir;  // записать текущий файл
    // correct offsets to data
    if (uNumSubDirs > 0) {
        size_t offset_to_data = sizeof(LOD::Directory) * uNumSubDirs;
        for (int i = 0; i < uNumSubDirs; i++) {
            pSubIndices[i].uOfsetFromSubindicesStart = offset_to_data;
            offset_to_data += pSubIndices[i].uDataSize;
        }
    }

    // construct lod file with added data
    fwrite(&header, sizeof(LOD::FileHeader), 1, tmp_file.get());
    fwrite(&Lindx, sizeof(LOD::Directory), 1, tmp_file.get());
    fseek(pFile, Lindx.uOfsetFromSubindicesStart, SEEK_SET);
    fwrite(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, tmp_file.get());

    size_t offset_to_data = sizeof(LOD::Directory) * uNumSubDirs;
    if (!bRewrite_data) offset_to_data -= sizeof(LOD::Directory);

    fseek(pFile, offset_to_data, SEEK_CUR);
    // copy from open lod to temp lod first half
    int to_copy_size = pSubIndices[insert_index].uOfsetFromSubindicesStart -
                       pSubIndices[0].uOfsetFromSubindicesStart;
    while (to_copy_size > 0) {
        int read_size = uIOBufferSize;
        if (to_copy_size <= uIOBufferSize) read_size = to_copy_size;
        if (fread(pIOBuffer, read_size, 1, pFile) != 1)
            return 1;
        fwrite(pIOBuffer, 1, read_size, tmp_file.get());
        to_copy_size -= read_size;
    }
    // add container data
    fwrite(pDirData, 1, dir.uDataSize, tmp_file.get());  // Uninitialized memory access(tmp_file)
    if (bRewrite_data) fseek(pFile, size_correction, SEEK_CUR);

    // add remainng data  last half
    int curr_position = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    to_copy_size = ftell(pFile) - curr_position;
    fseek(pFile, curr_position, SEEK_SET);
    while (to_copy_size > 0) {
        int read_size = uIOBufferSize;
        if (to_copy_size <= uIOBufferSize) read_size = to_copy_size;
        if (fread(pIOBuffer, read_size, 1, pFile) != 1)
            return 1;
        fwrite(pIOBuffer, 1, read_size, tmp_file.get());
        to_copy_size -= read_size;
    }

    // replace old file by new with added data
    tmp_file.reset();
    CloseWriteFile();
    std::filesystem::remove(pLODPath);
    std::filesystem::rename(tempPath, pLODPath);
    CloseWriteFile();

    // reload new
    LoadFile(pLODPath, 0);  // isFileOpened == true, next file
    return 0;
}

unsigned int LOD::WriteableFile::Write(const std::string &file_name, const Blob &data) {
    return Write(file_name, data.data(), data.size(), 0);
}

LOD::WriteableFile::WriteableFile() {
    pIOBuffer = nullptr;
    uIOBufferSize = 0;
    uLODDataSize = 0;
    pOutputFileHandle = nullptr;
}

bool LOD::WriteableFile::LoadFile(const std::string &filePath, bool bWriting) {
    pFile = fopen(filePath.c_str(), bWriting ? "rb" : "rb+");
    if (pFile == nullptr) {
        return false;  // возможно файл не закрыт, поэтому не открывается
    }

    pLODPath = filePath;
    if (fread(&header, sizeof(LOD::FileHeader), 1, pFile) != 1)
        return false;

    LOD::Directory lod_indx;
    if (fread(&lod_indx, sizeof(LOD::Directory), 1, pFile) != 1)
        return false;

    fseek(pFile, 0, SEEK_SET);
    isFileOpened = true;
    pContainerName = "chapter";
    uLODDataSize = lod_indx.uDataSize;
    uNumSubDirs = lod_indx.uNumSubIndices;
    assert(uNumSubDirs <= 300);

    uOffsetToSubIndex = lod_indx.uOfsetFromSubindicesStart;
    fseek(pFile, uOffsetToSubIndex, SEEK_SET);

    if (uNumSubDirs && fread(pSubIndices, sizeof(LOD::Directory) * uNumSubDirs, 1, pFile) != 1)
        return false;
    return true;
}

void LOD::WriteableFile::AllocSubIndicesAndIO(unsigned int uNumSubIndices,
                                     unsigned int uBufferSize) {
    if (pSubIndices) {
        logger->warning("Attempt to reset a LOD subindex!");
        free(pSubIndices);
        pSubIndices = nullptr;
    }
    pSubIndices = (LOD::Directory *)malloc(sizeof(LOD::Directory) * uNumSubIndices);
    if (pIOBuffer) {
        logger->warning("Attempt to reset a LOD IObuffer!");
        free(pIOBuffer);
        pIOBuffer = nullptr;
        uIOBufferSize = 0;
    }
    if (uBufferSize) {
        pIOBuffer = (uint8_t*)malloc(uBufferSize);
        uIOBufferSize = uBufferSize;
    }
}

void LOD::WriteableFile::FreeSubIndexAndIO() {
    free(pSubIndices);
    pSubIndices = nullptr;
    free(pIOBuffer);
    pIOBuffer = nullptr;
}

LOD::File::File() : isFileOpened(false) {
    pFile = nullptr;
    pSubIndices = nullptr;
    Close();
}

LOD::File::~File() {
    if (isFileOpened) {
        fclose(pFile);
    }
}

bool LOD::File::Open(const std::string &sFilename) {
    if (!OpenFile(sFilename)) {
        return false;
    }

    if (!LoadHeader()) {
        return false;
    }

    return LoadSubIndices(pRoot.front().pFilename);
}

bool LOD::File::OpenFile(const std::string &filePath) {
    if (isFileOpened) {
        Close();
    }

    pFile = fopen(filePath.c_str(), "rb");
    if (pFile == nullptr) {
        return false;
    }

    pLODPath = filePath;

    return true;
}

bool LOD::File::LoadHeader() {
    if (pFile == nullptr) {
        return false;
    }

    fseek(pFile, 0, SEEK_SET);

    if (fread(&header, sizeof(LOD::FileHeader), 1, pFile) != 1) {
        return false;
    }
    for (unsigned int i = 0; i < header.uNumIndices; i++) {
        LOD::Directory dir;
        if (fread(&dir, sizeof(LOD::Directory), 1, pFile) != 1) {
            pRoot.clear();
            return false;
        }
        pRoot.push_back(dir);
    }

    fseek(pFile, 0, SEEK_SET);

    isFileOpened = true;

    return true;
}

bool LOD::File::LoadSubIndices(const std::string &pContainer) {
    ResetSubIndices();

    for (LOD::Directory &dir : pRoot) {
        if (iequals(pContainer, dir.pFilename)) {
            pContainerName = pContainer;
            uOffsetToSubIndex = dir.uOfsetFromSubindicesStart;
            uNumSubDirs = dir.uNumSubIndices;
            fseek(pFile, uOffsetToSubIndex, SEEK_SET);
            pSubIndices = (LOD::Directory *)malloc(sizeof(LOD::Directory) * uNumSubDirs);

            if (pSubIndices) {
                if (fread(pSubIndices, sizeof(LOD::Directory) * uNumSubDirs, 1, pFile) != 1)
                    return false;
            }
            return true;
        }
    }
    return false;
}

LOD::Directory::Directory() {
    memset(pFilename, 0, 16);
    this->uOfsetFromSubindicesStart = 0;
    this->uDataSize = 0;
    this->uNumSubIndices = 0;
    this->dword_000018 = 0;
    this->priority = 0;
}

bool LOD::File::DoesContainerExist(const std::string &pContainer) {
    for (size_t i = 0; i < uNumSubDirs; ++i) {
        if (iequals(pContainer, pSubIndices[i].pFilename)) {
            return true;
        }
    }

    return false;
}

FILE *LOD::File::FindContainer(const std::string &pContainer_Name, size_t *data_size) const {
    if (!isFileOpened) {
        return nullptr;
    }
    if (data_size != nullptr) {
        *data_size = 0;
    }

    for (uint i = 0; i < uNumSubDirs; ++i) {
        if (iequals(pContainer_Name, pSubIndices[i].pFilename)) {
            fseek(pFile, uOffsetToSubIndex + pSubIndices[i].uOfsetFromSubindicesStart, SEEK_SET);
            if (data_size != nullptr) {
                *data_size = pSubIndices[i].uDataSize;
            }
            return pFile;
        }
    }

    return nullptr;
}

Blob LOD::File::LoadRaw(const std::string &pContainer) const {
    size_t size = 0;
    FILE *File = FindContainer(pContainer, &size);
    if (!File) {
        Error("Unable to load %s", pContainer.c_str());
        return Blob();
    }

    return Blob::read(File, size);
}

Blob LOD::File::LoadCompressedTexture(const std::string &pContainer) {
    return lod::decodeCompressed(LoadRaw(pContainer));
}

Blob LOD::File::LoadCompressed(const std::string &pContainer) {
    return lod::decodeCompressed(LoadRaw(pContainer));
}

int LOD::File::GetSubNodeIndex(const std::string &name) const {
    for (size_t index = 0; index < uNumSubDirs; index++) {
        if (name == pSubIndices[index].pFilename) {
            return index;
        }
    }

    return -1;
}

bool Initialize_GamesLOD_NewLOD() {
    pGames_LOD = std::make_unique<LodReader>(makeDataPath("data", "games.lod"));
    pSave_LOD = std::make_unique<LodReader>();
    return true;
}
