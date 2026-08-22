#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/NativePath.h"
#include "Utility/System/Os.h"

UNIT_TEST(NativePath, Wtf8RoundTrip) {
    // The conversion goes through wchar_t on Windows, so WTF-8 has to survive, unpaired surrogates included.
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

UNIT_TEST(NativePath, Parent) {
    EXPECT_EQ(NativePath("a/b/c.txt").parent(), NativePath("a/b"));
    EXPECT_EQ(NativePath("c.txt").parent(), NativePath()); // No parent means an empty path.
    EXPECT_EQ(NativePath().parent(), NativePath());
}

UNIT_TEST(NativePath, DisplayString) {
    EXPECT_EQ(NativePath::fromWtf8("a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt").displayString(), "a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt");

    // WTF-8-encoded surrogates are not valid UTF-8, so they have to come out as replacement characters.
    std::string display = NativePath::fromWtf8("lol\xed\xb0\x80kek.txt").displayString();
    EXPECT_TRUE(display.starts_with("lol"));
    EXPECT_TRUE(display.ends_with("kek.txt"));
    EXPECT_NE(display.find("\xEF\xBF\xBD"), std::string::npos); // U+FFFD.
    EXPECT_EQ(display.find("\xed\xb0\x80"), std::string::npos);
}

#ifndef _WINDOWS
UNIT_TEST(NativePath, InvalidUtf8RoundTrip) {
    // File names on POSIX are byte strings, so fromWtf8 / toWtf8 have to pass invalid UTF-8 through as-is. "\xD0" is
    // an incomplete UTF-8 sequence, "\xFF" can't appear in UTF-8 at all.
    for (std::string_view name : {"lol\xD0kek.txt", "lol\xFFkek.txt", "trailing\xD0"})
        EXPECT_EQ(NativePath::fromWtf8(name).toWtf8(), name);
}
#endif

#if !defined(_WINDOWS) && !defined(__APPLE__)
UNIT_TEST(NativePath, InvalidUtf8FileNames) {
    // A name with invalid UTF-8 in it is not just convertible, but is also usable to actually open a file. Not on
    // APFS though - it only takes file names that are valid UTF-8, thus no MacOS here. And we write into the temp
    // dir b/c the build dir can be on an APFS-backed mount in a dev container.
    NativePath tmpDir = NativePath::fromStdPath(std::filesystem::temp_directory_path());

    for (std::string_view name : {"tmp_lol\xD0kek.txt", "tmp_lol\xFFkek.txt", "tmp_trailing\xD0"}) {
        NativePath path = tmpDir / NativePath::fromWtf8(name);

        std::ofstream stream(path.toStdPath());
        ASSERT_TRUE(stream.is_open()) << name;
        stream << "lol";
        stream.close();

        EXPECT_TRUE(os::exists(path)) << name;
        EXPECT_TRUE(os::remove(path)) << name;
    }
}
#endif
