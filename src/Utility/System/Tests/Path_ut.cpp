#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/PathView.h"
#include "Utility/System/Fs.h"

UNIT_TEST(Path, ConstructorKeepsBytes) {
    // The constructor stores what it was given. Normal form is something you ask for, because a path bound for the
    // OS should keep its dots - the OS resolves them itself, and through symlinks, which a lexical pass can't.
    EXPECT_EQ(Path("a/b/../c").string(), "a/b/../c");
    EXPECT_EQ(Path("a/b/../c").normalized().string(), "a/c");

    EXPECT_FALSE(Path(".").isEmpty()); // "." is a path, not nothing - startup reads an empty path as "not supplied".
    EXPECT_TRUE(Path(".").normalized().isEmpty());

    EXPECT_EQ(Path("a/b/").string(), "a/b/"); // A trailing separator survives, which is the POSIX "must be a dir".
    EXPECT_EQ(Path("a/b/").normalized().string(), "a/b");
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
    // operator/ concatenates and nothing more - it does not normalize the seam. Callers that want normal form ask
    // for it, which for file system paths happens once at the public boundary.
    EXPECT_EQ((Path("a/b") / Path("c.txt")).string(), "a/b/c.txt");
    EXPECT_EQ((Path("a/b/") / Path("c.txt")).string(), "a/b/c.txt"); // No doubled separator.
    EXPECT_EQ((Path("a") / Path("../b")).string(), "a/../b");
    EXPECT_EQ((Path("a") / Path()).string(), "a");
    EXPECT_EQ((Path() / Path("a")).string(), "a");

    // A rooted tail replaces the head instead of being appended to it.
    EXPECT_EQ((Path("a/b") / Path("/c.txt")).string(), "/c.txt");
}

UNIT_TEST(Path, WithExtension) {
    EXPECT_EQ(Path("a/b.json").withExtension(".mm7").string(), "a/b.mm7");
    EXPECT_EQ(Path("a/b").withExtension(".mm7").string(), "a/b.mm7");
    EXPECT_EQ(Path("a/b.json").withExtension("").string(), "a/b");
    EXPECT_EQ(Path("a.tar.gz").withExtension(".zip").string(), "a.tar.zip"); // Only the last extension goes.
    EXPECT_EQ(Path("a/.bashrc").withExtension(".txt").string(), "a/.bashrc.txt"); // A dotfile has no extension.
    EXPECT_EQ(Path("a.d/b").withExtension(".txt").string(), "a.d/b.txt"); // Dots in directory names don't count.

    // A path with no file name gains one rather than being renamed. std::filesystem does the same.
    EXPECT_EQ(Path("a/b/").withExtension(".txt").string(), "a/b/.txt");
    EXPECT_EQ(Path("").withExtension(".txt").string(), ".txt");
}

UNIT_TEST(Path, IsExtension) {
    // withExtension asserts this, so it is the whole precondition. A trailing dot is out because Windows trims one
    // and POSIX doesn't, so the same literal would name two different files.
    for (std::string_view extension : {"", ".", ".txt", ".tar.gz", ".txt "})
        EXPECT_TRUE(Path::isExtension(extension)) << extension;
    for (std::string_view extension : {"..", "...", ".txt.", "txt", "/x", ".a/b"})
        EXPECT_FALSE(Path::isExtension(extension)) << extension;

    EXPECT_FALSE(Path::isExtension(std::string_view(".a\0b", 4))); // A NUL can't be in a file name.

    // The identity: every path's own extension is a valid one, so putting it back is a no-op.
    for (std::string_view path : {".", "..", "...", "....", "...a", "..a", "a.", "a..", "a.txt.", "a.tar.gz",
                                  ".bashrc", "..a.txt", "a/b", ""}) {
        EXPECT_TRUE(Path::isExtension(Path(path).extension())) << path;
        EXPECT_EQ(Path(path).withExtension(Path(path).extension()), Path(path)) << path;
    }
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

    EXPECT_EQ(Path("//").normalized().string(), "/"); // Only "//" followed by a share name is root syntax here.
    EXPECT_EQ(Path("//server/x").parent(), Path("//server/")); // The share is one value, however it's spelled.
    EXPECT_EQ(Path("//server").normalized().string(), "//server/"); // A root carries its separator.

    // An extended-length prefix is a root, in the same sense a drive is - Win32 parses nothing inside one, so the
    // volume or device that follows anchors the path and ".." can't climb above it.
    EXPECT_EQ(Path("//?/C:/Games").root(), "//?/C:/");
    EXPECT_EQ(Path("//./COM1").root(), "//./COM1");
    EXPECT_EQ(Path("//?/UNC/server/share/x").root(), "//?/UNC/server/share/");
    EXPECT_EQ(Path("//?/C:/a/../b").normalized().string(), "//?/C:/b");
    EXPECT_EQ(Path("//?/C:/..").normalized().string(), "//?/C:/"); // Clamped, like any other root.

    // A root is copied through normalization verbatim, so something needing normalization can't be part of one -
    // otherwise the dots would reach Win32, which does not resolve them inside an extended-length path.
    EXPECT_EQ(Path("//?/../x").root(), "/");
    EXPECT_EQ(Path("//?/../x").normalized().string(), "/x");
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
    // The rules normalized() applies. Two of them are deliberate divergences from std::filesystem, which is why
    // they're pinned here rather than left to the oracle test below.
    auto normalized = [] (std::string_view path) { return Path(path).normalized().string(); };

    EXPECT_EQ(normalized("."), ""); // "Here" is the empty path.
    EXPECT_EQ(normalized("a/./b"), "a/b");
    EXPECT_EQ(normalized("a//b/"), "a/b"); // Separators collapse, trailing one goes.
    EXPECT_EQ(normalized("a/b/../c"), "a/c");
    EXPECT_EQ(normalized("../a"), "../a"); // A leading ".." run survives - it's escaping.
    EXPECT_EQ(normalized("a/../../b"), "../b");
    EXPECT_EQ(normalized("/.."), "/"); // Clamped by the root, same as POSIX defines it.
    EXPECT_EQ(normalized("/../../a"), "/a");

    for (std::string_view path : {"", ".", "..", "a/./b", "a//b/", "a/b/../c", "../a", "/..", "a/b/"})
        EXPECT_EQ(Path(path).normalized().normalized(), Path(path).normalized()) << path; // Idempotent.

    for (std::string_view path : {"", "a", "a/b", "..", "../a", "/", "/a"})
        EXPECT_TRUE(Path(path).isNormalized()) << path;
    for (std::string_view path : {".", "a/", "a//b", "a/./b", "a/b/../c"})
        EXPECT_FALSE(Path(path).isNormalized()) << path;
}

