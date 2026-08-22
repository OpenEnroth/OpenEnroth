#include <algorithm>
#include <filesystem>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/ScopeGuard.h"

#include "Utility/System/Os.h"

UNIT_TEST(Os, CwdAbsolute) {
    EXPECT_EQ(os::cwd().toStdPath(), std::filesystem::current_path());
    EXPECT_EQ(os::absolute(NativePath()), os::cwd()); // An empty path resolves to the cwd itself.
    EXPECT_EQ(os::absolute("a"), os::cwd() / NativePath("a"));
}

UNIT_TEST(Os, ExistsStat) {
    ScopedTestFile tmp("tmp_os_test.txt", "lol");

    EXPECT_TRUE(os::exists("tmp_os_test.txt"));
    EXPECT_EQ(os::stat("tmp_os_test.txt"), FileStat(FILE_REGULAR, 3));

    EXPECT_FALSE(os::exists("tmp_os_doesnt_exist"));
    EXPECT_EQ(os::stat("tmp_os_doesnt_exist"), FileStat());
}

UNIT_TEST(Os, LsRemoveMkdirs) {
    MM_AT_SCOPE_EXIT(os::remove("tmp_os_dir"));

    os::mkdirs("tmp_os_dir/a/b");
    EXPECT_EQ(os::stat("tmp_os_dir/a/b").type, FILE_DIRECTORY);

    ScopedTestFile tmp("tmp_os_dir/1.txt", "");
    std::vector<DirectoryEntry> entries = os::ls("tmp_os_dir");
    std::ranges::sort(entries); // ls doesn't promise any particular order.
    EXPECT_EQ(entries, std::vector<DirectoryEntry>({
        {"1.txt", FILE_REGULAR},
        {"a", FILE_DIRECTORY}
    }));

    // ls never throws, not for a path that doesn't exist, and not for a file either.
    EXPECT_TRUE(os::ls("tmp_os_doesnt_exist").empty());
    EXPECT_TRUE(os::ls("tmp_os_dir/1.txt").empty());

    EXPECT_TRUE(os::remove("tmp_os_dir"));
    EXPECT_FALSE(os::remove("tmp_os_dir")); // Nothing left to remove.
    EXPECT_FALSE(os::exists("tmp_os_dir"));
}

UNIT_TEST(Os, AsciiLiterals) {
    // Every os function has an AsciiLiteral overload. Without them these calls don't compile - reaching a NativePath
    // parameter from a literal needs two user-defined conversions, and C++ allows only one.
    ScopedTestFile tmp("tmp_os_literal.txt", "lol");

    EXPECT_TRUE(os::exists("tmp_os_literal.txt"));
    EXPECT_EQ(os::stat("tmp_os_literal.txt"), FileStat(FILE_REGULAR, 3));
    EXPECT_EQ(os::absolute("tmp_os_literal.txt"), os::cwd() / NativePath("tmp_os_literal.txt"));
}
