#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Interface/FileSystemException.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Merging/MergingFileSystem.h"
#include "Library/FileSystem/Sub/SubFileSystem.h"

static void expectNotAccessible(const std::function<void()> &call, std::string_view path) {
    EXPECT_THAT(call, testing::ThrowsMessage<FileSystemException>(testing::HasSubstr("not accessible"))) << path;
}

UNIT_TEST(FileSystem, PublicApiNormalizes) {
    // Every public method normalizes its path, so all of these spellings name the same file.
    MemoryFileSystem fs("ram");
    fs.write("/foo/bar", Blob::fromString("lol"));

    for (std::string_view path : {"/foo/bar", "/foo//bar//", "/foo\\bar", "//foo/bar", "\\foo\\bar", "/./foo/./bar", "/foo/../foo/bar", "/a/../foo/bar", "/../foo/bar"}) {
        EXPECT_TRUE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat(FILE_REGULAR, 3)) << path;
        EXPECT_EQ(fs.read(path).str(), "lol") << path;
        EXPECT_EQ(fs.openForReading(path)->readAll(), "lol") << path;
        EXPECT_EQ(fs.displayPath(path), fs.displayPath("/foo/bar")) << path;
    }

    for (std::string_view path : {"/w/x", "/w\\x", "/./w/./x", "/w/../w/x"}) {
        fs.write(path, Blob::fromString(std::string(path)));
        EXPECT_EQ(fs.read("/w/x").str(), path) << path;
    }

    std::vector<DirectoryEntry> entries;
    fs.ls("/./foo/.", &entries);
    EXPECT_EQ(entries, std::vector<DirectoryEntry>({{"bar", FILE_REGULAR}}));
    EXPECT_EQ(fs.ls("/foo/../foo/"), entries);
}

UNIT_TEST(FileSystem, RelativePathsAreRefused) {
    // A path that doesn't start with a separator names nothing in a file system, whichever way it's spelled.
    MemoryFileSystem fs("ram");
    fs.write("/foo", Blob::fromString("lol"));

    for (std::string_view path : {"foo", "foo/bar", "foo\\bar", ".", "./foo", "..", "../foo", "..\\foo", "foo/../../foo", "../", "C:/foo", "C:foo", "C:"}) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat()) << path;
        expectNotAccessible([&] { (void) fs.read(path); }, path);
        expectNotAccessible([&] { fs.write(path, Blob()); }, path);
        expectNotAccessible([&] { (void) fs.ls(path); }, path);
        expectNotAccessible([&] { (void) fs.remove(path); }, path);
        expectNotAccessible([&] { (void) fs.openForReading(path); }, path);
        expectNotAccessible([&] { (void) fs.openForWriting(path); }, path);
        EXPECT_NO_THROW((void) fs.displayPath(path)) << path;
    }
}

UNIT_TEST(FileSystem, ParentOfRootIsRoot) {
    // Climbing above the root stops at the root.
    MemoryFileSystem fs("ram");
    fs.write("/foo", Blob::fromString("lol"));

    EXPECT_EQ(fs.stat("/.."), FileStat(FILE_DIRECTORY, 0));
    EXPECT_EQ(fs.read("/../foo").str(), "lol");
    EXPECT_EQ(fs.read("/a/b/../../../foo").str(), "lol");
    EXPECT_EQ(fs.displayPath("/.."), "ram://");
}

UNIT_TEST(FileSystem, DriveLikeNamesAreNames) {
    // A name that looks like a drive is an ordinary name, and doesn't get mixed up with what comes after its colon.
    MemoryFileSystem fs("ram");
    fs.write("/C:/foo", Blob::fromString("lol"));
    fs.write("/a/c:foo", Blob::fromString("kek"));
    fs.write("/c:foo", Blob::fromString("cheburek"));
    fs.write("/foo", Blob::fromString("foo"));

    EXPECT_EQ(fs.read("/C:/foo").str(), "lol");
    EXPECT_EQ(fs.read("/a/c:foo").str(), "kek");
    EXPECT_EQ(fs.read("/c:foo").str(), "cheburek");
    EXPECT_EQ(fs.read("/foo").str(), "foo");
    EXPECT_EQ(fs.ls("/a"), std::vector<DirectoryEntry>({{"c:foo", FILE_REGULAR}}));
    EXPECT_EQ(fs.displayPath("/a/c:foo"), "ram://a/c:foo");
}

UNIT_TEST(FileSystem, RootIsNotRemovable) {
    // Every spelling of the root is refused.
    MemoryFileSystem fs("ram");
    fs.write("/foo", Blob::fromString("lol"));

    for (std::string_view path : {"/", "/foo/..", "/.."})
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;

    EXPECT_TRUE(fs.exists("/foo"));
}

UNIT_TEST(FileSystem, EmptyPathIsRefused) {
    // An empty path names nothing, it is not a spelling of the root.
    MemoryFileSystem fs("ram");
    fs.write("/foo", Blob::fromString("lol"));

    EXPECT_FALSE(fs.exists(""));
    EXPECT_EQ(fs.stat(""), FileStat());
    EXPECT_ANY_THROW((void) fs.ls(""));
    EXPECT_THAT([&] { (void) fs.read(""); }, testing::ThrowsMessage<FileSystemException>(testing::HasSubstr("read ''")));
    EXPECT_ANY_THROW(fs.write("", Blob::fromString("kek")));
    EXPECT_ANY_THROW((void) fs.openForReading(""));
    EXPECT_ANY_THROW((void) fs.openForWriting(""));
    EXPECT_ANY_THROW((void) fs.remove(""));

    EXPECT_TRUE(fs.exists("/"));
    EXPECT_EQ(fs.read("/foo").str(), "lol");
}

UNIT_TEST(FileSystem, NormalPathOverloads) {
    // A NormalPath goes through the same checks as a plain path, and its path() names the same thing.
    MemoryFileSystem fs("ram");
    fs.write("/foo", Blob::fromString("lol"));

    EXPECT_EQ(fs.read(NormalPath("/./foo")).str(), "lol");
    EXPECT_TRUE(fs.exists(NormalPath("/")));
    EXPECT_TRUE(fs.exists(NormalPath("/").path()));
    EXPECT_EQ(fs.stat(NormalPath("/")), FileStat(FILE_DIRECTORY, 0));

    EXPECT_FALSE(fs.exists(NormalPath("../foo")));
    EXPECT_FALSE(fs.exists(NormalPath("")));
    expectNotAccessible([&] { (void) fs.read(NormalPath("../foo")); }, "../foo");
    EXPECT_EQ(fs.displayPath(NormalPath("../foo")), fs.displayPath("../foo"));
}

UNIT_TEST(FileSystem, ErrorAboutRootNamesRoot) {
    // An error about the root names it the way displayPath does.
    MemoryFileSystem fs("ram");
    SubFileSystem sub("/a", &fs);

    EXPECT_EQ(sub.displayPath("/"), "ram://a");
    EXPECT_THAT([&] { (void) sub.read("/"); }, testing::ThrowsMessage<FileSystemException>(testing::HasSubstr("'ram://a'")));
}

UNIT_TEST(FileSystem, DisplayPathOfInaccessiblePath) {
    // An inaccessible path is shown next to the root, the implementation never sees it.
    MemoryFileSystem fs("ram");
    MergingFileSystem merged({&fs});

    EXPECT_EQ(fs.displayPath(".."), "ram://..");
    EXPECT_EQ(fs.displayPath("a/../../b"), "ram://../b");
    EXPECT_EQ(merged.displayPath("../b"), "ram://../b");
}
