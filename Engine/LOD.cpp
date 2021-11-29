#include <numeric>

#include "Engine/LOD.h"

#include "Engine/Engine.h"
#include "Engine/ZlibWrapper.h"
#include "Engine/Graphics/HWLContainer.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Serialization/LegacyImages.h"

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
LOD::Container* pGames_LOD = nullptr;

int _6A0CA4_lod_binary_search;
int _6A0CA8_lod_unused;


static int _get_file_header_length(LOD_VERSION lod_version) {
    switch (lod_version) {
    case LOD_VERSION_MM6: return sizeof(File_Image_Mm6);
    case LOD_VERSION_MM8: return sizeof(File_Image_Mm8);
    default: Error("Unsupported LOD write format: %d", lod_version);
    }
}


static int _get_directory_write_size(LOD_VERSION lod_version) {
    switch (lod_version) {
    case LOD_VERSION_MM6: return sizeof(Directory_Image_Mm6);
    case LOD_VERSION_MM8: return sizeof(Directory_Image_Mm6);
    default: Error("Unsupported LOD write format: %d", lod_version);
    }
}


static std::vector<LOD::File> _read_directory_files(
    const LOD::Directory& dir,
    FILE *f,
    int num_files,
    LOD_VERSION lod_version
) {
    std::vector<LOD::File> files;

    fseek(f, dir.files_start, SEEK_SET);
    for (int i = 0; i < num_files; ++i) {
        switch (lod_version) {
        case LOD_VERSION_MM6: {
            File_Image_Mm6 mm6;
            Assert(1 == fread(&mm6, sizeof(mm6), 1, f));

            LOD::File file;
            file.name = mm6.name;
            file.offset = mm6.data_offset;
            file.size = mm6.size;
            files.push_back(file);
            continue;
        }
        case LOD_VERSION_MM8: {
            File_Image_Mm8 mm8;
            Assert(1 == fread(&mm8, sizeof(mm8), 1, f));

            LOD::File file;
            file.name = mm8.name;
            file.offset = mm8.unk_12;
            file.size = mm8.unk_13;
            files.push_back(file);
            continue;

        }
        default: Error("Unsupported LOD file version: %d", (int)lod_version);
        }
    }

    Assert(files.size() == num_files);
    return files;
}

static LOD::Directory _read_directory(
    FILE* f,
    LOD_VERSION lod_version
) {
    Directory_Image_Mm6 img;
    fread(&img, sizeof(img), 1, f);

    LOD::Directory dir;
    dir.name = img.pFilename;
    dir.files_start = img.data_offset;
    dir.files = _read_directory_files(dir, f, img.num_items, lod_version);

    return dir;
}


static void _write_file_header(
    FILE* f,
    LOD_VERSION lod_version,
    const LOD::File& file
) {
    switch (lod_version) {
    case LOD_VERSION_MM6: {
        File_Image_Mm6 file_image;
        strcpy_s(file_image.name, file.name.c_str());
        file_image.data_offset = file.offset;
        file_image.size = file.size;
        file_image.dword_000018 = 0;
        file_image.num_items = 0;
        file_image.priority = 0;

        Assert(1 == fwrite(&file_image, sizeof(file_image), 1, f));
        break;
    }
    default: Error("Cannot write LOD file version: %d", (int)lod_version);
    }
}


static void _write_file(
    FILE* f,
    LOD_VERSION lod_version,
    const LOD::File& file,
    size_t file_header_write_ptr,
    const void *file_bytes
) {
    // write header
    fseek(f, file_header_write_ptr, SEEK_SET);
    _write_file_header(f, lod_version, file);

    // write bytes
    fseek(f, file.offset, SEEK_SET);
    fwrite(file_bytes, 1, file.size, f);
}


static void _write_directory_header(
    FILE* f,
    LOD_VERSION lod_version,
    const LOD::Directory& dir
) {
    Directory_Image_Mm6 dir_image;
    strcpy_s(dir_image.pFilename, dir.name.c_str());
    dir_image.data_offset = dir.files_start;
    dir_image.uDataSize = dir.size_in_bytes();
    dir_image.dword_000018 = 0;
    dir_image.num_items = dir.files.size();
    dir_image.priority = 0;
    Assert(1 == fwrite(&dir_image, sizeof(dir_image), 1, f));
}


