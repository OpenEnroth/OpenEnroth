#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Interface/NormalPath.h"

UNIT_TEST(NormalPath, Normalizes) {
    // A backslash is a separator on every platform, then the path is brought to normal form.
    auto testOne = [] (std::string_view path, std::string_view result) {
        EXPECT_EQ(NormalPath(path).path().str(), result) << "for '" << path << "'";
    };

    testOne("/", "/");
    testOne("/.", "/");
    testOne("/a/..", "/");
    testOne("/a/b", "/a/b");
    testOne("/a\\b", "/a/b");
    testOne("\\a", "/a");
    testOne("/./a//b/", "/a/b");
    testOne("/a/../..", "/");
    testOne("/../x", "/x");
    testOne("//a", "/a");
    testOne("///a/b", "/a/b");
    testOne("\\\\a", "/a");
    testOne("/c:/..", "/");

    testOne("", "");
    testOne(".", ".");
    testOne("a/..", ".");
    testOne("a/b", "a/b");
    testOne("a/../..", "..");
}

UNIT_TEST(NormalPath, DefaultIsRoot) {
    // A default-constructed path names the root, and so does its view.
    EXPECT_EQ(NormalPath().path().str(), "/");
    EXPECT_TRUE(NormalPathView(NormalPath()).isRoot());
    EXPECT_TRUE(NormalPathView().isRoot());
}

UNIT_TEST(NormalPath, Tails) {
    // A tail is a path of its own, and nothing left after the last segment is the root.
    NormalPath path("/a/b");
    NormalPathView view = path;
    std::string_view first = *view.split().begin();
    std::string_view last = *std::next(view.split().begin());

    EXPECT_EQ(view.tailAt(first).str(), "/a/b");
    EXPECT_EQ(view.tailAt(last).str(), "/b");
    EXPECT_EQ(view.tailAfter(std::string_view()).str(), "/a/b");
    EXPECT_EQ(view.tailAfter(first).str(), "/b");
    EXPECT_TRUE(view.tailAfter(last).isRoot());
}

UNIT_TEST(NormalPath, DriveLikeNamesAreNames) {
    // A name that looks like a drive is an ordinary name, in the path and in a tail of it, on every platform.
    auto segments = [] (NormalPathView path) -> std::vector<std::string> {
        return path.split();
    };

    NormalPath path("/a/c:b");
    NormalPathView view = path;
    EXPECT_EQ(segments(view), std::vector<std::string>({"a", "c:b"}));
    EXPECT_EQ(segments(view.tailAt(*std::next(view.split().begin()))), std::vector<std::string>({"c:b"}));
    EXPECT_EQ(segments(NormalPath("/c:b")), std::vector<std::string>({"c:b"}));
    EXPECT_EQ(segments(NormalPath("/c:/d")), std::vector<std::string>({"c:", "d"}));
}

UNIT_TEST(NormalPath, IsAccessible) {
    // A file system can act on a path that starts with a separator, and on nothing else.
    auto testOne = [] (std::string_view path, bool result) {
        EXPECT_EQ(NormalPath(path).isAccessible(), result) << "for '" << path << "'";
    };

    testOne("/", true);
    testOne("/a", true);
    testOne("\\a", true);
    testOne("/a/./b", true);
    testOne("/../a", true);
    testOne("/c:b", true);
    testOne("/a/c:b", true);
    testOne("//server/share", true);

    testOne("", false);
    testOne(".", false);
    testOne("a", false);
    testOne("a/b", false);
    testOne("..", false);
    testOne("../a", false);
    testOne("C:/a", false);
    testOne("C:a", false);
}

UNIT_TEST(NormalPath, Join) {
    // Joining accessible paths keeps them normal, a side naming the root included.
    auto testOne = [] (std::string_view head, std::string_view tail, std::string_view result) {
        EXPECT_EQ((NormalPath(head) / NormalPath(tail)).path().str(), result) << "for '" << head << "' and '" << tail << "'";

        NormalPath appended(head);
        appended /= NormalPath(tail);
        EXPECT_EQ(appended.path().str(), result) << "for '" << head << "' and '" << tail << "'";
    };

    testOne("/", "/", "/");
    testOne("/", "/b", "/b");
    testOne("/a", "/", "/a");
    testOne("/a", "/b", "/a/b");
    testOne("/a/b", "/c/d", "/a/b/c/d");
    testOne("/a\\b", "/./c", "/a/b/c");
    testOne("/a", "/c:b", "/a/c:b");
}
