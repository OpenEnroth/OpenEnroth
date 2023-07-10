#include "Engine/LOD.h"

#include <filesystem>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/EngineIocContainer.h"

#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Image.h"

#include "Library/Compression/Compression.h"
#include "Library/Logger/Logger.h"

#include "Utility/Memory/FreeDeleter.h"
#include "Utility/String.h"
#include "Utility/DataPath.h"

LODFile_IconsBitmaps *pIcons_LOD = nullptr;
LODFile_IconsBitmaps *pIcons_LOD_mm6 = nullptr;
LODFile_IconsBitmaps *pIcons_LOD_mm8 = nullptr;

LODFile_IconsBitmaps *pBitmaps_LOD = nullptr;
LODFile_IconsBitmaps *pBitmaps_LOD_mm6 = nullptr;
LODFile_IconsBitmaps *pBitmaps_LOD_mm8 = nullptr;

LODFile_Sprites *pSprites_LOD = nullptr;
LODFile_Sprites *pSprites_LOD_mm6 = nullptr;
LODFile_Sprites *pSprites_LOD_mm8 = nullptr;

LOD::WriteableFile *pSave_LOD = nullptr; // LOD pointing to the savegame file currently being processed
LOD::File *pGames_LOD = nullptr; // LOD pointing to data/games.lod

struct FileCloser {
    void operator()(FILE *file) {
        if (file)
            fclose(file);
    }
};

void LODFile_IconsBitmaps::reserveLoadedTextures() {
    _reservedCount = _textures.size();
}

void LODFile_Sprites::reserveLoadedSprites() {  // final init
    _reservedCount = _sprites.size();
}

#pragma pack(push, 1)
struct LODSpriteLine {
    int16_t begin;
    int16_t end;
    uint32_t offset;
};
#pragma pack(pop)

int LODFile_Sprites::LoadSpriteFromFile(LODSprite *pSprite, const std::string &pContainer) {
    FILE *File = FindContainer(pContainer, 0);
    if (File == nullptr) {
        return -1;
    }

    if (fread(pSprite, sizeof(LODSpriteHeader), 1, File) != 1)
        return -1;

    strcpy(pSprite->pName, pContainer.c_str());
    std::unique_ptr<LODSpriteLine[]> pSpriteLines(new LODSpriteLine[pSprite->uHeight]);
    if (fread(pSpriteLines.get(), sizeof(LODSpriteLine) * pSprite->uHeight, 1, File) != 1)
        return -1;

    Blob bytes = Blob::read(File, pSprite->uSpriteSize);
    if (pSprite->uDecompressedSize)
        bytes = zlib::Uncompress(bytes, pSprite->uDecompressedSize);

    pSprite->bitmap = GrayscaleImage::solid(pSprite->uWidth, pSprite->uHeight, 0);
    for (uint i = 0; i < pSprite->uHeight; i++) {
        if (pSpriteLines[i].begin >= 0) {
            memcpy(pSprite->bitmap[i].data() + pSpriteLines[i].begin,
                static_cast<const char *>(bytes.data()) + pSpriteLines[i].offset,
                pSpriteLines[i].end - pSpriteLines[i].begin);
        }
    }

    return 1;
}

bool LODFile_Sprites::open(const std::string &pFilename, const std::string &folder) {
    if (!Open(pFilename)) {
        return false;
    }

    return LoadSubIndices(folder);
}

Sprite *LODFile_Sprites::loadSprite(const std::string &pContainerName) {
    for (Sprite &pSprite : _sprites) {
        if (pSprite.pName == pContainerName) {
            return &pSprite;
        }
    }

    FILE *sprite_file = FindContainer(pContainerName, 0);
    if (!sprite_file) {
        return nullptr;
    }

    static_assert(sizeof(LODSpriteHeader) == 32, "Wrong type size");

    LODSprite *header = new LODSprite();
    LoadSpriteFromFile(header, pContainerName);  // this line is not present here in the original.
                                                 // necessary for GrayFace's mouse picking fix

    // if (uNumLoadedSprites == 879) __debugbreak();

    Sprite &sprite = _sprites.emplace_back();
    sprite.pName = pContainerName;
    sprite.uWidth = header->uWidth;
    sprite.uHeight = header->uHeight;
    sprite.texture = assets->getSprite(pContainerName);
    sprite.sprite_header = header;
    return &sprite;
}

