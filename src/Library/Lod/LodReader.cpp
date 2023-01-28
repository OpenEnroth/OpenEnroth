#include "LodReader.h"

#include <cassert>
#include <map>

#include "Engine/ZlibWrapper.h"
#include "Library/Lod/Internal/LodDirectory.h"
#include "Library/Lod/Internal/LodDirectoryHeader.h"
#include "Library/Lod/Internal/LodFile.h"
#include "Library/Lod/Internal/LodFileHeader.h"
#include "Library/Lod/Internal/LodHeader.h"
#include "Utility/String.h"


static inline size_t _get_directory_header_img_size(LodVersion lod_version) {
    switch (lod_version) {
    case LodVersion::Mm6:
    case LodVersion::GameMm6:
    case LodVersion::Mm7:
    case LodVersion::Mm8:
        return sizeof(LodDirectoryHeader_Mm6);
    }

    assert(true && "Unsupported LOD version");
    return 0;
}


static bool _lod_parse_version(const LodHeader_Mm6 &header, LodVersion &out_version) {
    static std::map<std::string, LodVersion> version_map = {
        {"MMVI", LodVersion::Mm6},
        {"GameMMVI", LodVersion::GameMm6},
        {"MMVII", LodVersion::Mm7},
        {"MMVIII", LodVersion::Mm8},
    };

    auto it = version_map.find((const char *)header.version);
    if (it != version_map.end()) {
        out_version = it->second;
        return true;
    }

    //Error("Unknown LOD version: %s", version_string);
    return false;
}


static bool _lod_parse_header(FILE *fp, LodVersion &out_version, std::string &out_description, size_t &out_num_expected_directories) {
    LodHeader_Mm6 header;
    if (1 != fread(&header, sizeof(header), 1, fp)) {
        return false;
    }

    if (header.signature[0] != 'L'
        || header.signature[1] != 'O'
        || header.signature[2] != 'D'
        || header.signature[3] != 0
    ) {
        return false;
    }

    LodVersion version;
    if (!_lod_parse_version(header, version)) {
        return false;
    }

    out_version = version;
    out_description = std::string((const char *)header.description);
    out_num_expected_directories = header.num_directories;
    return true;
}


static inline void _lod_read_directory_files(
    FILE *fp,
    LodVersion version,
    LodDirectory &dir,
    size_t num_expected_files
) {
    dir.files.clear();

    fseek(fp, dir.file_headers_offset, SEEK_SET);
    for (size_t i = 0; i < num_expected_files; ++i) {
        switch (version) {
        case LodVersion::Mm6:
        case LodVersion::GameMm6:
        case LodVersion::Mm7: {
            LodFileHeader_Mm6 header;
            assert(1 == fread(&header, sizeof(header), 1, fp));

            LodFile file;
            file.name = std::string((char *)header.name);
            file.data_offset = dir.file_headers_offset + header.data_offset;
            file.size = header.size;
            dir.files.push_back(file);
            break;
        }

        case LodVersion::Mm8: {
            LodFileHeader_Mm8 header;
            assert(1 == fread(&header, sizeof(header), 1, fp));

            LodFile file;
            file.name = std::string((char *)header.name);
            file.data_offset = dir.file_headers_offset + header.data_offset;
            file.size = header.data_size;
            dir.files.push_back(file);
            break;
        }
        }
    }
}


static bool _lod_read_index(FILE *fp, LodVersion version, size_t num_expected_directories, std::vector<LodDirectory> &out_index) {
    std::vector<LodDirectory> dirs;

    size_t read_size = _get_directory_header_img_size(version);
    size_t items_read = 0;

    size_t dir_read_ptr = ftell(fp);
    for (size_t i = 0; i < num_expected_directories; ++i) {
        LodDirectoryHeader_Mm6 img;
        fseek(fp, dir_read_ptr, SEEK_SET);
        items_read += fread(&img, read_size, 1, fp);
        dir_read_ptr += read_size;

        LodDirectory dir;
        dir.name = (char*)img.filename;
        dir.file_headers_offset = img.data_offset;
        _lod_read_directory_files(fp, version, dir, img.num_items);

        dirs.push_back(dir);
    }

    out_index = dirs;
    return true;
}


std::unique_ptr<LodReader> LodReader::open(const std::string &filename) {
    auto lod = std::make_unique<LodReader>();
    if (nullptr == lod) {
        return nullptr;
    }

    auto fp = fopen(filename.c_str(), "rb");
    if (nullptr == fp) {
        return nullptr;
    }

    size_t num_expected_directories = 0;
    bool is_lod = _lod_parse_header(fp, lod->_version, lod->_description, num_expected_directories);
    if (!is_lod) {
        fclose(fp);
        return nullptr;
    }

    bool is_index_ok = _lod_read_index(fp, lod->_version, num_expected_directories, lod->_index);

    if (is_index_ok) {
        lod->_fp = fp;
        return lod;
    }

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
        return Blob();
    }

    fseek(_fp, file->data_offset, SEEK_SET);
    if (_is_file_compressed(*file)) {
        LodFileCompressionHeader_Mm6 header;
        assert(1 == fread(&header, sizeof(header), 1, _fp));

        if (header.decompressed_size) {
            Blob result = Blob::Allocate(header.compressed_size);
            fread(result.data(), 1, header.compressed_size, _fp);
            return zlib::Uncompress(result, header.decompressed_size);
        } else {
            Blob res = Blob::Allocate(header.compressed_size);
            assert(1 == fread(res.data(), res.size(), 1, _fp));
        }
    }
    
    Blob res = Blob::Allocate(file->size);
    assert(1 == fread(res.data(), res.size(), 1, _fp));

    return res;
}


bool LodReader::_is_file_compressed(const LodFile& file) {
    if (file.size <= sizeof(LodFileCompressionHeader_Mm6)) {
        return false;
    }

    size_t prev_pos = ftell(_fp);
    fseek(_fp, file.data_offset, SEEK_SET);

    LodFileCompressionHeader_Mm6 header;
    assert(1 == fread(&header, sizeof(header), 1, _fp));
    fseek(_fp, prev_pos, SEEK_SET);

    return header.version == 91969 && !memcmp(header.signature, "mvii", 4);
}