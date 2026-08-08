#include <filesystem>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>
#include <memory>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Native/NativeFileSystem.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Encoding.h"

UNIT_TEST(NativeFileSystem, LsRoot) {
    // Make sure passing empty paths works as intended.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs1(""); // Current dir.
    std::vector<DirectoryEntry> entries = fs1.ls("");
    EXPECT_TRUE(std::ranges::find(entries, "1.txt", &DirectoryEntry::name) != std::ranges::end(entries))
        << "size = " << entries.size() << ", [0] = " << (entries.empty() ? "<nothing>" : entries[0].name);

    NativeFileSystem fs2("this_dir_doesnt_exist"); // Non-existent dir.
    EXPECT_TRUE(fs2.ls("").empty());

    NativeFileSystem fs3("1.txt"); // Not-a-dir.
    EXPECT_TRUE(fs3.ls("").empty());
}

UNIT_TEST(NativeFileSystem, LsFile) {
    // Make sure ls() throws when called on a file.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs(""); // Current dir.
    EXPECT_ANY_THROW((void) fs.ls("1.txt"));
}

UNIT_TEST(NativeFileSystem, LsNonExistent) {
    // Make sure ls() throws when called on a folder that doesn't exist.
    NativeFileSystem fs(""); // Current dir.
    EXPECT_ANY_THROW((void) fs.ls("this_dir_doesnt_exist"));
}

UNIT_TEST(NativeFileSystem, ExistsRoot) {
    // Make sure exists("") works as intented.
    NativeFileSystem fs1(""); // Current dir.
    EXPECT_TRUE(fs1.exists(""));

    NativeFileSystem fs2("this_dir_doesnt_exist");
    EXPECT_TRUE(fs2.exists(""));

    ScopedTestFile tmp("1.txt", "");
    NativeFileSystem fs3("1.txt");
    EXPECT_TRUE(fs3.exists(""));
}

UNIT_TEST(NativeFileSystem, StatRoot) {
    // Make sure stat("") works as intented.
    NativeFileSystem fs1(""); // Current dir.
    EXPECT_EQ(fs1.stat("").type, FILE_DIRECTORY);

    NativeFileSystem fs2("this_dir_doesnt_exist"); // Non-existent dir.
    EXPECT_EQ(fs2.stat("").type, FILE_DIRECTORY);

    ScopedTestFile tmp("1.txt", "");
    NativeFileSystem fs3("1.txt"); // Not-a-dir.
    EXPECT_EQ(fs3.stat("").type, FILE_DIRECTORY);
}

UNIT_TEST(NativeFileSystem, ReadRootAsFile) {
    // Root is always assumed to be a dir, we can't read it as a file even if it IS a file.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs("1.txt");
    EXPECT_ANY_THROW((void) fs.read(""));
}

UNIT_TEST(NativeFileSystem, WriteRootAsFile) {
    // Root is always assumed to be a dir, we can't write it as a file if it doesn't exist.
    NativeFileSystem fs("1.txt");
    EXPECT_ANY_THROW(fs.write("", Blob()));
}

UNIT_TEST(NativeFileSystem, DisplayPathSymmetry) {
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs("");
    Blob blob = fs.read("1.txt");
    std::unique_ptr<InputStream> stream = fs.openForReading("1.txt");

    EXPECT_TRUE(blob.displayPath().ends_with("1.txt"));
    EXPECT_TRUE(std::filesystem::path(blob.displayPath()).is_absolute());
    EXPECT_EQ(blob.displayPath(), stream->displayPath());
}

UNIT_TEST(NativeFileSystem, EscapingPaths) {
    ScopedTestFolder tmp("a");
    ScopedTestFile tmp2("1.txt", "");
    ScopedTestFile tmp3("a/1.txt", "");

    NativeFileSystem fs("a");

    EXPECT_FALSE(fs.exists(".."));
    EXPECT_FALSE(fs.stat(".."));
    EXPECT_ANY_THROW((void) fs.ls(".."));
    EXPECT_ANY_THROW((void) fs.read("../1.txt"));
    EXPECT_ANY_THROW((void) fs.openForReading("../1.txt"));
    EXPECT_ANY_THROW(fs.write("../1.txt", Blob()));
    EXPECT_ANY_THROW((void) fs.openForWriting("../1.txt"));
    EXPECT_ANY_THROW(fs.remove("../1.txt"));
}

