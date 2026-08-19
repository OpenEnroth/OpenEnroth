#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/NativePath.h"

UNIT_TEST(NativePath, Wtf8RoundTrip) {
    // The conversion goes through wchar_t on Windows, so WTF8 has to survive, unpaired surrogates included.
    for (std::string_view path : {"a/b/c.txt", "\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(NativePath::fromWtf8(path).toWtf8(), path);
}

UNIT_TEST(NativePath, StdPathRoundTrip) {
    std::filesystem::path cwd = std::filesystem::current_path();
    EXPECT_EQ(NativePath::fromStdPath(cwd).toStdPath(), cwd);
}

UNIT_TEST(NativePath, Literals) {
    // ASCII literals construct directly, everything else goes through fromWtf8.
    EXPECT_EQ(NativePath("a/b/c.txt"), NativePath::fromWtf8("a/b/c.txt"));
    EXPECT_EQ(NativePath(""), NativePath());
}

UNIT_TEST(NativePath, Composition) {
    EXPECT_EQ((NativePath("a/b") / NativePath("c.txt")).toWtf8(), "a/b/c.txt");
}

UNIT_TEST(NativePath, WithExtension) {
    EXPECT_EQ(NativePath("a/b.json").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b.json").withExtension("").toWtf8(), "a/b");
}

UNIT_TEST(NativePath, Absolute) {
    EXPECT_EQ(NativePath().absolute().toStdPath(), std::filesystem::current_path());
    EXPECT_EQ(NativePath("a").absolute().toStdPath(), std::filesystem::current_path() / "a");
}

#ifndef _WINDOWS
UNIT_TEST(NativePath, InvalidUtf8FileNames) {
    // File names on Linux are byte strings, so fromWtf8 / toWtf8 have to pass invalid UTF-8 through as-is. "\xD0" is
    // an incomplete UTF-8 sequence, "\xFF" can't appear in UTF-8 at all.
    for (std::string_view name : {"tmp_lol\xD0kek.txt", "tmp_lol\xFFkek.txt", "tmp_trailing\xD0"}) {
        NativePath path = NativePath::fromWtf8(name);
        EXPECT_EQ(path.toWtf8(), name);

        // APFS rejects such names, and so do APFS-backed mounts in a dev container, so we skip instead of asserting.
        std::ofstream stream(path.toStdPath());
        if (!stream.is_open())
            GTEST_SKIP() << "File system rejected an invalid UTF-8 name.";
        stream << "lol";
        stream.close();

        EXPECT_TRUE(std::filesystem::exists(path.toStdPath())) << name;
        EXPECT_TRUE(std::filesystem::remove(path.toStdPath())) << name;
    }
}
#endif
