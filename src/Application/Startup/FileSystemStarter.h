#pragma once

#include <memory>

#include "Utility/System/Path.h"

class FileSystem;

class FileSystemStarter {
 public:
    FileSystemStarter();
    ~FileSystemStarter();

    void initUserFs(bool ramFs, const Path &path);
    void initDataFs(const Path &path, bool pathOverridesBuiltIn);

 private:
    std::unique_ptr<FileSystem> _userFs;
    std::unique_ptr<FileSystem> _dataEmbeddedFs;
    std::unique_ptr<FileSystem> _dataNativeFs;
    std::unique_ptr<FileSystem> _dataLowercaseFs;
    std::unique_ptr<FileSystem> _dataFs;
};
