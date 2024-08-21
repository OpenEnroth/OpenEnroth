#include "ScopedTestFile.h"

#include <filesystem>

#include "Utility/Streams/FileOutputStream.h"

ScopedTestFile::ScopedTestFile(std::string_view path, std::string_view contents) : _path(path) {
    FileOutputStream stream(_path);
    stream.write(contents);
    stream.close();
}

ScopedTestFile::~ScopedTestFile() {
    std::error_code ec;
    std::filesystem::remove(_path, ec);
}
