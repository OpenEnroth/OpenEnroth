#include "ScopedTestFile.h"

#include <utility>

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Fs.h"

ScopedTestFile::ScopedTestFile(Path path, std::string_view contents) : _path(std::move(path)) {
    FileOutputStream stream(_path);
    stream.write(contents);
    stream.close();
}

ScopedTestFile::~ScopedTestFile() {
    try {
        fs::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
