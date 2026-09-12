#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

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

UNIT_TEST(NativePath, NativeRoundTrip) {
    // The conversion to the OS encoding goes through wchar_t on Windows, so WTF-8 has to survive it, unpaired
    // surrogates included. That is the whole reason this class speaks WTF-8 rather than UTF-8.
    for (std::string_view path : {"a/b/c.txt", "\xd0\xbb\xd0\xbe\xd0\xbb.txt", "lol\xed\xb0\x80kek.txt"})
        EXPECT_EQ(NativePath::fromNative(NativePath::fromWtf8(path).native()).toWtf8(), path);
}

UNIT_TEST(NativePath, Literals) {
    // ASCII literals construct directly, everything else goes through fromWtf8.
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

    // An absolute tail replaces the head outright.
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

    // A path with no file name in it grows one, so the extension is all that's left of the last component.
    EXPECT_EQ(NativePath("").withExtension(".x").toWtf8(), ".x");
    EXPECT_EQ(NativePath("a/").withExtension(".x").toWtf8(), "a/.x");
    EXPECT_EQ(NativePath("/").withExtension(".x").toWtf8(), "/.x");
}

UNIT_TEST(NativePath, DottedNames) {
    // A name whose stem would be all dots has no extension, so that dropping the extension can't turn a file name
    // into a navigation token. The stem of "..." is "..", so "a/..." would otherwise become the parent of "a".
    // std::filesystem splits these the other way round, so these are ours rather than inherited.
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
    EXPECT_EQ((NativePath("C:") / NativePath("b")).toWtf8(), "C:b"); // Drive-relative, no separator inserted.
    EXPECT_EQ((NativePath("//server/share") / NativePath("f")).toWtf8(), "//server/share/f");

    // A bare drive letter is drive-relative, but a bare share name is already absolute. So a separator does go in
    // after it, and it replaces whatever it's appended to.
    EXPECT_EQ((NativePath("//server") / NativePath("share")).toWtf8(), "//server/share");
    EXPECT_EQ((NativePath("//server/share") / NativePath("//server")).toWtf8(), "//server");
    EXPECT_EQ((NativePath("//server") / NativePath("/share")).toWtf8(), "//server/share");
    EXPECT_EQ((NativePath("C:a") / NativePath("b")).toWtf8(), "C:a/b"); // Drive-relative with a name appends normally.

    // An empty tail leaves a separator only where the head can take one, and a bare drive letter can't.
    EXPECT_EQ((NativePath("C:") / NativePath("")).toWtf8(), "C:");
    EXPECT_EQ((NativePath("C:/a") / NativePath("")).toWtf8(), "C:/a/");
    EXPECT_EQ((NativePath("//server") / NativePath("")).toWtf8(), "//server/");

    // An extended-length path takes no forward slashes, Win32 does no parsing on those at all.
    EXPECT_EQ(NativePath::fromWtf8("\\\\?\\C:\\Games\\MM7").native(), L"\\\\?\\C:\\Games\\MM7");
    EXPECT_EQ(NativePath::fromWtf8("//?/C:/Games").native(), L"\\\\?\\C:\\Games");
    EXPECT_EQ(NativePath::fromWtf8("\\\\.\\COM1").native(), L"\\\\.\\COM1");
    EXPECT_EQ(NativePath::fromWtf8("C:/Games/MM7").native(), L"C:/Games/MM7"); // Everything else keeps them.

    // A root name is never a file name, so a dot inside one doesn't start an extension.
    EXPECT_EQ(NativePath("C:").withExtension(".x").toWtf8(), "C:.x");
    EXPECT_EQ(NativePath("//ser.ver").withExtension("").toWtf8(), "//ser.ver");
    EXPECT_EQ(NativePath("//ser.ver/a.txt").withExtension("").toWtf8(), "//ser.ver/a");
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
