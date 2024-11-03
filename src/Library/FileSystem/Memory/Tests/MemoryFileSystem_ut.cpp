#include <ranges>
#include <utility>
#include <memory>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Dump/FileSystemDump.h"

UNIT_TEST(MemoryFileSystem, EmptyRoot) {
    // Make sure accessing root works as expected.
    MemoryFileSystem fs("");
    EXPECT_TRUE(fs.ls("").empty());
    EXPECT_TRUE(fs.exists(""));
    EXPECT_EQ(fs.stat(""), FileStat(FILE_DIRECTORY, 0));

    EXPECT_ANY_THROW((void) fs.read(""));
    EXPECT_ANY_THROW(fs.write("", Blob()));
    EXPECT_ANY_THROW((void) fs.openForReading(""));
    EXPECT_ANY_THROW((void) fs.openForWriting(""));
    EXPECT_ANY_THROW(fs.remove(""));
    EXPECT_ANY_THROW(fs.rename("", ""));
    EXPECT_ANY_THROW(fs.rename("", "new"));

    fs.write("a/b.bin", Blob());
    EXPECT_ANY_THROW(fs.rename("a", ""));
}

UNIT_TEST(MemoryFileSystem, Ls) {
    MemoryFileSystem fs("");

    fs.write("a/b", Blob());
    fs.write("a/c/d", Blob());

    EXPECT_EQ(dumpFileSystem(&fs), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"a", FILE_DIRECTORY},
        {"a/b", FILE_REGULAR},
        {"a/c", FILE_DIRECTORY},
        {"a/c/d", FILE_REGULAR}
    }));

    EXPECT_ANY_THROW((void) fs.ls("a/b"));
    EXPECT_ANY_THROW((void) fs.ls("a/c/d"));
}

UNIT_TEST(MemoryFileSystem, ReadWrite) {
    MemoryFileSystem fs("");

    fs.write("a", Blob());
    EXPECT_EQ(fs.read("a").size(), 0);

    EXPECT_ANY_THROW(fs.write("a/b", Blob()));

    fs.write("b", Blob::fromString("123"));
    EXPECT_EQ(fs.read("b").string_view(), "123");
}

UNIT_TEST(MemoryFileSystem, ReadDir) {
    MemoryFileSystem fs("");

    fs.write("a/b/c", Blob());
    EXPECT_ANY_THROW((void) fs.read("a"));
    EXPECT_ANY_THROW((void) fs.read("a/b"));
}

