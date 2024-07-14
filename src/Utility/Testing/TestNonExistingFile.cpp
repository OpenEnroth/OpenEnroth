#include "TestNonExistingFile.h"

#include <filesystem>

TestNonExistingFile::TestNonExistingFile(std::string_view path) : _path(path) {
    std::error_code ec;
    std::filesystem::remove(_path, ec);
}

TestNonExistingFile::~TestNonExistingFile() {
    std::error_code ec;
    std::filesystem::remove(_path, ec);
}
