#include "ScopedTestFileSlot.h"

#include "Utility/System/Fs.h"

ScopedTestFileSlot::ScopedTestFileSlot(const NativePath &path) : _path(path) {
    fs::remove(_path); // An earlier run could have left the file behind.
}

ScopedTestFileSlot::~ScopedTestFileSlot() {
    try {
        fs::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