#ifndef _WINDOWS
UNIT_TEST(Path, PosixDoubleSlashRoot) {
    // POSIX says a path starting with exactly two slashes is implementation-defined, so we leave it alone instead
    // of reading a root name out of it the way libstdc++ does. Three or more slashes are just a root directory.
    EXPECT_EQ(Path("//a").root(), "//");
    EXPECT_EQ(Path("//a").normalized().string(), "//a");
    EXPECT_EQ(Path("///a").root(), "/");
    EXPECT_EQ(Path("///a").normalized().string(), "/a");
    EXPECT_EQ(Path("//a/../b").normalized().string(), "//b"); // Clamped by the root like any other absolute path.
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
    // Decomposition mirrors std::filesystem exactly, all-dot names included. So "..." is a stem of ".." plus an
    // extension of ".", and dropping that extension leaves "..". That is a navigation token, which is why this file
    // used to carry a rule against it - but the rule diverged from std for no gain, since withExtension's
    // precondition is now the guard and every production call site passes a literal.
    EXPECT_EQ(Path("a/...").extension(), ".");
    EXPECT_EQ(Path("a/...").stem(), "..");
    EXPECT_EQ(Path("a/...").withExtension("").string(), "a/..");

    EXPECT_EQ(Path("...a").extension(), ".a");
    EXPECT_EQ(Path("...a").stem(), "..");
    EXPECT_EQ(Path("..a").extension(), ".a");
    EXPECT_EQ(Path("a.").extension(), ".");
    EXPECT_EQ(Path("a.").stem(), "a");

    // The names std exempts outright.
    EXPECT_EQ(Path(".").extension(), "");
    EXPECT_EQ(Path("..").extension(), "");
    EXPECT_EQ(Path(".bashrc").extension(), "");
    EXPECT_EQ(Path("..a.txt").extension(), ".txt");

    // Pinned by the suite this file replaced: a single-dot stem, and a trailing-dot name whose extension is ".".
    EXPECT_EQ(Path("..wat").stem(), ".");
    EXPECT_EQ(Path("..wat").extension(), ".wat");
    EXPECT_EQ(Path("x/y/z/some.").stem(), "some");
    EXPECT_EQ(Path("x/y/z/some.").extension(), ".");
}

UNIT_TEST(Path, WithExtensionPrecondition) {
    // withExtension asserts isExtension, so a path-shaped argument is a programming error rather than something
    // with a defined result. All four production call sites pass ".mm7" or "".
    EXPECT_EQ(Path("a/b.txt").withExtension(".tar.gz").string(), "a/b.tar.gz");
    EXPECT_EQ(Path("a/b.txt").withExtension(".").string(), "a/b.");
    EXPECT_EQ(Path("a/b.txt").withExtension("").string(), "a/b");
}

