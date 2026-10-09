#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"

UNIT_TEST(FileSystem, PublicApiNormalizes) {
    // Every public method normalizes its path, so all of these spellings name the same file.
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
    }

    std::vector<DirectoryEntry> entries;
    fs.ls("./foo/.", &entries);
    EXPECT_EQ(entries, std::vector<DirectoryEntry>({{"bar", FILE_REGULAR}}));
    EXPECT_EQ(fs.ls("foo/../foo/"), entries);
}

UNIT_TEST(FileSystem, EscapingPathsAreRefused) {
    // A path that climbs above the root can't be reached, whichever spelling it uses.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"..", "../foo", "..\\foo", "foo/../../foo", "a/b/../../../c", "../"}) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat()) << path;
        EXPECT_ANY_THROW((void) fs.read(path)) << path;
        EXPECT_ANY_THROW(fs.write(path, Blob())) << path;
        EXPECT_ANY_THROW((void) fs.ls(path)) << path;
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;
        EXPECT_ANY_THROW((void) fs.openForReading(path)) << path;
        EXPECT_ANY_THROW((void) fs.openForWriting(path)) << path;
        EXPECT_NO_THROW((void) fs.displayPath(path)) << path;
    }
}

UNIT_TEST(FileSystem, RootedPathsAreRefused) {
    // A path with a root of its own names something outside the file system, whichever way it's spelled.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    std::vector<std::string_view> paths = {"/", "/foo", "//foo", "\\foo", "/../foo", "/foo/bar/"};
#ifdef _WINDOWS
    // On Windows a drive can show up only after normalizing, as in "./C:/foo", and a segment that starts like one
    // is refused too.
    for (std::string_view path : {"C:/foo", "C:foo", "C:", "./C:/foo", "a/../C:/foo", "a/c:foo", "a/C:", "//server/share/foo"})
        paths.push_back(path);
#endif

    for (std::string_view path : paths) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat()) << path;
        EXPECT_ANY_THROW((void) fs.read(path)) << path;
        EXPECT_ANY_THROW(fs.write(path, Blob())) << path;
        EXPECT_ANY_THROW((void) fs.ls(path)) << path;
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;
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
    // Every spelling of the root is refused.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"", ".", "/", "foo/.."})
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;

    EXPECT_TRUE(fs.exists("foo"));
}

UNIT_TEST(NormalPath, Join) {
    // Joining normal paths keeps them normal, an empty side included.
    auto testOne = [] (std::string_view head, std::string_view tail, std::string_view result) {
        EXPECT_EQ((NormalPath(head) / NormalPath(tail)).path().str(), result) << "for '" << head << "' and '" << tail << "'";
    };

    testOne("", "", "");
    testOne("", "b", "b");
    testOne("a", "", "a");
    testOne("a", "b", "a/b");
    testOne("a/b", "c/d", "a/b/c/d");
    testOne("a\\b", "./c", "a/b/c");
}
