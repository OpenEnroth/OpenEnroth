#include "ScopedTestFileSlot.h"

#include "Utility/System/Os.h"

ScopedTestFileSlot::ScopedTestFileSlot(const NativePath &path) : _path(path) {
    os::remove(_path); // Drop whatever an earlier run might have left behind.
}

ScopedTestFileSlot::~ScopedTestFileSlot() {
    try {
        os::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
