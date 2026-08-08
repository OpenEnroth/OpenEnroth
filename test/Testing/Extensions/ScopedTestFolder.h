#pragma once

#include <string_view>
#include <string>

/**
 * Helper class to create a temporary folder at the given path & remove it, with everything that's in it, when leaving
 * the current scope.
 */
class ScopedTestFolder {
 public:
    explicit ScopedTestFolder(std::string_view path);
    ~ScopedTestFolder();

 private:
    std::string _path;
};
