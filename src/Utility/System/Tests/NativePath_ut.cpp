#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Fs.h"
#include "Utility/System/NativePath.h"

UNIT_TEST(NativePath, ConversionsAreChecked) {
    static_assert(std::is_convertible_v<const char *, NativePath>);
    static_assert(std::is_convertible_v<std::string_view, NativePath>);
    static_assert(std::is_same_v<decltype(NativePath().str()), const std::string &>); // No copy on every call.
}

UNIT_TEST(NativePath, NativeRoundTrip) {
    // The conversion to the OS encoding goes through wchar_t on Windows, so WTF-8 has to survive it, unpaired
    // surrogates included.
    for (std::string_view path : {"\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(NativePath::fromNative(NativePath(path).native()).str(), path);
}

UNIT_TEST(NativePath, Composition) {
    auto testOne = [] (std::string_view head, std::string_view tail, std::string_view result) {
        EXPECT_EQ((NativePath(head) / NativePath(tail)).str(), result)
            << "for '" << head << "' / '" << tail << "'";
    };

    // An empty head contributes nothing, while an empty tail leaves a trailing separator behind.
    testOne("", "a", "a");
    testOne("a", "", "a/");

    // Exactly one separator goes in, whether or not the head already ends with one.
    testOne("a", "b", "a/b");
    testOne("a/", "b", "a/b");

    testOne("a/b", "/c", "/c"); // A rooted tail replaces a head that has no root name.
    testOne("a", "..", "a/.."); // Dot components are ordinary names here, nothing resolves them.
}

UNIT_TEST(NativePath, WithExtension) {
    auto testOne = [] (std::string_view path, std::string_view extension, std::string_view result) {
        EXPECT_EQ(NativePath(path).withExtension(extension).str(), result)
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

#ifdef _WINDOWS
UNIT_TEST(NativePath, WindowsRoots) {
    auto testJoin = [] (std::string_view head, std::string_view tail, std::string_view result) {
        EXPECT_EQ((NativePath(head) / NativePath(tail)).str(), result)
            << "for '" << head << "' / '" << tail << "'";
    };
    auto testExtension = [] (std::string_view path, std::string_view extension, std::string_view result) {
        EXPECT_EQ(NativePath(path).withExtension(extension).str(), result)
            << "for '" << path << "' with '" << extension << "'";
    };
    auto testNative = [] (std::string_view path, const std::wstring &result) {
        EXPECT_EQ(NativePath(path).native(), result) << "for '" << path << "'";
    };

    EXPECT_EQ(NativePath("a\\b").str(), "a/b"); // Both slashes separate components on Windows.

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
    testJoin("//?/UNC//share/a", "/b", "//?/UNC/b");

    // Extended-length and device paths go to Win32 with backslashes.
    testNative("//?/C:/Games", L"\\\\?\\C:\\Games");
    testNative("//./UNC/server/share/f", L"\\\\.\\UNC\\server\\share\\f");
    testNative("C:/Games/MM7", L"C:/Games/MM7"); // Everything else keeps them.
    EXPECT_EQ(NativePath::fromNative(L"C:\\a\\b").str(), "C:/a/b"); // Separators from the OS get converted too.

    // A root name is never a file name, so a dot inside one doesn't start an extension.
    testExtension("//ser.ver/sh.are", "", "//ser.ver/sh.are");
    testExtension("//ser.ver/sh.are/a.txt", "", "//ser.ver/sh.are/a");

    // A UNC root name has no file name, so an extension starts one under it rather than renaming the share.
    testExtension("//server/share", ".x", "//server/share/.x");
}

UNIT_TEST(NativePath, ExtendedLengthReachesWin32) {
    // Win32 only honors a literal "\\?\". With forward slashes a path over MAX_PATH fails to open, and a trailing dot
    // gets stripped off the file name.

    // A single component is capped at 255 characters, so it takes two to get over MAX_PATH wherever temp is.
    NativePath dir = fs::tmp() / NativePath("oe_" + std::string(150, 'd'));
    NativePath prefixed = NativePath("//?/" + dir.str());
    ScopedTestFolder folder(dir);

    for (const std::string &name : {"oe_" + std::string(150, 'x') + ".txt", std::string("oe_trailing_dot.")}) {
        ASSERT_NO_THROW(FileOutputStream(prefixed / NativePath(name)).close()) << name;

        // Not exists(), which would look the name up the same way the write did. A listing shows the name on disk.
        EXPECT_EQ(fs::ls(prefixed), std::vector<DirectoryEntry>({{name, FILE_REGULAR}})) << name;
        EXPECT_TRUE(fs::remove(prefixed / NativePath(name))) << name;
    }
}
#endif

UNIT_TEST(NativePath, DisplayString) {
    EXPECT_EQ(NativePath("a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt").displayString(), "a/b/\xd0\xbb\xd0\xbe\xd0\xbb.txt");

    // WTF-8-encoded surrogates are not valid UTF-8, so they have to come out as replacement characters.
    std::string display = NativePath("lol\xed\xb0\x80kek.txt").displayString();
    EXPECT_TRUE(display.starts_with("lol"));
    EXPECT_TRUE(display.ends_with("kek.txt"));
    EXPECT_NE(display.find("\xEF\xBF\xBD"), std::string::npos); // U+FFFD.
    EXPECT_EQ(display.find("\xed\xb0\x80"), std::string::npos);
}

UNIT_TEST(NativePath, Comparison) {
    // Paths compare as their stored strings, so a trailing or doubled separator makes a different path, and "a/b"
    // sorts after "a.b" where std::filesystem::path would put it first.
    EXPECT_NE(NativePath("a"), NativePath("a/"));
    EXPECT_LT(NativePath("a"), NativePath("b"));
    EXPECT_GT(NativePath("a/b"), NativePath("a.b"));
}

UNIT_TEST(NativePath, IsEmpty) {
    EXPECT_TRUE(NativePath().isEmpty());
    EXPECT_TRUE(NativePath("").isEmpty());
    EXPECT_FALSE(NativePath("a").isEmpty());
}

UNIT_TEST(NativePath, Format) {
    EXPECT_EQ(fmt::format("[{:>6}]", NativePath("a/b")), "[   a/b]"); // Format specs reach the string formatter.
}

UNIT_TEST(NativePath, LexicalCast) {
    // CLI11 binds NativePath options through this.
    NativePath path;
    EXPECT_TRUE(lexical_cast(std::string("a/b"), path));
    EXPECT_EQ(path, NativePath("a/b"));
}

UNIT_TEST(NativePath, InvalidUtf8RoundTrip) {
    // Nothing is validated on the way in, so invalid UTF-8 passes through the constructor and str as-is. "\xD0" is an
    // incomplete UTF-8 sequence, "\xFF" can't appear in UTF-8 at all.
    for (std::string_view name : {"lol\xD0kek.txt", "lol\xFFkek.txt", "trailing\xD0"})
        EXPECT_EQ(NativePath(name).str(), name);
}

#ifndef _WINDOWS
UNIT_TEST(NativePath, PosixSyntax) {
    auto testExtension = [] (std::string_view path, std::string_view extension, std::string_view result) {
        EXPECT_EQ(NativePath(path).withExtension(extension).str(), result)
            << "for '" << path << "' with '" << extension << "'";
    };

    // Backslashes and Windows roots are ordinary text on POSIX, where the only separator is a forward slash.
    EXPECT_EQ(NativePath("a\\b").str(), "a\\b");
    testExtension("a.b\\c", "", "a"); // One file name, so ".b\c" is its extension.
    testExtension("C:", ".x", "C:.x");
    testExtension("//a.b", "", "//a");
}
#endif

#if !defined(_WINDOWS) && !defined(__APPLE__)
UNIT_TEST(NativePath, InvalidUtf8FileNames) {
    // A name with invalid UTF-8 in it is not just convertible, it's also usable to actually open a file. APFS is the
    // exception, it only takes file names that are valid UTF-8, so this test doesn't run on MacOS.
    NativePath tmpDir = fs::tmp(); // A build dir can sit on an APFS-backed mount in a dev container.

    for (std::string_view name : {"tmp_lol\xD0kek.txt", "tmp_lol\xFFkek.txt", "tmp_trailing\xD0"}) {
        NativePath path = tmpDir / NativePath(name);

        ASSERT_NO_THROW(FileOutputStream(path).close()) << name;
        EXPECT_TRUE(fs::exists(path)) << name;
        EXPECT_TRUE(fs::remove(path)) << name;
    }
}
#endif
