#include <vector>
#include <memory>

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

UNIT_TEST(LowercaseFileSystem, KeepEmptyFolders) {
    MM_AT_SCOPE_EXIT(std::filesystem::remove_all("tmp_dir"));

    DirectoryFileSystem fs0("tmp_dir");
    fs0.write("a/b/c.bin", Blob());
    fs0.write("a/c/b.bin", Blob());

    // Check that LowercaseFileSystem keeps empty folders in this case.
    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.remove("a/b/c.bin"));
    EXPECT_TRUE(fs.remove("a/c/b.bin"));
    EXPECT_TRUE(fs0.exists("a/b"));
    EXPECT_TRUE(fs0.exists("a/c"));
    EXPECT_TRUE(fs.exists("a/b"));
    EXPECT_TRUE(fs.exists("a/c"));

    // Same check, just for a new LowercaseFileSystem for which we didn't call remove.
    LowercaseFileSystem fss(&fs0);
    EXPECT_TRUE(fss.exists("a/b"));
    EXPECT_TRUE(fss.exists("a/c"));
}

UNIT_TEST(LowercaseFileSystem, DropEmptyFolders) {
    MemoryFileSystem fs0("");
    fs0.write("a/b/c.bin", Blob());
    fs0.write("a/c/b.bin", Blob());

    // Check that LowercaseFileSystem drops empty folders in this case.
    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.remove("a/b/c.bin"));
    EXPECT_TRUE(fs.remove("a/c/b.bin"));
    EXPECT_FALSE(fs0.exists("a/b"));
    EXPECT_FALSE(fs0.exists("a/c"));
    EXPECT_FALSE(fs0.exists("a"));
    EXPECT_FALSE(fs.exists("a/b"));
    EXPECT_FALSE(fs.exists("a/c"));
    EXPECT_FALSE(fs.exists("a"));
}

UNIT_TEST(LowercaseFileSystem, Conflict) {
    MemoryFileSystem fs0("");
    fs0.write("A.bin", Blob());
    fs0.write("a.bin", Blob());

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.exists("a.bin"));
    EXPECT_EQ(fs.stat("a.bin"), FileStat(FILE_REGULAR, 0));
    EXPECT_EQ(fs.ls(""), std::vector<DirectoryEntry>({{"a.bin", FILE_REGULAR}}));

    EXPECT_ANY_THROW((void) fs.read("a.bin"));
    EXPECT_ANY_THROW(fs.write("a.bin", Blob()));
    EXPECT_ANY_THROW((void) fs.openForReading("a.bin"));
    EXPECT_ANY_THROW((void) fs.openForWriting("a.bin"));
    EXPECT_ANY_THROW((void) fs.remove("a.bin"));
    EXPECT_ANY_THROW(fs.rename("a.bin", "b.bin"));

    EXPECT_TRUE(fs0.exists("A.bin"));
    EXPECT_TRUE(fs0.exists("a.bin"));
}

UNIT_TEST(LowercaseFileSystem, ConflictFolders) {
    MemoryFileSystem fs0("");
    fs0.write("a/1", Blob());
    fs0.write("A/1", Blob());

    LowercaseFileSystem fs(&fs0);
    EXPECT_TRUE(fs.exists("a"));
    EXPECT_EQ(fs.stat("a"), FileStat(FILE_REGULAR, 0));
    EXPECT_FALSE(fs.exists("a/1"));
    EXPECT_EQ(fs.stat("a/1"), FileStat());
    EXPECT_EQ(fs.ls(""), std::vector<DirectoryEntry>({{"a", FILE_REGULAR}}));
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
    EXPECT_TRUE(fs.exists("a/a/a.bin"));

    fs0.clear();
    EXPECT_EQ(fs.stat("a/a/a.bin"), FileStat()); // stat() should call base->stat().
    EXPECT_EQ(fs.ls("a/a"), std::vector<DirectoryEntry>({{"a.bin", FILE_REGULAR}}));

    // Check that we don't blow up in spectacular ways. Throwing is OK.
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

UNIT_TEST(LowercaseFileSystem, DisplayPath) {
    MemoryFileSystem fs0("ram");
    fs0.write("A/A", Blob::fromString("A"));

    LowercaseFileSystem fs(&fs0);

    Blob blob = fs.read("a/a");
    EXPECT_EQ(blob.displayPath(), "ram://A/A");

    std::unique_ptr<InputStream> input = fs.openForReading("a/a");
    EXPECT_EQ(input->displayPath(), "ram://A/A");
    input->close();

    std::unique_ptr<OutputStream> output = fs.openForWriting("a/a");
    EXPECT_EQ(output->displayPath(), "ram://A/A");
    output->close();

    EXPECT_EQ(fs.displayPath("a/b/c"), "ram://A/b/c");
    EXPECT_EQ(fs.displayPath(""), "ram://");
}

UNIT_TEST(LowercaseFileSystem, RemoveDeep) {
    MemoryFileSystem fs0("ram");
    fs0.write("A/B/0", Blob::fromString("0"));
    fs0.write("A/B/1", Blob::fromString("1"));

    LowercaseFileSystem fs(&fs0);

    EXPECT_TRUE(fs.remove("a/b/0"));
    EXPECT_FALSE(fs.exists("a/b/0"));
    EXPECT_FALSE(fs0.exists("A/B/0"));
    EXPECT_TRUE(fs.exists("a/b/1"));
    EXPECT_TRUE(fs0.exists("A/B/1"));
    EXPECT_EQ(fs.ls("a/b"), std::vector<DirectoryEntry>({{"1", FILE_REGULAR}}));
    EXPECT_EQ(fs0.ls("A/B"), std::vector<DirectoryEntry>({{"1", FILE_REGULAR}}));
}

UNIT_TEST(LowercaseFileSystem, RenameOverConflict) {
    MemoryFileSystem fs0("ram");
    fs0.write("A", Blob::fromString(""));
    fs0.write("AAA", Blob::fromString(""));
    fs0.write("AAa", Blob::fromString(""));

    LowercaseFileSystem fs(&fs0);
    EXPECT_ANY_THROW(fs.rename("a", "aaa/b"));
}
