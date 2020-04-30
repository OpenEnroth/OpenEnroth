#include "Engine/LOD.h"

#include "Engine/Engine.h"
#include "Engine/ZlibWrapper.h"

#include "Engine/Graphics/HWLContainer.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Sprites.h"

#include "Platform/Api.h"

LODFile_IconsBitmaps *pEvents_LOD = nullptr;

LODFile_IconsBitmaps *pIcons_LOD = nullptr;
LODFile_IconsBitmaps *pIcons_LOD_mm6 = nullptr;
LODFile_IconsBitmaps *pIcons_LOD_mm8 = nullptr;

LODFile_IconsBitmaps *pBitmaps_LOD = nullptr;
LODFile_IconsBitmaps *pBitmaps_LOD_mm6 = nullptr;
LODFile_IconsBitmaps *pBitmaps_LOD_mm8 = nullptr;

LODFile_Sprites *pSprites_LOD = nullptr;
LODFile_Sprites *pSprites_LOD_mm6 = nullptr;
LODFile_Sprites *pSprites_LOD_mm8 = nullptr;

LOD::WriteableFile *pNew_LOD = nullptr;
LOD::File *pGames_LOD = nullptr;

int _6A0CA4_lod_binary_search;
int _6A0CA8_lod_unused;

inline int LODFile_IconsBitmaps::LoadDummyTexture() {
    for (unsigned int i = 0; i < uNumLoadedFiles; ++i)
        if (!strcmp(pTextures[i].header.pName, "pending")) return i;
    return LoadTextureFromLOD(&pTextures[uNumLoadedFiles], "pending",
                              TEXTURE_24BIT_PALETTE);
}

void LODFile_IconsBitmaps::_inlined_sub2() {
    ++uTexturePacksCount;
    if (!uNumPrevLoadedFiles) uNumPrevLoadedFiles = uNumLoadedFiles;
}

void LODFile_IconsBitmaps::_inlined_sub1() {
    dword_11B84 = uNumLoadedFiles;
}

void LODFile_Sprites::_inlined_sub1() {
    field_ECA0 = uNumLoadedSprites;
}

void LODFile_IconsBitmaps::_inlined_sub0() {
    dword_11B80 = uNumLoadedFiles;
    if (dword_11B84 < uNumLoadedFiles) dword_11B84 = uNumLoadedFiles;
}

void LODFile_Sprites::_inlined_sub0() {
    field_ECA4 = uNumLoadedSprites;
    if (field_ECA0 < uNumLoadedSprites) field_ECA0 = uNumLoadedSprites;
}

void LODFile_IconsBitmaps::RemoveTexturesFromTextureList() {
    if (this->uTexturePacksCount) {
        if ((this->uNumLoadedFiles - 1) >= this->uNumPrevLoadedFiles) {
            for (uint i = this->uNumLoadedFiles - 1;
                 i >= this->uNumPrevLoadedFiles; --i) {
                this->pTextures[i].Release();
            }
        }
        this->uNumLoadedFiles = this->uNumPrevLoadedFiles;
        this->uNumPrevLoadedFiles = 0;
        this->uTexturePacksCount = 0;
    }
}

void LODFile_IconsBitmaps::RemoveTexturesPackFromTextureList() {
    if (this->uTexturePacksCount) {
        this->uTexturePacksCount--;
        if (!this->uTexturePacksCount) {
            if ((this->uNumLoadedFiles - 1) >= this->uNumPrevLoadedFiles) {
                for (uint i = this->uNumLoadedFiles - 1;
                     i >= this->uNumPrevLoadedFiles; --i) {
                    this->pTextures[i].Release();
                }
            }
            this->uNumLoadedFiles = this->uNumPrevLoadedFiles;
            this->uNumPrevLoadedFiles = 0;
        }
    }
}

#pragma pack(push, 1)
struct LODSpriteLine {
    int16_t begin;
    int16_t end;
    uint32_t offset;
};
#pragma pack(pop)

