#include <string>
#include <utility>

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

UNIT_TEST(FileSystemPath, Parent) {
    EXPECT_FALSE(FileSystemPath("foo").isParentOf(FileSystemPath("foo")));
    EXPECT_TRUE(FileSystemPath("").isParentOf(FileSystemPath("")));
}

UNIT_TEST(FileSystemPath, EmptyChunks) {
    EXPECT_TRUE(FileSystemPath().chunks().empty());
    EXPECT_TRUE(FileSystemPath(".").chunks().empty());
    EXPECT_TRUE(FileSystemPath("..").chunks().empty());
    EXPECT_FALSE(FileSystemPath("a").chunks().empty());
}

UNIT_TEST(FileSystemPath, Tail) {
    FileSystemPath path("a/b/c");

    auto tails = [] (const FileSystemPath &path, std::string_view at) -> std::pair<FileSystemPath, FileSystemPath> {
        for (std::string_view chunk : path.chunks())
            if (chunk == at)
                return std::pair(path.tailAt(chunk), path.tailAfter(chunk));
        return {};
    };

    const auto [tail0, tail1] = tails(path, "a");
    EXPECT_EQ(tail0.string(), "a/b/c");
    EXPECT_EQ(tail1.string(), "b/c");

    const auto [tail2, tail3] = tails(path, "b");
    EXPECT_EQ(tail2.string(), "b/c");
    EXPECT_EQ(tail3.string(), "c");

    const auto [tail4, tail5] = tails(path, "c");
    EXPECT_EQ(tail4.string(), "c");
    EXPECT_EQ(tail5.string(), "");

    const auto [tail6, tail7] = tails(path, "d");
    EXPECT_EQ(tail6.string(), "");
    EXPECT_EQ(tail7.string(), "");
}

UNIT_TEST(FileSystemPath, Appended) {
    EXPECT_EQ(FileSystemPath("").appended("").string(), "");
    EXPECT_EQ(FileSystemPath("").appended("a").string(), "a");
    EXPECT_EQ(FileSystemPath("a").appended("").string(), "a");
    EXPECT_EQ(FileSystemPath("a").appended("b").string(), "a/b");
    EXPECT_EQ(FileSystemPath("a/b").appended("c").string(), "a/b/c");
    EXPECT_EQ(FileSystemPath("a/b").appended(FileSystemPath("c/d")).string(), "a/b/c/d");
}

UNIT_TEST(FileSystemPath, Append) {
    FileSystemPath path0("");
    path0.append("");
    EXPECT_EQ(path0.string(), "");

    FileSystemPath path1("");
    path1.append("a");
    EXPECT_EQ(path1.string(), "a");

    FileSystemPath path2("a");
    path2.append("");
    EXPECT_EQ(path2.string(), "a");

    FileSystemPath path3("a");
    path3.append("b");
    EXPECT_EQ(path3.string(), "a/b");

    FileSystemPath path4("a/b");
    path4.append("c");
    EXPECT_EQ(path4.string(), "a/b/c");

    FileSystemPath path5("a/b");
    path5.append(FileSystemPath("c/d"));
    EXPECT_EQ(path5.string(), "a/b/c/d");
}
