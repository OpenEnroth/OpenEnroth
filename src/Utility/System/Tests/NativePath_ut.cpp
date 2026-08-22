#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

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

UNIT_TEST(NativePath, Literals) {
    // ASCII literals construct directly, everything else goes through fromWtf8.
    EXPECT_EQ(NativePath("a/b/c.txt"), NativePath::fromWtf8("a/b/c.txt"));
    EXPECT_EQ(NativePath(""), NativePath());
}

UNIT_TEST(NativePath, Composition) {
    EXPECT_EQ((NativePath("a/b") / NativePath("c.txt")).toWtf8(), "a/b/c.txt");
    EXPECT_EQ((NativePath("a/b/") / NativePath("c.txt")).toWtf8(), "a/b/c.txt"); // No doubled separator.
    EXPECT_EQ((NativePath() / NativePath("c.txt")).toWtf8(), "c.txt");

    // A rooted tail replaces the head instead of being appended to it.
    EXPECT_EQ((NativePath("a/b") / NativePath("/c.txt")).toWtf8(), "/c.txt");
}

UNIT_TEST(NativePath, WithExtension) {
    EXPECT_EQ(NativePath("a/b.json").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b.json").withExtension("").toWtf8(), "a/b");
    EXPECT_EQ(NativePath("a/b.json").withExtension("mm7").toWtf8(), "a/b.mm7"); // The leading dot is optional.
    EXPECT_EQ(NativePath("a.tar.gz").withExtension(".zip").toWtf8(), "a.tar.zip"); // Only the last extension goes.
    EXPECT_EQ(NativePath("a/.bashrc").withExtension(".txt").toWtf8(), "a/.bashrc.txt"); // A dotfile has no extension.
    EXPECT_EQ(NativePath("a.d/b").withExtension(".txt").toWtf8(), "a.d/b.txt"); // Dots in directory names don't count.
}

#ifdef _WINDOWS
UNIT_TEST(NativePath, WindowsRoots) {
    EXPECT_EQ(NativePath::fromWtf8("a\\b").toWtf8(), "a/b"); // Both slashes separate components on Windows.

    EXPECT_EQ((NativePath("C:/a") / NativePath("D:/b")).toWtf8(), "D:/b"); // Another drive replaces everything.
    EXPECT_EQ((NativePath("C:/a") / NativePath("/b")).toWtf8(), "C:/b"); // A rooted tail keeps our drive.
    EXPECT_EQ((NativePath("C:/a") / NativePath("C:b")).toWtf8(), "C:/a/b"); // Same drive, so it's a plain append.
    EXPECT_EQ((NativePath("C:") / NativePath("b")).toWtf8(), "C:b"); // Drive-relative, no separator inserted.
    EXPECT_EQ((NativePath("//server/share") / NativePath("f")).toWtf8(), "//server/share/f");
}
#endif

UNIT_TEST(NativePath, LexicalOpsMatchStdFilesystem) {
    // Path manipulation is ours now instead of std::filesystem's, so check it against std::filesystem as an oracle.
    // Every string here is ASCII on purpose - on Windows std::filesystem::path converts narrow strings per the C
    // locale, so anything else would be comparing against an oracle that mangles its input.
    std::vector<std::string> paths = {
        "", ".", "..", "a", "a/", "/a", "a/b", "a/b/", "/", "a.txt", ".bashrc", "a.tar.gz", "a.d/b", "/a/b.c"
    };

    // Root names and backslash separators exist on Windows only. POSIX says a path starting with exactly two slashes
    // is implementation-defined, and libstdc++ reads it as a root name, while NativePath never does.
#ifdef _WINDOWS
    for (std::string_view windowsPath : {"C:", "C:a", "C:/a", "D:/b", "//server", "//server/share", "a\\b"})
        paths.emplace_back(windowsPath);
#endif

    for (const std::string &head : paths) {
        for (const std::string &tail : paths)
            EXPECT_EQ((NativePath::fromWtf8(head) / NativePath::fromWtf8(tail)).toWtf8(),
                      (std::filesystem::path(head) / std::filesystem::path(tail)).generic_string())
                << "'" << head << "' / '" << tail << "'";

        for (std::string_view extension : {"", ".x", "x", ".tar.gz"}) {
            std::filesystem::path expected = std::filesystem::path(head);
            expected.replace_extension(extension);
            EXPECT_EQ(NativePath::fromWtf8(head).withExtension(extension).toWtf8(), expected.generic_string())
                << "'" << head << "' + '" << extension << "'";
        }
    }
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
