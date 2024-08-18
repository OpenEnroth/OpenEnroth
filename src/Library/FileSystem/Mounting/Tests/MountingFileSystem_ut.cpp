#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Mounting/MountingFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Merging/MergingFileSystem.h"
#include "Library/FileSystem/Null/NullFileSystem.h"
#include "Library/FileSystem/Dump/FileSystemDump.h"

UNIT_TEST(MountingFileSystem, StatExists) {
    MemoryFileSystem mfs("");
    mfs.write("c/d", Blob());

    MountingFileSystem fs("");
    fs.mount("a/b", &mfs);

    EXPECT_TRUE(fs.exists("a"));
    EXPECT_TRUE(fs.exists("a/b"));
    EXPECT_TRUE(fs.exists("a/b/c"));
    EXPECT_TRUE(fs.exists("a/b/c/d"));

    EXPECT_EQ(fs.stat("a"), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b"), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b/c"), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b/c/d"), FileStat(FILE_REGULAR, 0));
}

UNIT_TEST(MountingFileSystem, Override) {
    MemoryFileSystem mfs("");
    mfs.write("a", Blob());

    MountingFileSystem fs("");
    fs.mount("", &mfs);
    fs.mount("a", &mfs);

    EXPECT_TRUE(fs.exists("a"));
    EXPECT_TRUE(fs.exists("a/a"));
    EXPECT_EQ(fs.stat("a"), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/a"), FileStat(FILE_REGULAR, 0));

    EXPECT_EQ(fs.ls(""), std::vector<DirectoryEntry>({{"a", FILE_DIRECTORY}}));
    EXPECT_EQ(fs.ls("a"), std::vector<DirectoryEntry>({{"a", FILE_REGULAR}}));
}

UNIT_TEST(MountingFileSystem, SchrodingerOverride) {
    MemoryFileSystem mfs1("");
    MemoryFileSystem mfs2("");
    MergingFileSystem sfs({&mfs1, &mfs2});

    MountingFileSystem fs("");
    fs.mount("", &sfs);

    mfs1.write("a/a.bin", Blob());
    mfs2.write("a", Blob());
    EXPECT_EQ(dumpFileSystem(&fs), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_REGULAR},
        {"a", FILE_DIRECTORY},
        {"a/a.bin", FILE_REGULAR}
    }));

    NullFileSystem nfs;
    fs.mount("a", &nfs);
    EXPECT_EQ(dumpFileSystem(&fs), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_DIRECTORY}
    }));
}

UNIT_TEST(MountingFileSystem, SimpleMerge) {
    MemoryFileSystem mfs1("");
    MemoryFileSystem mfs2("");

    MountingFileSystem fs("");
    fs.mount("", &mfs1);
    fs.mount("a", &mfs2);

    mfs1.write("b", Blob::fromString("b"));
    mfs2.write("a", Blob::fromString("a"));

    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_DIRECTORY},
        {"a/a", FILE_REGULAR, "a"},
        {"b", FILE_REGULAR, "b"}
    }));
}

UNIT_TEST(MountingFileSystem, WriteIntoVfs) {
    MemoryFileSystem mfs("");

    MountingFileSystem fs("");
    fs.mount("a/b/c/d", &mfs);
    fs.mount("a/b/g/e", &mfs);

    EXPECT_ANY_THROW(fs.write("a/b/1.bin", Blob()));
    EXPECT_ANY_THROW((void) fs.openForWriting("a/b/g/1.bin"));
}

UNIT_TEST(MountingFileSystem, ReadWriteThrough) {
    MemoryFileSystem mfs("");

    MountingFileSystem fs("");
    fs.mount("a", &mfs);
    fs.mount("a/b", &mfs);
    fs.mount("x", &mfs);

    fs.write("a/b/b", Blob::fromString("123"));

    EXPECT_EQ(mfs.stat("b"), FileStat(FILE_REGULAR, 3));
    EXPECT_EQ(fs.stat("a/b"), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.read("x/b").string_view(), "123");
    EXPECT_EQ(fs.read("a/b/b").string_view(), "123");
}

UNIT_TEST(MountingFileSystem, Remove) {
    MemoryFileSystem mfs("");

    MountingFileSystem fs("");
    fs.mount("a", &mfs);

    mfs.write("a", Blob());

    EXPECT_ANY_THROW(fs.remove("a"));
    EXPECT_TRUE(fs.remove("a/a"));
    EXPECT_FALSE(fs.remove("a/a"));
}

UNIT_TEST(MountingFileSystem, RenameSameFs) {
    MemoryFileSystem mfs("");

    MountingFileSystem fs("");
    fs.mount("a", &mfs);

    mfs.write("a", Blob::fromString("123"));

    fs.rename("a/a", "a/b");

    EXPECT_EQ(mfs.read("b").string_view(), "123");
    EXPECT_EQ(fs.read("a/b").string_view(), "123");
    EXPECT_EQ(fs.ls("a"), std::vector<DirectoryEntry>({{"b", FILE_REGULAR}}));
}

UNIT_TEST(MountingFileSystem, RenameDifferentFs) {
    MemoryFileSystem mfs1("");
    MemoryFileSystem mfs2("");

    MountingFileSystem fs("");
    fs.mount("1", &mfs1);
    fs.mount("2", &mfs2);

    mfs1.write("a", Blob::fromString("123"));

    fs.rename("1/a", "2/a");

    EXPECT_FALSE(mfs1.exists("a"));
    EXPECT_TRUE(mfs2.exists("a"));
    EXPECT_EQ(mfs2.read("a").string_view(), "123");
}

UNIT_TEST(MountingFileSystem, Binary) {
    MountingFileSystem fs("");
    fs.mount("0", &fs);
    fs.mount("1", &fs);

    EXPECT_TRUE(fs.exists("1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1/1"));
    EXPECT_TRUE(fs.exists("0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0/0"));
}

