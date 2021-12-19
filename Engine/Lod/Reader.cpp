#include "Engine/Lod/Reader.h"

#include "Engine/ErrorHandling.h"
#include "Engine/IocContainer.h"
#include "Engine/Lod/Utils.h"
#include "Platform/Api.h"

using namespace Lod;


std::shared_ptr<Reader> Reader::Open(const std::string& filename) {
    auto lod = std::make_shared<Reader>();
    if (lod == nullptr) {
        return lod;
    }

    if (!lod->OpenContainerFile(filename)) {
        return nullptr;
    }

    if (!lod->ParseHeader()) {
        return false;
    }

    if (!lod->OpenFolder(lod->_index.front()->name)) {
        return false;
    }

    return lod;
}


bool Reader::OpenContainerFile(const std::string& filename) {
    Assert(_file == nullptr);

    _file = fcaseopen(filename.c_str(), "r+b");
    if (_file == nullptr) {
        return false;
    }

    _filename = filename;
    return true;
}


bool Reader::ParseHeader() {
    Assert(_file != nullptr);

    size_t num_expected_directories = 0;
    bool is_lod = _read_lod_header(_file, _version, _description, num_expected_directories);
    if (!is_lod) {
        _log->Warning("%s is not a LOD file", _filename.c_str());
        return false;
    }

    _index = _read_directories(
        _file, _version, num_expected_directories
    );
    if (_index.size() != num_expected_directories) {
        _log->Warning(
            "%s is corrupt: expected %d directories, got %d",
            _filename.c_str(),
            num_expected_directories,
            _index.size()
        );
        return false;
    }

    fseek(_file, 0, SEEK_SET);
    return true;
}


Reader::Reader() {
    this->_log = Engine_::IocContainer::ResolveLogger();

    Close();
}

Reader::~Reader() {
    if (_file) {
        fclose(_file);
        _file = nullptr;
    }
}


std::shared_ptr<void> Reader::LoadRaw(
    const std::string& filename,
    size_t *out_file_size,
    bool fail_if_missing
) {
    if (out_file_size != nullptr) {
        *out_file_size = 0;
    }

    size_t size = 0;
    FILE *f = FindFile(filename, &size);
    if (!f) {
        if (fail_if_missing) {
            Error("Unable to load %s", filename.c_str());
        } else {
            _log->Warning("Unable to load %s", filename.c_str());
            return nullptr;
        }
    }

    std::shared_ptr<unsigned char> bytes(new unsigned char[size]);
    if (!bytes) {
        _log->Warning("Unable to allocate %zu bytes to load %s", size, filename.c_str());
        return nullptr;
    }

    size_t read = fread(bytes.get(), 1, size, f);
    if (read < size) {
        _log->Warning(
            "Byte count is less than expected when reading %s: %zu/%zu",
            filename.c_str(), read, size
        );
        return nullptr;
    }

    if (out_file_size != nullptr) {
        *out_file_size = size;
    }

    return bytes;
}