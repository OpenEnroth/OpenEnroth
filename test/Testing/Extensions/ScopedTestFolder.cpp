#include "ScopedTestFolder.h"

#include <gtest/gtest.h>

#include <filesystem>

ScopedTestFolder::ScopedTestFolder(const NativePath &path) : _path(path) {
    std::error_code ec;
    std::filesystem::remove_all(_path.toStdPath(), ec); // An earlier run could have left the folder behind.
    std::filesystem::create_directories(_path.toStdPath());

    EXPECT_TRUE(std::filesystem::exists(_path.toStdPath()));
}

ScopedTestFolder::~ScopedTestFolder() {
    std::error_code ec;
    std::filesystem::remove_all(_path.toStdPath(), ec);

    EXPECT_FALSE(std::filesystem::exists(_path.toStdPath())); // A folder left behind poisons the next run.
}
