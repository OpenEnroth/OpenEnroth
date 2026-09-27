#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <type_traits>

#include "Testing/Unit/UnitTest.h"

#include "Utility/System/NativePath.h"

template<class T>
concept BuildsFromStdPath = requires(const T &path) { NativePath::fromStdPath(path); };

UNIT_TEST(NativePath, ConversionsAreChecked) {
    // A string must not reach fromStdPath, because std::filesystem::path converts a narrow string per the C locale on
    // Windows. The deleted template overload is what turns each of these into a compile error.
    static_assert(BuildsFromStdPath<std::filesystem::path>);
    static_assert(!BuildsFromStdPath<std::string>);
    static_assert(!BuildsFromStdPath<std::string_view>);
    static_assert(!BuildsFromStdPath<const char *>);
    static_assert(!BuildsFromStdPath<std::wstring>);
    static_assert(!BuildsFromStdPath<std::u8string>);

    static_assert(std::is_convertible_v<const char *, NativePath>);
    static_assert(std::is_convertible_v<std::string_view, NativePath>);
    static_assert(std::is_same_v<decltype(NativePath().toWtf8()), const std::string &>); // No copy on every call.
}

UNIT_TEST(NativePath, Wtf8RoundTrip) {
    for (std::string_view path : {"a/b/c.txt", "\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(NativePath::fromWtf8(path).toWtf8(), path);
}

UNIT_TEST(NativePath, StdPathRoundTrip) {
    std::filesystem::path cwd = std::filesystem::current_path();
    EXPECT_EQ(NativePath::fromStdPath(cwd).toStdPath(), cwd);

    // A name that isn't ASCII goes through wchar_t on Windows, where a narrow conversion would mangle it.
    std::filesystem::path lol(u8"a/\u043b\u043e\u043b.txt");
    EXPECT_EQ(NativePath::fromStdPath(lol).toWtf8(), "a/\xd0\xbb\xd0\xbe\xd0\xbb.txt");
    EXPECT_EQ(NativePath::fromWtf8("a/\xd0\xbb\xd0\xbe\xd0\xbb.txt").toStdPath(), lol);
}

UNIT_TEST(NativePath, NativeRoundTrip) {
    // The conversion to the OS encoding goes through wchar_t on Windows, so WTF-8 has to survive it, unpaired
    // surrogates included.
    for (std::string_view path : {"a/b/c.txt", "\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(NativePath::fromNative(NativePath::fromWtf8(path).native()).toWtf8(), path);
}

UNIT_TEST(NativePath, Literals) {
    // The implicit constructors take the same bytes fromWtf8 does.
    EXPECT_EQ(NativePath("a/b/c.txt"), NativePath::fromWtf8("a/b/c.txt"));
    EXPECT_EQ(NativePath(""), NativePath());
}

UNIT_TEST(NativePath, Composition) {
    auto testOne = [] (std::string_view head, std::string_view tail, std::string_view result) {
        EXPECT_EQ((NativePath::fromWtf8(head) / NativePath::fromWtf8(tail)).toWtf8(), result)
            << "for '" << head << "' / '" << tail << "'";
    };

    // An empty head contributes nothing, while an empty tail leaves a trailing separator behind.
    testOne("", "", "");
    testOne("", "a", "a");
    testOne("a", "", "a/");
    testOne("/", "", "/");

    // Exactly one separator goes in, whether or not the head already ends with one.
    testOne("a", "b", "a/b");
    testOne("a/", "b", "a/b");
    testOne("a/b", "c", "a/b/c");
    testOne("a/b", "c/d", "a/b/c/d");
    testOne("a/b/", "c/d", "a/b/c/d");
    testOne("a", "b/", "a/b/");
    testOne("/", "a", "/a");
    testOne("/a", "b", "/a/b");

    // A rooted tail replaces a head that has no root name.
    testOne("", "/a", "/a");
    testOne("a/b", "/c", "/c");
    testOne("a/b", "/", "/");
    testOne("/a/b", "/c/d", "/c/d");

    // Dot components are ordinary names here, nothing resolves them.
    testOne(".", "a", "./a");
    testOne("..", "a", "../a");
    testOne("a", ".", "a/.");
    testOne("a", "..", "a/..");
    testOne("a/", "..", "a/..");
}

UNIT_TEST(NativePath, WithExtension) {
    EXPECT_EQ(NativePath("a/b.json").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b").withExtension(".mm7").toWtf8(), "a/b.mm7");
    EXPECT_EQ(NativePath("a/b.json").withExtension("").toWtf8(), "a/b");
    EXPECT_EQ(NativePath("a/b.json").withExtension("mm7").toWtf8(), "a/b.mm7"); // The leading dot is optional.
    EXPECT_EQ(NativePath("a.tar.gz").withExtension(".zip").toWtf8(), "a.tar.zip"); // Only the last extension goes.
    EXPECT_EQ(NativePath("a/.bashrc").withExtension(".txt").toWtf8(), "a/.bashrc.txt"); // A dotfile has no extension.
    EXPECT_EQ(NativePath("a.d/b").withExtension(".txt").toWtf8(), "a.d/b.txt"); // Dots in directory names don't count.
    EXPECT_EQ(NativePath("a.tar.gz").withExtension("").toWtf8(), "a.tar");
    EXPECT_EQ(NativePath("/a/b.c").withExtension("").toWtf8(), "/a/b");
    EXPECT_EQ(NativePath("a.txt").withExtension(".tar.gz").toWtf8(), "a.tar.gz"); // A dotted argument goes in whole.

    // A path with no file name gets the extension as its file name.
    EXPECT_EQ(NativePath("").withExtension(".x").toWtf8(), ".x");
    EXPECT_EQ(NativePath("a/").withExtension(".x").toWtf8(), "a/.x");
    EXPECT_EQ(NativePath("/").withExtension(".x").toWtf8(), "/.x");
}

UNIT_TEST(NativePath, DottedNames) {
    // A name whose stem would be all dots has no extension, so that dropping the extension can't turn a file name
    // into a navigation token. The stem of "..." is "..", so "a/..." would otherwise become the parent of "a".
    EXPECT_EQ(NativePath("a/...").withExtension("").toWtf8(), "a/...");
    EXPECT_EQ(NativePath("a/...a").withExtension("").toWtf8(), "a/...a");
    EXPECT_EQ(NativePath("...json").withExtension("").toWtf8(), "...json");
    EXPECT_EQ(NativePath("a/...").withExtension(".x").toWtf8(), "a/....x");
    EXPECT_EQ(NativePath(".").withExtension("").toWtf8(), ".");
    EXPECT_EQ(NativePath("..").withExtension("").toWtf8(), "..");

    // A stem that isn't all dots still splits normally.
    EXPECT_EQ(NativePath("..a.txt").withExtension("").toWtf8(), "..a");
}

#ifdef _WINDOWS
UNIT_TEST(NativePath, WindowsRoots) {
    EXPECT_EQ(NativePath::fromWtf8("a\\b").toWtf8(), "a/b"); // Both slashes separate components on Windows.

    EXPECT_EQ((NativePath("C:/a") / NativePath("D:/b")).toWtf8(), "D:/b"); // Another drive replaces everything.
    EXPECT_EQ((NativePath("C:/a") / NativePath("/b")).toWtf8(), "C:/b"); // A rooted tail keeps our drive.
    EXPECT_EQ((NativePath("C:/a") / NativePath("C:b")).toWtf8(), "C:/a/b"); // Same drive, so it's a plain append.
    EXPECT_EQ((NativePath("C:/a") / NativePath("D:b")).toWtf8(), "D:b"); // Another drive replaces, even a relative one.

    // A drive letter is an ASCII letter of either case followed by a colon, and nothing else is one.
    EXPECT_EQ((NativePath("c:/a") / NativePath("/b")).toWtf8(), "c:/b");
    EXPECT_EQ((NativePath("Ab") / NativePath("c")).toWtf8(), "Ab/c");
    EXPECT_EQ((NativePath("1:/a") / NativePath("/b")).toWtf8(), "/b");
    EXPECT_EQ((NativePath("C:/a") / NativePath("///b")).toWtf8(), "C:///b"); // Three slashes start no UNC root.
    EXPECT_EQ((NativePath("//server/share") / NativePath("f")).toWtf8(), "//server/share/f");

    // A bare drive letter takes a separator like any other root name. The drive-relative "C:x" has to be spelled out.
    EXPECT_EQ((NativePath("C:") / NativePath("b")).toWtf8(), "C:/b");
    EXPECT_EQ((NativePath("C:") / NativePath("C:b")).toWtf8(), "C:/b");
    EXPECT_EQ((NativePath("C:") / NativePath("")).toWtf8(), "C:/");
    EXPECT_EQ(NativePath("C:").withExtension(".x").toWtf8(), "C:/.x");
    EXPECT_EQ((NativePath("C:a") / NativePath("b")).toWtf8(), "C:a/b");

    // A bare server name is already absolute. So a separator goes in after it, and as a tail it replaces the head.
    EXPECT_EQ((NativePath("//server") / NativePath("share")).toWtf8(), "//server/share");
    EXPECT_EQ((NativePath("//server/share") / NativePath("//server")).toWtf8(), "//server");
    EXPECT_EQ((NativePath("//server") / NativePath("/share")).toWtf8(), "//server/share");

    // The share is part of the root name, so a rooted tail stays on the share rather than climbing to the server.
    EXPECT_EQ((NativePath("//server/share/a") / NativePath("/b")).toWtf8(), "//server/share/b");
    EXPECT_EQ((NativePath("//server/share/a") / NativePath("//server/other")).toWtf8(), "//server/other");
    EXPECT_EQ((NativePath("//server/share") / NativePath("")).toWtf8(), "//server/share/");

    // Extended-length root names are "//?/C:" and "//?/UNC/server/share", and a rooted tail keeps them too.
    EXPECT_EQ((NativePath("//?/UNC/server/share/a") / NativePath("/b")).toWtf8(), "//?/UNC/server/share/b");
    EXPECT_EQ((NativePath("//?/C:/Games") / NativePath("/anims")).toWtf8(), "//?/C:/anims");

    // The root name used to stop at "//?/unc" and "//./UNC", so a rooted tail climbed off the share. Win32 opens both.
    EXPECT_EQ((NativePath("//?/unc/server/share/a") / NativePath("/b")).toWtf8(), "//?/unc/server/share/b");
    EXPECT_EQ((NativePath("//./UNC/server/share/a") / NativePath("/b")).toWtf8(), "//./UNC/server/share/b");
    EXPECT_EQ((NativePath("//./Unc/server/share/a") / NativePath("/b")).toWtf8(), "//./Unc/server/share/b");
    EXPECT_EQ(NativePath("//./UNC/ser.ver/sh.are").withExtension("").toWtf8(), "//./UNC/ser.ver/sh.are");
    EXPECT_EQ(NativePath("//?/unc/ser.ver/sh.are").withExtension("").toWtf8(), "//?/unc/ser.ver/sh.are");
    EXPECT_EQ((NativePath("//?/uNc/server/share/a") / NativePath("/b")).toWtf8(), "//?/uNc/server/share/b");
    EXPECT_EQ((NativePath("//?/UNCx/server/share/a") / NativePath("/b")).toWtf8(), "//?/UNCx/b"); // Not "UNC".
    EXPECT_EQ((NativePath("//./COM1/a") / NativePath("/b")).toWtf8(), "//./COM1/b");

    // A missing component ends the root name early, wherever in the root name it's missing.
    EXPECT_EQ((NativePath("//server//x") / NativePath("/b")).toWtf8(), "//server/b");
    EXPECT_EQ((NativePath("//?//x") / NativePath("/b")).toWtf8(), "//?/b");
    EXPECT_EQ((NativePath("//?/UNC/") / NativePath("/b")).toWtf8(), "//?/UNC/b");
    EXPECT_EQ((NativePath("//?/UNC//share/a") / NativePath("/b")).toWtf8(), "//?/UNC/b");
    EXPECT_EQ((NativePath("//?/UNC/server") / NativePath("/b")).toWtf8(), "//?/UNC/server/b");
    EXPECT_EQ((NativePath("//?/UNC/server//a") / NativePath("/b")).toWtf8(), "//?/UNC/server/b");

    // An empty tail leaves a trailing separator, after a root name as well.
    EXPECT_EQ((NativePath("C:/a") / NativePath("")).toWtf8(), "C:/a/");
    EXPECT_EQ((NativePath("//server") / NativePath("")).toWtf8(), "//server/");

    // Extended-length and device paths go to Win32 with backslashes.
    EXPECT_EQ(NativePath::fromWtf8("\\\\?\\C:\\Games\\MM7").native(), L"\\\\?\\C:\\Games\\MM7");
    EXPECT_EQ(NativePath::fromWtf8("//?/C:/Games").native(), L"\\\\?\\C:\\Games");
    EXPECT_EQ(NativePath::fromWtf8("\\\\.\\COM1").native(), L"\\\\.\\COM1");
    EXPECT_EQ(NativePath::fromWtf8("//./UNC/server/share/f").native(), L"\\\\.\\UNC\\server\\share\\f");
    EXPECT_EQ(NativePath::fromWtf8("//?/UNC/server/share/f").native(), L"\\\\?\\UNC\\server\\share\\f");
    EXPECT_EQ(NativePath::fromWtf8("C:/Games/MM7").native(), L"C:/Games/MM7"); // Everything else keeps them.
    EXPECT_EQ(NativePath::fromWtf8("\\\\server\\share\\f").native(), L"//server/share/f");
    EXPECT_EQ(NativePath::fromNative(L"C:\\a\\b").toWtf8(), "C:/a/b"); // Separators from the OS get converted too.

    // A root name is never a file name, so a dot inside one doesn't start an extension.
    EXPECT_EQ(NativePath("//ser.ver").withExtension("").toWtf8(), "//ser.ver");
    EXPECT_EQ(NativePath("//ser.ver/sh.are").withExtension("").toWtf8(), "//ser.ver/sh.are");
    EXPECT_EQ(NativePath("//ser.ver/sh.are/a.txt").withExtension("").toWtf8(), "//ser.ver/sh.are/a");
    EXPECT_EQ(NativePath("//?/UNC/ser.ver/sh.are").withExtension("").toWtf8(), "//?/UNC/ser.ver/sh.are");

    // A UNC root name has no file name, so an extension starts one under it rather than renaming the share.
    EXPECT_EQ(NativePath("//server/share").withExtension(".x").toWtf8(), "//server/share/.x");
    EXPECT_EQ(NativePath("//server").withExtension(".x").toWtf8(), "//server/.x");
    EXPECT_EQ(NativePath("//server/share/").withExtension(".x").toWtf8(), "//server/share/.x");
}

UNIT_TEST(NativePath, ExtendedLengthReachesWin32) {
    // Win32 only honors a literal "\\?\", and handing it the stored forward slashes used to lose that. A path over
    // MAX_PATH then failed to open, and a trailing dot got stripped off the file name.
    std::filesystem::path temp = std::filesystem::temp_directory_path();
    std::string prefixed = "//?/" + NativePath::fromStdPath(temp).toWtf8();
    std::string longName = "oe_" + std::string(240, 'x') + ".txt";

    for (std::string_view name : {std::string_view(longName), std::string_view("oe_trailing_dot.")}) {
        NativePath path = NativePath::fromWtf8(prefixed + std::string(name));
        {
            std::ofstream stream(path.toStdPath());
            ASSERT_TRUE(stream.is_open()) << name;
        }

        std::filesystem::path onDisk(L"\\\\?\\" + (temp / name).wstring()); // Built by hand, NativePath not involved.
        EXPECT_TRUE(std::filesystem::exists(onDisk)) << name;
        EXPECT_TRUE(std::filesystem::remove(onDisk)) << name;
    }
}
#endif

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

    NativePath cwd = NativePath::fromStdPath(std::filesystem::current_path());
    EXPECT_EQ(cwd.absolute(), cwd); // An absolute path stays as it is.
}

UNIT_TEST(NativePath, Comparison) {
    // Paths compare as their stored strings, so a trailing or doubled separator makes a different path, and "a/b"
    // sorts after "a.b" where std::filesystem::path would put it first.
    EXPECT_NE(NativePath("a"), NativePath("b"));
    EXPECT_NE(NativePath("a"), NativePath("a/"));
    EXPECT_NE(NativePath("a/b"), NativePath("a//b"));
    EXPECT_LT(NativePath("a"), NativePath("b"));
    EXPECT_LT(NativePath("A"), NativePath("a"));
    EXPECT_GT(NativePath("a/b"), NativePath("a.b"));
}

UNIT_TEST(NativePath, IsEmpty) {
    EXPECT_TRUE(NativePath().isEmpty());
    EXPECT_TRUE(NativePath("").isEmpty());
    EXPECT_FALSE(NativePath("a").isEmpty());
    EXPECT_FALSE(NativePath("/").isEmpty());
}

UNIT_TEST(NativePath, Format) {
    EXPECT_EQ(fmt::format("[{}]", NativePath("a/b")), "[a/b]");
    EXPECT_EQ(fmt::format("[{:>6}]", NativePath("a/b")), "[   a/b]"); // Format specs reach the string formatter.
}

UNIT_TEST(NativePath, LexicalCast) {
    // CLI11 binds NativePath options through this, so it has to take any string, an empty one included.
    NativePath path("x");
    EXPECT_TRUE(lexical_cast(std::string("a/b"), path));
    EXPECT_EQ(path, NativePath("a/b"));
    EXPECT_TRUE(lexical_cast(std::string(""), path));
    EXPECT_TRUE(path.isEmpty());
}

#ifndef _WINDOWS
UNIT_TEST(NativePath, PosixSyntax) {
    // Backslashes and Windows roots are ordinary text on POSIX, where the only separator is a forward slash.
    EXPECT_EQ(NativePath::fromWtf8("a\\b").toWtf8(), "a\\b");
    EXPECT_EQ((NativePath("a\\b") / NativePath("c")).toWtf8(), "a\\b/c");
    EXPECT_EQ((NativePath("a") / NativePath("\\b")).toWtf8(), "a/\\b");
    EXPECT_EQ(NativePath("a.b\\c").withExtension("").toWtf8(), "a"); // One file name, so ".b\c" is its extension.
    EXPECT_EQ((NativePath("C:") / NativePath("x")).toWtf8(), "C:/x");
    EXPECT_EQ(NativePath("C:").withExtension(".x").toWtf8(), "C:.x");
    EXPECT_EQ((NativePath("a") / NativePath("//server")).toWtf8(), "//server");
    EXPECT_EQ(NativePath("//a.b").withExtension("").toWtf8(), "//a");
}

UNIT_TEST(NativePath, InvalidUtf8RoundTrip) {
    // File names on POSIX are byte strings, so fromWtf8 / toWtf8 have to pass invalid UTF-8 through as-is. "\xD0" is
    // an incomplete UTF-8 sequence, "\xFF" can't appear in UTF-8 at all.
    for (std::string_view name : {"lol\xD0kek.txt", "lol\xFFkek.txt", "trailing\xD0"})
        EXPECT_EQ(NativePath::fromWtf8(name).toWtf8(), name);
}
#endif

#if !defined(_WINDOWS) && !defined(__APPLE__)
UNIT_TEST(NativePath, InvalidUtf8FileNames) {
    // A name with invalid UTF-8 in it is not just convertible, it's also usable to actually open a file. APFS is the
    // exception, it only takes file names that are valid UTF-8, so this test doesn't run on MacOS.
    NativePath tmpDir = NativePath::fromStdPath(std::filesystem::temp_directory_path()); // A build dir can sit on an APFS-backed mount in a dev container.

    for (std::string_view name : {"tmp_lol\xD0kek.txt", "tmp_lol\xFFkek.txt", "tmp_trailing\xD0"}) {
        NativePath path = tmpDir / NativePath::fromWtf8(name);

        std::ofstream stream(path.toStdPath());
        ASSERT_TRUE(stream.is_open()) << name;
        stream << "lol";
        stream.close();

        EXPECT_TRUE(std::filesystem::exists(path.toStdPath())) << name;
        EXPECT_TRUE(std::filesystem::remove(path.toStdPath())) << name;
    }
}
#endif
