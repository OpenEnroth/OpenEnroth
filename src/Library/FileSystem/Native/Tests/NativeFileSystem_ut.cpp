#include <ranges>
#include <string>
#include <vector>
#include <memory>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Native/NativeFileSystem.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Fs.h"

UNIT_TEST(NativeFileSystem, LsRoot) {
    // Make sure passing empty paths works as intended.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs1(fs::cwd());
    std::vector<DirectoryEntry> entries = fs1.ls("");
    EXPECT_TRUE(std::ranges::find(entries, "1.txt", &DirectoryEntry::name) != std::ranges::end(entries))
        << "size = " << entries.size() << ", [0] = " << (entries.empty() ? "<nothing>" : entries[0].name);

    NativeFileSystem fs2(Path("this_dir_doesnt_exist")); // Non-existent dir.
    EXPECT_TRUE(fs2.ls("").empty());

    NativeFileSystem fs3(Path("1.txt")); // Not-a-dir.
    EXPECT_TRUE(fs3.ls("").empty());
}

UNIT_TEST(NativeFileSystem, LsFile) {
    // Make sure ls() throws when called on a file.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs(fs::cwd());
    EXPECT_THROW_MESSAGE((void) fs.ls("1.txt"), "is not a directory");
}

UNIT_TEST(NativeFileSystem, LsNonExistent) {
    // Make sure ls() throws when called on a folder that doesn't exist.
    NativeFileSystem fs(fs::cwd());
    EXPECT_THROW_MESSAGE((void) fs.ls("this_dir_doesnt_exist"), "doesn't exist");
}

UNIT_TEST(NativeFileSystem, ExistsRoot) {
    // Make sure exists("") works as intented.
    NativeFileSystem fs1(fs::cwd());
    EXPECT_TRUE(fs1.exists(""));

    NativeFileSystem fs2(Path("this_dir_doesnt_exist"));
    EXPECT_TRUE(fs2.exists(""));

    ScopedTestFile tmp("1.txt", "");
    NativeFileSystem fs3(Path("1.txt"));
    EXPECT_TRUE(fs3.exists(""));
}

UNIT_TEST(NativeFileSystem, StatRoot) {
    // Make sure stat("") works as intented.
    NativeFileSystem fs1(fs::cwd());
    EXPECT_EQ(fs1.stat("").type, FILE_DIRECTORY);

    NativeFileSystem fs2(Path("this_dir_doesnt_exist")); // Non-existent dir.
    EXPECT_EQ(fs2.stat("").type, FILE_DIRECTORY);

    ScopedTestFile tmp("1.txt", "");
    NativeFileSystem fs3(Path("1.txt")); // Not-a-dir.
    EXPECT_EQ(fs3.stat("").type, FILE_DIRECTORY);
}

UNIT_TEST(NativeFileSystem, ReadRootAsFile) {
    // Root is always assumed to be a dir, we can't read it as a file even if it IS a file.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs(Path("1.txt"));
    EXPECT_ANY_THROW((void) fs.read(""));
}

UNIT_TEST(NativeFileSystem, WriteRootAsFile) {
    // Root is always assumed to be a dir, we can't write it as a file if it doesn't exist.
    NativeFileSystem fs(Path("1.txt"));
    EXPECT_ANY_THROW(fs.write("", Blob()));
}

UNIT_TEST(NativeFileSystem, WriteCreatesDirs) {
    // Writing creates the missing directories, the root included.
    ScopedTestFolder dir("tmp_dfs_dir");
    NativeFileSystem fs(Path("tmp_dfs_dir/root"));

    fs.write("1.txt", Blob::fromString("a"));
    fs.openForWriting("a/b/2.txt")->write("bc");
    EXPECT_EQ(fs.read("1.txt").str(), "a");
    EXPECT_EQ(fs.read("a/b/2.txt").str(), "bc");
}

#ifndef _WINDOWS
UNIT_TEST(NativeFileSystem, LsHidesBackslashNames) {
    // A backslash is an ordinary character in a POSIX file name, but a separator in a FileSystem path.
    ScopedTestFolder dir("tmp_dfs_backslash");
    ScopedTestFile tmp("tmp_dfs_backslash/a\\b.txt", "");

    NativeFileSystem fs(Path("tmp_dfs_backslash"));
    EXPECT_TRUE(fs.ls("").empty());
}
#endif

UNIT_TEST(NativeFileSystem, DisplayPathSymmetry) {
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs(fs::cwd());
    Blob blob = fs.read("1.txt");
    std::unique_ptr<InputStream> stream = fs.openForReading("1.txt");

    EXPECT_EQ(blob.displayPath(), (fs::cwd() / Path("1.txt")).displayString());
    EXPECT_EQ(blob.displayPath(), stream->displayPath());
}

UNIT_TEST(NativeFileSystem, EscapingPaths) {
    ScopedTestFolder tmp("a");
    ScopedTestFile tmp2("1.txt", "");
    ScopedTestFile tmp3("a/1.txt", "");

    NativeFileSystem fs(Path("a"));

    EXPECT_FALSE(fs.exists(".."));
    EXPECT_FALSE(fs.stat(".."));
    EXPECT_ANY_THROW((void) fs.ls(".."));
    EXPECT_ANY_THROW((void) fs.read("../1.txt"));
    EXPECT_ANY_THROW((void) fs.openForReading("../1.txt"));
    EXPECT_ANY_THROW(fs.write("../1.txt", Blob()));
    EXPECT_ANY_THROW((void) fs.openForWriting("../1.txt"));
    EXPECT_ANY_THROW(fs.remove("../1.txt"));
}

UNIT_TEST(NativeFileSystem, EscapingDisplayPath) {
    NativeFileSystem fs(fs::cwd());

    EXPECT_TRUE(fs.displayPath("..").ends_with(".."));
}
