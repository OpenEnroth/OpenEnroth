#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/Logger/RotatingLogSink.h"

UNIT_TEST(RotatingLogSink, NonExistingDir) {
    MemoryFileSystem fs("memfs");
    RotatingLogSink sink("/some_dir/rotating_log.txt", &fs, 16); // Shouldn't throw, should create a dir.

    auto list = fs.ls("/some_dir");
    EXPECT_EQ(list.size(), 1);
    EXPECT_TRUE(list.at(0).name.starts_with("rotating_log"));
    EXPECT_TRUE(list.at(0).name.ends_with(".txt"));
}

UNIT_TEST(RotatingLogSink, DriveLikeName) {
    // A log name that looks like a drive stays in its directory, and rotation finds it again.
    MemoryFileSystem fs("memfs");
    RotatingLogSink("/logs/c:b.log", &fs, 16);
    RotatingLogSink("/c:b.log", &fs, 16);

    auto list = fs.ls("/logs");
    ASSERT_EQ(list.size(), 1);
    EXPECT_TRUE(list.at(0).name.starts_with("c:b"));
    EXPECT_TRUE(list.at(0).name.ends_with(".log"));
    EXPECT_EQ(fs.ls("/").size(), 2); // The second log and the "logs" folder.
}
