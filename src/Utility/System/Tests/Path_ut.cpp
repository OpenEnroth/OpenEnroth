#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Fs.h"
#include "Utility/System/Path.h"

template<class Head, class Tail>
concept Joinable = requires(const Head &head, const Tail &tail) { head / tail; };

UNIT_TEST(Path, ConversionsAreChecked) {
    // Only a literal and a std::string rvalue convert to a Path implicitly, every other copy is spelled out.
    static_assert(std::is_convertible_v<const char *, Path>);
    static_assert(std::is_convertible_v<std::string &&, Path>);
    static_assert(!std::is_convertible_v<std::string_view, Path>);
    static_assert(!std::is_convertible_v<const std::string &, Path>);
    static_assert(std::is_constructible_v<Path, std::string_view>);
    static_assert(std::is_constructible_v<Path, const std::string &>);
    static_assert(std::is_convertible_v<const char *, PathView>);
    static_assert(std::is_convertible_v<std::string_view, PathView>);
    static_assert(std::is_convertible_v<const std::string &, PathView>);
    static_assert(std::is_convertible_v<const Path &, PathView>);
    static_assert(!std::is_convertible_v<PathView, Path>);
    static_assert(std::is_constructible_v<Path, PathView>);
    static_assert(!std::is_constructible_v<Path, std::nullptr_t>);
    static_assert(!std::is_constructible_v<PathView, std::nullptr_t>);
}

