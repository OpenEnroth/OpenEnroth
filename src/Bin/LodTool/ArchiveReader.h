#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <memory>

#include "Utility/Memory/Blob.h"

class ArchiveReader {
 public:
    virtual ~ArchiveReader() = default;
    virtual Blob read(std::string_view filename) const = 0;
    virtual std::vector<std::string> ls() const = 0;

    /**
     * @param data                      Contents of a LOD, VID or SND archive.
     * @return                          Reader for the archive.
     * @throws Exception                If `data` is not an archive.
     */
    static std::unique_ptr<ArchiveReader> createArchiveReader(Blob data);
};
