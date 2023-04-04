#include "LodReader.h"

#include <algorithm>
#include <cassert>
#include <map>

// TODO(captainurist): errorhandling should be moved to utility/library out of the engine
#include "Engine/ErrorHandling.h"
#include "Library/Compression/Compression.h"
#include "Library/Lod/Internal/LodDirectory.h"
#include "Library/Lod/Internal/LodDirectoryHeader.h"
#include "Library/Lod/Internal/LodFile.h"
#include "Library/Lod/Internal/LodFileHeader.h"
#include "Library/Lod/Internal/LodHeader.h"
#include "Utility/String.h"


static inline size_t _getDirectoryHeaderImgSize(LodVersion lod_version) {
    switch (lod_version) {
    case LOD_VERSION_MM6:
    case LOD_VERSION_MM6_GAME:
    case LOD_VERSION_MM7:
    case LOD_VERSION_MM8:
        return sizeof(LodDirectoryHeader_Mm6);
    }

    Error("Unknown LOD version: %u", lod_version);
}


static bool _lodHeaderParseVersion(const LodHeader_Mm6 &header, LodVersion &out_version) {
    static std::map<std::string, LodVersion> version_map = {
        {"MMVI",     LOD_VERSION_MM6},
        {"GameMMVI", LOD_VERSION_MM6_GAME},
        {"MMVII",    LOD_VERSION_MM7},
        {"MMVIII",   LOD_VERSION_MM8},
    };

    auto it = version_map.find((const char *)header.version.data());
    if (it != version_map.end()) {
        out_version = it->second;
        return true;
    }

    Warn("Unknown LOD version: %s", (const char *)header.version.data());
    return false;
}


static bool _lodParseHeader(FILE *fp, LodVersion &out_version, std::string &out_description, size_t &out_num_expected_directories) {
    LodHeader_Mm6 header;
    if (1 != fread(&header, sizeof(header), 1, fp)) {
        return false;
    }

    if (memcmp(header.signature.data(), "LOD\0", sizeof(header.signature))) {
        Warn("Not a LOD file");
        return false;
    }

    LodVersion version;
    if (!_lodHeaderParseVersion(header, version)) {
        return false;
    }

    out_version = version;
    out_description = std::string((const char *)header.description.data());
    out_num_expected_directories = header.numDirectories;
    return true;
}


static inline void _lodParseDirectoryFiles(
    FILE *fp,
    LodVersion version,
    LodDirectory &dir,
    size_t num_expected_files
) {
    dir.files.clear();

    fseek(fp, dir.fileHeadersOffset, SEEK_SET);
    for (size_t i = 0; i < num_expected_files; ++i) {
        switch (version) {
        case LOD_VERSION_MM6:
        case LOD_VERSION_MM6_GAME:
        case LOD_VERSION_MM7: {
            LodFileHeader_Mm6 header;
            assert(1 == fread(&header, sizeof(header), 1, fp));

            LodFile file;
            file.name = std::string((char *)header.name.data());
            file.dataOffset = dir.fileHeadersOffset + header.dataOffset;
            file.dataSize = header.size;
            dir.files.push_back(file);
            break;
        }

        case LOD_VERSION_MM8: {
            LodFileHeader_Mm8 header;
            assert(1 == fread(&header, sizeof(header), 1, fp));

            LodFile file;
            file.name = std::string((char *)header.name.data());
            file.dataOffset = dir.fileHeadersOffset + header.dataOffset;
            file.dataSize = header.dataSize;
            dir.files.push_back(file);
            break;
        }
        }
    }
}


static bool _lodParseDirectories(FILE *fp, LodVersion version, size_t num_expected_directories, std::vector<LodDirectory> &out_index) {
    std::vector<LodDirectory> dirs;

    size_t read_size = _getDirectoryHeaderImgSize(version);
    size_t items_read = 0;

    size_t dir_read_ptr = ftell(fp);
    for (size_t i = 0; i < num_expected_directories; ++i) {
        LodDirectoryHeader_Mm6 img;
        fseek(fp, dir_read_ptr, SEEK_SET);
        items_read += fread(&img, read_size, 1, fp);
        dir_read_ptr += read_size;

        LodDirectory dir;
        dir.name = std::string((const char *)img.filename.data());
        dir.fileHeadersOffset = img.dataOffset;
        _lodParseDirectoryFiles(fp, version, dir, img.numFiles);

        dirs.push_back(dir);
    }

    out_index = dirs;
    return true;
}


std::unique_ptr<LodReader> LodReader::open(const std::string &filename) {
    auto lod = std::make_unique<LodReader>();
    if (nullptr == lod) {
        Warn("LodReader::open: out of memory loading: %s", filename.c_str());
        return nullptr;
    }

    auto fp = fopen(filename.c_str(), "rb");
    if (nullptr == fp) {
        Warn("LodReader::open: file not found: %s", filename.c_str());
        return nullptr;
    }

    size_t num_expected_directories = 0;
    bool is_lod = _lodParseHeader(fp, lod->_version, lod->_description, num_expected_directories);
    if (!is_lod) {
        Warn("LodReader::open: invalid LOD file: %s", filename.c_str());
        fclose(fp);
        return nullptr;
    }

    bool is_index_ok = _lodParseDirectories(fp, lod->_version, num_expected_directories, lod->_index);
    if (is_index_ok) {
        lod->_fp = fp;
        return lod;
    }

    Warn("LodReader::open: corrupt directory index: %s", filename.c_str());
    fclose(fp);
    return nullptr;
}


bool LodReader::exists(const std::string &filename) const {
    const auto &dir = _index.front(); // only first dir is ever used, no matter names
    return dir.files.cend() != std::find_if(
        dir.files.cbegin(),
        dir.files.cend(),
        [&](const LodFile &file) {return iequals(file.name, filename); }
    );
}


Blob LodReader::read(const std::string &filename) {
    const auto &dir = _index.front(); // only first dir is ever used, no matter names
    const auto &file = std::find_if(
        dir.files.cbegin(),
        dir.files.cend(),
        [&](const LodFile &file) {return iequals(file.name, filename); }
    );

    if (dir.files.cend() == file) {
        Warn("LodReader::read: file not found: %s", filename.c_str());
        return Blob();
    }

    fseek(_fp, file->dataOffset, SEEK_SET);
    if (_isFileCompressed(*file)) {
        LodFileCompressionHeader_Mm6 header;
        assert(1 == fread(&header, sizeof(header), 1, _fp));

        if (0 != header.decompressedSize) {
            return zlib::Uncompress(Blob::read(_fp, header.compressedSize), header.decompressedSize);
        } else {
            return Blob::read(_fp, header.compressedSize);
        }
    }

    return Blob::read(_fp, file->dataSize);
}


bool LodReader::_isFileCompressed(const LodFile &file) {
    if (file.dataSize <= sizeof(LodFileCompressionHeader_Mm6)) {
        return false;
    }

    size_t prev_pos = ftell(_fp);
    fseek(_fp, file.dataOffset, SEEK_SET);

    LodFileCompressionHeader_Mm6 header;
    assert(1 == fread(&header, sizeof(header), 1, _fp));
    fseek(_fp, prev_pos, SEEK_SET);

    return header.version == 91969 && !memcmp(header.signature.data(), "mvii", 4);
}
