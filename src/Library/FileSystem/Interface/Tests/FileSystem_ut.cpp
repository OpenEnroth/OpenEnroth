#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"

UNIT_TEST(FileSystem, PublicApiNormalizes) {
    // Every public method normalizes its path, so all of these spellings name the same file.
    MemoryFileSystem fs("ram");
    fs.write("foo/bar", Blob::fromString("lol"));

    for (std::string_view path : {"foo/bar", "/foo/bar", "//foo//bar//", "foo\\bar", "\\foo\\bar", "./foo/./bar",
                                  "foo/../foo/bar", "a/../foo/bar"}) {
        EXPECT_TRUE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat(FILE_REGULAR, 3)) << path;
        EXPECT_EQ(fs.read(path).str(), "lol") << path;
        EXPECT_EQ(fs.openForReading(path)->readAll(), "lol") << path;
        EXPECT_EQ(fs.displayPath(path), fs.displayPath("foo/bar")) << path;
    }

    for (std::string_view path : {"w/x", "/w/x", "w\\x", "./w/./x", "w/../w/x"}) {
        fs.write(path, Blob::fromString(std::string(path)));
        EXPECT_EQ(fs.read("w/x").str(), path) << path;
    }

    std::vector<DirectoryEntry> entries;
    fs.ls("./foo/.", &entries);
    EXPECT_EQ(entries, std::vector<DirectoryEntry>({{"bar", FILE_REGULAR}}));
    EXPECT_EQ(fs.ls("/foo/../foo/"), entries);
}

UNIT_TEST(FileSystem, EscapingPathsAreRefused) {
    // A path that climbs above the root can't be reached, whichever spelling it uses.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"..", "../foo", "/../foo", "\\..\\foo", "foo/../../foo", "a/b/../../../c", "../"}) {
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

#ifdef _WINDOWS
UNIT_TEST(FileSystem, DrivePathsAreRefused) {
    // A drive names a root of its own, and can show up only after normalizing, as in "./C:/foo".
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"C:/foo", "C:foo", "C:", "./C:/foo", "a/../C:/foo"}) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_ANY_THROW((void) fs.read(path)) << path;
        EXPECT_ANY_THROW(fs.write(path, Blob())) << path;
        EXPECT_NO_THROW((void) fs.displayPath(path)) << path;
    }

    // Leading separators are dropped before the root is read, so these aren't shares.
    EXPECT_TRUE(fs.exists("//foo"));
    EXPECT_TRUE(fs.exists("\\\\foo"));
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
