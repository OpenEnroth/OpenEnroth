#pragma once

#include <memory>
#include <string_view>

class FileSystem;

class FileSystemStarter {
 public:
    FileSystemStarter();
    ~FileSystemStarter();

    void initUserFs(bool ramFs, std::string_view path);
    void initDataFs(std::string_view path, bool pathOverridesBuiltIn);

 private:
    std::unique_ptr<FileSystem> _userFs;
    std::unique_ptr<FileSystem> _dataEmbeddedFs;
    std::unique_ptr<FileSystem> _dataDirFs;
    std::unique_ptr<FileSystem> _dataDirLowercaseFs;
    std::unique_ptr<FileSystem> _dataFs;
};
