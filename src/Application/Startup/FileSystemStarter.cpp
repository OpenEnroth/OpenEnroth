#include "FileSystemStarter.h"

#include <memory>
#include <vector>
#include <utility>

#include "Library/FileSystem/Embedded/EmbeddedFileSystem.h"
#include "Library/FileSystem/Lowercase/LowercaseFileSystem.h"
#include "Library/FileSystem/Merging/MergingFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Native/NativeFileSystem.h"

#include "Engine/Resources/EngineFileSystem.h"

CMRC_DECLARE(openenroth);

FileSystemStarter::FileSystemStarter() = default;

FileSystemStarter::~FileSystemStarter() {
    ufs = nullptr;
    dfs = nullptr;
}

void FileSystemStarter::initUserFs(bool ramFs, const Path &path) {
    assert(ufs == nullptr);

    if (ramFs) {
        _userFs = std::make_unique<MemoryFileSystem>("ramfs");
    } else {
        _userFs = std::make_unique<NativeFileSystem>(path);
    }

    ufs = _userFs.get();
}

void FileSystemStarter::initDataFs(const Path &path, bool pathOverridesBuiltIn) {
    assert(dfs == nullptr);

    _dataEmbeddedFs = std::make_unique<EmbeddedFileSystem>(cmrc::openenroth::get_filesystem(), "embedded");
    _dataNativeFs = std::make_unique<NativeFileSystem>(path);
    _dataLowercaseFs = std::make_unique<LowercaseFileSystem>(_dataNativeFs.get());

    std::vector<const FileSystem *> baseFileSystems = {_dataLowercaseFs.get(), _dataEmbeddedFs.get()};
    if (!pathOverridesBuiltIn)
        std::swap(baseFileSystems[0], baseFileSystems[1]);
    _dataFs = std::make_unique<MergingFileSystem>(std::move(baseFileSystems));

    dfs = _dataFs.get();
}
