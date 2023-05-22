#include "LodReader.h"

#include <algorithm>
#include <cassert>
#include <map>

// TODO(captainurist): errorhandling should be moved to utility/library out of the engine
#include "Engine/ErrorHandling.h"

#include "Library/Compression/Compression.h"
#include "Library/Lod/LodDefinitions.h"

#include "Utility/String.h"
#include "Utility/Streams/MemoryInputStream.h"

static size_t getDirectoryHeaderImgSize(LodVersion lod_version) {
    switch (lod_version) {
    case LOD_VERSION_MM6:
    case LOD_VERSION_MM6_GAME:
    case LOD_VERSION_MM7:
    case LOD_VERSION_MM8:
        return sizeof(LodDirectoryHeader_Mm6);
    }

    Error("Unknown LOD version: %u", lod_version);
    return 0;
}

static bool lodHeaderParseVersion(const LodHeader_Mm6 &header, LodVersion &out_version) {
    static std::map<std::string, LodVersion> versionMap = {
        {"MMVI",     LOD_VERSION_MM6},
        {"GameMMVI", LOD_VERSION_MM6_GAME},
        {"MMVII",    LOD_VERSION_MM7},
        {"MMVIII",   LOD_VERSION_MM8},
    };

    auto it = versionMap.find((const char *)header.version.data());
    if (it != versionMap.end()) {
        out_version = it->second;
        return true;
    }

    return false;
}

bool LodReader::parseDirectoryFiles(size_t numFiles, size_t filesOffset) {
    size_t fileHeaderSize = 0;

    switch (_version) {
      case LOD_VERSION_MM6:
      case LOD_VERSION_MM6_GAME:
      case LOD_VERSION_MM7:
        fileHeaderSize = sizeof(LodFileHeader_Mm6);
        break;

      case LOD_VERSION_MM8:
        fileHeaderSize = sizeof(LodFileHeader_Mm8);
        break;

      default:
        assert(false);
    }

    Blob filesBlob = _lod.subBlob(filesOffset, fileHeaderSize*numFiles);

    if (filesBlob.size() < fileHeaderSize*numFiles) {
        return false;
    }

    MemoryInputStream stream(filesBlob.data(), filesBlob.size());

    for (size_t i = 0; i < numFiles; ++i) {
        FileEntryDesc fileEntry;
        switch (_version) {
            case LOD_VERSION_MM6:
            case LOD_VERSION_MM6_GAME:
            case LOD_VERSION_MM7: {
                LodFileHeader_Mm6 header;
                deserialize(stream, &header);

                fileEntry.name = std::string((char *)header.name.data());
                fileEntry.offset = filesOffset + header.dataOffset;
                fileEntry.size = header.size;
                break;
            }

            case LOD_VERSION_MM8: {
                LodFileHeader_Mm8 header;
                deserialize(stream, &header);

                fileEntry.name = std::string((char *)header.name.data());
                fileEntry.offset = filesOffset + header.dataOffset;
                fileEntry.size = header.dataSize;
                break;
            }
        }

        _files.push_back(fileEntry);
    }

    return true;
}

bool LodReader::parseDirectories(size_t numDirectories, size_t dirOffset) {
    assert(getDirectoryHeaderImgSize(_version) == sizeof(LodDirectoryHeader_Mm6));

    _files.clear();

    if (numDirectories != 1) {
        // While LOD structure itself support multiple directories, all LOD files associated with
        // vanilla MM6/7/8 games use single directory.
        Warn("LOD file have more that one directory, files will be read only from the first one.");
    }

    Blob dirBlob = _lod.subBlob(dirOffset, sizeof(LodDirectoryHeader_Mm6));

    if (dirBlob.size() < sizeof(LodDirectoryHeader_Mm6)) {
        return false;
    }

    MemoryInputStream stream(dirBlob.data(), dirBlob.size());
    LodDirectoryHeader_Mm6 dirHeader;
    deserialize(stream, &dirHeader);

    return parseDirectoryFiles(dirHeader.numFiles, dirHeader.dataOffset);
}

std::unique_ptr<LodReader> LodReader::open(const std::string &filename) {
    std::unique_ptr<LodReader> lod = std::make_unique<LodReader>();

    if (nullptr == lod) {
        Warn("LodReader::open: out of memory loading: %s", filename.c_str());
        return nullptr;
    }

    lod->_lod = Blob::fromFile(filename);
    if (lod->_lod.size() == 0) {
        Warn("LodReader::open: file not found: %s", filename.c_str());
        return nullptr;
    }

    if (lod->_lod.size() < sizeof(LodHeader_Mm6)) {
        Warn("LodReader::open: invalid LOD file: %s", filename.c_str());
        return nullptr;
    }

    MemoryInputStream stream(lod->_lod.data(), lod->_lod.size());
    LodHeader_Mm6 header;
    deserialize(stream, &header);

    if (memcmp(header.signature.data(), "LOD\0", sizeof(header.signature))) {
        Warn("LodReader::open: invalid LOD file: %s", filename.c_str());
        return nullptr;
    }

    if (!lodHeaderParseVersion(header, lod->_version)) {
        Warn("LodReader::open: unknown LOD version: %s", (const char *)header.version.data());
        return nullptr;
    }

    if (!lod->parseDirectories(header.numDirectories, sizeof(LodHeader_Mm6))) {
        Warn("LodReader::open: corrupt directory index: %s", filename.c_str());
        return nullptr;
    }

    lod->_description = std::string((const char *)header.description.data());
    return lod;
}

bool LodReader::exists(const std::string &filename) const {
    return _files.cend() != std::find_if(_files.cbegin(), _files.cend(), [&](const FileEntryDesc &file) { return iequals(file.name, filename); });
}

Blob LodReader::read(const std::string &filename) {
    const auto &file = std::find_if(_files.cbegin(), _files.cend(), [&](const FileEntryDesc &file) { return iequals(file.name, filename); });

    if (_files.cend() == file) {
        Warn("LodReader::read: file not found: %s", filename.c_str());
        return Blob();
    }

    FileCompressionDesc compDesc;
    if (isFileCompressed(*file, &compDesc)) {
        if (0 != compDesc.decompressedSize) {
            return zlib::Uncompress(_lod.subBlob(compDesc.compressedOffs, compDesc.compressedSize), compDesc.decompressedSize);
        } else {
            return _lod.subBlob(compDesc.compressedOffs, compDesc.compressedSize);
        }
    }

    return _lod.subBlob(file->offset, file->size);
}

void LodReader::ls() const {
    for (const FileEntryDesc &desc : _files) {
        printf("\"%s\": 0x%lx:%ld\n", desc.name.c_str(), desc.offset, desc.size);
    }
}

bool LodReader::isFileCompressed(const FileEntryDesc &desc, FileCompressionDesc *compDesc) {
    if (desc.size <= sizeof(LodFileCompressionHeader_Mm6)) {
        return false;
    }

    Blob fileBlob = _lod.subBlob(desc.offset, desc.size);
    MemoryInputStream stream(fileBlob.data(), fileBlob.size());
    LodFileCompressionHeader_Mm6 header;
    deserialize(stream, &header);

    if (header.version == 91969 && !memcmp(header.signature.data(), "mvii", 4)) {
        compDesc->compressedSize = header.compressedSize;
        compDesc->decompressedSize = header.decompressedSize;
        compDesc->compressedOffs = desc.offset + sizeof(LodFileCompressionHeader_Mm6);
        return true;
    }

    return false;
}
