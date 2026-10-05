#include <ranges>
#include <string>
#include <vector>
#include <memory>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Native/NativeFileSystem.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Encoding.h"
#include "Utility/System/Fs.h"

UNIT_TEST(NativeFileSystem, LsRoot) {
    // Make sure passing empty paths works as intended.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs1(fs::cwd());
    std::vector<DirectoryEntry> entries = fs1.ls("");
    EXPECT_TRUE(std::ranges::find(entries, "1.txt", &DirectoryEntry::name) != std::ranges::end(entries))
        << "size = " << entries.size() << ", [0] = " << (entries.empty() ? "<nothing>" : entries[0].name);

    NativeFileSystem fs2(Path("this_dir_doesnt_exist")); // Non-existent dir.
    EXPECT_TRUE(fs2.ls("").empty());

    NativeFileSystem fs3(Path("1.txt")); // Not-a-dir.
    EXPECT_TRUE(fs3.ls("").empty());
}

UNIT_TEST(NativeFileSystem, LsFile) {
    // Make sure ls() throws when called on a file.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs(fs::cwd());
    EXPECT_THROW_MESSAGE((void) fs.ls("1.txt"), "is not a directory");
}

UNIT_TEST(NativeFileSystem, LsNonExistent) {
    // Make sure ls() throws when called on a folder that doesn't exist.
    NativeFileSystem fs(fs::cwd());
    EXPECT_THROW_MESSAGE((void) fs.ls("this_dir_doesnt_exist"), "doesn't exist");
}

UNIT_TEST(NativeFileSystem, ExistsRoot) {
    // Make sure exists("") works as intented.
    NativeFileSystem fs1(fs::cwd());
    EXPECT_TRUE(fs1.exists(""));

    NativeFileSystem fs2(Path("this_dir_doesnt_exist"));
    EXPECT_TRUE(fs2.exists(""));

    ScopedTestFile tmp("1.txt", "");
    NativeFileSystem fs3(Path("1.txt"));
    EXPECT_TRUE(fs3.exists(""));
}

UNIT_TEST(NativeFileSystem, StatRoot) {
    // Make sure stat("") works as intented.
    NativeFileSystem fs1(fs::cwd());
    EXPECT_EQ(fs1.stat("").type, FILE_DIRECTORY);

    NativeFileSystem fs2(Path("this_dir_doesnt_exist")); // Non-existent dir.
    EXPECT_EQ(fs2.stat("").type, FILE_DIRECTORY);

    ScopedTestFile tmp("1.txt", "");
    NativeFileSystem fs3(Path("1.txt")); // Not-a-dir.
    EXPECT_EQ(fs3.stat("").type, FILE_DIRECTORY);
}

UNIT_TEST(NativeFileSystem, ReadRootAsFile) {
    // Root is always assumed to be a dir, we can't read it as a file even if it IS a file.
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs(Path("1.txt"));
    EXPECT_ANY_THROW((void) fs.read(""));
}

UNIT_TEST(NativeFileSystem, WriteRootAsFile) {
    // Root is always assumed to be a dir, we can't write it as a file if it doesn't exist.
    NativeFileSystem fs(Path("1.txt"));
    EXPECT_ANY_THROW(fs.write("", Blob()));
}

UNIT_TEST(NativeFileSystem, WriteCreatesDirs) {
    // Writing creates the missing directories, the root included.
    ScopedTestFolder dir("tmp_dfs_dir");
    NativeFileSystem fs(Path("tmp_dfs_dir/root"));

    fs.write("1.txt", Blob::fromString("a"));
    fs.openForWriting("a/b/2.txt")->write("bc");
    EXPECT_EQ(fs.read("1.txt").str(), "a");
    EXPECT_EQ(fs.read("a/b/2.txt").str(), "bc");
}

#ifndef _WINDOWS
UNIT_TEST(NativeFileSystem, LsHidesBackslashNames) {
    // A backslash is an ordinary character in a POSIX file name, but a separator in a FileSystem path.
    ScopedTestFolder dir("tmp_dfs_backslash");
    ScopedTestFile tmp("tmp_dfs_backslash/a\\b.txt", "");

    NativeFileSystem fs(Path("tmp_dfs_backslash"));
    EXPECT_TRUE(fs.ls("").empty());
}
#endif

UNIT_TEST(NativeFileSystem, DisplayPathSymmetry) {
    ScopedTestFile tmp("1.txt", "");

    NativeFileSystem fs(fs::cwd());
    Blob blob = fs.read("1.txt");
    std::unique_ptr<InputStream> stream = fs.openForReading("1.txt");

    EXPECT_EQ(blob.displayPath(), (fs::cwd() / Path("1.txt")).displayString());
    EXPECT_EQ(blob.displayPath(), stream->displayPath());
}

