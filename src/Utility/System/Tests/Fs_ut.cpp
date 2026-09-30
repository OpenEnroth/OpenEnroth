#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/Fs.h"

UNIT_TEST(Fs, ExistsStat) {
    // Files and missing paths.
    ScopedTestFile tmp("tmp_fs_test.txt", "lol");

    EXPECT_TRUE(fs::exists("tmp_fs_test.txt"));
    EXPECT_EQ(fs::stat("tmp_fs_test.txt"), FileStat(FILE_REGULAR, 3));

    EXPECT_FALSE(fs::exists("tmp_fs_doesnt_exist"));
    EXPECT_EQ(fs::stat("tmp_fs_doesnt_exist"), FileStat());
}

UNIT_TEST(Fs, LsRemoveMkdirs) {
    // A directory tree gets created, listed and removed.
    ScopedTestFolder dir("tmp_fs_dir");

    fs::mkdirs("tmp_fs_dir/a/b");
    EXPECT_TRUE(fs::exists("tmp_fs_dir/a/b"));
    EXPECT_EQ(fs::stat("tmp_fs_dir/a/b"), FileStat(FILE_DIRECTORY, 0));
    fs::mkdirs("tmp_fs_dir/a/b"); // Already exists.

    ScopedTestFile tmp("tmp_fs_dir/1.txt", "");
    std::vector<DirectoryEntry> entries = fs::ls("tmp_fs_dir");
    std::ranges::sort(entries);
    EXPECT_EQ(entries, std::vector<DirectoryEntry>({{"1.txt", FILE_REGULAR}, {"a", FILE_DIRECTORY}}));

    EXPECT_TRUE(fs::remove("tmp_fs_dir"));
    EXPECT_FALSE(fs::remove("tmp_fs_dir"));
    EXPECT_FALSE(fs::exists("tmp_fs_dir"));
}

UNIT_TEST(Fs, LsNonAscii) {
    // Non-ASCII names have to come back from ls unchanged.
    std::string name = "\xD1\x84\xD0\xB0\xD0\xB9\xD0\xBB.txt"; // "файл.txt" in UTF-8.
    ScopedTestFolder dir("tmp_fs_non_ascii");
    ScopedTestFile tmp(NativePath("tmp_fs_non_ascii") / NativePath(name), "");

    EXPECT_EQ(fs::ls("tmp_fs_non_ascii"), std::vector<DirectoryEntry>({{name, FILE_REGULAR}}));
}

UNIT_TEST(Fs, LsNotADirectory) {
    // ls throws for a path that isn't a directory, so an empty listing always means an empty directory.
    ScopedTestFile file("tmp_fs_not_a_dir.txt", "lol");

    EXPECT_THROW((void) fs::ls("tmp_fs_doesnt_exist"), std::runtime_error);
    EXPECT_THROW((void) fs::ls("tmp_fs_not_a_dir.txt"), std::runtime_error);

    EXPECT_THROW(fs::mkdirs("tmp_fs_not_a_dir.txt"), std::runtime_error);

    ScopedTestFolder dir("tmp_fs_empty_dir");
    EXPECT_TRUE(fs::ls("tmp_fs_empty_dir").empty());
}

UNIT_TEST(Fs, Absolute) {
    // A relative path resolves against the cwd, and an absolute path stays as it is.
    NativePath cwd = fs::cwd();
    EXPECT_EQ(fs::absolute("a"), cwd / NativePath("a"));
    EXPECT_EQ(fs::absolute(cwd), cwd);
}

UNIT_TEST(Fs, EmptyPath) {
    // An empty path is invalid, and behaves as a path that doesn't exist and can't be created.
    EXPECT_FALSE(fs::exists(""));
    EXPECT_EQ(fs::stat(""), FileStat());
    EXPECT_THROW((void) fs::ls(""), std::runtime_error);
    EXPECT_FALSE(fs::remove(""));
    EXPECT_THROW(fs::mkdirs(""), std::runtime_error);
    EXPECT_THROW((void) fs::absolute(""), std::runtime_error);
}

UNIT_TEST(Fs, TempDir) {
    // The temp directory exists.
    EXPECT_EQ(fs::stat(fs::tempDir()).type, FILE_DIRECTORY);
}
