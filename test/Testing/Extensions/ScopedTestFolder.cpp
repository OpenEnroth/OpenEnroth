#include "ScopedTestFolder.h"

#include "Utility/System/Fs.h"

ScopedTestFolder::ScopedTestFolder(const Path &path) : _path(path) {
    fs::remove(_path); // Drop whatever an earlier run might have left behind.
    fs::mkdirs(_path);
}

ScopedTestFolder::~ScopedTestFolder() {
    try {
        fs::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
