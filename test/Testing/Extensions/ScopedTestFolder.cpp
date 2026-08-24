#include "ScopedTestFolder.h"

#include "Utility/System/Os.h"

ScopedTestFolder::ScopedTestFolder(const NativePath &path) : _path(path) {
    os::remove(_path); // Drop whatever an earlier run might have left behind.
    os::mkdirs(_path);
}

ScopedTestFolder::~ScopedTestFolder() {
    try {
        os::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
