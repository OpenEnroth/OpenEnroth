#pragma once

#include <string>

#include "Engine/Lod/Version.h"


bool _read_header(
    FILE* f,
    LOD::LOD_VERSION& out_lod_version,
    std::string& out_description,
    size_t& out_num_directories
);