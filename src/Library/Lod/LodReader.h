#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Library/Lod/LodVersion.h"
#include "Library/Lod/Internal/LodDirectory.h"
#include "Library/Lod/Internal/LodFile.h"
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

    inline ~LodReader() {
        if (nullptr != _fp) {
            fclose(_fp);
        }
    }

    bool exists(const std::string &filename) const;
    Blob read(const std::string &filename);

    const auto begin() const {
        return _index.front().files.cbegin();
    }

    const auto end() const {
        return _index.front().files.cend();
    }

 private:
    bool _isFileCompressed(const LodFile &file);

#ifdef _DEBUG
    std::string _filename;
#endif
    FILE *_fp;
    LodVersion _version;
    std::string _description;
    std::vector<LodDirectory> _index;
};
