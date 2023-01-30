#pragma once

#include <string>


struct LodFile final {
    std::string name;
    size_t dataOffset = 0;
    size_t dataSize = 0;
};