void LODFile_IconsBitmaps::releaseUnreserved() {
    for (size_t i = this->_reservedCount; i < this->_textures.size(); i++) {
        this->_textures[i].Release();
    }
    this->_textures.resize(this->_reservedCount);
}

void LODFile_Sprites::releaseUnreserved() {
    for (size_t i = this->_reservedCount; i < this->_sprites.size(); i++) {
        this->_sprites[i].Release();
    }
    this->_sprites.resize(this->_reservedCount);
}

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

void LODSprite::Release() {
    this->word_1A = 0;
    this->bitmap.reset();
    this->pName[0] = 0;
    this->word_16 = 0;
    this->uPaletteId = 0;
    this->uTexturePitch = 0;
    this->uHeight = 0;
    this->uWidth = 0;
    this->uSpriteSize = 0;
}

void Sprite::Release() {
    this->sprite_header->Release();
    this->texture->Release();
    this->texture = nullptr;
    this->pName = "null";
}

bool LODFile_IconsBitmaps::open(const std::string &pFilename, const std::string &pFolderName) {
    _reader = LodReader::open(pFilename);
    return true;
}

LODFile_Sprites::~LODFile_Sprites() {
    for (size_t i = 0; i < this->_sprites.size(); ++i) {
        this->_sprites[i].Release();
    }
}

LODFile_Sprites::LODFile_Sprites() : LOD::File() {}

LODFile_IconsBitmaps::~LODFile_IconsBitmaps() {
    for (uint i = 0; i < this->_textures.size(); i++) {
        this->_textures[i].Release();
    }
}

LODFile_IconsBitmaps::LODFile_IconsBitmaps() = default;

bool LOD::WriteableFile::_4621A7() {  // закрыть и загрузить записываемый ф-л(при
                                    // сохранении)
    CloseWriteFile();
    return LoadFile(pLODPath, 0);
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
    Assert(uNumSubDirs < 299);

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
    Assert(uNumSubDirs <= 300);

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
    FILE *File = FindContainer(pContainer, 0);
    if (!File) {
        Error("Unable to load %s", pContainer.c_str());
        return Blob();
    }

    TextureHeader DstBuf;
    if (fread(&DstBuf, sizeof(TextureHeader), 1, File) != 1)
        return Blob();

    if (DstBuf.uDecompressedSize) {
        return zlib::Uncompress(Blob::read(File, DstBuf.uTextureSize), DstBuf.uDecompressedSize);
    } else {
        return Blob::read(File, DstBuf.uTextureSize);
    }
}

Blob LOD::File::LoadCompressed(const std::string &pContainer) {
    FILE *File = FindContainer(pContainer, 0);
    if (!File) {
        Error("Unable to load %s", pContainer.c_str());
        return Blob();
    }

    CompressedHeader header;
    if (fread(&header, sizeof(CompressedHeader), 1, File) != 1)
        return Blob();

    if (header.uVersion != 91969 || (memcmp(&header.pMagic, "mvii", 4) != 0)) {
        Error("Unable to load %s", pContainer.c_str());
        return Blob();
    }

    Blob result = Blob::read(File, header.uCompressedSize);
    if (header.uDecompressedSize)
        result = zlib::Uncompress(result, header.uDecompressedSize);
    return result;
}

int LOD::File::GetSubNodeIndex(const std::string &name) const {
    for (size_t index = 0; index < uNumSubDirs; index++) {
        if (name == pSubIndices[index].pFilename) {
            return index;
        }
    }

    return -1;
}

