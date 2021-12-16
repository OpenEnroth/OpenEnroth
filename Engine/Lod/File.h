#pragma once

#include <string>


namespace LOD {
class File {
    std::string name;
    size_t offset;
    size_t size;
    void* data;
};
};  // namespace LOD
