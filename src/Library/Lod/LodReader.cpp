#include "LodReader.h"

#include <algorithm>
#include <cassert>
#include <map>

// TODO(captainurist): errorhandling should be moved to utility/library out of the engine
#include "Engine/ErrorHandling.h"
#include "Library/Compression/Compression.h"
#include "Library/Lod/LodDefinitions.h"
#include "Utility/String.h"

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

bool LodReader::parseDirectoryFiles(DirectoryDesc &desc) {
    fseek(_fp, desc.offs, SEEK_SET);
    for (size_t i = 0; i < desc.numFiles; ++i) {
        FileEntryDesc fileEntry;
        std::string dirPrefix = "/";

        if (desc.dirName.size()) {
            dirPrefix += desc.dirName + "/"; 
        }

        switch (_version) {
            case LOD_VERSION_MM6:
            case LOD_VERSION_MM6_GAME:
            case LOD_VERSION_MM7: {
                LodFileHeader_Mm6 header;
                if (!fread(&header, sizeof(header), 1, _fp)) {
                    return false;
                }

                fileEntry.flatName = dirPrefix + std::string((char *)header.name.data());
                fileEntry.offset = desc.offs + header.dataOffset;
                fileEntry.size = header.size;
                break;
            }

            case LOD_VERSION_MM8: {
                LodFileHeader_Mm8 header;
                if (!fread(&header, sizeof(header), 1, _fp)) {
                    return false;
                }

                fileEntry.flatName = dirPrefix + std::string((char *)header.name.data());
                fileEntry.offset = desc.offs + header.dataOffset;
                fileEntry.size = header.dataSize;
                break;
            }
        }

        _files.push_back(fileEntry);
    }

    return true;
}

bool LodReader::parseDirectories(size_t numDirectories) {
    std::vector<DirectoryDesc> dirs;
    size_t dirSize = getDirectoryHeaderImgSize(_version);

    assert(dirSize == sizeof(LodDirectoryHeader_Mm6));

    _files.clear();
    for (size_t i = 0; i < numDirectories; ++i) {
        LodDirectoryHeader_Mm6 head;
        DirectoryDesc desc;

        if (!fread(&head, dirSize, 1, _fp)) {
            return false;
        }

        desc.dirName = std::string((const char *)head.filename.data());
        desc.offs = head.dataOffset;
        desc.numFiles = head.numFiles;

        dirs.push_back(desc);
    }

    for (DirectoryDesc &desc : dirs) {
        if (!parseDirectoryFiles(desc)) {
            return false;
        }
    }

    return true;
}

std::unique_ptr<LodReader> LodReader::open(const std::string &filename, const std::string &defaultDirectory) {
    LodHeader_Mm6 header;
    auto lod = std::make_unique<LodReader>();

    if (nullptr == lod) {
        Warn("LodReader::open: out of memory loading: %s", filename.c_str());
        return nullptr;
    }

    lod->_fp = fopen(filename.c_str(), "rb");
    if (nullptr == lod->_fp) {
        Warn("LodReader::open: file not found: %s", filename.c_str());
        return nullptr;
    }

    if (1 != fread(&header, sizeof(header), 1, lod->_fp)) {
        Warn("LodReader::open: invalid LOD file: %s", filename.c_str());
        return nullptr;
    }

    if (memcmp(header.signature.data(), "LOD\0", sizeof(header.signature))) {
        Warn("LodReader::open: invalid LOD file: %s", filename.c_str());
        return nullptr;
    }

    if (!lodHeaderParseVersion(header, lod->_version)) {
        Warn("LodReader::open: unknown LOD version: %s", (const char *)header.version.data());
        return nullptr;
    }

    if (!lod->parseDirectories(header.numDirectories)) {
        Warn("LodReader::open: corrupt directory index: %s", filename.c_str());
        return nullptr;
    }

    lod->_description = std::string((const char *)header.description.data());
    lod->_pwd = "/";

    lod->cd(defaultDirectory);
    return lod;
}

bool LodReader::exists(const std::string &filename) const {
    std::string checkName = _pwd + filename;

    if (filename[0] == '/') {
        checkName = filename;
    }

    return _files.cend() != std::find_if(_files.cbegin(), _files.cend(), [&](const FileEntryDesc &file) { return iequals(file.flatName, checkName); });
}

Blob LodReader::read(const std::string &filename) {
    std::string checkName = _pwd + filename;

    if (filename[0] == '/') {
        checkName = filename;
    }

    const auto &file = std::find_if(_files.cbegin(), _files.cend(), [&](const FileEntryDesc &file) { return iequals(file.flatName, checkName); });

    if (_files.cend() == file) {
        Warn("LodReader::read: file not found: %s", filename.c_str());
        return Blob();
    }

    fseek(_fp, file->offset, SEEK_SET);
    if (isFileCompressed(*file)) {
        LodFileCompressionHeader_Mm6 header;
        assert(1 == fread(&header, sizeof(header), 1, _fp));

        if (0 != header.decompressedSize) {
            return zlib::Uncompress(Blob::read(_fp, header.compressedSize), header.decompressedSize);
        } else {
            return Blob::read(_fp, header.compressedSize);
        }
    }

    return Blob::read(_fp, file->size);
}

void LodReader::cd(const std::string &filename) {
    if (!filename.size() || filename == ".") {
        return;
    }

    if (filename == ".." && _pwd != "/") {
        size_t pos = _pwd.rfind('/', _pwd.size() - 1);
        _pwd.erase();
        return;
    }

    std::string filenameTmp = filename;
    if (filenameTmp[0] == '/') {
        while (filenameTmp.size() > 1 && filenameTmp[1] == '/') {
            filenameTmp.erase(0, 1);
        }
    } else {
        filenameTmp = _pwd + filenameTmp;
    }

    if (filenameTmp.back() != '/') {
        filenameTmp += "/";
    }

    for (FileEntryDesc &desc : _files) {
        if (!desc.flatName.compare(0, filenameTmp.size(), filenameTmp)) {
            _pwd = filenameTmp;
            return;
        }
    }

    Warn("Cannot cd to '%s', current dir = '%s'", filename.c_str(), _pwd.c_str());
}

void LodReader::ls() const {
    for (const FileEntryDesc &desc : _files) {
        printf("\"%s\": 0x%lx:%ld\n", desc.flatName.c_str(), desc.offset, desc.size);
    }
}

bool LodReader::isFileCompressed(const FileEntryDesc &desc) {
    if (desc.size <= sizeof(LodFileCompressionHeader_Mm6)) {
        return false;
    }

    size_t prev_pos = ftell(_fp);
    fseek(_fp, desc.offset, SEEK_SET);

    LodFileCompressionHeader_Mm6 header;
    assert(1 == fread(&header, sizeof(header), 1, _fp));
    fseek(_fp, prev_pos, SEEK_SET);

    return header.version == 91969 && !memcmp(header.signature.data(), "mvii", 4);
}
