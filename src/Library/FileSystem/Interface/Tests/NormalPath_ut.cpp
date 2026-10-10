#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Interface/NormalPath.h"

UNIT_TEST(NormalPath, Normalizes) {
    // A backslash is a separator on every platform, then the path is brought to normal form.
    auto testOne = [] (std::string_view path, std::string_view result) {
        EXPECT_EQ(NormalPath(path).path().str(), result) << "for '" << path << "'";
    };

    testOne("", "");
    testOne(".", "");
    testOne("a/b", "a/b");
    testOne("a\\b", "a/b");
    testOne("./a//b/", "a/b");
    testOne("a/../..", "..");
    testOne("/a/..", "/");
}

UNIT_TEST(NormalPath, IsAccessible) {
    // A file system can act on a path that has no root and doesn't escape.
    auto testOne = [] (std::string_view path, bool result) {
        EXPECT_EQ(NormalPath(path).isAccessible(), result) << "for '" << path << "'";
    };

    testOne("", true);
    testOne("a", true);
    testOne("a/./b", true);
    testOne("a/..", true);
    testOne("..", false);
    testOne("a/../..", false);
    testOne("/", false);
    testOne("/a", false);
    testOne("\\a", false);
#ifdef _WINDOWS
    testOne("C:/a", false);
    testOne("C:a", false);
    testOne("./C:/a", false);
    testOne("a/c:b", false); // A tail starting at "c:b" would read as a drive.
    testOne("a/b:c", false);
    testOne("a/bc:d", true);
#else
    testOne("C:/a", true);
    testOne("a/c:b", true);
#endif
}

UNIT_TEST(NormalPath, Join) {
    // Joining accessible paths keeps them normal, an empty side included.
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
