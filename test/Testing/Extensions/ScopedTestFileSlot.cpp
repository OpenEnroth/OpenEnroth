#include "ScopedTestFileSlot.h"

#include "Utility/System/Fs.h"

ScopedTestFileSlot::ScopedTestFileSlot(const Path &path) : _path(path) {
    fs::remove(_path); // Drop whatever an earlier run might have left behind.
}

ScopedTestFileSlot::~ScopedTestFileSlot() {
    try {
        fs::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
