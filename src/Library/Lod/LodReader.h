#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Library/Lod/LodVersion.h"
#include "Utility/Memory/Blob.h"

/**
 * A single stop shop to read LOD files.
 * Even though LODs support a multi-directory structure, in reality vanilla games only ever had a single directory each.
 * 
 * Given that we don't plan to expand the LOD format support, when resolving the files this class always looks
 *      into the first available directory, which is consistent with the vanilla behaviour.
 */
class LodReader final {
 public:
    static std::unique_ptr<LodReader> open(const std::string &filename);

    inline ~LodReader() {}

    bool exists(const std::string &filename) const;
    Blob read(const std::string &filename);

    void ls() const;

 private:
    struct FileEntryDesc {
        std::string name{};
        size_t offset = 0;
        size_t size = 0;
    };

    struct FileCompressionDesc {
        size_t compressedSize;
        size_t decompressedSize;
        size_t compressedOffs;
    };

    bool parseDirectories(size_t numDirectories, size_t dirOffset);
    bool parseDirectoryFiles(size_t numFiles, size_t filesOffset);

    bool isFileCompressed(const FileEntryDesc &desc, FileCompressionDesc *compDesc);

    Blob _lod = Blob();
    LodVersion _version = LOD_VERSION_MM6;
    std::string _description = "";

    std::vector<FileEntryDesc> _files{};
};
