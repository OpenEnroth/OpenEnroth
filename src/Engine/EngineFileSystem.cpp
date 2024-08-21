#include "EngineFileSystem.h"

#include <memory>
#include <vector>

#include "Library/FileSystem/Directory/DirectoryFileSystem.h"
#include "Library/FileSystem/Embedded/EmbeddedFileSystem.h"
#include "Library/FileSystem/Lowercase/LowercaseFileSystem.h"
#include "Library/FileSystem/Merging/MergingFileSystem.h"
#include "Library/FileSystem/Masking/MaskingFileSystem.h"

ProxyFileSystem *dfs = nullptr;
ProxyFileSystem *ufs = nullptr;

CMRC_DECLARE(openenroth);

EngineFileSystem::EngineFileSystem(std::string_view dataPath, std::string_view userPath) {
    _dataEmbeddedFs = std::make_unique<EmbeddedFileSystem>(cmrc::openenroth::get_filesystem(), "embedded");
    _dataDirFs = std::make_unique<DirectoryFileSystem>(dataPath);
    _dataDirLowercaseFs = std::make_unique<LowercaseFileSystem>(_dataDirFs.get());
    _defaultDataFs = std::make_unique<MergingFileSystem>(std::vector<const FileSystem *>({_dataDirLowercaseFs.get(), _dataEmbeddedFs.get()}));
    _dataFs = std::make_unique<ProxyFileSystem>(_defaultDataFs.get());

    _defaultUserFs = std::make_unique<DirectoryFileSystem>(userPath);
    _userFs = std::make_unique<ProxyFileSystem>(_defaultUserFs.get());

    assert(dfs == nullptr && ufs == nullptr);
    dfs = _dataFs.get();
    ufs = _userFs.get();
}

EngineFileSystem::~EngineFileSystem() {
    assert(dfs == _dataFs.get() && ufs == _userFs.get());
    dfs = nullptr;
    ufs = nullptr;
}

ProxyFileSystem *EngineFileSystem::dataFs() {
    return _dataFs.get();
}

ProxyFileSystem *EngineFileSystem::userFs() {
    return _userFs.get();
}

FileSystem *EngineFileSystem::defaultDataFs() {
    return _defaultDataFs.get();
}

FileSystem *EngineFileSystem::defaultUserFs() {
    return _defaultUserFs.get();
}