UNIT_TEST(Path, Split) {
    // The root is not a segment, so a path is its root joined with its segments. split() works on the bytes it is
    // given - it is normal form that makes "." disappear and leaves ".." only as a leading run.
    auto segments = [] (const Path &path) {
        std::vector<std::string> result;
        for (std::string_view chunk : path.split())
            result.emplace_back(chunk);
        return result;
    };

    EXPECT_EQ(segments(Path("a/b/c")), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(segments(Path("/a/b")), std::vector<std::string>({"a", "b"})); // Root dropped.
    EXPECT_EQ(segments(Path("a/./b")), std::vector<std::string>({"a", ".", "b"})); // Raw, not normalized.
    EXPECT_EQ(segments(Path("a/./b").normalized()), std::vector<std::string>({"a", "b"}));
    EXPECT_EQ(segments(Path("../a")), std::vector<std::string>({"..", "a"}));
    EXPECT_TRUE(segments(Path("")).empty());
    EXPECT_TRUE(segments(Path(".").normalized()).empty()); // "." normalizes to the empty path.
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

UNIT_TEST(Path, DegenerateSplit) {
    // A split with nothing in it has no buffer at all - it marks itself past-the-end by putting its begin pointer
    // above its end, so asking it for a string view yields one of negative length. tailAfter takes an empty chunk
    // to mean "all of it", which is the way to reach that view, and it has to notice.
    for (std::string_view path : {"", "/"}) {
        PathSplit split = Path(path).split();
        EXPECT_TRUE(split.empty()) << path;
        EXPECT_TRUE(split.tailAfter(std::string_view()).isEmpty()) << path;
        EXPECT_TRUE(split.tailAfter(std::string_view()).string().empty()) << path;
    }

    // A split that does have chunks answers normally for the same argument.
    EXPECT_EQ(Path("a/b").split().tailAfter(std::string_view()).string(), "a/b");
}

UNIT_TEST(Path, IsNormalized) {
    // isNormalized has to agree with "normalized() would change nothing", or the asserts in the file system layer
    // are checking something other than what the boundary establishes.
    for (std::string_view path : {"", ".", "..", "a", "a/", "a//b", "a/./b", "a/b/../c", "../a", "/", "/a", "/..",
                                  "a/b/", "./a", "a/..", "//", "///a", "..a", "a...", "some."})
        EXPECT_EQ(Path(path).isNormalized(), Path(path).normalized() == Path(path)) << path;
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
        "a/./b", "a//b", "a/b/../c", "../a", "/..", "a/../..",
        "...", "a/.../b", "a...", "some.", "..wat" // Dotted names, where our old rule diverged from std.
    };

    // Root names and backslash separators exist on Windows only. POSIX says a path starting with exactly two slashes
    // is implementation-defined, and Path preserves it there instead of reading a root name out of it. The UNC and
    // extended-length spellings are here to settle where MSVC puts the root - a disagreement shows up as a parent
    // mismatch, because parent() clamps at the root.
#ifdef _WINDOWS
    for (std::string_view windowsPath : {"C:/a", "D:/b", "//server", "//server/share", "a\\b",
                                         "//server/share/x", "//?/UNC/server/share", "//?/UNC/server/share/x",
                                         "//?/C:/x"})
        paths.emplace_back(windowsPath);
#endif

    for (const std::string &head : paths) {
        std::string normal = oracle(head);
        EXPECT_EQ(Path(head).normalized().string(), normal) << "normalizing '" << head << "'";

        for (const std::string &tail : paths) {
            // Our rule is that any rooted tail replaces the head. std::filesystem instead keeps the head's root
            // name when the tail has a root directory but no root name, so on Windows it turns "/b" into "C:/b" -
            // laundering an absolute path into a relative one, which is exactly what we don't want. And operator/
            // concatenates without normalizing, so the comparison is against the normalized join.
            std::string expected = Path(tail).isAbsolute()
                ? oracle(tail)
                : oracle(std::filesystem::path(head) / std::filesystem::path(tail));

            EXPECT_EQ((Path(head) / Path(tail)).normalized().string(), expected)
                << "'" << head << "' / '" << tail << "'";
        }

        // Against the normal form, so that the oracle's retained trailing separator doesn't shift the answer.
        EXPECT_EQ(Path(normal).parent().string(), oracle(std::filesystem::path(normal).parent_path()))
            << "parent of '" << head << "'";

        // Decomposition agrees with std everywhere now, so the oracle is the authority here rather than a
        // documented divergence.
        EXPECT_EQ(Path(head).extension(), std::filesystem::path(head).extension().generic_string())
            << "extension of '" << head << "'";
        EXPECT_EQ(Path(head).stem(), std::filesystem::path(head).stem().generic_string())
            << "stem of '" << head << "'";

        for (std::string_view extension : {"", ".x", ".tar.gz"}) { // isExtension rejects "x", and that's asserted.
            std::filesystem::path expected = std::filesystem::path(normal);
            expected.replace_extension(extension);
            // Normalized on our side too, since withExtension doesn't normalize and the oracle does - mirroring
            // std means "..wat" without an extension is ".", which is a path that spells the empty one.
            EXPECT_EQ(Path(normal).withExtension(extension).normalized().string(), oracle(expected))
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
