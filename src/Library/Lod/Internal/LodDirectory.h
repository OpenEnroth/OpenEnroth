#pragma once

#include <string>
#include <vector>

#include "Library/Lod/Internal/LodFile.h"


struct LodDirectory final {
    std::string name;
    std::vector<LodFile> files;

    size_t file_headers_offset = 0;
};
