#pragma once

#include <string_view>

#include "Utility/System/Path.h"

/**
 * Helper class to create a temporary file at the given path with the given contents & remove it when leaving the
 * current scope.
 */
class ScopedTestFile {
 public:
    ScopedTestFile(PathView path, std::string_view contents);
    ~ScopedTestFile();

 private:
    Path _path;
};
