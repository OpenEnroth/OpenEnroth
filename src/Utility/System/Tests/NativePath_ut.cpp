#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/NativePath.h"
#include "Utility/System/Os.h"

UNIT_TEST(NativePath, Wtf8RoundTrip) {
    // The conversion goes through wchar_t on Windows, so WTF-8 has to survive, unpaired surrogates included.
    for (std::string_view path : {"a/b/c.txt", "\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(NativePath(path).string(), path);
}

UNIT_TEST(NativePath, NativeRoundTrip) {
    std::filesystem::path cwd = std::filesystem::current_path();
    EXPECT_EQ(NativePath::fromNative(cwd.native()).native(), cwd.native());
}

UNIT_TEST(NativePath, Literals) {
    // A byte string constructs directly.
    EXPECT_EQ(NativePath("a/b/c.txt"), NativePath("a/b/c.txt"));
    EXPECT_EQ(NativePath(""), NativePath());
}

UNIT_TEST(NativePath, Composition) {
    EXPECT_EQ((NativePath("a/b") / NativePath("c.txt")).string(), "a/b/c.txt");
    EXPECT_EQ((NativePath("a/b/") / NativePath("c.txt")).string(), "a/b/c.txt"); // No doubled separator.
    EXPECT_EQ((NativePath() / NativePath("c.txt")).string(), "c.txt");

    // A rooted tail replaces the head instead of being appended to it.
    EXPECT_EQ((NativePath("a/b") / NativePath("/c.txt")).string(), "/c.txt");
}

UNIT_TEST(NativePath, WithExtension) {
    EXPECT_EQ(NativePath("a/b.json").withExtension(".mm7").string(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b").withExtension(".mm7").string(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b.json").withExtension("").string(), "a/b");
    EXPECT_EQ(NativePath("a/b.json").withExtension("mm7").string(), "a/b.mm7"); // The leading dot is optional.
    EXPECT_EQ(NativePath("a.tar.gz").withExtension(".zip").string(), "a.tar.zip"); // Only the last extension goes.
    EXPECT_EQ(NativePath("a/.bashrc").withExtension(".txt").string(), "a/.bashrc.txt"); // A dotfile has no extension.
    EXPECT_EQ(NativePath("a.d/b").withExtension(".txt").string(), "a.d/b.txt"); // Dots in directory names don't count.
}

#ifdef _WINDOWS
UNIT_TEST(NativePath, WindowsRoots) {
    EXPECT_EQ(NativePath("a\\b").string(), "a/b"); // Both slashes separate components on Windows.

    EXPECT_EQ((NativePath("C:/a") / NativePath("D:/b")).string(), "D:/b"); // Another drive replaces everything.
    EXPECT_EQ((NativePath("C:/a") / NativePath("/b")).string(), "C:/b"); // A rooted tail keeps our drive.
    EXPECT_EQ((NativePath("C:/a") / NativePath("C:b")).string(), "C:/a/b"); // Same drive, so it's a plain append.
    EXPECT_EQ((NativePath("C:") / NativePath("b")).string(), "C:b"); // Drive-relative, no separator inserted.
    EXPECT_EQ((NativePath("//server/share") / NativePath("f")).string(), "//server/share/f");

    // A bare drive letter is drive-relative, but a bare share name is already absolute. So a separator does go in
    // after it, and it replaces whatever it's appended to.
    EXPECT_EQ((NativePath("//server") / NativePath("share")).string(), "//server/share");
    EXPECT_EQ((NativePath("//server/share") / NativePath("//server")).string(), "//server");
}
#endif

// std::filesystem defers normalization to lexically_normal(), and even then keeps a trailing separator and spells
// "here" as ".". NativePath does neither - see the class docs. Those two rules are the whole divergence, so folding
// them into the oracle keeps everything else - the ".." collapse, "." removal, separator dedup, root clamping -
// genuinely checked against std::filesystem.
static std::string oracle(const std::filesystem::path &path) {
    std::filesystem::path normal = path.lexically_normal();
    std::string result = normal.generic_string();

    if (result.size() > normal.root_path().generic_string().size() && result.back() == '/')
        result.pop_back();
    if (result == ".")
        result.clear();

    return result;
}

UNIT_TEST(NativePath, NormalForm) {
    // Every NativePath is in lexical normal form, and these are the rules. They're pinned here rather than left to
    // the std::filesystem oracle because two of them are deliberate divergences from it.
    EXPECT_EQ(NativePath(".").string(), ""); // "Here" is the empty path.
    EXPECT_EQ(NativePath("a/./b").string(), "a/b");
    EXPECT_EQ(NativePath("a//b/").string(), "a/b"); // Separators collapse, trailing one goes.
    EXPECT_EQ(NativePath("a/b/../c").string(), "a/c");
    EXPECT_EQ(NativePath("../a").string(), "../a"); // A leading ".." run survives - it's escaping.
    EXPECT_EQ(NativePath("a/../../b").string(), "../b");
    EXPECT_EQ(NativePath("/..").string(), "/"); // Clamped by the root, same as POSIX defines it.
    EXPECT_EQ(NativePath("/../../a").string(), "/a");

    // Normalization is idempotent.
    for (std::string_view path : {"", ".", "..", "a/./b", "a//b/", "a/b/../c", "../a", "/.."})
        EXPECT_EQ(NativePath(NativePath(path).string()), NativePath(path)) << path;
}

UNIT_TEST(NativePath, Dichotomy) {
    // root() is the primitive, and isAbsolute / isRelative are exact complements of it on every path.
    for (std::string_view path : {"", ".", "..", "a", "a/b", "/", "/a", "C:x", "a\\b"}) {
        NativePath nativePath(path);
        EXPECT_EQ(nativePath.isAbsolute(), !nativePath.root().empty()) << path;
        EXPECT_EQ(nativePath.isRelative(), !nativePath.isAbsolute()) << path;
    }

    EXPECT_TRUE(NativePath("/lol").isAbsolute()); // On every platform, Windows included.
    EXPECT_EQ(NativePath("/lol").root(), "/");
    EXPECT_TRUE(NativePath("a/b").isRelative());
    EXPECT_EQ(NativePath("a/b").root(), "");

    // A rooted tail replaces the head, so the absolute intent survives the join instead of being laundered away.
    EXPECT_EQ((NativePath("base") / NativePath("/x")).string(), "/x");

    // A bare drive letter is not root syntax - there's no cross-platform meaning to preserve.
    EXPECT_TRUE(NativePath("C:x").isRelative());
    EXPECT_EQ(NativePath("C:x").name(), "C:x");
}

UNIT_TEST(NativePath, Anatomy) {
    EXPECT_EQ(NativePath("a/b.txt").name(), "b.txt");
    EXPECT_EQ(NativePath("a/b.txt").stem(), "b");
    EXPECT_EQ(NativePath("a/b.txt").extension(), ".txt");

    EXPECT_EQ(NativePath("a/.bashrc").extension(), ""); // A leading dot doesn't start an extension.
    EXPECT_EQ(NativePath("a/.bashrc").stem(), ".bashrc");
    EXPECT_EQ(NativePath("a.tar.gz").extension(), ".gz"); // Only the last one counts.
    EXPECT_EQ(NativePath("a.tar.gz").stem(), "a.tar");
    EXPECT_EQ(NativePath("a.d/b").extension(), ""); // Dots in directory names don't count.

    EXPECT_EQ(NativePath("").name(), "");
    EXPECT_EQ(NativePath("/").name(), "");
}

UNIT_TEST(NativePath, Escaping) {
    EXPECT_TRUE(NativePath("..").isEscaping());
    EXPECT_TRUE(NativePath("../a").isEscaping());
    EXPECT_TRUE(NativePath("a/../../b").isEscaping()); // Normalizes to "../b".

    EXPECT_FALSE(NativePath("").isEscaping());
    EXPECT_FALSE(NativePath("a/../b").isEscaping()); // Normalizes to "b".
    EXPECT_FALSE(NativePath("..a").isEscaping()); // A file that merely starts with dots.
    EXPECT_FALSE(NativePath("/..").isEscaping()); // Clamped by the root, so nothing escapes.
}

UNIT_TEST(NativePath, LexicalOpsMatchStdFilesystem) {
    // Path manipulation is ours now instead of std::filesystem's, so check it against std::filesystem as an oracle.
    // Every string here is ASCII on purpose - on Windows std::filesystem::path converts narrow strings per the C
    // locale, so anything else would be comparing against an oracle that mangles its input.
    std::vector<std::string> paths = {
        "", ".", "..", "a", "a/", "/a", "a/b", "a/b/", "/", "a.txt", ".bashrc", "a.tar.gz", "a.d/b", "/a/b.c",
        "a/./b", "a//b", "a/b/../c", "../a", "/..", "a/../.."
    };

    // Root names and backslash separators exist on Windows only. POSIX says a path starting with exactly two slashes
    // is implementation-defined, and NativePath preserves it there instead of reading a root name out of it.
#ifdef _WINDOWS
    for (std::string_view windowsPath : {"C:/a", "D:/b", "//server", "//server/share", "a\\b"})
        paths.emplace_back(windowsPath);
#endif

    for (const std::string &head : paths) {
        std::string normal = oracle(head);
        EXPECT_EQ(NativePath(head).string(), normal) << "normalizing '" << head << "'";

        for (const std::string &tail : paths)
            EXPECT_EQ((NativePath(head) / NativePath(tail)).string(),
                      oracle(std::filesystem::path(head) / std::filesystem::path(tail)))
                << "'" << head << "' / '" << tail << "'";

        // Against the normal form, so that the oracle's retained trailing separator doesn't shift the answer.
        EXPECT_EQ(NativePath(head).parent().string(), oracle(std::filesystem::path(normal).parent_path()))
            << "parent of '" << head << "'";

        for (std::string_view extension : {"", ".x", "x", ".tar.gz"}) {
            std::filesystem::path expected = std::filesystem::path(normal);
            expected.replace_extension(extension);
            EXPECT_EQ(NativePath(head).withExtension(extension).string(), oracle(expected))
                << "'" << head << "' + '" << extension << "'";
        }
    }
}

UNIT_TEST(NativePath, Parent) {
    EXPECT_EQ(NativePath("a/b/c.txt").parent(), NativePath("a/b"));
    EXPECT_EQ(NativePath("c.txt").parent(), NativePath()); // No parent means an empty path.
    EXPECT_EQ(NativePath().parent(), NativePath());
}

UNIT_TEST(NativePath, DisplayString) {
    EXPECT_EQ(NativePath("a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt").displayString(), "a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt");

    // WTF-8-encoded surrogates are not valid UTF-8, so they have to come out as replacement characters.
    std::string display = NativePath("lol\xed\xb0\x80kek.txt").displayString();
    EXPECT_TRUE(display.starts_with("lol"));
    EXPECT_TRUE(display.ends_with("kek.txt"));
    EXPECT_NE(display.find("\xEF\xBF\xBD"), std::string::npos); // U+FFFD.
    EXPECT_EQ(display.find("\xed\xb0\x80"), std::string::npos);
}

#ifndef _WINDOWS
UNIT_TEST(NativePath, InvalidUtf8RoundTrip) {
    // File names on POSIX are byte strings, so construction has to pass invalid UTF-8 through as-is. "\xD0" is
    // an incomplete UTF-8 sequence, "\xFF" can't appear in UTF-8 at all.
    for (std::string_view name : {"lol\xD0kek.txt", "lol\xFFkek.txt", "trailing\xD0"})
        EXPECT_EQ(NativePath(name).string(), name);
}
#endif

#if !defined(_WINDOWS) && !defined(__APPLE__)
UNIT_TEST(NativePath, InvalidUtf8FileNames) {
    // A name with invalid UTF-8 in it is not just convertible, but is also usable to actually open a file. Not on
    // APFS though - it only takes file names that are valid UTF-8, thus no MacOS here. And we write into the temp
    // dir b/c the build dir can be on an APFS-backed mount in a dev container.
    NativePath tmpDir = NativePath::fromNative(std::filesystem::temp_directory_path().native());

    for (std::string_view name : {"tmp_lol\xD0kek.txt", "tmp_lol\xFFkek.txt", "tmp_trailing\xD0"}) {
        NativePath path = tmpDir / NativePath(name);

        std::ofstream stream(path.native().c_str());
        ASSERT_TRUE(stream.is_open()) << name;
        stream << "lol";
        stream.close();

        EXPECT_TRUE(os::exists(path)) << name;
        EXPECT_TRUE(os::remove(path)) << name;
    }
}
#endif