UNIT_TEST(Path, NativeRoundTrip) {
    // The conversion to the OS encoding goes through wchar_t on Windows, so WTF-8 has to survive it, unpaired
    // surrogates included.
    for (std::string_view path : {"\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(Path::fromNative(Path(path).native()).str(), path);
}

UNIT_TEST(Path, Composition) {
    auto testOne = [] (std::string_view head, std::string_view tail, std::string_view result) {
        EXPECT_EQ((Path(head) / tail).str(), result)
            << "for '" << head << "' / '" << tail << "'";
    };

    // An empty head contributes nothing, while an empty tail leaves a trailing separator behind.
    testOne("", "a", "a");
    testOne("a", "", "a/");

    // Exactly one separator goes in, whether or not the head already ends with one or more.
    testOne("a", "b", "a/b");
    testOne("a/", "b", "a/b");
    testOne("a//", "b", "a/b");
    testOne("//", "x", "/x"); // A root directory stays one, it doesn't become the "//" of a UNC root on Windows.

    testOne("a/b", "/c", "/c"); // A rooted tail replaces a head that has no root name.
    testOne("a", "..", "a/.."); // Dot components are ordinary names here, nothing resolves them.
}

UNIT_TEST(Path, WithExtension) {
    auto testOne = [] (std::string_view path, std::string_view extension, std::string_view result) {
        EXPECT_EQ(Path(path).withExtension(extension).str(), result)
            << "for '" << path << "' with '" << extension << "'";
    };

    testOne("a/b.json", ".mm7", "a/b.mm7");
    testOne("a/b", ".mm7", "a/b.mm7");
    testOne("a/b.json", "", "a/b");
    testOne("a/b.json", "mm7", "a/b.mm7"); // The leading dot is optional.
    testOne("a.tar.gz", ".zip", "a.tar.zip"); // Only the last extension goes.
    testOne("a/.bashrc", ".txt", "a/.bashrc.txt"); // A dotfile has no extension.
    testOne("a.d/b", ".txt", "a.d/b.txt"); // Dots in directory names don't count.

    // A path with no file name gets the extension as its file name.
    testOne("", ".x", ".x");
    testOne("a/", ".x", "a/.x");

    // A name whose stem would be all dots has no extension, so that dropping the extension can't turn a file name
    // into a navigation token. The stem of "..." is "..", so "a/..." would otherwise become the parent of "a".
    testOne("a/.", "", "a/.");
    testOne("a/..", "", "a/..");
    testOne("a/...", "", "a/...");
    testOne("..a.txt", "", "..a"); // A stem that isn't all dots still splits normally.
}

UNIT_TEST(Path, Normalized) {
    // normalized() resolves separators, "." and ".." lexically, and its result is already normal.
    auto testOne = [] (std::string_view path, std::string_view result) {
        Path normal = Path(path).normalized();
        EXPECT_EQ(normal.str(), result) << "for '" << path << "'";
        EXPECT_TRUE(normal.isNormalized()) << "for '" << path << "'";
        EXPECT_EQ(normal.normalized(), normal) << "for '" << path << "'";
        EXPECT_EQ(Path(path).isNormalized(), path == result) << "for '" << path << "'";
    };

    testOne("", "");
    testOne(".", ".");
    testOne("./", ".");
    testOne("a/..", ".");
    testOne("a/b", "a/b");
    testOne("a/b/", "a/b");
    testOne("a//b", "a/b");
    testOne("a/./b/.", "a/b");
    testOne("./a/b", "a/b");
    testOne("a/b/..", "a");
    testOne("a/../b", "b");
    testOne("a/b/../..", ".");
    testOne("..", "..");
    testOne("../..", "../..");
    testOne("../../", "../..");
    testOne("a/../../b", "../b");
    testOne("../a/b", "../a/b");
    testOne("a/.../b/...", "a/.../b/...");

    // A root directory stays, and nothing is above it.
    testOne("/", "/");
    testOne("/a//b/", "/a/b");
    testOne("/./a", "/a");
    testOne("/..", "/");
    testOne("/a/../..", "/");
    testOne("/../a", "/a");
    testOne("///a", "/a");

#ifdef _WINDOWS
    testOne("C:", "C:");
    testOne("C:/", "C:/");
    testOne("c:/./a", "c:/a");
    testOne("C:\\a\\..\\b", "C:/b");
    testOne("a\\b", "a/b"); // Normal form spells every separator as "/".
    testOne("a/b\\", "a/b");
    testOne("\\\\server\\share\\a", "//server/share/a");
    testOne("\\\\?\\C:\\a\\..\\b", "//?/C:/b");
    testOne("C:/a/../..", "C:/");
    testOne("C:..", "C:.."); // Drive relative, so the ".." climbs the current directory of drive C.
    testOne("C:a/../..", "C:..");
    testOne("//server/share", "//server/share");
    testOne("//server/share/a/..", "//server/share/");
    testOne("//server/share/..", "//server/share/");
    testOne("./C:/a", "C:/a"); // Dropping the "." leaves the drive letter leading.
    testOne("./C:.", "C:"); // The "." after the new drive goes too.
    testOne("./C:./a", "C:a");
    testOne("a/../C:/x", "C:/x"); // The drive ends up leading, so the result is absolute.
    testOne("//a/b", "//a/b"); // A UNC root.
    testOne("//?/UNC//share/a", "//?/UNC//share/a"); // An empty server, which the root keeps.
    testOne("//?/UNC//share/..", "//?/UNC//share/");
    testOne("//server//x/a", "//server/x/a"); // Win32 reads this as share "x" on "server".
    testOne("//server///x", "//server/x");
#else
    testOne("//a/b", "/a/b");
#endif
}

UNIT_TEST(Path, IsEscaping) {
    // A path escapes when its ".." segments climb above where it starts.
    auto testOne = [] (std::string_view path, bool result) {
        EXPECT_EQ(Path(path).isEscaping(), result) << "for '" << path << "'";
    };

    testOne("", false);
    testOne(".", false);
    testOne("a", false);
    testOne("a/..", false);
    testOne("a/../b", false);
    testOne("..", true);
    testOne("../a", true);
    testOne("a/../..", true);
    testOne("a//..//..", true); // Doesn't need normal form.
    testOne("/..", false); // A path with a root directory never escapes.
    testOne("/../..", false);
#ifdef _WINDOWS
    testOne("C:..", true); // Above the current directory of drive C.
    testOne("C:a/../..", true);
    testOne("C:/..", false);
    testOne("//server/share/..", false);
#endif
}

UNIT_TEST(Path, Decomposition) {
    // parent, name, stem and extension split a path lexically.
    auto testOne = [] (std::string_view path, std::string_view parent, std::string_view name, std::string_view stem,
                       std::string_view extension) {
        Path p(path);
        EXPECT_EQ(p.parent().str(), parent) << "for '" << path << "'";
        EXPECT_EQ(p.name(), name) << "for '" << path << "'";
        EXPECT_EQ(p.stem(), stem) << "for '" << path << "'";
        EXPECT_EQ(p.extension(), extension) << "for '" << path << "'";
    };

    testOne("", "", "", "", "");
    testOne(".", ".", ".", ".", "");
    testOne("b", ".", "b", "b", "");
    testOne("b.c", ".", "b.c", "b", ".c");
    testOne("a/b", "a", "b", "b", "");
    testOne("a/b.c", "a", "b.c", "b", ".c");
    testOne("a/", "a", "", "", "");
    testOne("1/2/3/xyz.txt", "1/2/3", "xyz.txt", "xyz", ".txt");
    testOne("x.y/z.f/a.b.c.d", "x.y/z.f", "a.b.c.d", "a.b.c", ".d");
    testOne("x/y/z/some.", "x/y/z", "some.", "some", ".");
    testOne(".hidden", ".", ".hidden", ".hidden", "");
    testOne("..", ".", "..", "..", "");
    testOne("../..", "..", "..", "..", "");
    testOne("..wat", ".", "..wat", "..wat", ""); // The stem would be ".", all dots, so there's no extension.
    testOne("a/...", "a", "...", "...", "");

    // The parent keeps the root.
    testOne("/", "/", "", "", "");
    testOne("/a", "/", "a", "a", "");
    testOne("/a/b", "/a", "b", "b", "");
#ifdef _WINDOWS
    testOne("C:", "C:", "", "", "");
    testOne("C:/", "C:/", "", "", "");
    testOne("C:/a.txt", "C:/", "a.txt", "a", ".txt");
    testOne("C:a", "C:", "a", "a", "");
    testOne("//server/share/a", "//server/share/", "a", "a", "");
    testOne("//?/UNC//share/a", "//?/UNC//share/", "a", "a", "");
#endif
}

UNIT_TEST(Path, Root) {
    // root() is the root name and root directory a path starts with.
    auto testOne = [] (std::string_view path, std::string_view root) {
        EXPECT_EQ(Path(path).root(), root) << "for '" << path << "'";
    };

    testOne("", "");
    testOne("a/b", "");
    testOne("/", "/");
    testOne("/a", "/");
#ifdef _WINDOWS
    testOne("C:", "C:");
    testOne("C:a", "C:");
    testOne("C:/a", "C:/");
    testOne("//server/share/a", "//server/share/");
    testOne("//?/UNC//share/a", "//?/UNC//share/");
    testOne("\\\\server\\share\\a", "\\\\server\\share\\");
    testOne("C:\\a", "C:\\");
#endif
}

UNIT_TEST(Path, Split) {
    // split() yields the segments after the root, with no empty ones.
    auto segments = [] (std::string_view path) -> std::vector<std::string> {
        return Path(path).split();
    };

    EXPECT_TRUE(segments("").empty());
    EXPECT_TRUE(std::ranges::empty(PathView().split())); // A default view has no string at all, not even an empty one.
    EXPECT_TRUE(segments("/").empty());
    EXPECT_TRUE(segments("///").empty());
    EXPECT_EQ(segments("a//b/"), std::vector<std::string>({"a", "b"}));
    EXPECT_EQ(segments("/a//b//"), std::vector<std::string>({"a", "b"}));
    EXPECT_EQ(segments("a"), std::vector<std::string>({"a"}));
    EXPECT_EQ(segments("../a/b"), std::vector<std::string>({"..", "a", "b"}));
    EXPECT_EQ(segments("/a/b"), std::vector<std::string>({"a", "b"})); // The root isn't a segment.
#ifdef _WINDOWS
    EXPECT_EQ(segments("C:/a"), std::vector<std::string>({"a"}));
    EXPECT_EQ(segments("C:a"), std::vector<std::string>({"a"}));
    EXPECT_TRUE(segments("//server/share").empty()); // The share is part of the root.
#endif

    static_assert(std::ranges::borrowed_range<PathSplit>);
    Path path("a/b/c");
    const PathSplit split = path.split();
    auto pos = std::ranges::find(split, "b");
    ASSERT_NE(pos, split.end());
    EXPECT_EQ(path.tailAt(*pos).str(), "b/c");
}

UNIT_TEST(Path, Tails) {
    // tailAt starts at a segment, tailAfter starts after it and its separators.
    using Tails = std::pair<std::string, std::string>;
    auto tails = [] (const Path &path, std::string_view at) -> Tails {
        for (std::string_view segment : path.split())
            if (segment == at)
                return {std::string(path.tailAt(segment).str()), std::string(path.tailAfter(segment).str())};
        return {};
    };

    EXPECT_EQ(tails("a/b/c", "a"), Tails("a/b/c", "b/c"));
    EXPECT_EQ(tails("a/b/c", "b"), Tails("b/c", "c"));
    EXPECT_EQ(tails("a/b/c", "c"), Tails("c", ""));
    EXPECT_EQ(tails("/a/b", "a"), Tails("a/b", "b")); // The root isn't part of a tail.
    EXPECT_EQ(tails("a//b//", "a"), Tails("a//b//", "b//")); // tailAfter drops the whole run of separators.
    EXPECT_EQ(tails("a//b//", "b"), Tails("b//", "")); // tailAt keeps what follows, trailing separators included.
#ifdef _WINDOWS
    EXPECT_EQ(tails("C:/a/b", "a"), Tails("a/b", "b"));
    EXPECT_EQ(tails("a\\b/\\c", "b"), Tails("b/\\c", "c"));
    EXPECT_EQ(PathView(tails("a/c:b", "a").second).root(), "c:"); // A segment can be a drive.
#endif

    // An empty segment gives the whole path, root included.
    Path path("a/b/c");
    EXPECT_EQ(path.tailAfter(std::string_view()).str(), "a/b/c");
    Path rooted("/a/b");
    EXPECT_EQ(rooted.tailAfter(std::string_view()).str(), "/a/b");
}

UNIT_TEST(Path, Append) {
    // Appending joins the same way operator/ does, also when the tail points into the path itself.
    auto testOne = [] (std::string_view head, std::string_view tail, std::string_view result) {
        Path appended(head);
        appended /= tail;
        EXPECT_EQ(appended.str(), result) << "for '" << head << "' and '" << tail << "'";
        EXPECT_EQ((Path(head) / tail).str(), result) << "for '" << head << "' and '" << tail << "'";
    };

    testOne("", "a", "a");
    testOne("a", "", "a/");
    testOne("a", "b/c", "a/b/c");
    testOne("a/", "b", "a/b");
    testOne("/", "a", "/a");
    testOne("a", "/b", "/b");
#ifdef _WINDOWS
    testOne("C:", "a", "C:/a");
    testOne("a\\", "b", "a\\b");
    testOne("C:\\a", "b", "C:\\a/b");
    testOne("C:/a", "D:b", "D:b");
    testOne("C:/a", "c:b", "C:/a/b");
#endif

    std::string longName(40, 'x'); // Longer than the small string buffer, so appending reallocates.
    Path self(longName);
    self /= self;
    EXPECT_EQ(self.str(), longName + "/" + longName);

    std::string otherName(40, 'y');
    Path tailed(longName + "/" + otherName);
    tailed /= tailed.tailAt(*std::next(tailed.split().begin()));
    EXPECT_EQ(tailed.str(), longName + "/" + otherName + "/" + otherName);

    Path literal("a");
    literal /= "b";
    EXPECT_EQ(literal.str(), "a/b");

    // Any string joins with a path on either side, but not with another string.
    std::string string = "a";
    std::string_view stringView = string;
    EXPECT_EQ(("a" / Path("b")).str(), "a/b");
    EXPECT_EQ((string / Path("b")).str(), "a/b");
    EXPECT_EQ((stringView / PathView("b")).str(), "a/b");
    EXPECT_EQ((Path("a") / string / stringView / "b").str(), "a/a/a/b");
    static_assert(!Joinable<std::string, std::string>);

    std::string buffer;
    buffer.reserve(64);
    buffer = "a";
    Path inPlace(std::move(buffer));
    const char *data = inPlace.str().data();
    inPlace /= "b";
    EXPECT_EQ(inPlace.str().data(), data); // A tail without a root goes into the same buffer.

    Path head("a");
    Path tail("b/c");
    EXPECT_EQ(PathView(head) / PathView(tail), Path("a/b/c"));
}

#ifdef _WINDOWS
UNIT_TEST(Path, WindowsRoots) {
    auto testJoin = [] (std::string_view head, std::string_view tail, std::string_view result) {
        EXPECT_EQ((Path(head) / tail).str(), result)
            << "for '" << head << "' / '" << tail << "'";
    };
    auto testExtension = [] (std::string_view path, std::string_view extension, std::string_view result) {
        EXPECT_EQ(Path(path).withExtension(extension).str(), result)
            << "for '" << path << "' with '" << extension << "'";
    };
    auto testNative = [] (std::string_view path, const std::wstring &result) {
        EXPECT_EQ(Path(path).native(), result) << "for '" << path << "'";
    };

    EXPECT_EQ(Path("a\\b").str(), "a\\b"); // Kept as given, though both slashes separate components on Windows.
    EXPECT_EQ(Path("a\\b").name(), "b");
    EXPECT_EQ(Path("a\\b").parent().str(), "a");
    std::vector<std::string> segments = Path("a\\b/c").split();
    EXPECT_EQ(segments, std::vector<std::string>({"a", "b", "c"}));

    testExtension("a", ".b\\c", "a.b\\c");

    testJoin("a/\\", "b", "a/b"); // A run of separators keeps its first one, spelled as given.
    testJoin("a\\/", "b", "a\\b");
    testJoin("C:/a", "D:/b", "D:/b"); // Another drive replaces everything.
    testJoin("C:/a", "/b", "C:/b"); // A rooted tail keeps our drive.
    testJoin("C:/a", "D:b", "D:b"); // Another drive replaces, even a relative one.

    // Drive letters are case-insensitive, so a tail in the other case names the same drive.
    testJoin("C:/a", "c:b", "C:/a/b");
    testJoin("C:", "c:b", "C:/b");

    // A drive letter is an ASCII letter of either case followed by a colon, and nothing else is one.
    testJoin("c:/a", "/b", "c:/b");
    testJoin("Ab", "c", "Ab/c");
    testJoin("1:/a", "/b", "/b");
    testJoin("C:/a", "///b", "C:///b"); // Three slashes start no UNC root.

    // A bare drive letter takes a separator like any other root name. The drive-relative "C:x" has to be spelled out.
    testJoin("C:", "b", "C:/b");

    // A bare server name is already absolute. So a separator goes in after it, and as a tail it replaces the head.
    testJoin("//server", "share", "//server/share");
    testJoin("//server/share", "//server", "//server");

    // The share is part of the root name, so a rooted tail stays on the share rather than climbing to the server.
    testJoin("//server/share/a", "/b", "//server/share/b");

    // Extended-length root names are "//?/C:" and "//?/UNC/server/share", and a rooted tail keeps them too.
    testJoin("//?/UNC/server/share/a", "/b", "//?/UNC/server/share/b");
    testJoin("//?/C:/Games", "/anims", "//?/C:/anims");

    // Win32 opens a share after "//./" too, and reads "UNC" in any case, so a rooted tail stays on the share there.
    testJoin("//?/unc/server/share/a", "/b", "//?/unc/server/share/b");
    testJoin("//./UNC/server/share/a", "/b", "//./UNC/server/share/b");
    testJoin("//?/uNc/server/share/a", "/b", "//?/uNc/server/share/b");
    testJoin("//?/UNCx/server/share/a", "/b", "//?/UNCx/b"); // Not "UNC".
    testJoin("//./C:/Games", "/anims", "//./C:/anims");

    // Win32 collapses doubled separators in a plain UNC root, so "//server//x" is share "x" on "server". After "//?/"
    // it doesn't, and a missing component ends the root name early.
    testJoin("//server//x", "/b", "//server//x/b");
    testJoin("//server///x", "/b", "//server///x/b");
    testJoin("//server//", "/b", "//server/b");
    testJoin("//?/UNC/", "/b", "//?/UNC/b");
    testJoin("//?/UNC//share/a", "/b", "//?/UNC//share/b"); // An empty component after "//?/" is still a component.

    // Extended-length and device paths go to Win32 with backslashes.
    testNative("//?/C:/Games", L"\\\\?\\C:\\Games");
    testNative("//./UNC/server/share/f", L"\\\\.\\UNC\\server\\share\\f");
    testNative("C:/Games/MM7", L"C:/Games/MM7"); // Everything else keeps them.
    EXPECT_EQ(Path::fromNative(L"C:\\a\\b").str(), "C:\\a\\b");
    testNative("\\\\?\\C:/Games", L"\\\\?\\C:\\Games"); // Either spelling of the prefix.

    // A root name is never a file name, so a dot inside one doesn't start an extension.
    testExtension("//ser.ver/sh.are", "", "//ser.ver/sh.are");
    testExtension("//ser.ver/sh.are/a.txt", "", "//ser.ver/sh.are/a");

    // A UNC root name has no file name, so an extension starts one under it rather than renaming the share.
    testExtension("//server/share", ".x", "//server/share/.x");
}

UNIT_TEST(Path, ExtendedLengthReachesWin32) {
    // Win32 only honors a literal "\\?\". With forward slashes a path over MAX_PATH fails to open, and a trailing dot
    // gets stripped off the file name.

    // A single component is capped at 255 characters, so it takes two to get over MAX_PATH wherever temp is.
    Path dir = fs::tmp() / ("oe_" + std::string(150, 'd'));
    Path prefixed = Path("//?/" + dir.str());
    ScopedTestFolder folder(dir);

    for (const std::string &name : {"oe_" + std::string(150, 'x') + ".txt", std::string("oe_trailing_dot.")}) {
        ASSERT_NO_THROW(FileOutputStream(prefixed / name).close()) << name;

        // Not exists(), which would look the name up the same way the write did. A listing shows the name on disk.
        EXPECT_EQ(fs::ls(prefixed), std::vector<DirectoryEntry>({{name, FILE_REGULAR}})) << name;
        EXPECT_TRUE(fs::remove(prefixed / name)) << name;
    }
}
#endif

UNIT_TEST(Path, DisplayString) {
    EXPECT_EQ(Path("a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt").displayString(), "a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt");

    // WTF-8-encoded surrogates are not valid UTF-8, so they have to come out as replacement characters.
    std::string display = Path("lol\xed\xb0\x80kek.txt").displayString();
    EXPECT_TRUE(display.starts_with("lol"));
    EXPECT_TRUE(display.ends_with("kek.txt"));
    EXPECT_NE(display.find("\xEF\xBF\xBD"), std::string::npos); // U+FFFD.
    EXPECT_EQ(display.find("\xed\xb0\x80"), std::string::npos);
}

UNIT_TEST(Path, Comparison) {
    // Paths compare as their stored strings, so a trailing or doubled separator or a backslash makes a different
    // path, and "a/b" sorts after "a.b" where std::filesystem::path would put it first.
    EXPECT_NE(Path("a"), Path("a/"));
    EXPECT_NE(Path("a\\b"), Path("a/b"));
    EXPECT_LT(Path("a"), Path("b"));
    EXPECT_GT(Path("a/b"), Path("a.b"));

    // Paths, views and strings compare with each other in either order.
    Path path("a/b");
    PathView view = path;
    EXPECT_EQ(path, view);
    EXPECT_EQ(view, path);
    EXPECT_EQ(view, "a/b");
    EXPECT_EQ("a/b", view);
    EXPECT_EQ(path, "a/b");
    EXPECT_LT(view, Path("b"));
    EXPECT_GT(Path("b"), view);

    std::string string = "a/b";
    std::string_view stringView = string;
    EXPECT_EQ(path, string);
    EXPECT_EQ(string, path);
    EXPECT_EQ(path, stringView);
    EXPECT_EQ(stringView, path);
    EXPECT_LT(path, std::string("b"));
}

UNIT_TEST(Path, Hash) {
    // A path and a view of it hash the same, so either can look the other up.
    Path path("a/b");
    EXPECT_EQ(std::hash<Path>()(path), std::hash<PathView>()(path));
    EXPECT_EQ(std::hash<PathView>()(path.parent()), std::hash<Path>()(Path("a")));

    std::unordered_set<Path, std::hash<Path>, std::equal_to<>> paths = {Path("a/b")};
    EXPECT_TRUE(paths.contains(path.parent() / "b"));
    EXPECT_TRUE(paths.contains(PathView(path)));
    EXPECT_TRUE(paths.contains("a/b"));
    EXPECT_TRUE(paths.contains(std::string("a/b")));
}

UNIT_TEST(Path, IsEmpty) {
    EXPECT_TRUE(Path().isEmpty());
    EXPECT_TRUE(Path("").isEmpty());
    EXPECT_FALSE(Path("a").isEmpty());
}

UNIT_TEST(Path, Format) {
    EXPECT_EQ(fmt::format("[{:>6}]", Path("a/b")), "[   a/b]"); // Format specs reach the string formatter.
    Path path("a/b");
    EXPECT_EQ(fmt::format("[{:>6}]", path.parent()), "[     a]"); // A view formats the same way.
}

UNIT_TEST(Path, LexicalCast) {
    // CLI11 binds Path options through this.
    Path path;
    EXPECT_TRUE(lexical_cast(std::string("a/b"), path));
    EXPECT_EQ(path, Path("a/b"));
}

UNIT_TEST(Path, InvalidUtf8RoundTrip) {
    // Nothing is validated on the way in, so invalid UTF-8 passes through the constructor and str as-is. "\xD0" is an
    // incomplete UTF-8 sequence, "\xFF" can't appear in UTF-8 at all.
    for (std::string_view name : {"lol\xD0kek.txt", "lol\xFFkek.txt", "trailing\xD0"})
        EXPECT_EQ(Path(name).str(), name);
}

#ifndef _WINDOWS
UNIT_TEST(Path, PosixSyntax) {
    auto testExtension = [] (std::string_view path, std::string_view extension, std::string_view result) {
        EXPECT_EQ(Path(path).withExtension(extension).str(), result)
            << "for '" << path << "' with '" << extension << "'";
    };

    // Backslashes and Windows roots are ordinary text on POSIX, where the only separator is a forward slash.
    EXPECT_EQ(Path("a\\b").str(), "a\\b");
    EXPECT_EQ(Path("C:/a").root(), "");
    EXPECT_EQ(Path("C:a").parent().str(), ".");
    testExtension("a.b\\c", "", "a"); // One file name, so ".b\c" is its extension.
    testExtension("C:", ".x", "C:.x");
    testExtension("//a.b", "", "//a");
}
#endif

#if !defined(_WINDOWS) && !defined(__APPLE__)
UNIT_TEST(Path, InvalidUtf8FileNames) {
    // A name with invalid UTF-8 in it is not just convertible, it's also usable to actually open a file. APFS is the
    // exception, it only takes file names that are valid UTF-8, so this test doesn't run on MacOS.
    Path tmpDir = fs::tmp(); // A build dir can sit on an APFS-backed mount in a dev container.

    for (std::string_view name : {"tmp_lol\xD0kek.txt", "tmp_lol\xFFkek.txt", "tmp_trailing\xD0"}) {
        Path path = tmpDir / name;

        ASSERT_NO_THROW(FileOutputStream(path).close()) << name;
        EXPECT_TRUE(fs::exists(path)) << name;
        EXPECT_TRUE(fs::remove(path)) << name;
    }
}
#endif
