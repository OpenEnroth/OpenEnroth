#include <filesystem>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/Os.h"

UNIT_TEST(Os, Cwd) {
    EXPECT_EQ(std::filesystem::path(os::cwd().native()), std::filesystem::current_path());
}

UNIT_TEST(Os, Absolute) {
    EXPECT_EQ(os::absolute(NativePath()), os::cwd());
    EXPECT_EQ(os::absolute(NativePath::fromWtf8("a")), os::cwd() / NativePath::fromWtf8("a"));
}

UNIT_TEST(Os, ExistsStat) {
    ScopedTestFile tmp("tmp_os_test.txt", "lol");

    EXPECT_TRUE(os::exists(NativePath::fromWtf8("tmp_os_test.txt")));
    EXPECT_EQ(os::stat(NativePath::fromWtf8("tmp_os_test.txt")), FileStat(FILE_REGULAR, 3));

    EXPECT_FALSE(os::exists(NativePath::fromWtf8("tmp_os_doesnt_exist")));
    EXPECT_EQ(os::stat(NativePath::fromWtf8("tmp_os_doesnt_exist")), FileStat());
}

UNIT_TEST(Os, Ls) {
    ScopedTestFolder tmp("tmp_os_dir");
    ScopedTestFile tmp2("tmp_os_dir/1.txt", "");

    EXPECT_EQ(os::ls(NativePath::fromWtf8("tmp_os_dir")), std::vector<DirectoryEntry>({{"1.txt", FILE_REGULAR}}));
    EXPECT_TRUE(os::ls(NativePath::fromWtf8("tmp_os_doesnt_exist")).empty()); // ls never throws.
    EXPECT_TRUE(os::ls(NativePath::fromWtf8("tmp_os_dir/1.txt")).empty()); // Not even for a file.
}

UNIT_TEST(Os, RemoveMkdirs) {
    os::mkdirs(NativePath::fromWtf8("tmp_os_dir/a/b"));
    EXPECT_EQ(os::stat(NativePath::fromWtf8("tmp_os_dir/a/b")).type, FILE_DIRECTORY);

    EXPECT_TRUE(os::remove(NativePath::fromWtf8("tmp_os_dir")));
    EXPECT_FALSE(os::remove(NativePath::fromWtf8("tmp_os_dir"))); // Nothing left to remove.
    EXPECT_FALSE(os::exists(NativePath::fromWtf8("tmp_os_dir")));
}
