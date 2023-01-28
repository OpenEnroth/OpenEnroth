#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Library/Lod/LodVersion.h"
#include "Library/Lod/Internal/LodDirectory.h"
#include "Library/Lod/Internal/LodFile.h"
#include "Utility/Memory/Blob.h"


class LodReader final {
public:
    static std::unique_ptr<LodReader> open(const std::string &filename);

    bool exists(const std::string &filename) const;
    Blob read(const std::string &filename);

private:
    bool _is_file_compressed(const LodFile& file);

    FILE *_fp;
    LodVersion _version;
    std::string _description;
    std::vector<LodDirectory> _index;
};
