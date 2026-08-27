#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/PathView.h"
#include "Utility/System/Fs.h"

UNIT_TEST(Path, Wtf8RoundTrip) {
    // The conversion goes through wchar_t on Windows, so WTF-8 has to survive, unpaired surrogates included.
    for (std::string_view path : {"a/b/c.txt", "\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(Path(path).string(), path);
}

UNIT_TEST(Path, NativeRoundTrip) {
    // The wide conversion on Windows has to be lossless both ways, or paths stop round-tripping through the OS.
    // Not byte-identical though - what comes back from the OS has backslashes in it, and those are separators.
    Path path = Path::fromNative(std::filesystem::current_path().native());
    EXPECT_EQ(Path::fromNative(path.native()), path);
    EXPECT_FALSE(path.isEmpty());
}

UNIT_TEST(Path, StringConversions) {
    // A literal, a std::string and a string_view all land on the same Path - the constructor is the only conversion
    // there is now that fromWtf8 is gone.
    std::string string = "a/b/c.txt";
    EXPECT_EQ(Path("a/b/c.txt"), Path(string));
    EXPECT_EQ(Path("a/b/c.txt"), Path(std::string_view(string)));
    EXPECT_EQ(Path(""), Path());
}

UNIT_TEST(Path, Composition) {
    EXPECT_EQ((Path("a/b") / Path("c.txt")).string(), "a/b/c.txt");
    EXPECT_EQ((Path("a/b/") / Path("c.txt")).string(), "a/b/c.txt"); // No doubled separator.
    EXPECT_EQ((Path() / Path("c.txt")).string(), "c.txt");

    // A rooted tail replaces the head instead of being appended to it.
    EXPECT_EQ((Path("a/b") / Path("/c.txt")).string(), "/c.txt");
}

UNIT_TEST(Path, WithExtension) {
    EXPECT_EQ(Path("a/b.json").withExtension(".mm7").string(), "a/b.mm7");
    EXPECT_EQ(Path("a/b").withExtension(".mm7").string(), "a/b.mm7");
    EXPECT_EQ(Path("a/b.json").withExtension("").string(), "a/b");
    EXPECT_EQ(Path("a/b.json").withExtension("mm7").string(), "a/b.mm7"); // The leading dot is optional.
    EXPECT_EQ(Path("a.tar.gz").withExtension(".zip").string(), "a.tar.zip"); // Only the last extension goes.
    EXPECT_EQ(Path("a/.bashrc").withExtension(".txt").string(), "a/.bashrc.txt"); // A dotfile has no extension.
    EXPECT_EQ(Path("a.d/b").withExtension(".txt").string(), "a.d/b.txt"); // Dots in directory names don't count.
}

#ifdef _WINDOWS
UNIT_TEST(Path, WindowsRoots) {
    EXPECT_EQ(Path("a\\b").string(), "a/b"); // Both slashes separate components on Windows.

    EXPECT_EQ((Path("C:/a") / Path("D:/b")).string(), "D:/b"); // Another drive replaces everything.
    EXPECT_EQ((Path("//server/share") / Path("f")).string(), "//server/share/f");
    EXPECT_EQ((Path("//server") / Path("share")).string(), "//server/share");

    // A rooted tail replaces the head whatever its root is, so an absolute intent survives the join instead of
    // being laundered into a relative one. std::filesystem instead keeps the head's drive here, giving "C:/b".
    EXPECT_EQ((Path("C:/a") / Path("/b")).string(), "/b");

    // A bare drive letter is not root syntax - there is no cross-platform meaning to preserve - so it joins as an
    // ordinary segment. Which means a relative head can concatenate into something that parses absolute. That is
    // degenerate but inert, since the boundary rejects the ingredients and the result alike.
    EXPECT_EQ((Path("C:/a") / Path("C:b")).string(), "C:/a/C:b");
    EXPECT_EQ((Path("C:") / Path("b")).string(), "C:/b");
    EXPECT_TRUE((Path("C:") / Path("b")).isAbsolute());

    EXPECT_EQ(Path("//").string(), "/"); // Only "//" followed by a share name is root syntax here.
    EXPECT_EQ(Path("//server/x").parent(), Path("//server")); // The share name is one value, however it's spelled.
    EXPECT_EQ(Path("//server").string(), "//server/"); // A root carries its separator.
}
#endif

// std::filesystem defers normalization to lexically_normal(), and even then keeps a trailing separator and spells
// "here" as ".". Path does neither - see the class docs. Those two rules are the whole divergence, so folding
// them into the oracle keeps everything else - the ".." collapse, "." removal, separator dedup, root clamping -
// genuinely checked against std::filesystem.
static std::string oracle(const std::filesystem::path &path) {
    std::filesystem::path normal = path.lexically_normal();
    std::string result = normal.generic_string();

    if (result.size() > normal.root_path().generic_string().size() && result.back() == '/')
        result.pop_back();
    if (result == ".")
        result.clear();

#ifdef _WINDOWS
    // A root carries its separator for us, so that "//server" and "//server/" are one path. std::filesystem keeps
    // them distinct, and hands back a bare share name without one. There is no canonical form that agrees with it
    // on both spellings, which is the price of making them equal.
    if (result.starts_with("//") && result.find('/', 2) == std::string::npos)
        result += '/';
#endif

    return result;
}

UNIT_TEST(Path, NormalForm) {
    // Every Path is in lexical normal form, and these are the rules. They're pinned here rather than left to
    // the std::filesystem oracle because two of them are deliberate divergences from it.
    EXPECT_EQ(Path(".").string(), ""); // "Here" is the empty path.
    EXPECT_EQ(Path("a/./b").string(), "a/b");
    EXPECT_EQ(Path("a//b/").string(), "a/b"); // Separators collapse, trailing one goes.
    EXPECT_EQ(Path("a/b/../c").string(), "a/c");
    EXPECT_EQ(Path("../a").string(), "../a"); // A leading ".." run survives - it's escaping.
    EXPECT_EQ(Path("a/../../b").string(), "../b");
    EXPECT_EQ(Path("/..").string(), "/"); // Clamped by the root, same as POSIX defines it.
    EXPECT_EQ(Path("/../../a").string(), "/a");

    // Normalization is idempotent.
    for (std::string_view path : {"", ".", "..", "a/./b", "a//b/", "a/b/../c", "../a", "/.."})
        EXPECT_EQ(Path(Path(path).string()), Path(path)) << path;
}

#ifndef _WINDOWS
UNIT_TEST(Path, PosixDoubleSlashRoot) {
    // POSIX says a path starting with exactly two slashes is implementation-defined, so we leave it alone instead
    // of reading a root name out of it the way libstdc++ does. Three or more slashes are just a root directory.
    EXPECT_EQ(Path("//a").string(), "//a");
    EXPECT_EQ(Path("//a").root(), "//");
    EXPECT_EQ(Path("///a").string(), "/a");
    EXPECT_EQ(Path("///a").root(), "/");
    EXPECT_EQ(Path("//a/../b").string(), "//b"); // Clamped by the root like any other absolute path.
}
#endif

UNIT_TEST(Path, Dichotomy) {
    // root() is the primitive, and isAbsolute / isRelative are exact complements of it on every path.
    for (std::string_view path : {"", ".", "..", "a", "a/b", "/", "/a", "C:x", "a\\b"}) {
        Path testPath(path);
        EXPECT_EQ(testPath.isAbsolute(), !testPath.root().empty()) << path;
        EXPECT_EQ(testPath.isRelative(), !testPath.isAbsolute()) << path;
    }

    EXPECT_TRUE(Path("/lol").isAbsolute()); // On every platform, Windows included.
    EXPECT_EQ(Path("/lol").root(), "/");
    EXPECT_TRUE(Path("a/b").isRelative());
    EXPECT_EQ(Path("a/b").root(), "");

    // A rooted tail replaces the head, so the absolute intent survives the join instead of being laundered away.
    EXPECT_EQ((Path("base") / Path("/x")).string(), "/x");

    // A bare drive letter is not root syntax - there's no cross-platform meaning to preserve.
    EXPECT_TRUE(Path("C:x").isRelative());
    EXPECT_EQ(Path("C:x").name(), "C:x");
}

UNIT_TEST(Path, Anatomy) {
    EXPECT_EQ(Path("a/b.txt").name(), "b.txt");
    EXPECT_EQ(Path("a/b.txt").stem(), "b");
    EXPECT_EQ(Path("a/b.txt").extension(), ".txt");

    EXPECT_EQ(Path("a/.bashrc").extension(), ""); // A leading dot doesn't start an extension.
    EXPECT_EQ(Path("a/.bashrc").stem(), ".bashrc");
    EXPECT_EQ(Path("a.tar.gz").extension(), ".gz"); // Only the last one counts.
    EXPECT_EQ(Path("a.tar.gz").stem(), "a.tar");
    EXPECT_EQ(Path("a.d/b").extension(), ""); // Dots in directory names don't count.

    EXPECT_EQ(Path("").name(), "");
    EXPECT_EQ(Path("/").name(), "");
}

UNIT_TEST(Path, DottedNames) {
    // A name whose stem would be all dots has no extension, so that dropping the extension can't turn a name into a
    // navigation token - the stem of "..." is "..", which would collapse "a/..." to nothing. This is reachable from
    // data, not just from a hostile caller: a trace file named "...json" comes off disk and RetraceTest calls
    // withExtension("") on every name it lists. std::filesystem parses these the other way round, which is why they
    // are pinned here instead of being left to the oracle.
    EXPECT_EQ(Path("a/...").extension(), "");
    EXPECT_EQ(Path("a/...").stem(), "...");
    EXPECT_EQ(Path("a/...a").extension(), "");
    EXPECT_EQ(Path("...json").extension(), "");

    EXPECT_EQ(Path("a/...").withExtension(""), Path("a/..."));
    EXPECT_EQ(Path("a/...a").withExtension(""), Path("a/...a"));
    EXPECT_EQ(Path("...json").withExtension(""), Path("...json"));
    EXPECT_EQ(Path("a/...").withExtension(".x"), Path("a/....x"));

    // A stem that isn't all dots still splits normally.
    EXPECT_EQ(Path("..a.txt").extension(), ".txt");
    EXPECT_EQ(Path("..a.txt").withExtension(""), Path("..a"));
}

UNIT_TEST(Path, WithExtensionIsTotal) {
    // Path never throws and never asserts, and an extension can come from user data, so every argument has to have a
    // defined result. A path-shaped one extends the path instead of just the name, which is what std::filesystem
    // does too - the alternative would be silently mangling the argument. The escaping case reports itself as
    // escaping, which is what the file system layer refuses on.
    EXPECT_EQ(Path("a/b").withExtension("/c/d/e"), Path("a/b./c/d/e"));
    EXPECT_EQ(Path("a").withExtension("./../../x"), Path("../x"));
    EXPECT_TRUE(Path("a").withExtension("./../../x").isEscaping());
    EXPECT_EQ(Path("a").withExtension("/x"), Path("a./x"));
    EXPECT_EQ(Path("a").withExtension("../../.."), Path(".."));

    // Whatever the argument, the result is still in normal form.
    for (std::string_view extension : {"", ".x", "x", "./../../x", "/x", "../../..", "a/b"}) {
        Path path = Path("a/b.txt").withExtension(extension);
        EXPECT_EQ(Path(path.string()), path) << extension;
    }
}

UNIT_TEST(Path, Split) {
    // The root is not a segment, so a path is its root joined with its segments. Under normal form "." never shows
    // up as one, and ".." only as the leading run of an escaping path.
    auto segments = [] (const Path &path) {
        std::vector<std::string> result;
        for (std::string_view chunk : path.split())
            result.emplace_back(chunk);
        return result;
    };

    EXPECT_EQ(segments(Path("a/b/c")), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(segments(Path("/a/b")), std::vector<std::string>({"a", "b"})); // Root dropped.
    EXPECT_EQ(segments(Path("a/./b")), std::vector<std::string>({"a", "b"}));
    EXPECT_EQ(segments(Path("../a")), std::vector<std::string>({"..", "a"}));
    EXPECT_TRUE(segments(Path("")).empty());
    EXPECT_TRUE(segments(Path(".")).empty()); // "." is the empty path.
    EXPECT_TRUE(segments(Path("/")).empty()); // A bare root has no segments.
}

UNIT_TEST(Path, SplitTails) {
    // tailAt / tailAfter slice the original buffer, so they're O(1) and the result is a view into it. That's what
    // the file system trie walks on.
    Path path("a/b/c");

    for (std::string_view chunk : path.split()) {
        if (chunk == "b") {
            EXPECT_EQ(path.split().tailAt(chunk).string(), "b/c");
            EXPECT_EQ(path.split().tailAfter(chunk).string(), "c");
            EXPECT_EQ(path.split().tailAt(chunk).string().data(), path.string().data() + 2); // Same buffer.
        }
        if (chunk == "c")
            EXPECT_TRUE(path.split().tailAfter(chunk).isEmpty()); // Nothing after the last one.
    }

    EXPECT_EQ(path.split().tailAfter(std::string_view()).string(), "a/b/c"); // Empty chunk means the whole path.
}

UNIT_TEST(PathView, FromPath) {
    Path path("a/b");
    PathView view = path; // Implicit.

    EXPECT_EQ(view.string(), "a/b");
    EXPECT_FALSE(view.isEmpty());
    EXPECT_FALSE(view.isEscaping());
    EXPECT_TRUE(PathView(Path("../a")).isEscaping());
    EXPECT_TRUE(PathView().isEmpty());
}

UNIT_TEST(Path, Escaping) {
    EXPECT_TRUE(Path("..").isEscaping());
    EXPECT_TRUE(Path("../a").isEscaping());
    EXPECT_TRUE(Path("a/../../b").isEscaping()); // Normalizes to "../b".

    EXPECT_FALSE(Path("").isEscaping());
    EXPECT_FALSE(Path("a/../b").isEscaping()); // Normalizes to "b".
    EXPECT_FALSE(Path("..a").isEscaping()); // A file that merely starts with dots.
    EXPECT_FALSE(Path("/..").isEscaping()); // Clamped by the root, so nothing escapes.
}

UNIT_TEST(Path, LexicalOpsMatchStdFilesystem) {
    // Path manipulation is ours now instead of std::filesystem's, so check it against std::filesystem as an oracle.
    // Every string here is ASCII on purpose - on Windows std::filesystem::path converts narrow strings per the C
    // locale, so anything else would be comparing against an oracle that mangles its input.
    std::vector<std::string> paths = {
        "", ".", "..", "a", "a/", "/a", "a/b", "a/b/", "/", "a.txt", ".bashrc", "a.tar.gz", "a.d/b", "/a/b.c",
        "a/./b", "a//b", "a/b/../c", "../a", "/..", "a/../.."
    };

    // Root names and backslash separators exist on Windows only. POSIX says a path starting with exactly two slashes
    // is implementation-defined, and Path preserves it there instead of reading a root name out of it.
#ifdef _WINDOWS
    for (std::string_view windowsPath : {"C:/a", "D:/b", "//server", "//server/share", "a\\b"})
        paths.emplace_back(windowsPath);
#endif

    for (const std::string &head : paths) {
        std::string normal = oracle(head);
        EXPECT_EQ(Path(head).string(), normal) << "normalizing '" << head << "'";

        for (const std::string &tail : paths) {
            // Our rule is that any rooted tail replaces the head. std::filesystem instead keeps the head's root
            // name when the tail has a root directory but no root name, so on Windows it turns "/b" into "C:/b" -
            // laundering an absolute path into a relative one, which is exactly what we don't want.
            std::string expected = Path(tail).isAbsolute()
                ? oracle(tail)
                : oracle(std::filesystem::path(head) / std::filesystem::path(tail));

            EXPECT_EQ((Path(head) / Path(tail)).string(), expected) << "'" << head << "' / '" << tail << "'";
        }

        // Against the normal form, so that the oracle's retained trailing separator doesn't shift the answer.
        EXPECT_EQ(Path(head).parent().string(), oracle(std::filesystem::path(normal).parent_path()))
            << "parent of '" << head << "'";

        for (std::string_view extension : {"", ".x", "x", ".tar.gz"}) {
            std::filesystem::path expected = std::filesystem::path(normal);
            expected.replace_extension(extension);
            EXPECT_EQ(Path(head).withExtension(extension).string(), oracle(expected))
                << "'" << head << "' + '" << extension << "'";
        }
    }
}

UNIT_TEST(Path, Parent) {
    EXPECT_EQ(Path("/a").parent(), Path("/")); // Clamped by the root, not emptied.
    EXPECT_EQ(Path("../..").parent(), Path("..")); // The lexical parent, which is not the semantic one.

    EXPECT_EQ(Path("a/b/c.txt").parent(), Path("a/b"));
    EXPECT_EQ(Path("c.txt").parent(), Path()); // No parent means an empty path.
    EXPECT_EQ(Path().parent(), Path());
}

UNIT_TEST(Path, DisplayString) {
    EXPECT_EQ(Path("a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt").displayString(), "a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt");

    // WTF-8-encoded surrogates are not valid UTF-8, so they have to come out as replacement characters.
    std::string display = Path("lol\xed\xb0\x80kek.txt").displayString();
    EXPECT_TRUE(display.starts_with("lol"));
    EXPECT_TRUE(display.ends_with("kek.txt"));
    EXPECT_NE(display.find("\xEF\xBF\xBD"), std::string::npos); // U+FFFD.
    EXPECT_EQ(display.find("\xed\xb0\x80"), std::string::npos);
}

#ifndef _WINDOWS
UNIT_TEST(Path, InvalidUtf8RoundTrip) {
    // File names on POSIX are byte strings, so construction has to pass invalid UTF-8 through as-is. "\xD0" is
    // an incomplete UTF-8 sequence, "\xFF" can't appear in UTF-8 at all.
    for (std::string_view name : {"lol\xD0kek.txt", "lol\xFFkek.txt", "trailing\xD0"})
        EXPECT_EQ(Path(name).string(), name);
}
#endif

#if !defined(_WINDOWS) && !defined(__APPLE__)
UNIT_TEST(Path, InvalidUtf8FileNames) {
    // A name with invalid UTF-8 in it is not just convertible, but is also usable to actually open a file. Not on
    // APFS though - it only takes file names that are valid UTF-8, thus no MacOS here. And we write into the temp
    // dir b/c the build dir can be on an APFS-backed mount in a dev container.
    Path tmpDir = Path::fromNative(std::filesystem::temp_directory_path().native());

    for (std::string_view name : {"tmp_lol\xD0kek.txt", "tmp_lol\xFFkek.txt", "tmp_trailing\xD0"}) {
        Path path = tmpDir / Path(name);

        std::ofstream stream(path.native().c_str());
        ASSERT_TRUE(stream.is_open()) << name;
        stream << "lol";
        stream.close();

        EXPECT_TRUE(fs::exists(path)) << name;
        EXPECT_TRUE(fs::remove(path)) << name;
    }
}
#endif
