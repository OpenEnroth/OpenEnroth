#include <iterator>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Interface/NormalPath.h"

UNIT_TEST(NormalPath, Normalizes) {
    // A backslash is a separator on every platform, then the path is brought to normal form.
    auto testOne = [] (std::string_view path, std::string_view result) {
        EXPECT_EQ(NormalPath(path).str(), result) << "for '" << path << "'";
    };

    testOne("", "");
    testOne(".", "");
    testOne("./", "");
    testOne("a/..", "");
    testOne("a/b", "a/b");
    testOne("a\\b", "a/b");
    testOne("./a//b/", "a/b");
    testOne("/a/..", "/");
}

UNIT_TEST(NormalPath, DotDotStopsAtRoot) {
    // ".." with nothing left to climb out of stays at the root.
    auto testOne = [] (std::string_view path, std::string_view result) {
        EXPECT_EQ(NormalPath(path).str(), result) << "for '" << path << "'";
    };

    testOne("..", "");
    testOne("../..", "");
    testOne("a/../..", "");
    testOne("../a", "a");
    testOne("..\\a", "a");
    testOne("a/../../b/c", "b/c");
    testOne("./..", "");
    testOne("../.", "");
#ifdef _WINDOWS
    testOne("../C:.", "C:"); // Dropping the ".." exposes a drive, and the result is still normalized.
#endif
}

UNIT_TEST(NormalPath, DefaultIsEmpty) {
    // A default-constructed path and a default-constructed view are both empty.
    EXPECT_TRUE(NormalPath().isEmpty());
    EXPECT_TRUE(NormalPathView().isEmpty());
}

UNIT_TEST(NormalPath, TailAfterLastSegmentIsEmpty) {
    // The tail after the last segment is empty, and the tail at the last segment is that segment.
    NormalPath path("a/b");
    NormalPathView view = path;
    std::string_view last = *std::next(view.split().begin());
    EXPECT_TRUE(view.tailAfter(last).isEmpty());
    EXPECT_EQ(view.tailAt(last).str(), "b");
}

UNIT_TEST(NormalPath, IsAccessible) {
    // A file system can act on a path that has no root, and on Windows has no segment starting with a drive.
    auto testOne = [] (std::string_view path, bool result) {
        EXPECT_EQ(NormalPath(path).isAccessible(), result) << "for '" << path << "'";
    };

    testOne("", true);
    testOne("a", true);
    testOne("a/b", true);
    testOne("/", false);
    testOne("/a", false);
    testOne("\\a", false);
#ifdef _WINDOWS
    testOne("C:/a", false);
    testOne("C:a", false);
    testOne("./C:/a", false);
    testOne("a/c:b", false); // A tail starting at "c:b" would read as a drive.
    testOne("C:..", false);
    testOne("../C:a", false);
    testOne("a/b:c", false);
    testOne("a/bc:d", true);
#else
    testOne("C:/a", true);
    testOne("a/c:b", true);
#endif
}

UNIT_TEST(NormalPath, Join) {
    // Joining accessible paths gives a normal path, and an empty side adds no separator.
    auto testOne = [] (std::string_view head, std::string_view tail, std::string_view result) {
        EXPECT_EQ((NormalPath(head) / NormalPath(tail)).str(), result) << "for '" << head << "' and '" << tail << "'";

        NormalPath appended(head);
        appended /= NormalPath(tail);
        EXPECT_EQ(appended.str(), result) << "for '" << head << "' and '" << tail << "'";
    };

    testOne("", "", "");
    testOne("", "b", "b");
    testOne("a", "", "a");
    testOne("a", "b", "a/b");
    testOne("a/b", "c/d", "a/b/c/d");
}
