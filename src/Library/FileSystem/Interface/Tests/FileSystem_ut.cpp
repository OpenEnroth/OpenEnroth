#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"

UNIT_TEST(FileSystem, PublicApiNormalizes) {
    // Every public entry point normalizes what it is given, so the private _ methods can assume normal form. The
    // dotted spellings below all name the same file. Covering more than read matters - the one bug this file used to
    // miss was write taking a different code path from read, which a read-only test can't see.
    MemoryFileSystem fs("ram");
    fs.write("foo/bar", Blob::fromString("lol"));

    for (std::string_view path : {"foo/bar", "./foo/./bar", "foo/../foo/bar", "foo//bar", "a/../foo/bar"}) {
        EXPECT_TRUE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat(FILE_REGULAR, 3)) << path;
        EXPECT_EQ(fs.read(path).str(), "lol") << path;
        EXPECT_EQ(fs.openForReading(path)->readAll(), "lol") << path;
    }

    for (std::string_view path : {"w/x", "./w/./x", "w/../w/x"}) {
        fs.write(path, Blob::fromString(std::string(path)));
        EXPECT_EQ(fs.read("w/x").str(), path) << path;
    }

    std::vector<DirectoryEntry> entries;
    fs.ls("./foo/.", &entries); // The two-arg form goes through the same boundary.
    EXPECT_EQ(entries, std::vector<DirectoryEntry>({{"bar", FILE_REGULAR}}));
    EXPECT_EQ(fs.ls("foo/../foo"), entries);
}

UNIT_TEST(FileSystem, AbsolutePathsAreRefused) {
    // An absolute path names a location of its own rather than something under the root, so it is refused wherever
    // it appears. There is no "/foo means foo" rule - one behaviour per input, which is what stops an entry point
    // from quietly acquiring a second one.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"/foo", "//foo//bar", "/foo/bar/", "/"}) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat()) << path;
        EXPECT_ANY_THROW((void) fs.read(path)) << path;
        EXPECT_ANY_THROW(fs.write(path, Blob())) << path;
        EXPECT_ANY_THROW((void) fs.ls(path)) << path;
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;
        EXPECT_ANY_THROW((void) fs.openForReading(path)) << path;
        EXPECT_ANY_THROW((void) fs.openForWriting(path)) << path;
    }

#ifdef _WINDOWS
    for (std::string_view path : {"C:/Windows", "//server/share", "//?/C:/x"}) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_ANY_THROW((void) fs.read(path)) << path;
    }
#endif
}

UNIT_TEST(FileSystem, EscapingPathsAreRefused) {
    // An escaping path points outside the root. These are all relative, so they reach the escaping check rather than
    // being caught by the absolute one first - a leading slash would test the wrong branch.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"..", "../foo", "foo/../../foo", "a/b/../../../c", "../"}) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat()) << path;
        EXPECT_ANY_THROW((void) fs.read(path)) << path;
        EXPECT_ANY_THROW(fs.write(path, Blob())) << path;
        EXPECT_ANY_THROW((void) fs.ls(path)) << path;
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;
    }

    // Not escaping, and they resolve to something that exists.
    EXPECT_TRUE(fs.exists("a/../foo"));
    EXPECT_TRUE(fs.exists("./foo"));
}

UNIT_TEST(FileSystem, RootIsNotRemovable) {
    // The root has no parent to be removed from, and every spelling of it has to be refused.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"", ".", "foo/.."})
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;

    EXPECT_TRUE(fs.exists("foo"));
}