static void _write_directory(
    FILE* f,
    LOD_VERSION lod_version,
    const LOD::Directory& dir
) {
    _write_directory_header(f, lod_version, dir);

    if (dir.files.size() > 0) {
        fseek(f, dir.files_start, SEEK_SET);
        for (const auto& file : dir.files) {
            _write_file_header(f, lod_version, file);
        }
    }
}

static void _write_directories(
    FILE *f,
    LOD_VERSION lod_version,
    int num_items,
    LOD::Directory *items
) {
    int write_size = _get_directory_write_size(lod_version);
    for (int i = 0; i < num_items; ++i) {
        fwrite(
            items + i,
            write_size,
            1,
            f
        );
    }
}


static std::vector<std::shared_ptr<LOD::Directory>> _read_directories(
    FILE* f,
    LOD_VERSION lod_version,
    int num_expected_directories
) {
    std::vector<std::shared_ptr<LOD::Directory>> dirs;

    int write_size = _get_directory_write_size(lod_version);
    int items_read = 0;

    size_t dir_read_ptr = ftell(f);
    for (int i = 0; i < num_expected_directories; ++i) {
        Directory_Image_Mm6 img;
        fseek(f, dir_read_ptr, SEEK_SET);
        items_read += fread(&img, write_size, 1, f);
        dir_read_ptr += write_size;

        auto dir = std::make_shared<LOD::Directory>();
        dir->name = img.pFilename;
        dir->files_start = img.data_offset;
        dir->files = _read_directory_files(*dir, f, img.num_items, lod_version);

        dirs.push_back(dir);
    }

    Assert(num_expected_directories == items_read);
    return dirs;
}


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

void LODFile_Sprites::_inlined_sub1() {  // final init
    field_ECA0 = uNumLoadedSprites;
}

void LODFile_IconsBitmaps::_inlined_sub0() {
    dword_11B80 = uNumLoadedFiles;
    if (dword_11B84 < uNumLoadedFiles) dword_11B84 = uNumLoadedFiles;
}

void LODFile_Sprites::_inlined_sub0() {  // 2nd init
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
    FILE *File = FindFile(pContainer, 0);
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

    return OpenFolder("sprites08");
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

    FILE *sprite_file = FindFile(pContainerName, 0);
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
    DeleteSpritesRange(field_ECA0, uNumLoadedSprites);
    uNumLoadedSprites = field_ECA0;
}

void LOD::Container::Close() {
    if (!isFileOpened) {
        return;
    }

    _current_folder == nullptr;
    _index.clear();

    fclose(pFile);
    isFileOpened = false;
    _6A0CA8_lod_unused = 0;
}

int LOD::WriteableFile::CreateEmptyLod(
    LOD::FileHeader* pHeader,
    const std::string& root_name,
    const std::string& lod_name
) {
    if (isFileOpened) return 1;
    if (root_name.empty()) {
        return 2;
    }

    pFile = fcaseopen(lod_name.c_str(), "wb+");
    if (!pFile) return 3;

    pLODName = lod_name;

    strcpy(pHeader->pSignature, "LOD");
    pHeader->LODSize = 100;
    pHeader->num_directories = 1;

    LOD::Directory dir;
    dir.name = root_name;
    dir.files_start = sizeof(LOD::FileHeader) + 1 * _get_directory_write_size(GetVersion());

    fwrite(pHeader, sizeof(LOD::FileHeader), 1, pFile);
    _write_directory(pFile, GetVersion(), dir);
    fclose(pFile);
    pFile = nullptr;
    return 0;
}

void LOD::Container::ResetSubIndices() {
    if (!isFileOpened) {
        return;
    }

    _current_folder = nullptr;
}

void LOD::WriteableFile::ResetSubIndices() {
    LOD::Container::ResetSubIndices();
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
    this->sprite_header->Release();
    this->texture->Release();
    this->texture = nullptr;
    this->pName = "null";
    this->uPaletteID = 0;
}

