#pragma once

#include <string>


namespace Lod {
// A file in a Lod container
class File {
public:
    std::string name;
    size_t offset;
    size_t size;
    void* data;
};
};  // namespace Lod
