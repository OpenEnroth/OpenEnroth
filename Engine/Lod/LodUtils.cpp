#include "Engine/Lod/LodUtils.h"


#pragma pack(push, 1)
struct LodHeaderImage_Mm6 {
    inline FileHeader() {
        memset(signature, 0, sizeof(signature));
        memset(version, 0, sizeof(version));
        memset(description, 0, sizeof(description));
        memset(array_0000B0, 0, sizeof(array_0000B0));

        size = 0;
        dword_0000A8 = 0;
        num_directories = 0;
    }

    uint8_t signature[4];
    uint8_t version[80];
    uint8_t description[80];
    uint32_t size;
    uint32_t dword_0000A8;
    uint32_t num_directories;
    uint8_t array_0000B0[80];
};
#pragma pack(pop)


static inline LOD::LOD_VERSION _get_version(const std::string& version_string) {
    static std::map<std::string, LOD::LOD_VERSION> version_map = {
        {"MMVI", LOD_VERSION_MM6},
        {"GameMMVI", LOD_VERSION_GAME_MM6},
        {"MMVII", LOD_VERSION_MM7},
        {"MMVIII", LOD_VERSION_MM8},
    };

    auto it = version_map.find(version_string);
    if (it != version_map.end()) {
        return it->second;
    }
    Error("Unknown LOD version: %s", (int)version);
}


bool _read_header(
    FILE* f,
    LOD_VERSION& out_lod_version,
    std::string& out_description,
    size_t& out_num_directories
) {
    out_lod_version = -1;
    out_description = "";
    out_num_directories = 0;

    LodHeaderImage_Mm6 header;
    Assert(1 == fread(&header, sizeof(header), 1, f));

    if (strcmp(header.signature, "LOD")) {
        return false;
    }

    out_lod_version = _get_version(header.version);
    out_description = header.description;
    out_num_directories = header.num_directories;
    return true;
}