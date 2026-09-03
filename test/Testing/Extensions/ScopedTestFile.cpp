#include "ScopedTestFile.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Fs.h"

ScopedTestFile::ScopedTestFile(const Path &path, std::string_view contents) : _path(path) {
    FileOutputStream stream(_path);
    stream.write(contents);
    stream.close();
}

ScopedTestFile::~ScopedTestFile() {
    try {
        fs::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
