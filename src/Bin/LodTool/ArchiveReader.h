#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <memory>

#include "Utility/Memory/Blob.h"
#include "Utility/System/Path.h"

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

    /**
     * @param path                      Path to a file, or to an entry inside an archive, as in `"games.lod/d.odm"`.
     * @return                          Contents of the file or the archive entry, as stored.
     * @throws Exception                If there is no such file or entry.
     */
    static Blob readFile(PathView path);
};