int LODFile_IconsBitmaps::LoadTextureFromLOD(Texture_MM7 *pOutTex, const std::string &pContainer) {
    if (!_reader->exists(pContainer))
        return -1;

    BlobInputStream input(_reader->read(pContainer));

    TextureHeader *header = &pOutTex->header;
    input.readOrFail(header, sizeof(TextureHeader));

    strncpy(header->pName.data(), pContainer.c_str(), 16);

    // ICONS
    if (!header->uDecompressedSize) {
        pOutTex->paletted_pixels = (uint8_t *)malloc(header->uTextureSize);
        if (header->uTextureSize)
            input.readOrFail(pOutTex->paletted_pixels, header->uTextureSize);
    } else {
        // TODO(captainurist): just store Blob in pOutTex
        Blob pixels = zlib::Uncompress(input.readBlobOrFail(header->uTextureSize), header->uDecompressedSize);
        pOutTex->paletted_pixels = (uint8_t *)malloc(pixels.size());
        memcpy(pOutTex->paletted_pixels, pixels.data(), pixels.size());
        header->uTextureSize = pixels.size();
    }

    pOutTex->pPalette24 = (uint8_t *)malloc(0x300);
    input.readOrFail(pOutTex->pPalette24, 0x300);

    if (header->pBits & 2) {
        pOutTex->pLevelOfDetail1 =
            &pOutTex->paletted_pixels[header->uSizeOfMaxLevelOfDetail];
        // v8->pLevelOfDetail2 =
        // &v8->pLevelOfDetail1[v8->uSizeOfMaxLevelOfDetail >> 2];
        // v8->pLevelOfDetail3 =
        // &v8->pLevelOfDetail2[v8->uSizeOfMaxLevelOfDetail >> 4];
    } else {
        pOutTex->pLevelOfDetail1 = 0;
        // v8->pLevelOfDetail2 = 0;
        // v8->pLevelOfDetail3 = 0;
    }

    for (int v41 = 1; v41 < 15; ++v41) {
        if (1 << v41 == header->uTextureWidth) header->uWidthLn2 = v41;
    }
    for (int v42 = 1; v42 < 15; ++v42) {
        if (1 << v42 == header->uTextureHeight) header->uHeightLn2 = v42;
    }

    header->uWidthMinus1 = (1 << header->uWidthLn2) - 1;
    header->uHeightMinus1 = (1 << header->uHeightLn2) - 1;

    return 1;
}

Texture_MM7 *LODFile_IconsBitmaps::loadTexture(const std::string &pContainer, bool useDummyOnError) {
    for (Texture_MM7 &pTexture : _textures) {
        if (iequals(pContainer.data(), pTexture.header.pName.data())) {
            return &pTexture;
        }
    }

    if (LoadTextureFromLOD(&_textures.emplace_back(), pContainer) != -1)
        return &_textures.back();
    _textures.pop_back();

    if (!useDummyOnError)
        return nullptr;

    for (Texture_MM7 &pTexture : _textures) {
        if (iequals(pTexture.header.pName.data(), "pending")) {
            return &pTexture;
        }
    }

    if (LoadTextureFromLOD(&_textures.emplace_back(), "pending") != -1)
        return &_textures.back();
    _textures.pop_back();

    return nullptr;
}

Blob LODFile_IconsBitmaps::LoadCompressedTexture(const std::string &pContainer) {
    BlobInputStream input(_reader->read(pContainer));

    TextureHeader DstBuf;
    input.readOrFail(&DstBuf, sizeof(TextureHeader));

    if (DstBuf.uDecompressedSize) {
        return zlib::Uncompress(input.readBlobOrFail(DstBuf.uTextureSize), DstBuf.uDecompressedSize);
    } else {
        return input.readBlobOrFail(DstBuf.uTextureSize);
    }
}

bool Initialize_GamesLOD_NewLOD() {
    pGames_LOD = new LOD::File();
    if (pGames_LOD->Open(makeDataPath("data", "games.lod"))) {
        pSave_LOD = new LOD::WriteableFile;
        pSave_LOD->AllocSubIndicesAndIO(300, 100000);
        return true;
    }
    return false;
}