bool LODFile_IconsBitmaps::Load(const std::string& filename, const String& folder) {
    return Open(filename) && OpenFolder(folder);
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

LODFile_Sprites::LODFile_Sprites() : LOD::Container() {
    field_ECA4 = 0;
    field_ECA0 = 0;
    pHardwareSprites = 0;
    // can_load_hardware_sprites = 0;
    field_ECB4 = 0;
    uNumLoadedSprites = 0;
    field_ECA8 = 0;
}

LODFile_IconsBitmaps::~LODFile_IconsBitmaps() {
    for (uint i = 0; i < this->uNumLoadedFiles; i++) {
        this->pTextures[i].Release();
    }
    free(this->pHardwareSurfaces);
    free(this->pHardwareTextures);
    free(this->ptr_011BB4);
}

LODFile_IconsBitmaps::LODFile_IconsBitmaps() : LOD::Container() {
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
    this->pFacesLock = 0;
    this->dword_11B88 = 0;
}

bool LOD::WriteableFile::_4621A7() {  // закрыть и загрузить записываемый ф-л(при
                                    // сохранении)
    CloseWriteFile();
    return LoadFile(pLODName, 0);
}

int LOD::WriteableFile::FixDirectoryOffsets() {
    size_t total_size = _current_folder->size_in_bytes();

    // fix offsets
    Assert(false); // fix this mess
    //int temp_offset = sizeof(LOD::FileHeader)
    //    + _current_folder->files.size() * _get_directory_write_size(GetVersion());
    //for (int i = 0; i < _current_folder_num_items; i++) {
    //    _current_folder_items[i].data_offset = temp_offset;
    //    temp_offset += _current_folder_items[i].uDataSize;
    //}

    /*String Filename = "lod.tmp";
    FILE *tmp_file = fcaseopen(Filename.c_str(), "wb+");
    if (tmp_file == nullptr) {
        return 5;
    }

    fwrite(&_header, sizeof(LOD::FileHeader), 1, tmp_file);

    LOD::Directory chapter;
    chapter.name = "chapter";
    chapter.files_start = _current_folder->files_start;
    //LOD::Directory Lindx;
    //strcpy(Lindx.pFilename, "chapter");
    //Lindx.data_offset = _current_folder_ptr;              // 10h 16
    //Lindx.uDataSize = _current_folder_num_items * _get_directory_write_size(GetVersion())
    //    + total_size;                                     // 14h 20
    //Lindx.dword_000018 = 0;                               // 18h 24
    //Lindx.num_items = _current_folder_num_items;          // 1ch 28
    //Lindx.priority = 0;                                   // 1Eh 30
    //_write_directories(tmp_file, GetVersion(), 1, &Lindx);
    //_write_directories(tmp_file, GetVersion(), _current_folder_num_items, _current_folder_items);
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
    LoadFile(pLODName, false);*/

    return 0;
}

bool LOD::WriteableFile::AppendFileToCurrentDirectory(
    const std::string &file_name,
    const void* file_bytes,
    size_t file_size
) {
    LOD::File file;
    file.name = file_name;
    file.size = file_size;
    file.offset = 0;

    _current_folder->files.push_back(file);

    fwrite(file_bytes, 1, file.size, pOutputFileHandle);
    return true;
}

int LOD::WriteableFile::CreateTempFile() {
    if (!isFileOpened) return 1;

    _current_folder = nullptr;
    pOutputFileHandle = fcaseopen("lodapp.tmp", "wb+");
    return pOutputFileHandle ? 1 : 7;
}

void LOD::WriteableFile::CloseWriteFile() {
    if (isFileOpened) {
        _current_folder = nullptr;
        _6A0CA8_lod_unused = 0;

        isFileOpened = false;
        fflush(pFile);
        fclose(pFile);
        pFile = nullptr;
    }
}

bool LOD::WriteableFile::AddFileToCurrentDirectory(
    const String &file_name,
    const void *file_bytes,
    size_t file_size,
    int _unused
) {
    const char *tmp_filename = "lod.tmp";

    if (!isFileOpened || _current_folder == nullptr) {
        return false;
    }

    FILE* tmp_file = fcaseopen(tmp_filename, "wb+");
    if (!tmp_file) {
        return false;
    }

    LOD::File file;
    file.name = file_name;
    file.size = file_size;

    // insert new data in sorted index lod file
    bool overwriting = false;
    int insert_index = -1;

    // find a file to replace or break when in order alphabetically
    for (size_t i = 0; i < _current_folder->files.size(); i++) {
        int comp_res = _stricmp(
            _current_folder->files[i].name.c_str(),
            file.name.c_str()
        );
        if (comp_res == 0) {
            // replacing a file
            insert_index = i;
            overwriting = true;
            break;
        } else if (comp_res > 0) {
            // past alphabetic order now
            insert_index = i;
            break;
        }
    }

    if (insert_index == -1) {
        // in case we had 0 files to compare to
        insert_index = 0;
    }

    int num_old_files = _current_folder->files.size();
    int num_new_files = num_old_files + (overwriting ? 0 : 1);

    // start offset to write file headers
    size_t file_header_write_ptr = sizeof(LOD::FileHeader)
        + sizeof(Directory_Image_Mm6);

    // start offset to write file data
    size_t file_data_write_ptr = sizeof(LOD::FileHeader)
        + sizeof(Directory_Image_Mm6)
        + num_new_files * _get_file_header_length(GetVersion());

    fwrite(&_header, sizeof(LOD::FileHeader), 1, tmp_file);

    LOD::Directory chapter;
    chapter.name = "chapter";
    chapter.files_start = file_header_write_ptr;
    chapter.files = _current_folder->files;
    _write_directory_header(tmp_file, GetVersion(), chapter);

    // copy all files along with a new one
    for (int i = 0; i < num_old_files; ++i) {
        // write new file outright
        if (i == insert_index) {
            LOD::File file;
            file.name = file_name;
            file.size = file_size;
            file.offset = file_data_write_ptr;

            _write_file(
                tmp_file, GetVersion(), file, file_header_write_ptr, file_bytes
            );

            file_header_write_ptr += _get_file_header_length(GetVersion());
            file_data_write_ptr += file_size;
        }

        // skip the original file if overwritten
        if (i == insert_index && overwriting) {
            continue;
        }

        // write any other file (including original, if not overwriting)
        LOD::File file = _current_folder->files[i];
        file.offset = file_data_write_ptr;

        _write_file(
            tmp_file, GetVersion(), file, file_header_write_ptr, file_bytes
        );

        file_header_write_ptr += _get_file_header_length(GetVersion());
        file_data_write_ptr += file_size;
    }

    fclose(tmp_file);

    // replace old file by new with added data
    CloseWriteFile();
    remove(pLODName.c_str());
    rename(tmp_filename, pLODName.c_str());

    // reload new
    LoadFile(pLODName, 0);  // isFileOpened == true, next file
    return true;
}

LOD::WriteableFile::WriteableFile() {
    uLODDataSize = 0;
    pOutputFileHandle = nullptr;
}

bool LOD::WriteableFile::LoadFile(const std::string& filename, bool bWriting) {
    pFile = fcaseopen(filename.c_str(), bWriting ? "rb" : "rb+");
    if (pFile == nullptr) {
        return false;
    }

    pLODName = filename;
    fread(&_header, sizeof(LOD::FileHeader), 1, pFile);

    auto chapter = std::make_shared<LOD::Directory>();
    *chapter = _read_directory(pFile, GetVersion());

    isFileOpened = true;
    uLODDataSize = chapter->size_in_bytes();

    _current_folder = chapter;
    return true;
}


void LOD::WriteableFile::FreeSubIndexAndIO() {
    _current_folder = nullptr;
}

LOD::Container::Container() : isFileOpened(false) {
    pFile = nullptr;
    _current_folder = nullptr;
    Close();
}

LOD::Container::~Container() {
    if (isFileOpened) {
        fclose(pFile);
    }
}

bool LOD::Container::Open(const std::string& sFilename) {
    if (!OpenFile(sFilename)) {
        return false;
    }

    if (!LoadHeader()) {
        return false;
    }

    return OpenFolder(_index.front()->name);
}

bool LOD::Container::OpenFile(const String &sFilename) {
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

bool LOD::Container::LoadHeader() {
    if (pFile == nullptr) {
        return false;
    }

    fseek(pFile, 0, SEEK_SET);

    if (fread(&_header, sizeof(LOD::FileHeader), 1, pFile) != 1) {
        return false;
    }
    _index = _read_directories(pFile, GetVersion(), _header.num_directories);

    fseek(pFile, 0, SEEK_SET);

    isFileOpened = true;

    return true;
}

bool LOD::Container::OpenFolder(const std::string& folder) {
    if (_current_folder && folder == _current_folder->name) {
        return true;
    }

    ResetSubIndices();

    for (auto& dir : _index) {
        if (_stricmp(folder.c_str(), dir->name.c_str())) {
            continue;
        }

        _current_folder = dir;
        return true;
    }
    return false;
}


bool LOD::Container::FileExists(const std::string& filename) {
    for (const auto& file : _current_folder->files) {
        if (!_stricmp(file.name.c_str(), filename.c_str())) {
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

FILE *LOD::Container::FindFile(const std::string& filename, size_t *out_file_size) {
    if (!isFileOpened) {
        return nullptr;
    }
    if (out_file_size != nullptr) {
        *out_file_size = 0;
    }

    for (const auto& file : _current_folder->files) {
        if (_stricmp(file.name.c_str(), filename.c_str())) {
            continue;
        }

        fseek(pFile, _current_folder->files_start + file.offset, SEEK_SET);
        if (out_file_size != nullptr) {
            *out_file_size = file.size;
        }
        return pFile;
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
                FILE *File = FindFile(this->pTextures[i].header.pName);
                if (File) {
                    TextureHeader_Mm6 DstBuf;
                    fread(&DstBuf, 1, sizeof(TextureHeader_Mm6), File);
                    fseek(File, DstBuf.uTextureSize, 1);
                    fread(this->pTextures[i].pPalette24, 1, 0x300, File);
                }
            }
        }
    }
}

void *LOD::Container::LoadRaw(const String &pContainer, size_t *data_size) {
    if (data_size != nullptr) {
        *data_size = 0;
    }

    size_t size = 0;
    FILE *File = FindFile(pContainer, &size);
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

void *LOD::Container::LoadCompressed2(const std::string& flename, size_t* out_file_size) {
    if (out_file_size != nullptr) {
        *out_file_size = 0;
    }

    FILE *File = FindFile(flename);
    if (!File) {
        Error("Unable to load %s", flename.c_str());
    }

    int compressed_size = 0;
    unsigned int decompressed_size = 0;
    switch (GetVersion()) {
    case LOD_VERSION_MM6: {
        TextureHeader_Mm6 DstBuf;
        fread(&DstBuf, 1, sizeof(TextureHeader_Mm6), File);
        compressed_size = DstBuf.uTextureSize;
        decompressed_size = DstBuf.uDecompressedSize;
        break;
    }
    case LOD_VERSION_MM8: {
        TextureHeader_Mm8 DstBuf;
        fread(&DstBuf, 1, sizeof(TextureHeader_Mm8), File);
        compressed_size = DstBuf.uTextureSize;
        decompressed_size = DstBuf.uDecompressedSize;
        break;
    }
    default: Error("Unsupported LoadCompressed2 version: %d", (int)GetVersion());
    }

    void* result = nullptr;
    if (decompressed_size) {
        result = malloc(decompressed_size);
        void *tmp_buf = malloc(compressed_size);
        fread(tmp_buf, 1, compressed_size, File);
        zlib::Uncompress(result, &decompressed_size, tmp_buf, compressed_size);
        compressed_size = decompressed_size;
        free(tmp_buf);
    } else {
        decompressed_size = compressed_size;
        result = malloc(decompressed_size);
        fread(result, 1, decompressed_size, File);
    }

    if (out_file_size != nullptr) {
        *out_file_size = decompressed_size;
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

void *LOD::Container::LoadCompressed(const String &pContainer, size_t *data_size) {
    static_assert(sizeof(CompressedHeader) == 16, "Wrong type size");

    void *result = nullptr;
    if (data_size != nullptr) {
        *data_size = 0;
    }

    FILE *File = FindFile(pContainer, 0);
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

int LOD::Container::GetSubNodeIndex(const std::string& name) const {
    for (size_t index = 0; index < _current_folder->files.size(); index++) {
        if (name == _current_folder->files[index].name) {
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

    FILE *File = FindFile(pContainer);
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
    FILE *pFile = FindFile(pContainer, &data_size);
    if (pFile == nullptr) {
        return -1;
    }

    TextureHeader_Mm6* header = &pOutTex->header;
    fread(header, 1, sizeof(TextureHeader_Mm6), pFile);
    strncpy(header->pName, pContainer, 16);
    data_size -= sizeof(TextureHeader_Mm6);

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
    pGames_LOD = new LOD::Container();
    if (pGames_LOD->Open(assets_locator->LocateDataFile("games.lod"))) {
        pNew_LOD = new LOD::WriteableFile;
        return true;
    }
    return false;
}
