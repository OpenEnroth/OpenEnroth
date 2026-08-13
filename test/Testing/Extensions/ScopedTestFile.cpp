#include "ScopedTestFile.h"

#include <filesystem>

#include "Utility/Streams/FileOutputStream.h"

ScopedTestFile::ScopedTestFile(const NativePath &path, std::string_view contents) : _path(path) {
    FileOutputStream stream(_path);
    stream.write(contents);
    stream.close();
}

ScopedTestFile::~ScopedTestFile() {
    std::error_code ec;
    std::filesystem::remove(_path.toStdPath(), ec);
}
