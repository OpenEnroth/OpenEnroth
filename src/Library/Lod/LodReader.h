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
    static std::shared_ptr<LodReader> open(const std::string &filename, const std::string &defaultDirectory = ".");

    inline ~LodReader() {
        if (nullptr != _fp) {
            fclose(_fp);
        }
    }

    bool exists(const std::string &filename) const;
    Blob read(const std::string &filename);

    void cd(const std::string &filename);
    void ls() const;

 private:
    struct FileEntryDesc {
        std::string flatName{};
        size_t offset = 0;
        size_t size = 0;
    };

    struct DirectoryDesc {
        std::string dirName{};
        size_t offs{};
        int numFiles{};
    };

    bool parseDirectories(size_t numDirectories);
    bool parseDirectoryFiles(DirectoryDesc &desc);

    bool isFileCompressed(const FileEntryDesc &desc);

    FILE *_fp = nullptr;
    LodVersion _version = LOD_VERSION_MM6;
    std::string _description = "";
    std::string _pwd = "";

    std::vector<FileEntryDesc> _files{};
};