UNIT_TEST(NativeFileSystem, EscapingPaths) {
    ScopedTestFolder tmp("a");
    ScopedTestFile tmp2("1.txt", "");
    ScopedTestFile tmp3("a/1.txt", "");

    NativeFileSystem fs(Path("a"));

    EXPECT_FALSE(fs.exists(".."));
    EXPECT_FALSE(fs.stat(".."));
    EXPECT_ANY_THROW((void) fs.ls(".."));
    EXPECT_ANY_THROW((void) fs.read("../1.txt"));
    EXPECT_ANY_THROW((void) fs.openForReading("../1.txt"));
    EXPECT_ANY_THROW(fs.write("../1.txt", Blob()));
    EXPECT_ANY_THROW((void) fs.openForWriting("../1.txt"));
    EXPECT_ANY_THROW(fs.remove("../1.txt"));
}

UNIT_TEST(NativeFileSystem, EscapingDisplayPath) {
    NativeFileSystem fs(fs::cwd());

    EXPECT_TRUE(fs.displayPath("..").ends_with(".."));
}

UNIT_TEST(NativeFileSystem, DisplayPathOfMissingFile) {
    // FileSystemException formats the very path it complains about, so displayPath has to work for a missing file.
    ScopedTestFolder tmp("tmp_native_dir");
    NativeFileSystem fs("tmp_native_dir");

    EXPECT_EQ(fs.displayPath("a/doesnt_exist.txt"), (fs::absolute("tmp_native_dir") / Path("a/doesnt_exist.txt")).displayString());
}

UNIT_TEST(NativeFileSystem, NonAsciiFileNames) {
    // A non-ASCII name has to come back from ls and stay usable.
    ScopedTestFolder tmp("tmp_native_dir");
    ScopedTestFile tmp2("tmp_native_dir/\xD0\xBB\xD0\xBE\xD0\xBB.txt", "lol"); // "лол.txt" in UTF-8.

    NativeFileSystem fs("tmp_native_dir");
    std::vector<DirectoryEntry> entries = fs.ls("");
    ASSERT_EQ(entries.size(), 1);

    std::string name = entries[0].name;
    EXPECT_EQ(name, "\xD0\xBB\xD0\xBE\xD0\xBB.txt");
    EXPECT_TRUE(fs.exists(name));
    EXPECT_EQ(fs.stat(name), FileStat(FILE_REGULAR, 3));
    EXPECT_EQ(fs.read(name).str(), "lol");
    EXPECT_EQ(fs.openForReading(name)->readAll(), "lol");

    fs.write(name + ".2", Blob::fromString("kek"));
    entries = fs.ls("");
    EXPECT_TRUE(std::ranges::find(entries, name + ".2", &DirectoryEntry::name) != std::ranges::end(entries));
}

#ifdef _WINDOWS
UNIT_TEST(NativeFileSystem, WindowsOddFileNames) {
    // Win32 doesn't validate UTF-16 in file names, so unpaired surrogates and non-characters are all valid.
    const wchar_t *nativeNames[] = {
        L"lol\xDC00kek.txt", // Unpaired trail surrogate.
        L"lol\xD800kek.txt", // Unpaired lead surrogate.
        L"lol\xD800", // Lead surrogate at the very end.
        L"lol\xFFFE\xFFFF\xFDD0kek.txt", // Non-characters.
        L"lol\xD83D\xDE00kek.txt", // A valid pair.
    };

    for (const wchar_t *nativeName : nativeNames) {
        ScopedTestFolder tmp("tmp_native_dir");
        Path nativePath = Path("tmp_native_dir") / Path::fromNative(nativeName);
        ScopedTestFile file(nativePath, "lol");

        NativeFileSystem fs("tmp_native_dir");
        std::vector<DirectoryEntry> entries = fs.ls("");
        ASSERT_EQ(entries.size(), 1u) << txt::wideToWtf8(nativeName);
        std::string name = entries[0].name;

        EXPECT_EQ(txt::wtf8ToWide(name), nativeName);
        EXPECT_TRUE(fs.exists(name));
        EXPECT_EQ(fs.stat(name), FileStat(FILE_REGULAR, 3));
        EXPECT_EQ(fs.read(name).str(), "lol");
        EXPECT_EQ(fs.openForReading(name)->readAll(), "lol");
        EXPECT_EQ(fs.displayPath(name), fs::absolute(nativePath).displayString());

        fs.write(name + ".2", Blob::fromString("kek"));
        EXPECT_TRUE(fs::exists(Path(nativePath.str() + ".2")));
    }
}

UNIT_TEST(NativeFileSystem, WindowsSurrogateBytes) {
    // An unpaired surrogate is WTF-8 encoded like any other code point, so \xDC00 lists as ED B0 80.
    ScopedTestFolder tmp("tmp_native_dir");
    ScopedTestFile file(Path("tmp_native_dir") / Path::fromNative(L"lol\xDC00kek.txt"), "lol");

    NativeFileSystem fs("tmp_native_dir");
    std::vector<DirectoryEntry> entries = fs.ls("");
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].name, "lol\xED\xB0\x80kek.txt");
}
#endif
