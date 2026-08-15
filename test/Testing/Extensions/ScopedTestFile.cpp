#include "ScopedTestFile.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Os.h"

ScopedTestFile::ScopedTestFile(const NativePath &path, std::string_view contents) : _path(path) {
    FileOutputStream stream(_path);
    stream.write(contents);
    stream.close();
}

ScopedTestFile::~ScopedTestFile() {
    try {
        os::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
