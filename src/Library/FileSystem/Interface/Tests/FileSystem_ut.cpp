#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Interface/FileSystemException.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/FileSystem/Sub/SubFileSystem.h"

static void expectNotAccessible(const std::function<void()> &call, std::string_view path) {
    EXPECT_THAT(call, testing::ThrowsMessage<FileSystemException>(testing::HasSubstr("not accessible"))) << path;
}

UNIT_TEST(FileSystem, PublicApiNormalizes) {
    // The PathView overloads normalize their path, so each spelling in a loop names the same file.
    MemoryFileSystem fs("ram");
    fs.write("foo/bar", Blob::fromString("lol"));

    for (std::string_view path : {"foo/bar", "foo//bar//", "foo\\bar", "./foo/./bar", "foo/../foo/bar", "a/../foo/bar"}) {
        EXPECT_TRUE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat(FILE_REGULAR, 3)) << path;
        EXPECT_EQ(fs.read(path).str(), "lol") << path;
        EXPECT_EQ(fs.openForReading(path)->readAll(), "lol") << path;
        EXPECT_EQ(fs.displayPath(path), fs.displayPath("foo/bar")) << path;
    }

    for (std::string_view path : {"w/x", "w\\x", "./w/./x", "w/../w/x"}) {
        fs.write(path, Blob::fromString(std::string(path)));
        EXPECT_EQ(fs.read("w/x").str(), path) << path;
        fs.openForWriting(path)->write("kek");
        EXPECT_EQ(fs.read("w/x").str(), "kek") << path;
        EXPECT_TRUE(fs.remove(path)) << path;
        EXPECT_FALSE(fs.exists("w/x")) << path;
    }

    std::vector<DirectoryEntry> entries;
    fs.ls("./foo/.", &entries);
    EXPECT_EQ(entries, std::vector<DirectoryEntry>({{"bar", FILE_REGULAR}}));
    EXPECT_EQ(fs.ls("foo/../foo/"), entries);
}

UNIT_TEST(FileSystem, DotDotStopsAtRoot) {
    // ".." above the root stays at the root.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    EXPECT_EQ(fs.read("../foo").str(), "lol");
    EXPECT_EQ(fs.stat(".."), FileStat(FILE_DIRECTORY, 0));
}

UNIT_TEST(FileSystem, InaccessiblePathsAreRefused) {
    // Every method refuses a path with a root, or on Windows one with a segment starting with a drive, and
    // displayPath still shows it.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    std::vector<std::string_view> paths = {"/", "/foo", "//foo", "\\foo", "/../foo", "/foo/bar/"};
#ifdef _WINDOWS
    // On Windows a drive can also show up after normalizing, as in "./C:/foo", and a segment that starts like one
    // is refused too.
    for (std::string_view path : {"C:/foo", "C:foo", "C:", "./C:/foo", "a/../C:/foo", "../C:/foo", "..\\C:foo", "a/c:foo", "a/C:", "//server/share/foo"})
        paths.push_back(path);
#endif

    for (std::string_view path : paths) {
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

#ifndef _WINDOWS
UNIT_TEST(FileSystem, DriveLikeNamesAreNames) {
    // Outside Windows a drive letter is an ordinary name.
    MemoryFileSystem fs("ram");
    fs.write("C:/foo", Blob::fromString("lol"));
    fs.write("a/c:foo", Blob::fromString("kek"));

    EXPECT_EQ(fs.read("C:/foo").str(), "lol");
    EXPECT_EQ(fs.read("a/c:foo").str(), "kek");
}
#endif

UNIT_TEST(FileSystem, RootIsNotRemovable) {
    // No spelling of the root can be removed.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"", ".", "foo/..", ".."})
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;

    EXPECT_TRUE(fs.exists("foo"));
}

UNIT_TEST(FileSystem, NormalPathOverloads) {
    // A NormalPath goes through the same checks as a plain path.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    EXPECT_EQ(fs.read(NormalPath("foo")).str(), "lol");
    EXPECT_TRUE(fs.exists(NormalPath()));
    EXPECT_EQ(fs.stat(NormalPath()), FileStat(FILE_DIRECTORY, 0));

    EXPECT_FALSE(fs.exists(NormalPath("/foo")));
    expectNotAccessible([&] { (void) fs.read(NormalPath("/foo")); }, "/foo");
    EXPECT_EQ(fs.displayPath(NormalPath("/foo")), fs.displayPath("/foo"));
}

UNIT_TEST(FileSystem, ErrorAboutRootNamesRoot) {
    // An error about the root names it the way displayPath does.
    MemoryFileSystem fs("ram");
    SubFileSystem sub("a", &fs);

    EXPECT_EQ(sub.displayPath(""), "ram://a");
    EXPECT_THAT([&] { (void) sub.read(""); }, testing::ThrowsMessage<FileSystemException>(testing::HasSubstr("'ram://a'")));
}

UNIT_TEST(FileSystem, DisplayPathOfInaccessiblePath) {
    // An inaccessible path is shown next to the root, without reaching the implementation.
    MemoryFileSystem fs("ram");
    SubFileSystem sub("a", &fs);

    EXPECT_EQ(fs.displayPath("/x"), "ram:///x");
    EXPECT_EQ(sub.displayPath("/x"), "ram://a//x"); // SubFileSystem asserts on an inaccessible path.
}
