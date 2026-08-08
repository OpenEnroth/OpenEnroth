#include <filesystem>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/NativePath.h"

UNIT_TEST(NativePath, Wtf8RoundTrip) {
    // The conversion goes through wchar_t on Windows, so WTF-8 has to survive, unpaired surrogates included.
    for (std::string_view path : {"a/b/c.txt", "\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(NativePath::fromWtf8(path).toWtf8(), path);
}

UNIT_TEST(NativePath, StdPathRoundTrip) {
    std::filesystem::path cwd = std::filesystem::current_path();
    EXPECT_EQ(NativePath::fromStdPath(cwd).toStdPath(), cwd);
}

UNIT_TEST(NativePath, Composition) {
    EXPECT_EQ((NativePath::fromWtf8("a/b") / NativePath::fromWtf8("c.txt")).toWtf8(), "a/b/c.txt");
}

UNIT_TEST(NativePath, WithExtension) {
    EXPECT_EQ(NativePath::fromWtf8("a/b.json").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath::fromWtf8("a/b").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath::fromWtf8("a/b.json").withExtension("").toWtf8(), "a/b");
}

UNIT_TEST(NativePath, Absolute) {
    EXPECT_EQ(NativePath().absolute().toStdPath(), std::filesystem::current_path());
    EXPECT_EQ(NativePath::fromWtf8("a").absolute().toStdPath(), std::filesystem::current_path() / "a");
}
