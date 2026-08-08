#include "ScopedTestFolder.h"

#include <filesystem>

ScopedTestFolder::ScopedTestFolder(std::string_view path) : _path(path) {
    std::error_code ec;
    std::filesystem::remove_all(_path, ec); // Drop whatever an earlier run might have left behind.

    std::filesystem::create_directories(_path);
}

ScopedTestFolder::~ScopedTestFolder() {
    std::error_code ec;
    std::filesystem::remove_all(_path, ec);
}