int LODFile_Sprites::LoadSpriteFromFile(LODSprite *pSprite, const String &pContainer) {
    FILE *File = FindContainer(pContainer, 0);
    if (File == nullptr) {
        return -1;
    }

    fread(pSprite, sizeof(LODSpriteHeader), 1, File);
    strcpy(pSprite->pName, pContainer.c_str());
    LODSpriteLine *pSpriteLines = new LODSpriteLine[pSprite->uHeight];
    fread(pSpriteLines, sizeof(LODSpriteLine), pSprite->uHeight, File);

    uint8_t *pDecompressedBytes = nullptr;

    int Sizea = pSprite->uSpriteSize;
    if (pSprite->uDecompressedSize) {
        pDecompressedBytes = (uint8_t*)malloc(pSprite->uDecompressedSize);
        void *DstBufa = malloc(Sizea);
        fread(DstBufa, 1, Sizea, File);
        zlib::Uncompress(pDecompressedBytes,
                            (unsigned int *)&pSprite->uDecompressedSize,
                            DstBufa, Sizea);
        pSprite->uSpriteSize = pSprite->uDecompressedSize;
        free(DstBufa);
    } else {
        pDecompressedBytes = (uint8_t*)malloc(Sizea);
        fread(pDecompressedBytes, 1, Sizea, File);
    }

    pSprite->bitmap = new uint8_t[pSprite->uWidth * pSprite->uHeight];
    memset(pSprite->bitmap, 0, pSprite->uWidth * pSprite->uHeight);
    for (uint i = 0; i < pSprite->uHeight; i++) {
        if (pSpriteLines[i].begin >= 0) {
            memcpy(pSprite->bitmap + (i * pSprite->uWidth) + pSpriteLines[i].begin,
                pDecompressedBytes + pSpriteLines[i].offset,
                pSpriteLines[i].end - pSpriteLines[i].begin);
        }
    }

    delete[] pSpriteLines;

    return 1;
}

bool LODFile_Sprites::LoadSprites(const String &pFilename) {
    if (!Open(pFilename)) {
        return false;
    }

    return LoadSubIndices("sprites08");
}

int LODFile_Sprites::LoadSprite(const char *pContainerName, unsigned int uPaletteID) {
    for (int i = 0; i < uNumLoadedSprites; ++i) {
        if (pHardwareSprites[i].pName == pContainerName) {
            return i;
        }
    }

    if (uNumLoadedSprites >= MAX_LOD_SPRITES) return -1;
    // if not loaded - load from file

    if (!pHardwareSprites) {
        pHardwareSprites = new Sprite[MAX_LOD_SPRITES];
    }

    FILE *sprite_file = FindContainer(pContainerName, 0);
    if (!sprite_file) {
        return -1;
    }

    static_assert(sizeof(LODSpriteHeader) == 32, "Wrong type size");

    LODSprite *header = new LODSprite();
    LoadSpriteFromFile(header, pContainerName);  // this line is not present here in the original.
                                                 // necessary for Grayface's mouse picking fix

    // if (uNumLoadedSprites == 879) __debugbreak();

    pHardwareSprites[uNumLoadedSprites].pName = pContainerName;
    pHardwareSprites[uNumLoadedSprites].uBufferWidth = header->uWidth;
    pHardwareSprites[uNumLoadedSprites].uBufferHeight = header->uHeight;
    pHardwareSprites[uNumLoadedSprites].uPaletteID = uPaletteID;
    pHardwareSprites[uNumLoadedSprites].texture = assets->GetSprite(pContainerName, uPaletteID, uNumLoadedSprites);
    pHardwareSprites[uNumLoadedSprites].sprite_header = header;

    HWLTexture *hwl = render->LoadHwlSprite(pContainerName);
    if (hwl) {
        pHardwareSprites[uNumLoadedSprites].uBufferWidth = hwl->uBufferWidth;
        pHardwareSprites[uNumLoadedSprites].uBufferHeight = hwl->uBufferHeight;
        pHardwareSprites[uNumLoadedSprites].uAreaX = hwl->uAreaX;
        pHardwareSprites[uNumLoadedSprites].uAreaY = hwl->uAreaY;
        pHardwareSprites[uNumLoadedSprites].uAreaWidth = hwl->uAreaWidth;
        pHardwareSprites[uNumLoadedSprites].uAreaHeight = hwl->uAreaHeigth;

        delete[] hwl->pPixels;
        delete hwl;
    }

    ++uNumLoadedSprites;
    return uNumLoadedSprites - 1;
}

void LODFile_Sprites::ReleaseLostHardwareSprites() {}

void LODFile_Sprites::ReleaseAll() {}

void LODFile_Sprites::MoveSpritesToVideoMemory() {}

SoftwareBillboard *LODSprite::_4AD2D1_overlays(struct SoftwareBillboard *a2, int a3) {
    SoftwareBillboard *result = a2;
    unsigned int v4 = a2->uTargetPitch;

    uint16_t *pTarget = (uint16_t*)a2->pTarget;
    uint16_t *pPalette = a2->pPalette;
    for (int y = 0; y < uHeight; y++) {
        for (int x = 0; x < uWidth; x++) {
            uint8_t color = bitmap[y * uWidth + x];
            if (color != 0) {
                pTarget[((a2->screen_space_y + y) * a2->uTargetPitch) + a2->screen_space_x + x] = pPalette[color];
            }
        }
    }
    return result;
}

