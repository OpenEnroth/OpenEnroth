#include "ScopedTestFolder.h"

#include <gtest/gtest.h>

#include "Utility/System/Fs.h"

ScopedTestFolder::ScopedTestFolder(const Path &path) : _path(path) {
    fs::remove(_path); // Drop whatever an earlier run might have left behind.
    fs::mkdirs(_path);

    // A silently failing mkdirs would otherwise show up as an unrelated failure further down the test.
    EXPECT_TRUE(fs::exists(_path));
}

ScopedTestFolder::~ScopedTestFolder() {
    try {
        fs::remove(_path);
        EXPECT_FALSE(fs::exists(_path)); // Likewise, a folder left behind would poison the next run.
    } catch (...) {} // Cleanup errors shouldn't throw out of a dtor.
}
