#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "Library/Lod/LodInfo.h"
#include "Library/Magic/MagicEnums.h"
#include "Utility/Memory/Blob.h"

class ArchiveReader {
 public:
    virtual ~ArchiveReader() = default;
    virtual MagicFileFormat format() const = 0;
    virtual std::optional<LodInfo> info() const = 0;
    virtual Blob read(std::string_view filename) const = 0;
    virtual std::vector<std::string> ls() const = 0;

    static std::unique_ptr<ArchiveReader> createArchiveReader(std::string_view path);
};
