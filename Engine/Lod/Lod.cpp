#include "Engine/Lod/Lod.h"
#include "Engine/Lod/LodUtils.h"

using LOD::Lod;


static shared_ptr<Lod> Lod::Open(const std::string& filename) {
    auto lod = std::make_shared<Lod>();
    if (lod == nullptr) {
        return lod;
    }

    if (!lod->OpenContainerFile(sFilename)) {
        return nullptr;
    }

    if (!lod->ParseHeader()) {
        return false;
    }

    return OpenFolder(_index.front()->name);
}


bool Lod::OpenContainerFile(const std::string& filename) {
    Assert(_file == nullptr);

    _file = fcaseopen(filename.c_str(), "r+b");
    if (_file == nullptr) {
        return false;
    }

    _filename = filename;
    return true;
}


bool Lod::ParseHeader() {
    Assert(_file != nullptr);

    size_t num_expected_directories = 0;
    bool is_lod = _read_header(_file, _version, _description, num_expected_directories);
    if (!is_lod) {
        logger->Warning("%s is not a LOD file", _filename.c_str());
        return false;
    }

    _index = _read_directories(
        _file, _version, num_expected_directories
    );
    if (_index.size() != num_expected_directories) {
        logger->Warning(
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
