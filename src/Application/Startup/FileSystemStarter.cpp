#include "FileSystemStarter.h"

#include <memory>
#include <vector>

#include "Library/FileSystem/Directory/DirectoryFileSystem.h"
#include "Library/FileSystem/Embedded/EmbeddedFileSystem.h"
#include "Library/FileSystem/Lowercase/LowercaseFileSystem.h"
#include "Library/FileSystem/Merging/MergingFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"

#include "Engine/EngineFileSystem.h"

CMRC_DECLARE(openenroth);

FileSystemStarter::FileSystemStarter() = default;

FileSystemStarter::~FileSystemStarter() {
    ufs = nullptr;
    dfs = nullptr;
}

void FileSystemStarter::initUserFs(bool ramFs, std::string_view path) {
    assert(ufs == nullptr);

    if (ramFs) {
        _userFs = std::make_unique<MemoryFileSystem>("ramfs");
    } else {
        _userFs = std::make_unique<DirectoryFileSystem>(path);
    }

    ufs = _userFs.get();
}

void FileSystemStarter::initDataFs(std::string_view path) {
    assert(dfs == nullptr);

    _dataEmbeddedFs = std::make_unique<EmbeddedFileSystem>(cmrc::openenroth::get_filesystem(), "embedded");
    _dataDirFs = std::make_unique<DirectoryFileSystem>(path);
    _dataDirLowercaseFs = std::make_unique<LowercaseFileSystem>(_dataDirFs.get());
    _dataFs = std::make_unique<MergingFileSystem>(std::vector<const FileSystem *>({_dataDirLowercaseFs.get(), _dataEmbeddedFs.get()}));

    dfs = _dataFs.get();
}