void LODFile_IconsBitmaps::ReleaseAll2() {
    for (uint i = (uint)this->dword_11B84; i < this->uNumLoadedFiles; i++) {
        this->pTextures[i].Release();
    }
    this->uTexturePacksCount = 0;
    this->uNumPrevLoadedFiles = 0;
    this->uNumLoadedFiles = this->dword_11B84;
}

void LODFile_Sprites::DeleteSomeOtherSprites() {
    int *v1 = (int *)&this->uNumLoadedSprites;
    int *v2 = &this->field_ECA0;
    DeleteSpritesRange(field_ECA0, uNumLoadedSprites);

    // testing - do no reset sprite count as we are not clearing sprites
    // see sprite::release below

    *v1 = *v2;
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
    _6A0CA8_lod_unused = 0;
}

int LOD::WriteableFile::CreateNewLod(LOD::FileHeader *pHeader,
                                     const String &root_name, const String &lod_name) {
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
    pLODName = lod_name;

    pFile = fcaseopen(pLODName.c_str(), "wb+");
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

void LODFile_Sprites::DeleteSomeSprites() {
    int *v1 = (int *)&this->uNumLoadedSprites;
    int *v2 = &this->field_ECA8;
    DeleteSpritesRange(this->field_ECA8, this->uNumLoadedSprites);
    *v1 = *v2;
}

void LODFile_Sprites::DeleteSpritesRange(int uStartIndex, int uStopIndex) {
    if (this->pHardwareSprites) {
        if (uStartIndex < uStopIndex) {
            for (int i = uStartIndex; i < uStopIndex; i++) {
                pHardwareSprites[i].Release();
            }
        }
    }
}

void LODSprite::Release() {
    if (!(this->word_1A & 0x400)) {
        delete[] bitmap;
    }

    this->word_1A = 0;
    this->bitmap = nullptr;
    this->pName[0] = 0;
    this->word_16 = 0;
    this->uPaletteId = 0;
    this->uTexturePitch = 0;
    this->uHeight = 0;
    this->uWidth = 0;
    this->uSpriteSize = 0;
}

void Sprite::Release() {
    return;

    // testing actually clearing sprites on release
    // causes error - if sprite is requested multiple times asset manager can delete
    // image which may be referenced elsewhere resulting in MAV

    // bodge for the time being, not resetting loading sprites
    // reduces memory leak on level transition

    this->sprite_header->Release();
    this->texture->Release();
    this->texture = NULL;
    this->pName = "NULL";
    this->uPaletteID = 0;
}

bool LODFile_IconsBitmaps::Load(const String &pLODFilename, const String &pFolderName) {
    if (!Open(pLODFilename)) {
        return false;
    }

    return LoadSubIndices(pFolderName);
}

void LODFile_IconsBitmaps::ReleaseAll() {
    for (uint i = 0; i < this->uNumLoadedFiles; i++) {
        this->pTextures[i].Release();
    }
    this->uTexturePacksCount = 0;
    this->uNumPrevLoadedFiles = 0;
    this->dword_11B84 = 0;
    this->dword_11B80 = 0;
    this->uNumLoadedFiles = 0;
}

unsigned int LODFile_IconsBitmaps::FindTextureByName(const char *pName) {
    for (uint i = 0; i < this->uNumLoadedFiles; i++) {
        if (!_stricmp(this->pTextures[i].header.pName, pName)) return i;
    }
    return -1;
}

void LODFile_IconsBitmaps::SyncLoadedFilesCount() {
    Texture_MM7 *pTex;  // edx@1

    int loaded_files = this->uNumLoadedFiles;
    for (pTex = &this->pTextures[loaded_files]; !pTex->header.pName[0]; --pTex)
        --loaded_files;
    if (loaded_files < (signed int)this->uNumLoadedFiles) {
        ++loaded_files;
        this->uNumLoadedFiles = loaded_files;
    }
}

LODFile_Sprites::~LODFile_Sprites() {
    if (this->pHardwareSprites) {
        for (int i = 0; i < this->uNumLoadedSprites; ++i) {
            this->pHardwareSprites[i].Release();
        }
    }
}

LODSprite::~LODSprite() {
    if (!(this->word_1A & 0x400)) {
        delete[] bitmap;
    }
    bitmap = nullptr;
}

LODFile_Sprites::LODFile_Sprites() : LOD::File() {
    field_ECA4 = 0;
    field_ECA0 = 0;
    pHardwareSprites = 0;
    // can_load_hardware_sprites = 0;
    field_ECB4 = 0;
    uNumLoadedSprites = 0;
}

LODFile_IconsBitmaps::~LODFile_IconsBitmaps() {
    for (uint i = 0; i < this->uNumLoadedFiles; i++) {
        this->pTextures[i].Release();
    }
    free(this->pHardwareSurfaces);
    free(this->pHardwareTextures);
    free(this->ptr_011BB4);
    // LOD::File::vdtor((LOD::File *)v1);
}

LODFile_IconsBitmaps::LODFile_IconsBitmaps() : LOD::File() {
    /*v2 = v1->pTextures;
    v3 = 1000;
    do
    {
      Texture_MM7::Texture_MM7(v2);
      ++v2;
      --v3;
    }
    while ( v3 );*/
    this->uTexturePacksCount = 0;
    this->uNumPrevLoadedFiles = 0;
    this->dword_11B84 = 0;
    this->dword_11B80 = 0;
    this->uNumLoadedFiles = 0;
    this->_011BA4_debug_paletted_pixels_uncompressed = false;
    // this->can_load_hardware_sprites = 0;
    this->pHardwareSurfaces = 0;
    this->pHardwareTextures = 0;
    this->ptr_011BB4 = 0;
    this->uTextureRedBits = 0;
    this->uTextureGreenBits = 0;
    this->uTextureBlueBits = 0;
}

bool LOD::WriteableFile::_4621A7() {  // закрыть и загрузить записываемый ф-л(при
                                    // сохранении)
    CloseWriteFile();
    return LoadFile(pLODName, 0);
}

int LOD::WriteableFile::FixDirectoryOffsets() {
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

    String Filename = "lod.tmp";
    FILE *tmp_file = fcaseopen(Filename.c_str(), "wb+");
    if (tmp_file == nullptr) {
        return 5;
    }

    fwrite((const void *)&header, sizeof(LOD::FileHeader), 1, tmp_file);

    LOD::Directory Lindx;
    strcpy(Lindx.pFilename, "chapter");
    Lindx.uOfsetFromSubindicesStart = uOffsetToSubIndex;  // 10h 16
    Lindx.uDataSize =
        sizeof(LOD::Directory) * uNumSubDirs + total_size;  // 14h 20
    Lindx.dword_000018 = 0;                                 // 18h 24
    Lindx.uNumSubIndices = uNumSubDirs;                     // 1ch 28
    Lindx.priority = 0;                                  // 1Eh 30
    fwrite(&Lindx, sizeof(LOD::Directory), 1, tmp_file);
    fwrite(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, tmp_file);
    fseek(pOutputFileHandle, 0, 0);
    while (total_size > 0) {
        int write_size = uIOBufferSize;
        if (total_size <= uIOBufferSize) {
            write_size = total_size;
        }
        fread(pIOBuffer, 1, write_size, pOutputFileHandle);
        fwrite(pIOBuffer, 1, write_size, tmp_file);
        total_size -= write_size;
    }

    fclose(tmp_file);
    fclose(pOutputFileHandle);
    CloseWriteFile();
    remove("lodapp.tmp");
    remove(pLODName.c_str());
    rename(Filename.c_str(), pLODName.c_str());
    CloseWriteFile();
    LoadFile(pLODName.c_str(), 0);

    return 0;
}

bool LOD::WriteableFile::AppendDirectory(const String &file_name, const void *pData, size_t data_size) {
    Assert(uNumSubDirs < 299);

    LOD::Directory dir;
    strcpy(dir.pFilename, file_name.c_str());
    dir.uDataSize = data_size;

    memcpy(&pSubIndices[uNumSubDirs++], &dir, sizeof(LOD::Directory));
    fwrite(pData, 1, dir.uDataSize, pOutputFileHandle);
    return true;
}

int LOD::WriteableFile::CreateTempFile() {
    if (!isFileOpened) return 1;

    if (pIOBuffer && uIOBufferSize) {
        uNumSubDirs = 0;
        pOutputFileHandle = fcaseopen("lodapp.tmp", "wb+");
        return pOutputFileHandle ? 1 : 7;
    } else {
        return 5;
    }
}

void LOD::WriteableFile::CloseWriteFile() {
    if (isFileOpened) {
        pContainerName[0] = 0;
        _6A0CA8_lod_unused = 0;

        isFileOpened = false;
        fflush(pFile);
        fclose(pFile);
        pFile = nullptr;
    }
    // else
    // __debugbreak();
}

unsigned int LOD::WriteableFile::Write(const String &file_name, const void *pDirData, size_t size, int a4) {
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
        int comp_res = _stricmp(pSubIndices[i].pFilename, dir.pFilename);
        if (comp_res == 0) {
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
        } else if (comp_res > 0) {
            if (insert_index == -1) {
                insert_index = i;
                break;
            }
        }
    }

    int size_correction = 0;
    String Filename = "lod.tmp";
    FILE *tmp_file = fcaseopen(Filename.c_str(), "wb+");
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
            memcpy(&pSubIndices[i], &pSubIndices[i - 1],
                   sizeof(LOD::Directory));  // Uninitialized memory access
    }
    // insert
    memcpy(&pSubIndices[insert_index], &dir, sizeof(LOD::Directory));  // записать текущий файл
    // correct offsets to data
    if (uNumSubDirs > 0) {
        size_t offset_to_data = sizeof(LOD::Directory) * uNumSubDirs;
        for (int i = 0; i < uNumSubDirs; i++) {
            pSubIndices[i].uOfsetFromSubindicesStart = offset_to_data;
            offset_to_data += pSubIndices[i].uDataSize;
        }
    }

    // construct lod file with added data
    fwrite(&header, sizeof(LOD::FileHeader), 1, tmp_file);
    fwrite(&Lindx, sizeof(LOD::Directory), 1, tmp_file);
    fseek(pFile, Lindx.uOfsetFromSubindicesStart, SEEK_SET);
    fwrite(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, tmp_file);

    size_t offset_to_data = sizeof(LOD::Directory) * uNumSubDirs;
    if (!bRewrite_data) offset_to_data -= sizeof(LOD::Directory);

    fseek(pFile, offset_to_data, SEEK_CUR);
    // copy from open lod to temp lod first half
    int to_copy_size = pSubIndices[insert_index].uOfsetFromSubindicesStart -
                       pSubIndices[0].uOfsetFromSubindicesStart;
    while (to_copy_size > 0) {
        int read_size = uIOBufferSize;
        if (to_copy_size <= uIOBufferSize) read_size = to_copy_size;
        fread(pIOBuffer, 1, read_size, pFile);
        fwrite(pIOBuffer, 1, read_size, tmp_file);
        to_copy_size -= read_size;
    }
    // add container data
    fwrite(pDirData, 1, dir.uDataSize, tmp_file);  // Uninitialized memory access(tmp_file)
    if (bRewrite_data) fseek(pFile, size_correction, SEEK_CUR);

    // add remainng data  last half
    int curr_position = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    to_copy_size = ftell(pFile) - curr_position;
    fseek(pFile, curr_position, SEEK_SET);
    while (to_copy_size > 0) {
        int read_size = uIOBufferSize;
        if (to_copy_size <= uIOBufferSize) read_size = to_copy_size;
        fread(pIOBuffer, 1, read_size, pFile);
        fwrite(pIOBuffer, 1, read_size, tmp_file);
        to_copy_size -= read_size;
    }

    // replace old file by new with added data
    fclose(tmp_file);
    CloseWriteFile();
    remove(pLODName.c_str());
    rename(Filename.c_str(), pLODName.c_str());
    CloseWriteFile();

    // reload new
    LoadFile(pLODName, 0);  // isFileOpened == true, next file
    return 0;
}

