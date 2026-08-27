#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"

UNIT_TEST(FileSystem, RootRelativePaths) {
    // A path in a file system is relative to its root, so a leading separator means nothing - the class docs promise
    // that "/foo" and "foo" name the same file. This used to fall out of FileSystemPath's normalization, which
    // stripped leading separators. Path keeps them, since to Path a leading separator is a root, so the file system
    // has to strip them at its own boundary now.
    MemoryFileSystem fs("ram");
    fs.write("foo/bar", Blob::fromString("lol"));

    for (std::string_view path : {"foo/bar", "/foo/bar", "//foo//bar", "/foo/bar/", "./foo/./bar", "foo/../foo/bar"})
        EXPECT_EQ(fs.read(path).str(), "lol") << path;
}

UNIT_TEST(FileSystem, EscapingPathsAreRefused) {
    // An escaping path points outside the root and is never accessible. A leading separator doesn't rescue it - the
    // file system strips those before normalizing precisely so that "/../.." stays escaping instead of being
    // clamped to the root the way Path would clamp an absolute path.
    MemoryFileSystem fs("ram");
    fs.write("foo", Blob::fromString("lol"));

    for (std::string_view path : {"..", "../foo", "/../foo", "foo/../../foo", "/../.."}) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_EQ(fs.stat(path), FileStat()) << path;
        EXPECT_ANY_THROW((void) fs.read(path)) << path;
        EXPECT_ANY_THROW(fs.write(path, Blob())) << path;
        EXPECT_ANY_THROW((void) fs.remove(path)) << path;
    }
}

UNIT_TEST(FileSystem, AbsolutePathsAreRefused) {
    // An absolute path names a location of its own rather than something under the root. On Windows that matters: a
    // path starting with a drive letter would replace the root outright once NativeFileSystem composes the two,
    // which is an escape. The drive cases are only absolute on Windows, hence the ifdef.
    MemoryFileSystem fs("ram");

    EXPECT_TRUE(fs.exists("")); // Our own root, which is the empty path.

#ifdef _WINDOWS
    for (std::string_view path : {"C:/Windows", "//server/share"}) {
        EXPECT_FALSE(fs.exists(path)) << path;
        EXPECT_ANY_THROW((void) fs.read(path)) << path;
    }
#endif
}
