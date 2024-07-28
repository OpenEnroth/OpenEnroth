#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Lowercase/LowercaseFileSystem.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Directory/DirectoryFileSystem.h"

#include "Utility/ScopeGuard.h"

UNIT_TEST(LowercaseFileSystem, Empty) {
    MemoryFileSystem fs0("");
    LowercaseFileSystem fs(&fs0);

    EXPECT_TRUE(fs.ls("").empty());
    EXPECT_TRUE(fs.exists(""));
    EXPECT_EQ(fs.stat(""), FileStat(FILE_DIRECTORY, 0));
}

UNIT_TEST(LowercaseFileSystem, ExistsStatUppercase) {
    MemoryFileSystem fs0("");
    fs0.write("A.bin", Blob());

    LowercaseFileSystem fs(&fs0);
    EXPECT_FALSE(fs.exists("A.bin"));
    EXPECT_EQ(fs.stat("A.bin"), FileStat());
}

UNIT_TEST(LowercaseFileSystem, EmptyFolders) {
    MM_AT_SCOPE_EXIT(std::filesystem::remove_all("tmp_dir"));

    DirectoryFileSystem fs0("tmp_dir");
    fs0.write("a/b/c.bin", Blob());
    fs0.write("a/c/b.bin", Blob());
    fs0.write("b/a/a.bin", Blob());
    EXPECT_TRUE(fs0.remove("a/b/c.bin"));
    EXPECT_TRUE(fs0.remove("a/c/b.bin"));
    EXPECT_TRUE(fs0.exists("a/b"));
    EXPECT_TRUE(fs0.exists("a/c"));

    // Check that LowercaseFileSystem doesn't keep empty folders.
    LowercaseFileSystem fs(&fs0);
    EXPECT_FALSE(fs.exists("a/b"));
    EXPECT_FALSE(fs.exists("a/c"));
    EXPECT_FALSE(fs.exists("a"));
    EXPECT_TRUE(fs.exists("b"));
    EXPECT_EQ(fs.ls(""), std::vector<DirectoryEntry>({{"b", FILE_DIRECTORY}}));
}

UNIT_TEST(LowercaseFileSystem, Conflict) {
    MemoryFileSystem fs0("");
    fs0.write("A.bin", Blob());
    fs0.write("a.bin", Blob());

    EXPECT_ANY_THROW(LowercaseFileSystem fs(&fs0));
}

UNIT_TEST(LowercaseFileSystem, Lowercase) {
    MemoryFileSystem fs0("");
    fs0.write("A.bin", Blob::fromString("123"));
    fs0.write("a/B/C/1.bin", Blob::fromString("321"));
    fs0.write("a/C/C/1.bin", Blob::fromString("111"));

    LowercaseFileSystem fs(&fs0);
    EXPECT_EQ(fs.read("a.bin").string_view(), "123");
    EXPECT_EQ(fs.read("a/b/c/1.bin").string_view(), "321");
    EXPECT_EQ(fs.read("a/c/c/1.bin").string_view(), "111");
}

UNIT_TEST(LowercaseFileSystem, Shenanigans) {
    MemoryFileSystem fs0("");
    fs0.write("A/A/A.bin", Blob::fromString("123"));

    LowercaseFileSystem fs(&fs0);
    fs0.clear();

    EXPECT_TRUE(fs.exists("a/a/a.bin"));
    EXPECT_EQ(fs.stat("a/a/a.bin"), FileStat());
    EXPECT_EQ(fs.ls("a/a"), std::vector<DirectoryEntry>({{"a.bin", FILE_REGULAR}}));

    EXPECT_ANY_THROW((void) fs.read("a/a/a.bin"));
    EXPECT_ANY_THROW((void) fs.openForReading("a/a/a.bin"));
}

UNIT_TEST(LowercaseFileSystem, Write) {
    MemoryFileSystem fs0("");
    fs0.write("B/B.bin", Blob::fromString("B"));

    LowercaseFileSystem fs(&fs0);
    fs.write("a/a.bin", Blob::fromString("a"));
    fs.write("b/b.bin", Blob::fromString("bbb"));

    EXPECT_TRUE(fs.exists("a/a.bin"));
    EXPECT_TRUE(fs.exists("b/b.bin"));
    EXPECT_EQ(fs.ls("a"), std::vector<DirectoryEntry>({{"a.bin", FILE_REGULAR}}));
    EXPECT_EQ(fs.ls("b"), std::vector<DirectoryEntry>({{"b.bin", FILE_REGULAR}}));

    EXPECT_EQ(fs0.read("a/a.bin").string_view(), "a");
    EXPECT_EQ(fs0.read("B/B.bin").string_view(), "bbb");
}

