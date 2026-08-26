#include <algorithm>
#include <filesystem>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/ScopeGuard.h"

#include "Utility/System/Fs.h"

UNIT_TEST(Fs, CwdAbsolute) {
    EXPECT_EQ(fs::cwd(), Path::fromNative(std::filesystem::current_path().native()));
    EXPECT_EQ(fs::absolute(Path()), fs::cwd()); // An empty path resolves to the cwd itself.
    EXPECT_EQ(fs::absolute("a"), fs::cwd() / Path("a"));
}

UNIT_TEST(Fs, ExistsStat) {
    ScopedTestFile tmp("tmp_fs_test.txt", "lol");

    EXPECT_TRUE(fs::exists("tmp_fs_test.txt"));
    EXPECT_EQ(fs::stat("tmp_fs_test.txt"), FileStat(FILE_REGULAR, 3));

    EXPECT_FALSE(fs::exists("tmp_fs_doesnt_exist"));
    EXPECT_EQ(fs::stat("tmp_fs_doesnt_exist"), FileStat());
}

UNIT_TEST(Fs, LsRemoveMkdirs) {
    MM_AT_SCOPE_EXIT(fs::remove("tmp_fs_dir"));

    fs::mkdirs("tmp_fs_dir/a/b");
    EXPECT_EQ(fs::stat("tmp_fs_dir/a/b").type, FILE_DIRECTORY);

    ScopedTestFile tmp("tmp_fs_dir/1.txt", "");
    std::vector<DirectoryEntry> entries = fs::ls("tmp_fs_dir");
    std::ranges::sort(entries); // ls doesn't promise any particular order.
    EXPECT_EQ(entries, std::vector<DirectoryEntry>({
        {"1.txt", FILE_REGULAR},
        {"a", FILE_DIRECTORY}
    }));

    // ls never throws, not for a path that doesn't exist, and not for a file either.
    EXPECT_TRUE(fs::ls("tmp_fs_doesnt_exist").empty());
    EXPECT_TRUE(fs::ls("tmp_fs_dir/1.txt").empty());

    EXPECT_TRUE(fs::remove("tmp_fs_dir"));
    EXPECT_FALSE(fs::remove("tmp_fs_dir")); // Nothing left to remove.
    EXPECT_FALSE(fs::exists("tmp_fs_dir"));
}

UNIT_TEST(Fs, StringLiterals) {
    // The fs functions take a Path, which is implicitly constructible from a byte string, so a literal reaches
    // them in one user-defined conversion and needs no overload of its own.
    ScopedTestFile tmp("tmp_fs_literal.txt", "lol");

    EXPECT_TRUE(fs::exists("tmp_fs_literal.txt"));
    EXPECT_EQ(fs::stat("tmp_fs_literal.txt"), FileStat(FILE_REGULAR, 3));
    EXPECT_EQ(fs::absolute("tmp_fs_literal.txt"), fs::cwd() / Path("tmp_fs_literal.txt"));
}
