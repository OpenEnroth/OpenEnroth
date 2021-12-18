#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Engine/Lod/Directory.h"
#include "Engine/Lod/Version.h"


namespace Lod {
int _get_directory_header_img_size(LOD_VERSION lod_version);


bool _read_lod_header(
    FILE* f,
    LOD_VERSION& out_lod_version,
    std::string& out_description,
    size_t& out_num_directories
);


std::vector<std::shared_ptr<Directory>> _read_directories(
    FILE* f,
    LOD_VERSION lod_version,
    int num_expected_directories
);
};  // namespace Lod
