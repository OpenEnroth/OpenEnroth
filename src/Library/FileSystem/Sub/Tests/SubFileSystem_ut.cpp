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
    EXPECT_EQ(content.string_view(), "hello");
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
    EXPECT_EQ(content.string_view(), "common");
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
}
