#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Interface/FileSystemPath.h"

UNIT_TEST(FileSystemPath, Normalization) {
    EXPECT_EQ(FileSystemPath("foo/bar").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("/foo/bar").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("foo/bar/").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("/foo/bar/").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("/foo//bar/").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("//foo//bar//").string(), "foo/bar");

    EXPECT_EQ(FileSystemPath("..").string(), "");
    EXPECT_EQ(FileSystemPath("../..").string(), "");
    EXPECT_EQ(FileSystemPath("/../..").string(), "");
    EXPECT_EQ(FileSystemPath("../../").string(), "");
    EXPECT_EQ(FileSystemPath("/../../").string(), "");
    EXPECT_EQ(FileSystemPath("//..//..//").string(), "");

    EXPECT_EQ(FileSystemPath("").string(), "");
    EXPECT_EQ(FileSystemPath("/").string(), "");
    EXPECT_EQ(FileSystemPath("//").string(), "");
    EXPECT_EQ(FileSystemPath("/////////").string(), "");
    EXPECT_EQ(FileSystemPath(".").string(), "");
    EXPECT_EQ(FileSystemPath("/.").string(), "");
    EXPECT_EQ(FileSystemPath("./").string(), "");
    EXPECT_EQ(FileSystemPath("/./").string(), "");
    EXPECT_EQ(FileSystemPath("//.//").string(), "");

    EXPECT_EQ(FileSystemPath("foo/bar/.").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("foo/./bar").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("./foo/bar").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("./foo/./bar/.").string(), "foo/bar");

    EXPECT_EQ(FileSystemPath("foo/bar/..").string(), "foo");
    EXPECT_EQ(FileSystemPath("//foo//bar//..//").string(), "foo");
    EXPECT_EQ(FileSystemPath("foo/../bar").string(), "bar");
    EXPECT_EQ(FileSystemPath("//foo//..//bar//").string(), "bar");
    EXPECT_EQ(FileSystemPath("../foo/bar").string(), "foo/bar");
    EXPECT_EQ(FileSystemPath("//..//foo//bar//").string(), "foo/bar");

    EXPECT_EQ(FileSystemPath("foo/bar/../..").string(), "");
    EXPECT_EQ(FileSystemPath("foo/../../bar").string(), "bar");
    EXPECT_EQ(FileSystemPath("../../foo/bar").string(), "foo/bar");

    EXPECT_EQ(FileSystemPath("foo/.../bar/...").string(), "foo/.../bar/...");
}

#if 0
UNIT_TEST(Path, concatenation) {
    EXPECT_EQ((NormalizedPath("foo") / "").string(), "foo");
    EXPECT_EQ((NormalizedPath("foo") / ".").string(), "foo");
    EXPECT_EQ((NormalizedPath("foo") / "/").string(), "foo");
    EXPECT_EQ((NormalizedPath("foo") / "///////").string(), "foo");
    EXPECT_EQ((NormalizedPath("foo") / "./././././.").string(), "foo");

    EXPECT_EQ((NormalizedPath("foo") / "bar").string(), "foo/bar");
    EXPECT_EQ((NormalizedPath("foo") / "bar/baz").string(), "foo/bar/baz");
    EXPECT_EQ((NormalizedPath("foo") / "bar/baz//////").string(), "foo/bar/baz");

    EXPECT_EQ((NormalizedPath("foo") / "..").string(), "");
    EXPECT_EQ((NormalizedPath("foo") / "../").string(), "");

    EXPECT_EQ((NormalizedPath("foo/bar") / "./../..").string(), "");
}
#endif

UNIT_TEST(Path, Parent) {
    EXPECT_FALSE(FileSystemPath("foo").isParentOf(FileSystemPath("foo")));
    EXPECT_TRUE(FileSystemPath("").isParentOf(FileSystemPath("")));
}
