#pragma once

#include <string_view>
#include <string>

/**
 * Helper class to create a temporary file at the given path with the given contents & remove it when leaving the
 * current scope.
 */
class ScopedTestFile {
 public:
    ScopedTestFile(std::string_view path, std::string_view contents);
    ~ScopedTestFile();

 private:
    std::string _path;
};