LOD::WriteableFile::WriteableFile() {
    pIOBuffer = nullptr;
    uIOBufferSize = 0;
}

bool LOD::WriteableFile::LoadFile(const String &pFilename, bool bWriting) {
    pFile = fcaseopen(pFilename.c_str(), bWriting ? "rb" : "rb+");
    if (pFile == nullptr) {
        return false;  // возможно файл не закрыт, поэтому не открывается
    }

    pLODName = pFilename;
    fread(&header, sizeof(LOD::FileHeader), 1, pFile);

    LOD::Directory lod_indx;
    fread(&lod_indx, sizeof(LOD::Directory), 1, pFile);

    fseek(pFile, 0, SEEK_SET);
    isFileOpened = true;
    pContainerName = "chapter";
    uLODDataSize = lod_indx.uDataSize;
    uNumSubDirs = lod_indx.uNumSubIndices;
    Assert(uNumSubDirs <= 300);

    uOffsetToSubIndex = lod_indx.uOfsetFromSubindicesStart;
    fseek(pFile, uOffsetToSubIndex, SEEK_SET);

    fread(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, pFile);
    return true;
}

void LOD::WriteableFile::AllocSubIndicesAndIO(unsigned int uNumSubIndices,
                                     unsigned int uBufferSize) {
    if (pSubIndices) {
        logger->Warning("Attempt to reset a LOD subindex!");
        free(pSubIndices);
        pSubIndices = nullptr;
    }
    pSubIndices = (LOD::Directory *)malloc(sizeof(LOD::Directory) * uNumSubIndices);
    if (pIOBuffer) {
        logger->Warning("Attempt to reset a LOD IObuffer!");
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

bool LOD::File::Open(const String &sFilename) {
    if (!OpenFile(sFilename)) {
        return false;
    }

    if (!LoadHeader()) {
        return false;
    }

    return LoadSubIndices(pRoot.front().pFilename);
}

bool LOD::File::OpenFile(const String &sFilename) {
    if (isFileOpened) {
        Close();
    }

    pFile = fcaseopen(sFilename.c_str(), "rb");
    if (pFile == nullptr) {
        return false;
    }

    pLODName = sFilename;

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

bool LOD::File::LoadSubIndices(const String &pContainer) {
    ResetSubIndices();

    for (LOD::Directory &dir : pRoot) {
        if (!_stricmp(pContainer.c_str(), dir.pFilename)) {
            pContainerName = pContainer;
            uOffsetToSubIndex = dir.uOfsetFromSubindicesStart;
            uNumSubDirs = dir.uNumSubIndices;
            fseek(pFile, uOffsetToSubIndex, SEEK_SET);
            pSubIndices = (LOD::Directory *)malloc(sizeof(LOD::Directory) * uNumSubDirs);

            if (pSubIndices) {
                fread(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, pFile);
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

bool LOD::File::DoesContainerExist(const String &pContainer) {
    for (size_t i = 0; i < uNumSubDirs; ++i) {
        if (!_stricmp(pContainer.c_str(), pSubIndices[i].pFilename)) {
            return true;
        }
    }

    return false;
}

int LODFile_Sprites::_461397() {
    this->field_ECA8 = this->uNumLoadedSprites;
    if (this->uNumLoadedSprites < this->field_ECA0)
        this->field_ECA8 = this->field_ECA0;
    if (this->field_ECA0 < this->field_ECA4) field_ECA0 = this->field_ECA4;
    return this->uNumLoadedSprites;
}

FILE *LOD::File::FindContainer(const String &pContainer_Name, size_t *data_size) {
    if (!isFileOpened) {
        return nullptr;
    }
    if (data_size != nullptr) {
        *data_size = 0;
    }

    for (uint i = 0; i < uNumSubDirs; ++i) {
        if (!_stricmp(pContainer_Name.c_str(), pSubIndices[i].pFilename)) {
            fseek(pFile, uOffsetToSubIndex + pSubIndices[i].uOfsetFromSubindicesStart, SEEK_SET);
            if (data_size != nullptr) {
                *data_size = pSubIndices[i].uDataSize;
            }
            return pFile;
        }
    }

    return nullptr;
}

void LODFile_IconsBitmaps::SetupPalettes(unsigned int uTargetRBits,
                                         unsigned int uTargetGBits,
                                         unsigned int uTargetBBits) {
    if (this->uTextureRedBits != uTargetRBits ||
        this->uTextureGreenBits != uTargetGBits ||
        this->uTextureBlueBits != uTargetBBits) {  // Uninitialized memory access
        this->uTextureRedBits = uTargetRBits;
        this->uTextureGreenBits = uTargetGBits;
        this->uTextureBlueBits = uTargetBBits;
        for (unsigned int i = 0; i < this->uNumLoadedFiles; ++i) {
            if (this->pTextures[i].pPalette24) {
                FILE *File = FindContainer(this->pTextures[i].header.pName);
                if (File) {
                    TextureHeader DstBuf;
                    fread(&DstBuf, 1, sizeof(TextureHeader), File);
                    fseek(File, DstBuf.uTextureSize, 1);
                    fread(this->pTextures[i].pPalette24, 1, 0x300, File);
                }
            }
        }
    }
}

void *LOD::File::LoadRaw(const String &pContainer, size_t *data_size) {
    if (data_size != nullptr) {
        *data_size = 0;
    }

    size_t size = 0;
    FILE *File = FindContainer(pContainer, &size);
    if (!File) {
        Error("Unable to load %s", pContainer.c_str());
        return nullptr;
    }

    void *result = malloc(size);
    if (fread(result, size, 1, File) != 1) {
        free(result);
        result = 0;
        size = 0;
    }

    if (data_size != nullptr) {
        *data_size = size;
    }

    return result;
}

void *LOD::File::LoadCompressedTexture(const String &pContainer, size_t *data_size) {
    void *result = nullptr;
    if (data_size != nullptr) {
        *data_size = 0;
    }

    FILE *File = FindContainer(pContainer, 0);
    if (!File) {
        Error("Unable to load %s", pContainer.c_str());
        return nullptr;
    }

    TextureHeader DstBuf;
    fread(&DstBuf, 1, sizeof(TextureHeader), File);

    if (DstBuf.uDecompressedSize) {
        result = malloc(DstBuf.uDecompressedSize);
        void *tmp_buf = malloc(DstBuf.uTextureSize);
        fread(tmp_buf, 1, DstBuf.uTextureSize, File);
        zlib::Uncompress(result, &DstBuf.uDecompressedSize, tmp_buf,
                         DstBuf.uTextureSize);
        DstBuf.uTextureSize = DstBuf.uDecompressedSize;
        free(tmp_buf);
    } else {
        result = malloc(DstBuf.uTextureSize);
        fread(result, 1, DstBuf.uTextureSize, File);
    }

    if (data_size != nullptr) {
        *data_size = DstBuf.uTextureSize;
    }

    return result;
}

#pragma pack(push, 1)
struct CompressedHeader {
    uint32_t uVersion;
    char pMagic[4];
    uint32_t uCompressedSize;
    uint32_t uDecompressedSize;
};
#pragma pack(pop)

void *LOD::File::LoadCompressed(const String &pContainer, size_t *data_size) {
    static_assert(sizeof(CompressedHeader) == 16, "Wrong type size");

    void *result = nullptr;
    if (data_size != nullptr) {
        *data_size = 0;
    }

    FILE *File = FindContainer(pContainer, 0);
    if (!File) {
        Error("Unable to load %s", pContainer.c_str());
        return nullptr;
    }

    CompressedHeader header;
    fread(&header, 1, sizeof(CompressedHeader), File);
    if (header.uVersion != 91969 || (memcmp(&header.pMagic, "mvii", 4) != 0)) {
        Error("Unable to load %s", pContainer.c_str());
        return nullptr;
    }

    if (header.uDecompressedSize) {
        result = malloc(header.uDecompressedSize);
        void *tmp_buf = malloc(header.uCompressedSize);
        fread(tmp_buf, 1, header.uCompressedSize, File);
        zlib::Uncompress(result, &header.uDecompressedSize, tmp_buf, header.uCompressedSize);
        header.uCompressedSize = header.uDecompressedSize;
        free(tmp_buf);
    } else {
        result = malloc(header.uCompressedSize);
        fread(result, 1, header.uCompressedSize, File);
    }

    if (data_size != nullptr) {
        *data_size = header.uCompressedSize;
    }

    return result;
}

int LOD::File::GetSubNodeIndex(const String &name) const {
    for (size_t index = 0; index < uNumSubDirs; index++) {
        if (name == pSubIndices[index].pFilename) {
            return index;
        }
    }

    return -1;
}

void LODFile_IconsBitmaps::ReleaseHardwareTextures() {}

void LODFile_IconsBitmaps::ReleaseLostHardwareTextures() {}

int LODFile_IconsBitmaps::ReloadTexture(Texture_MM7 *pDst,
                                        const char *pContainer, int mode) {
    unsigned int v7;  // ebx@6
    unsigned int v8;  // ecx@6
    int result;       // eax@7
    // uint8_t v15;      // [sp+11h] [bp-3h]@13
    // uint8_t v16;      // [sp+12h] [bp-2h]@13
    // uint8_t DstBuf;   // [sp+13h] [bp-1h]@13
    void *DstBufa;    // [sp+1Ch] [bp+8h]@10
    void *Sourcea;    // [sp+20h] [bp+Ch]@10

    FILE *File = FindContainer(pContainer);
    if (File == nullptr) {
        return -1;
    }

    Texture_MM7 *v6 = pDst;
    if (pDst->paletted_pixels && mode == 2 && pDst->pPalette24 &&
        (v7 = pDst->header.uTextureSize, fread(pDst, 1, 0x30u, File),
         strcpy(pDst->header.pName, pContainer),
         v8 = pDst->header.uTextureSize, (int)v8 <= (int)v7)) {
        if (!pDst->header.uDecompressedSize ||
            this->_011BA4_debug_paletted_pixels_uncompressed) {
            fread(pDst->paletted_pixels, 1, pDst->header.uTextureSize, File);
        } else {
            Sourcea = malloc(pDst->header.uDecompressedSize);
            DstBufa = malloc(pDst->header.uTextureSize);
            fread(DstBufa, 1, pDst->header.uTextureSize, File);
            zlib::Uncompress(Sourcea, &v6->header.uDecompressedSize, DstBufa,
                             v6->header.uTextureSize);
            v6->header.uTextureSize = pDst->header.uDecompressedSize;
            free(DstBufa);
            memcpy(v6->paletted_pixels, Sourcea,
                   pDst->header.uDecompressedSize);
            free(Sourcea);
        }
        fread(pDst->pPalette24, 1, 0x300, File);
        result = 1;
    } else {
        result = -1;
    }
    return result;
}

int LODFile_IconsBitmaps::LoadTextureFromLOD(Texture_MM7 *pOutTex,
                                             const char *pContainer,
                                             enum TEXTURE_TYPE eTextureType) {
    int result;        // esi@14
    unsigned int v14;  // eax@21
    // size_t v22;        // ST2C_4@29
    // const void *v23;   // ecx@29

    size_t data_size = 0;
    FILE *pFile = FindContainer(pContainer, &data_size);
    if (pFile == nullptr) {
        return -1;
    }

    TextureHeader *header = &pOutTex->header;
    fread(header, 1, sizeof(TextureHeader), pFile);
    strncpy(header->pName, pContainer, 16);
    data_size -= sizeof(TextureHeader);

    // BITMAPS
    if ((header->pBits & 2) && strcmp(header->pName, "sptext01")) {
        if (!pHardwareSurfaces || !pHardwareTextures) {
            pHardwareSurfaces = new IDirectDrawSurface *[1000];
            memset(pHardwareSurfaces, 0, 1000 * sizeof(IDirectDrawSurface *));

            pHardwareTextures = new IDirect3DTexture2 *[1000];
            memset(pHardwareTextures, 0, 1000 * sizeof(IDirect3DTexture2 *));

            ptr_011BB4 = new char[1000];
            memset(ptr_011BB4, 0, 1000);
        }
        if (_strnicmp(pContainer, "wtrdr", 5)) {
            if (_strnicmp(pContainer, "WtrTyl", 6)) {
                v14 = uNumLoadedFiles;
            } else {
                render->hd_water_tile_id = uNumLoadedFiles;
                v14 = uNumLoadedFiles;
            }
            // result = render->LoadTexture(pContainer, pOutTex->palette_id1,
            // (void **)&pHardwareSurfaces[v14], (void
            // **)&pHardwareTextures[v14]);
            result = 1;
        } else {
            char *temp_container;
            temp_container = (char *)malloc(strlen(pContainer) + 2);
            *temp_container = 104;  // 'h'
            strcpy(temp_container + 1, pContainer);
            result = 1;
            free((void *)temp_container);
        }
        return result;
    }

    // ICONS
    if (!header->uDecompressedSize ||
        _011BA4_debug_paletted_pixels_uncompressed) {
        if (header->uTextureSize > data_size) {
            assert(false);
        }
        pOutTex->paletted_pixels = (uint8_t *)malloc(header->uTextureSize);
        fread(pOutTex->paletted_pixels, 1, header->uTextureSize, pFile);
        data_size -= header->uTextureSize;
    } else {
        if (header->uTextureSize > data_size) {
            assert(false);
        }
        pOutTex->paletted_pixels = (uint8_t *)malloc(header->uDecompressedSize);
        void *tmp_buf = malloc(header->uTextureSize);
        fread(tmp_buf, 1, (size_t)header->uTextureSize, pFile);
        data_size -= header->uTextureSize;
        zlib::Uncompress(pOutTex->paletted_pixels, &header->uDecompressedSize,
                         tmp_buf, header->uTextureSize);
        header->uTextureSize = header->uDecompressedSize;
        free(tmp_buf);
    }

    pOutTex->pPalette24 = nullptr;

    if (0x300 > data_size) {
        assert(false);
    }
    pOutTex->pPalette24 = (uint8_t *)malloc(0x300);
    fread(pOutTex->pPalette24, 1, 0x300, pFile);
    data_size -= 0x300;

    assert(data_size == 0);

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

Texture_MM7 *LODFile_IconsBitmaps::LoadTexturePtr(
    const char *pContainer, enum TEXTURE_TYPE uTextureType) {
    uint id = LoadTexture(pContainer, uTextureType);

    Assert(id != -1 && L"Texture_MM7 not found");

    return &pTextures[id];
}

unsigned int LODFile_IconsBitmaps::LoadTexture(const char *pContainer,
                                               enum TEXTURE_TYPE uTextureType) {
    for (uint i = 0; i < uNumLoadedFiles; ++i) {
        if (!_stricmp(pContainer, pTextures[i].header.pName)) {
            return i;
        }
    }

    Assert(uNumLoadedFiles < 1000);

    if (LoadTextureFromLOD(&pTextures[uNumLoadedFiles], pContainer,
                           uTextureType) == -1) {
        for (uint i = 0; i < uNumLoadedFiles; ++i) {
            if (!_stricmp(pTextures[i].header.pName, "pending")) {
                return i;
            }
        }
        LoadTextureFromLOD(&pTextures[uNumLoadedFiles], "pending",
                           uTextureType);
    }

    return uNumLoadedFiles++;
}

Texture_MM7 *LODFile_IconsBitmaps::GetTexture(int idx) {
    Assert(idx < MAX_LOD_TEXTURES, "Texture_MM7 index out of bounds (%u)", idx);
    if (idx == -1) {
        // logger->Warning("Texture_MM7 id = %d missing", idx);
        return pTextures + LoadDummyTexture();
    }
    return pTextures + idx;
}

bool Initialize_GamesLOD_NewLOD() {
    pGames_LOD = new LOD::File();
    if (pGames_LOD->Open(MakeDataPath("data/games.lod"))) {
        pNew_LOD = new LOD::WriteableFile;
        pNew_LOD->AllocSubIndicesAndIO(300, 100000);
        return true;
    }
    return false;
}
