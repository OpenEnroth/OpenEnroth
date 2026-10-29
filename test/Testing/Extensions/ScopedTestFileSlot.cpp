#include "ScopedTestFileSlot.h"

#include <utility>

#include "Utility/System/Fs.h"

ScopedTestFileSlot::ScopedTestFileSlot(Path path) : _path(std::move(path)) {
    fs::remove(_path); // An earlier run could have left the file behind.
}

ScopedTestFileSlot::~ScopedTestFileSlot() {
    try {
        fs::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