UNIT_TEST(LowercaseFileSystem, PruneRemove) {
    MemoryFileSystem fs0("");
    fs0.write("a/a", Blob());

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.remove("a/a"));

    EXPECT_FALSE(fs.exists("a"));
    EXPECT_TRUE(fs.ls("").empty());
    EXPECT_FALSE(fs0.exists("a")); // Because memory fs doesn't support empty dirs either.
}

UNIT_TEST(LowercaseFileSystem, PruneRename) {
    MemoryFileSystem fs0("");
    fs0.write("A/A/A", Blob::fromString("123"));

    LowercaseFileSystem fs(&fs0);
    fs.rename("a/a/a", "b/b/b");

    EXPECT_FALSE(fs.exists("a")); // Pruning happened.
    EXPECT_TRUE(fs.exists("b"));
    EXPECT_EQ(fs.read("b/b/b").string_view(), "123");

    EXPECT_FALSE(fs0.exists("A"));
    EXPECT_TRUE(fs0.exists("b"));
    EXPECT_EQ(fs0.read("b/b/b").string_view(), "123");
}

UNIT_TEST(LowercaseFileSystem, RenameReplace) {
    MemoryFileSystem fs0("");
    fs0.write("A/A/A", Blob::fromString("AAA"));
    fs0.write("B/B/B", Blob::fromString("BBB"));

    LowercaseFileSystem fs(&fs0);
    fs.rename("a/a/a", "b/b/b");

    EXPECT_FALSE(fs.exists("a"));
    EXPECT_TRUE(fs.exists("b"));
    EXPECT_EQ(fs.read("b/b/b").string_view(), "AAA");

    EXPECT_FALSE(fs0.exists("A"));
    EXPECT_TRUE(fs0.exists("B"));
    EXPECT_EQ(fs0.read("B/B/B").string_view(), "AAA");
}

UNIT_TEST(LowercaseFileSystem, RenameFolder) {
    MemoryFileSystem fs0("");
    fs0.write("A/A/A/A", Blob::fromString("AAAA"));
    fs0.write("B/tmp", Blob());

    LowercaseFileSystem fs(&fs0);
    fs.rename("a/a", "b/b");

    EXPECT_FALSE(fs.exists("a"));
    EXPECT_TRUE(fs.exists("b/b/a/a"));
    EXPECT_TRUE(fs.exists("b/tmp"));

    EXPECT_FALSE(fs0.exists("A"));
    EXPECT_TRUE(fs0.exists("B/b/A/A"));
    EXPECT_EQ(fs0.read("B/b/A/A").string_view(), "AAAA");
}

UNIT_TEST(LowercaseFileSystem, WriteUppercase) {
    MemoryFileSystem fs0("");
    LowercaseFileSystem fs(&fs0);

    EXPECT_ANY_THROW(fs.write("A", Blob()));
    EXPECT_ANY_THROW((void) fs.openForWriting("A"));
}

UNIT_TEST(LowercaseFileSystem, RenameRepeatedly) {
    MemoryFileSystem fs0("");
    fs0.write("A", Blob::fromString("A"));

    LowercaseFileSystem fs(&fs0);
    fs.rename("a", "b");
    fs.rename("b", "c");
    fs.rename("c", "d");
    fs.rename("d", "e/f/g/h");
    fs.rename("e/f/g/h", "a");

    EXPECT_TRUE(fs.exists("a"));
    EXPECT_EQ(fs.read("a").string_view(), "A");
    EXPECT_EQ(fs.ls(""), std::vector<DirectoryEntry>({{"a", FILE_REGULAR}}));
    EXPECT_FALSE(fs0.exists("A"));
    EXPECT_EQ(fs0.read("a").string_view(), "A");
    EXPECT_EQ(fs0.ls(""), std::vector<DirectoryEntry>({{"a", FILE_REGULAR}}));
}

UNIT_TEST(LowercaseFileSystem, RenameUppercase) {
    MemoryFileSystem fs0("");
    fs0.write("A", Blob::fromString("A"));

    LowercaseFileSystem fs(&fs0);
    EXPECT_ANY_THROW(fs.rename("a", "B"));
    EXPECT_TRUE(fs.exists("a"));
    EXPECT_EQ(fs.read("a").string_view(), "A");
}

UNIT_TEST(LowercaseFileSystem, RemoveRepeatedly) {
    MemoryFileSystem fs0("");
    fs0.write("A", Blob::fromString("A"));

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.remove("a"));
    EXPECT_FALSE(fs.remove("a"));
    EXPECT_FALSE(fs.remove("a"));

    EXPECT_FALSE(fs.exists("a"));
    EXPECT_TRUE(fs.ls("").empty());
    EXPECT_FALSE(fs0.exists("A"));
    EXPECT_TRUE(fs0.ls("").empty());
}
