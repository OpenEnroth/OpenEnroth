#pragma once

#include <string>


struct LodFile final {
    std::string name;
    size_t data_offset;
    size_t size;
};