UNIT_TEST(MemoryFileSystem, ExistsStat) {
    MemoryFileSystem fs("");

    fs.write("a/b/c", Blob::fromString("123"));
    EXPECT_TRUE(fs.exists("a"));
    EXPECT_TRUE(fs.exists("a/b"));
    EXPECT_TRUE(fs.exists("a/b/c"));
    EXPECT_EQ(fs.stat("a"), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b"), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.stat("a/b/c"), FileStat(FILE_REGULAR, 3));
}

UNIT_TEST(MemoryFileSystem, Streaming) {
    MemoryFileSystem fs("");

    EXPECT_ANY_THROW((void) fs.openForReading("a"));

    std::unique_ptr<OutputStream> output0 = fs.openForWriting("a");
    EXPECT_TRUE(fs.exists("a"));
    EXPECT_EQ(fs.stat("a"), FileStat(FILE_REGULAR, 0));
    EXPECT_ANY_THROW((void) fs.read("a"));
    EXPECT_ANY_THROW(fs.write("a", Blob()));
    EXPECT_ANY_THROW((void) fs.openForReading("a"));
    EXPECT_ANY_THROW((void) fs.openForWriting("a"));

    output0->write("123");
    output0->close();
    EXPECT_TRUE(fs.exists("a"));
    EXPECT_EQ(fs.stat("a"), FileStat(FILE_REGULAR, 3));
    EXPECT_EQ(fs.read("a").string_view(), "123");

    std::unique_ptr<InputStream> input0 = fs.openForReading("a");
    std::unique_ptr<InputStream> input1 = fs.openForReading("a");
    EXPECT_ANY_THROW((void) fs.openForWriting("a"));
    EXPECT_ANY_THROW(fs.write("a", Blob()));
    EXPECT_EQ(fs.read("a").string_view(), "123"); // read() works even when readers are active.

    EXPECT_EQ(input0->readAll(), "123");
    EXPECT_EQ(input1->readAll(), "123");
    EXPECT_ANY_THROW((void) fs.openForWriting("a")); // Still can't open for writing even when all read streams are at end.
    EXPECT_ANY_THROW(fs.write("a", Blob()));

    input0->close();
    EXPECT_ANY_THROW((void) fs.openForWriting("a")); // One reader still active, can't write.
    EXPECT_ANY_THROW(fs.write("a", Blob()));

    input1->close();
    fs.write("a", Blob());
    EXPECT_EQ(fs.stat("a"), FileStat(FILE_REGULAR, 0));
}

UNIT_TEST(MemoryFileSystem, Remove) {
    MemoryFileSystem fs("");

    fs.write("a", Blob::fromString("123"));
    std::unique_ptr<InputStream> input = fs.openForReading("a");

    EXPECT_TRUE(fs.remove("a"));
    EXPECT_FALSE(fs.exists("a"));
    EXPECT_EQ(fs.ls("").size(), 0);

    EXPECT_EQ(input->readAll(), "123"); // Input stream still readable, even though the file was removed.
    input->close();
}

UNIT_TEST(MemoryFileSystem, Lifetime) {
    std::unique_ptr<MemoryFileSystem> fs = std::make_unique<MemoryFileSystem>("");

    fs->write("a", Blob::fromString("123"));
    std::unique_ptr<InputStream> input = fs->openForReading("a");
    std::unique_ptr<OutputStream> output = fs->openForWriting("b");
    std::unique_ptr<OutputStream> output2 = fs->openForWriting("c");

    fs.reset();
    EXPECT_EQ(input->readAll(), "123"); // Input stream still readable, even though the FS was destroyed.
    input->close();

    // Output stream still writeable & closeable.
    output->write("123");
    output->close();

    // Closing in destructor also works.
    output2->write("456");
}

UNIT_TEST(MemoryFileSystem, Rename) {
    MemoryFileSystem fs("");

    fs.write("a/b/c", Blob::fromString("123"));

    std::unique_ptr<InputStream> input = fs.openForReading("a/b/c");
    std::unique_ptr<OutputStream> output = fs.openForWriting("a/b/d");

    EXPECT_ANY_THROW(fs.rename("a/b", "a/b/c"));
    EXPECT_ANY_THROW(fs.rename("a/b", "a/b/d"));
    EXPECT_ANY_THROW(fs.rename("a/b", "a/b/1"));
    EXPECT_ANY_THROW(fs.rename("a/b", "a"));

    fs.rename("a/b", "x/y");
    EXPECT_FALSE(fs.exists("a")); // "a" is now empty, so was trimmed.
    EXPECT_ANY_THROW((void) fs.ls("a"));

    EXPECT_EQ(input->readAll(), "123"); // Moving files around keeps the streams valid.
    output->write("1234");
    output->close();

    EXPECT_EQ(dumpFileSystem(&fs, FILE_SYSTEM_DUMP_WITH_CONTENTS), std::vector<FileSystemDumpEntry>({
        {"", FILE_DIRECTORY},
        {"x", FILE_DIRECTORY},
        {"x/y", FILE_DIRECTORY},
        {"x/y/c", FILE_REGULAR, "123"},
        {"x/y/d", FILE_REGULAR, "1234"}
    }));
}

UNIT_TEST(MemoryFileSystem, Overwrite) {
    MemoryFileSystem fs("");
    fs.write("a", Blob::fromString("a"));

    std::unique_ptr<OutputStream> output = fs.openForWriting("a");
    output->write("A");
    output->close();

    EXPECT_EQ(fs.read("a").string_view(), "A");
}

UNIT_TEST(MemoryFileSystem, DisplayPath) {
    MemoryFileSystem fs("mem");
    fs.write("a", Blob::fromString("a"));

    EXPECT_EQ(fs.read("a").displayPath(), "mem://a");

    std::unique_ptr<InputStream> input = fs.openForReading("a");
    EXPECT_EQ(input->displayPath(), "mem://a");
    input->close();

    std::unique_ptr<OutputStream> output = fs.openForWriting("b");
    EXPECT_EQ(output->displayPath(), "mem://b");
    output->close();

    // Also check that writing through a streaming interfaces preserves display path.
    EXPECT_EQ(fs.read("b").displayPath(), "mem://b");
}

UNIT_TEST(MemoryFileSystem, ExceptionMessage) {
    MemoryFileSystem fs("mem");

    EXPECT_THROW_MESSAGE((void) fs.read("a"), "mem://a");
    EXPECT_THROW_MESSAGE((void) fs.openForReading("a"), "mem://a");
    EXPECT_THROW_MESSAGE((void) fs.ls("a"), "mem://a");
}
