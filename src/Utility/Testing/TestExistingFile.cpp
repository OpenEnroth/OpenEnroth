#include "TestExistingFile.h"

#include <filesystem>

#include "Utility/Streams/FileOutputStream.h"

TestExistingFile::TestExistingFile(std::string_view path, std::string_view contents) : _path(path) {
    FileOutputStream stream(_path);
    stream.write(contents.data(), contents.size());
    stream.close();
}

TestExistingFile::~TestExistingFile() {
    std::error_code ec;
    std::filesystem::remove(_path, ec);
}
