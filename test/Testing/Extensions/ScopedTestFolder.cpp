#include "ScopedTestFolder.h"

#include <filesystem>

ScopedTestFolder::ScopedTestFolder(const NativePath &path) : _path(path) {
    std::error_code ec;
    std::filesystem::remove_all(_path.toStdPath(), ec); // Drop whatever an earlier run might have left behind.

    std::filesystem::create_directories(_path.toStdPath());
}

ScopedTestFolder::~ScopedTestFolder() {
    std::error_code ec;
    std::filesystem::remove_all(_path.toStdPath(), ec);
}