UNIT_TEST(NativeFileSystem, DisplayPath) {
    ScopedTestFolder tmp("tmp_native_dir");
    NativeFileSystem fs("tmp_native_dir");

    // displayPath must work for a path that doesn't exist - FileSystemException calls it on the very path it is
    // about to complain about.
    std::string path = fs.displayPath("a/doesnt_exist.txt");
    EXPECT_TRUE(std::filesystem::path(path).is_absolute());
    EXPECT_EQ(path.find('\\'), std::string::npos);
    EXPECT_TRUE(path.ends_with("tmp_native_dir/a/doesnt_exist.txt"));

    // Root maps to the root directory itself, with no trailing separator.
    EXPECT_EQ(std::filesystem::path(fs.displayPath("")), std::filesystem::absolute("tmp_native_dir"));

    // And what comes out of displayPath can be fed back into fromNativePath.
    auto [rootFs, rootPath] = NativeFileSystem::fromNativePath(path);
    EXPECT_EQ(rootFs->displayPath(rootPath), path);
}

UNIT_TEST(NativeFileSystem, EscapingDisplayPath) {
    NativeFileSystem fs("");

    EXPECT_TRUE(fs.displayPath("..").ends_with(".."));
}

UNIT_TEST(NativeFileSystem, FromNativePathIsCwdRelative) {
    // The returned path keeps every component of the cwd, that's the point of rooting the fs at the fs root.
    ScopedTestFile tmp("1.txt", "lol");

    auto [fs, path] = NativeFileSystem::fromNativePath("1.txt");
    EXPECT_EQ(fs->read(path).str(), "lol");

    auto [cwdFs, cwdPath] = NativeFileSystem::fromNativePath("");
    EXPECT_EQ(cwdPath / "1.txt", path); // Empty path means the cwd.
    EXPECT_EQ(std::filesystem::current_path(), std::filesystem::path(cwdFs->displayPath(cwdPath)));
}

UNIT_TEST(NativeFileSystem, FromNativePathIsRootedAtRoot) {
    // Whatever we pass in, the fs we get back is rooted at the root of the native fs.
    for (std::string_view nativePath : {"", "1.txt", "a/b/c", ".."}) {
        auto [fs, path] = NativeFileSystem::fromNativePath(nativePath);

        std::filesystem::path displayPath(fs->displayPath(path));
        EXPECT_TRUE(displayPath.is_absolute()) << nativePath;
        EXPECT_EQ(std::filesystem::path(fs->displayPath("")), displayPath.root_path()) << nativePath;
    }
}

UNIT_TEST(NativeFileSystem, FromNativePathNormalizes) {
    // ".." is collapsed lexically, so the result can never escape - not even for a path walking out of the root.
    auto [fs, path] = NativeFileSystem::fromNativePath("a/../b/./c");
    EXPECT_TRUE(path.string().ends_with("b/c"));
    EXPECT_FALSE(path.isEscaping());

    auto [rootFs, rootPath] = NativeFileSystem::fromNativePath("/../../..");
    EXPECT_TRUE(rootPath.isEmpty());
    EXPECT_FALSE(rootPath.isEscaping());
}

UNIT_TEST(NativeFileSystem, FromNativePathAbsolute) {
    // An absolute path comes back unchanged, minus the root.
    ScopedTestFolder tmp("tmp_native_dir");
    ScopedTestFile tmp2("tmp_native_dir/1.txt", "kek");

    std::filesystem::path absolutePath = std::filesystem::absolute("tmp_native_dir/1.txt");
    auto [fs, path] = NativeFileSystem::fromNativePath(absolutePath.generic_string());

    EXPECT_EQ(fs->read(path).str(), "kek");
    EXPECT_EQ(std::filesystem::path(fs->displayPath(path)), absolutePath);
}

