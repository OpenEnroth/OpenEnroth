#pragma once

#include <string_view>

#include "Utility/System/NativePath.h"

/**
 * Helper class to create a temporary file at the given path with the given contents & remove it when leaving the
 * current scope.
 */
class ScopedTestFile {
 public:
    ScopedTestFile(std::string_view path, std::string_view contents) : ScopedTestFile(NativePath::fromWtf8(path), contents) {}
    ScopedTestFile(const NativePath &path, std::string_view contents);
    ~ScopedTestFile();

 private:
    NativePath _path;
};
