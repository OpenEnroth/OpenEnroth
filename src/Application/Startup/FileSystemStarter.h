#pragma once

#include <memory>

#include "Utility/System/NativePath.h"

class FileSystem;

class FileSystemStarter {
 public:
    FileSystemStarter();
    ~FileSystemStarter();

    void initUserFs(bool ramFs, const NativePath &path);
    void initDataFs(const NativePath &path, bool pathOverridesBuiltIn);

 private:
    std::unique_ptr<FileSystem> _userFs;
    std::unique_ptr<FileSystem> _dataEmbeddedFs;
    std::unique_ptr<FileSystem> _dataDirFs;
    std::unique_ptr<FileSystem> _dataDirLowercaseFs;
    std::unique_ptr<FileSystem> _dataFs;
};
