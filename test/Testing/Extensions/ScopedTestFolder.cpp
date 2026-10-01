#include "ScopedTestFolder.h"

#include <gtest/gtest.h>

#include "Utility/System/Fs.h"

ScopedTestFolder::ScopedTestFolder(const NativePath &path) : _path(path) {
    fs::remove(_path); // An earlier run could have left the folder behind.
    fs::mkdirs(_path);

    EXPECT_TRUE(fs::exists(_path));
}

ScopedTestFolder::~ScopedTestFolder() {
    try {
        fs::remove(_path);
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.

    EXPECT_FALSE(fs::exists(_path)); // A folder left behind poisons the next run.
}
