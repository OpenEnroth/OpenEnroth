#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Engine/Lod/Directory.h"
#include "Engine/Lod/Version.h"


namespace Lod {
bool _read_lod_header(
    FILE* f,
    Version& out_lod_version,
    std::string& out_description,
    size_t& out_num_directories
);


std::vector<std::shared_ptr<Directory>> _read_directories(
    FILE* f,
    Version lod_version,
    int num_expected_directories
);
};  // namespace Lod
