#include <algorithm>
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
    std::string name = reinterpret_cast<const char *>(u8"файл.txt");
    ScopedTestFolder dir("tmp_fs_non_ascii");
    ScopedTestFile tmp(NativePath("tmp_fs_non_ascii") / NativePath(name), "");

    EXPECT_EQ(fs::ls("tmp_fs_non_ascii"), std::vector<DirectoryEntry>({{name, FILE_REGULAR}}));
}

UNIT_TEST(Fs, LsNotADirectory) {
    // ls lists nothing for a path that isn't a directory.
    ScopedTestFile file("tmp_fs_not_a_dir.txt", "lol");

    EXPECT_TRUE(fs::ls("tmp_fs_doesnt_exist").empty());
    EXPECT_TRUE(fs::ls("tmp_fs_not_a_dir.txt").empty());
}

UNIT_TEST(Fs, MkdirsEmptyPath) {
    // The empty path is the current directory, which exists.
    EXPECT_NO_THROW(fs::mkdirs(""));
}
