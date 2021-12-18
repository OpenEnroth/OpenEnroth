#include <cstdint>
#include <map>

#include "Engine/Lod/Utils.h"

#include "Engine/ErrorHandling.h"
#include "Engine/Serialization/LegacyImages.h"

using namespace Lod;


static inline LOD_VERSION _get_version(const std::string& version_string) {
    static std::map<std::string, LOD_VERSION> version_map = {
        {"MMVI", LOD_VERSION_MM6},
        {"GameMMVI", LOD_VERSION_GAME_MM6},
        {"MMVII", LOD_VERSION_MM7},
        {"MMVIII", LOD_VERSION_MM8},
    };

    auto it = version_map.find(version_string);
    if (it != version_map.end()) {
        return it->second;
    }

    Error("Unknown LOD version: %s", version_string);
}


int _get_directory_header_img_size(LOD_VERSION lod_version) {
    switch (lod_version) {
    case LOD_VERSION_MM6:
    case LOD_VERSION_GAME_MM6:
    case LOD_VERSION_MM7:
    case LOD_VERSION_MM8: return sizeof(LodDirectory_Image_Mm6);
    default: Error("Unsupported LOD write format: %d", (int)lod_version);
    }
}


bool _read_lod_header(
    FILE* f,
    LOD_VERSION& out_lod_version,
    std::string& out_description,
    size_t& out_num_directories
) {
    out_lod_version = (LOD_VERSION)-1;
    out_description = "";
    out_num_directories = 0;

    LodHeader_Image_Mm6 header;
    Assert(1 == fread(&header, sizeof(header), 1, f));

    if (strcmp((char *)header.signature, "LOD")) {
        return false;
    }

    out_lod_version = _get_version((char *)header.version);
    out_description = (char *)header.description;
    out_num_directories = header.num_directories;
    return true;
}


static inline void _read_directory_files(
    FILE* f,
    LOD_VERSION lod_version,
    std::shared_ptr<Directory> dir,
    int num_expected_files
) {
    dir->files.clear();

    fseek(f, dir->file_headers_offset, SEEK_SET);
    for (int i = 0; i < num_expected_files; ++i) {
        switch (lod_version) {
        case LOD_VERSION_MM6:
        case LOD_VERSION_GAME_MM6:
        case LOD_VERSION_MM7: {
            LodFile_Image_Mm6 mm6;
            Assert(1 == fread(&mm6, sizeof(mm6), 1, f));

            File file;
            file.name = (char*)mm6.name;
            file.offset = mm6.data_offset;
            file.size = mm6.size;
            dir->files.push_back(file);
            continue;
        }
        case LOD_VERSION_MM8: {
            LodFile_Image_Mm8 mm8;
            Assert(1 == fread(&mm8, sizeof(mm8), 1, f));

            File file;
            file.name = (char*)mm8.name;
            file.offset = mm8.unk_12;
            file.size = mm8.unk_13;
            dir->files.push_back(file);
            continue;

        }
        default: Error("Unsupported LOD file version: %d", (int)lod_version);
        }
    }

    Assert(dir->files.size() == num_expected_files);
}


std::vector<std::shared_ptr<Directory>> _read_directories(
    FILE* f,
    LOD_VERSION lod_version,
    int num_expected_directories
) {
    std::vector<std::shared_ptr<Directory>> dirs;

    int read_size = _get_directory_header_img_size(lod_version);
    int items_read = 0;

    size_t dir_read_ptr = ftell(f);
    for (int i = 0; i < num_expected_directories; ++i) {
        LodDirectory_Image_Mm6 img;
        fseek(f, dir_read_ptr, SEEK_SET);
        items_read += fread(&img, read_size, 1, f);
        dir_read_ptr += read_size;

        auto dir = std::make_shared<Directory>();
        dir->name = (char *)img.filename;
        dir->file_headers_offset = img.data_offset;
        _read_directory_files(f, lod_version, dir, img.num_items);

        dirs.push_back(dir);
    }

    Assert(num_expected_directories == items_read);
    return dirs;
}
