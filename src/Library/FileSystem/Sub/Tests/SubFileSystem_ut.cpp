#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Sub/SubFileSystem.h"

UNIT_TEST(SubFileSystem, ReadFile) {
    MemoryFileSystem base("memfs");
    base.write("dir/file.txt", Blob::fromString("hello"));

    SubFileSystem sub("dir", &base);

    EXPECT_TRUE(sub.exists("file.txt"));
    Blob content = sub.read("file.txt");
    EXPECT_EQ(content.str(), "hello");
}

UNIT_TEST(SubFileSystem, FileNotFound) {
    MemoryFileSystem base("memfs");
    SubFileSystem sub("dir", &base);

    EXPECT_FALSE(sub.exists("file.txt"));
}

UNIT_TEST(SubFileSystem, ListDirectory) {
    MemoryFileSystem base("memfs");
    base.write("shaders/a.vert", Blob::fromString("a"));
    base.write("shaders/b.frag", Blob::fromString("b"));

    SubFileSystem sub("shaders", &base);

    auto entries = sub.ls("");
    EXPECT_EQ(entries.size(), 2);
}

UNIT_TEST(SubFileSystem, NestedDirectory) {
    MemoryFileSystem base("memfs");
    base.write("shaders/include/common.vert", Blob::fromString("common"));

    SubFileSystem sub("shaders", &base);

    EXPECT_TRUE(sub.exists("include/common.vert"));
    Blob content = sub.read("include/common.vert");
    EXPECT_EQ(content.str(), "common");
}

UNIT_TEST(SubFileSystem, DisplayPath) {
    MemoryFileSystem base("memfs");
    base.write("shaders/test.vert", Blob::fromString("test"));

    SubFileSystem sub("shaders", &base);

    std::string path = sub.displayPath("test.vert");
    EXPECT_CONTAINS(path, "shaders");
    EXPECT_CONTAINS(path, "test.vert");
}

UNIT_TEST(SubFileSystem, CannotEscapeWithDotDot) {
    MemoryFileSystem base("memfs");
    base.write("secret.txt", Blob::fromString("secret"));
    base.write("shaders/test.vert", Blob::fromString("test"));

    SubFileSystem sub("shaders", &base);

    // Should not be able to access files outside the sub directory.
    EXPECT_FALSE(sub.exists("../secret.txt"));
    EXPECT_ANY_THROW((void) sub.read("../secret.txt"));
    EXPECT_ANY_THROW((void) sub.openForReading("../secret.txt"));

    // Same for pathological tails that try harder.
    EXPECT_FALSE(sub.exists("../../.."));
    EXPECT_FALSE(sub.exists("a/../../../secret.txt"));
    EXPECT_ANY_THROW((void) sub.read("a/../../../secret.txt"));
}

UNIT_TEST(SubFileSystem, BasePathMustBeRelative) {
    MemoryFileSystem base("memfs");

    EXPECT_ANY_THROW(SubFileSystem("/etc", &base));
    EXPECT_ANY_THROW(SubFileSystem("..", &base));
    EXPECT_ANY_THROW(SubFileSystem("a/../..", &base));

#ifdef _WINDOWS
    // These two used to read as relative segments, which let them through this guard. The base is composed with the
    // tail on every later call, so the result was an absolute path arriving at a forwarder, and forwarders skip
    // validation by design.
    EXPECT_ANY_THROW(SubFileSystem("C:", &base));
    EXPECT_ANY_THROW(SubFileSystem("C:x", &base));
#endif
}

UNIT_TEST(SubFileSystem, RootWhenBasePathDoesntExist) {
    // Root of a FileSystem always exists, even if the base path doesn't exist on the underlying file system.
    MemoryFileSystem base("memfs");
    SubFileSystem sub("this_dir_doesnt_exist", &base);

    EXPECT_TRUE(sub.exists(""));
    EXPECT_EQ(sub.stat(""), FileStat(FILE_DIRECTORY, 0));
    EXPECT_TRUE(sub.ls("").empty());

    // But it's a directory, so we can't read or write it as a file.
    EXPECT_ANY_THROW((void) sub.read(""));
    EXPECT_ANY_THROW(sub.write("", Blob()));

    // And ls of a non-root path that doesn't exist still throws.
    EXPECT_ANY_THROW((void) sub.ls("subdir"));
}

UNIT_TEST(SubFileSystem, RootWhenBasePathIsFile) {
    // Same deal when the base path points to a file.
    MemoryFileSystem base("memfs");
    base.write("1.txt", Blob::fromString("lol"));

    SubFileSystem sub("1.txt", &base);

    EXPECT_TRUE(sub.exists(""));
    EXPECT_EQ(sub.stat(""), FileStat(FILE_DIRECTORY, 0));
    EXPECT_TRUE(sub.ls("").empty());

    EXPECT_ANY_THROW((void) sub.read(""));
    EXPECT_ANY_THROW(sub.write("", Blob()));
}

UNIT_TEST(SubFileSystem, RootWhenBasePathIsEmpty) {
    // An empty base path just means "the whole base file system".
    MemoryFileSystem base("memfs");
    base.write("1.txt", Blob::fromString("lol"));

    SubFileSystem sub("", &base);

    EXPECT_TRUE(sub.exists(""));
    EXPECT_EQ(sub.stat(""), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(sub.ls("").size(), 1);
    EXPECT_EQ(sub.read("1.txt").str(), "lol");
}
