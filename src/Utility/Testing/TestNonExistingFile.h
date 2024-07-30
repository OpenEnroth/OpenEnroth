#pragma once

#include <string_view>
#include <string>

/**
 * Helper class to remove a file in ctor and when leaving the current scope.
 */
class TestNonExistingFile {
 public:
    explicit TestNonExistingFile(std::string_view path);
    ~TestNonExistingFile();

 private:
    std::string _path;
};
