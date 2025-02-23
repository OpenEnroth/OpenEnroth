#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Merging/MergingFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Dump/FileSystemDump.h"

UNIT_TEST(MergingFileSystem, Empty) {
    MergingFileSystem fs({});

    EXPECT_TRUE(fs.exists(""));
    EXPECT_FALSE(fs.exists("a"));
    EXPECT_EQ(fs.ls(""), std::vector<DirectoryEntry>());

    EXPECT_EQ(fs.stat("a"), FileStat(FILE_INVALID, 0));
    EXPECT_ANY_THROW((void) fs.read("a"));
    EXPECT_ANY_THROW((void) fs.openForReading("a"));
    EXPECT_ANY_THROW(fs.write("a", Blob::fromString("123")));
}

UNIT_TEST(MergingFileSystem, SimpleMerge) {
    MemoryFileSystem fs0("");
    fs0.write("a/b", Blob::fromString("B"));

    MemoryFileSystem fs1("");
    fs1.write("a/c/d", Blob::fromString("D"));

    MergingFileSystem fs({&fs0, &fs1});

    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_DIRECTORY},
        {"a/b", FILE_REGULAR, "B"},
        {"a/c", FILE_DIRECTORY},
        {"a/c/d", FILE_REGULAR, "D"}
    }));
}

UNIT_TEST(MergingFileSystem, ShrodingerMaxxxing) {
    MemoryFileSystem fs0("");
    fs0.write("a/b/c", Blob::fromString("C"));

    MemoryFileSystem fs1("");
    fs1.write("a/b", Blob::fromString("B"));

    MemoryFileSystem fs2("");
    fs2.write("a/c/d", Blob::fromString("D"));

    MergingFileSystem fs({&fs0, &fs1, &fs2});

    EXPECT_EQ(fs.stat("a/c"), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b"), FileStat(FILE_REGULAR, 1));
    EXPECT_TRUE(fs.exists("a/b"));
    EXPECT_EQ(fs.read("a/b").string_view(), "B");
    EXPECT_EQ(fs.openForReading("a/b")->readAll(), "B");

    // Implementation sorts, so it's OK to not re-sort here.
    EXPECT_EQ(fs.ls("a"), std::vector<DirectoryEntry>({{"b", FILE_REGULAR}, {"b", FILE_DIRECTORY}, {"c", FILE_DIRECTORY}}));

    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_DIRECTORY},
        {"a/b", FILE_REGULAR, "B"},
        {"a/b", FILE_DIRECTORY},
        {"a/b/c", FILE_REGULAR, "C"},
        {"a/c", FILE_DIRECTORY},
        {"a/c/d", FILE_REGULAR, "D"}
    }));
}

UNIT_TEST(MergingFileSystem, DisplayPathForExistingDir) {
    MemoryFileSystem fs0("fs0");
    MemoryFileSystem fs1("fs1");
    fs1.write("a/b", Blob::fromString("B"));

    MergingFileSystem fs({&fs0, &fs1});

    EXPECT_EQ(fs.displayPath("a"), "fs1://a");
    EXPECT_EQ(fs.displayPath("a/b"), "fs1://a/b");
}