UNIT_TEST(NativeFileSystem, NonAsciiFileNames) {
    // File names go through a narrow<->wide conversion on Windows, so a non-ASCII name has to survive a round trip
    // through ls and stay usable. On *nix names are just bytes, so this mostly guards the Windows side.
    ScopedTestFolder tmp("tmp_native_dir");
    ScopedTestFile tmp2("tmp_native_dir/\xD0\xBB\xD0\xBE\xD0\xBB.txt", "lol"); // "лол.txt".

    NativeFileSystem fs("tmp_native_dir");
    std::vector<DirectoryEntry> entries = fs.ls("");
    ASSERT_EQ(entries.size(), 1);

    std::string name = entries[0].name; // A copy - we re-ls into `entries` below.
    EXPECT_TRUE(fs.exists(name));
    EXPECT_EQ(fs.stat(name), FileStat(FILE_REGULAR, 3));
    EXPECT_EQ(fs.read(name).str(), "lol");
    EXPECT_EQ(fs.openForReading(name)->readAll(), "lol");
    EXPECT_TRUE(fs.displayPath(name).ends_with(name));

    // And a name that we create ourselves comes back the same way.
    fs.write(name + ".2", Blob::fromString("kek"));
    entries = fs.ls("");
    ASSERT_EQ(entries.size(), 2);
    EXPECT_TRUE(std::ranges::find(entries, name + ".2", &DirectoryEntry::name) != std::ranges::end(entries));
}

#ifdef _WINDOWS
UNIT_TEST(NativeFileSystem, WindowsOddFileNames) {
    // Win32 does no Unicode validation on file names - unpaired surrogates and non-characters are all fine. It only
    // forbids 0x00-0x1F, `\ / : * ? " < > |`, trailing spaces and dots, and the reserved device names.
    const wchar_t *nativeNames[] = {
        L"lol\xDC00kek.txt", // Unpaired trail surrogate.
        L"lol\xD800kek.txt", // Unpaired lead surrogate.
        L"lol\xD800", // ...and a trailing one, with nothing following it to pair up with.
        L"lol\xFFFE\xFFFF\xFDD0kek.txt", // Non-characters - valid code points that nothing is supposed to produce.
        L"lol\xD83D\xDE00kek.txt", // A valid pair, i.e. an astral code point.
    };

    for (const wchar_t *nativeName : nativeNames) {
        ScopedTestFolder tmp("tmp_native_dir");

        // Can't create these through the FileSystem interface - the point is that the name isn't expressible in UTF8.
        std::filesystem::path nativePath = std::filesystem::path(L"tmp_native_dir") / nativeName;
        { std::ofstream stream(nativePath); stream << "lol"; }
        ASSERT_TRUE(std::filesystem::exists(nativePath)) << txt::wideToWtf8(nativeName);

        NativeFileSystem fs("tmp_native_dir");

        std::vector<DirectoryEntry> entries = fs.ls("");
        ASSERT_EQ(entries.size(), 1u) << txt::wideToWtf8(nativeName);
        std::string name = entries[0].name;

        // The listed name round-trips back into the original, and is usable down to opening the file.
        EXPECT_EQ(txt::wtf8ToWide(name), nativeName);
        EXPECT_TRUE(fs.exists(name));
        EXPECT_EQ(fs.stat(name), FileStat(FILE_REGULAR, 3));
        EXPECT_EQ(fs.read(name).str(), "lol");
        EXPECT_EQ(fs.openForReading(name)->readAll(), "lol");
        EXPECT_TRUE(txt::wtf8ToWide(fs.displayPath(name)).ends_with(nativeName));

        // Writing such a name works too.
        fs.write(name + ".2", Blob::fromString("kek"));
        EXPECT_TRUE(std::filesystem::exists(std::filesystem::path(nativePath.native() + L".2")));
    }
}

UNIT_TEST(NativeFileSystem, WindowsSurrogateBytes) {
    // An unpaired surrogate is WTF8-encoded like any other code point in that range - `\xDC00` becomes `ED B0 80`.
    ScopedTestFolder tmp("tmp_native_dir");

    std::filesystem::path nativePath = std::filesystem::path(L"tmp_native_dir") / L"lol\xDC00kek.txt";
    { std::ofstream stream(nativePath); stream << "lol"; }
    ASSERT_TRUE(std::filesystem::exists(nativePath));

    NativeFileSystem fs("tmp_native_dir");
    std::vector<DirectoryEntry> entries = fs.ls("");
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].name, "lol\xed\xb0\x80kek.txt");
}
#endif
