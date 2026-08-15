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

UNIT_TEST(NativePath, Literals) {
    // ASCII literals construct directly, everything else goes through fromWtf8.
    EXPECT_EQ(NativePath("a/b/c.txt"), NativePath::fromWtf8("a/b/c.txt"));
    EXPECT_EQ(NativePath(""), NativePath());
}

UNIT_TEST(NativePath, NativeRoundTrip) {
    // The native form is wchar_t on Windows, so WTF-8 has to survive the round trip there too.
    for (std::string_view path : {"a/b/c.txt", "\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(NativePath::fromNative(NativePath::fromWtf8(path).native()).toWtf8(), path);
}

UNIT_TEST(NativePath, Composition) {
    EXPECT_EQ((NativePath("a/b") / NativePath("c.txt")).toWtf8(), "a/b/c.txt");
}

UNIT_TEST(NativePath, WithExtension) {
    EXPECT_EQ(NativePath("a/b.json").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b.json").withExtension("").toWtf8(), "a/b");
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
UNIT_TEST(NativePath, InvalidUtf8FileNames) {
    // File names on Linux are byte strings, so fromWtf8 / toWtf8 have to pass invalid UTF-8 through as-is. "\xD0" is
    // an incomplete UTF-8 sequence, "\xFF" can't appear in UTF-8 at all.
    for (std::string_view name : {"tmp_lol\xD0kek.txt", "tmp_lol\xFFkek.txt", "tmp_trailing\xD0"}) {
        NativePath path = NativePath::fromWtf8(name);
        EXPECT_EQ(path.toWtf8(), name);

        // APFS rejects such names, and so do APFS-backed mounts in a dev container, so we skip instead of asserting.
        std::ofstream stream(path.native());
        if (!stream.is_open())
            GTEST_SKIP() << "File system rejected an invalid UTF-8 name.";
        stream << "lol";
        stream.close();

        EXPECT_TRUE(os::exists(path)) << name;
        EXPECT_TRUE(os::remove(path)) << name;
    }
}
#endif
