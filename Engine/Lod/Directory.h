#pragma once

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "Engine/Lod/File.h"


namespace LOD {
class Directory {
    std::string name;
    std::vector<File> files;
    size_t file_headers_offset = 0;

    // size of all files in the directory
    inline size_t size_in_bytes() const {
        return std::accumulate(
            files.begin(),
            files.end(),
            size_t{ 0 },
            [](const size_t accumulated_size, const File& file) {
                return accumulated_size + file.size;
            }
        );
    }

    // sort files by name alphanumerically
    inline void sort_files() {
        std::sort(
            files.begin(),
            files.end(),
            [](const File& a, const File& b) {
                return 0 > _stricmp(
                    a.name.c_str(),
                    b.name.c_str()
                );
            }
        );
    }

    // recalculate file data offsets in the LOD file where this directory belongs
    inline void recalculate_offsets(size_t files_write_ptr) {
        files_write_ptr -= file_headers_offset; // not sure why it's done this way
        for (auto& file : files) {
            file.offset = files_write_ptr;
            files_write_ptr += file.size;
        }
    }
};
};  // namespace LOD
