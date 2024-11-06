#include <filesystem>
#include <ranges>
#include <string>
#include <vector>
#include <memory>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Directory/DirectoryFileSystem.h"

#include "Utility/Streams/FileOutputStream.h"

class TemporaryDir {
 public:
    explicit TemporaryDir(std::string_view name): _name(name) {
        if (std::filesystem::exists(_name))
            std::filesystem::remove_all(_name);

        std::filesystem::create_directory(name);
        EXPECT_TRUE(std::filesystem::exists(name));
    }

    ~TemporaryDir() {
        std::filesystem::remove_all(_name);
        EXPECT_FALSE(std::filesystem::exists(_name));
    }

 private:
    std::string _name;
};

UNIT_TEST(DirectoryFileSystem, LsRoot) {
    // Make sure passing empty paths works as intended.
    ScopedTestFile tmp("1.txt", "");

    DirectoryFileSystem fs1(""); // Current dir.
    std::vector<DirectoryEntry> entries = fs1.ls("");
    EXPECT_TRUE(std::ranges::find(entries, "1.txt", &DirectoryEntry::name) != std::ranges::end(entries))
        << "size = " << entries.size() << ", [0] = " << (entries.empty() ? "<nothing>" : entries[0].name);

    DirectoryFileSystem fs2("this_dir_doesnt_exist"); // Non-existent dir.
    EXPECT_TRUE(fs2.ls("").empty());

    DirectoryFileSystem fs3("1.txt"); // Not-a-dir.
    EXPECT_TRUE(fs3.ls("").empty());
}

UNIT_TEST(DirectoryFileSystem, LsFile) {
    // Make sure ls() throws when called on a file.
    ScopedTestFile tmp("1.txt", "");

    DirectoryFileSystem fs(""); // Current dir.
    EXPECT_ANY_THROW((void) fs.ls("1.txt"));
}

UNIT_TEST(DirectoryFileSystem, LsNonExistent) {
    // Make sure ls() throws when called on a folder that doesn't exist.
    DirectoryFileSystem fs(""); // Current dir.
    EXPECT_ANY_THROW((void) fs.ls("this_dir_doesnt_exist"));
}

UNIT_TEST(DirectoryFileSystem, ExistsRoot) {
    // Make sure exists("") works as intented.
    DirectoryFileSystem fs1(""); // Current dir.
    EXPECT_TRUE(fs1.exists(""));

    DirectoryFileSystem fs2("this_dir_doesnt_exist");
    EXPECT_TRUE(fs2.exists(""));

    ScopedTestFile tmp("1.txt", "");
    DirectoryFileSystem fs3("1.txt");
    EXPECT_TRUE(fs3.exists(""));
}

UNIT_TEST(DirectoryFileSystem, StatRoot) {
    // Make sure stat("") works as intented.
    DirectoryFileSystem fs1(""); // Current dir.
    EXPECT_EQ(fs1.stat("").type, FILE_DIRECTORY);

    DirectoryFileSystem fs2("this_dir_doesnt_exist"); // Non-existent dir.
    EXPECT_EQ(fs2.stat("").type, FILE_DIRECTORY);

    ScopedTestFile tmp("1.txt", "");
    DirectoryFileSystem fs3("1.txt"); // Not-a-dir.
    EXPECT_EQ(fs3.stat("").type, FILE_DIRECTORY);
}

UNIT_TEST(DirectoryFileSystem, ReadRootAsFile) {
    // Root is always assumed to be a dir, we can't read it as a file even if it IS a file.
    ScopedTestFile tmp("1.txt", "");

    DirectoryFileSystem fs("1.txt");
    EXPECT_ANY_THROW((void) fs.read(""));
}

UNIT_TEST(DirectoryFileSystem, WriteRootAsFile) {
    // Root is always assumed to be a dir, we can't write it as a file if it doesn't exist.
    DirectoryFileSystem fs("1.txt");
    EXPECT_ANY_THROW(fs.write("", Blob()));
}

UNIT_TEST(DirectoryFileSystem, DisplayPathSymmetry) {
    ScopedTestFile tmp("1.txt", "");

    DirectoryFileSystem fs("");
    Blob blob = fs.read("1.txt");
    std::unique_ptr<InputStream> stream = fs.openForReading("1.txt");

    EXPECT_TRUE(blob.displayPath().ends_with("1.txt"));
    EXPECT_TRUE(std::filesystem::path(blob.displayPath()).is_absolute());
    EXPECT_EQ(blob.displayPath(), stream->displayPath());
}

UNIT_TEST(DirectoryFileSystem, EscapingPaths) {
    TemporaryDir tmp("a");
    ScopedTestFile tmp2("1.txt", "");
    ScopedTestFile tmp3("a/1.txt", "");

    DirectoryFileSystem fs("a");

    EXPECT_FALSE(fs.exists(".."));
    EXPECT_FALSE(fs.stat(".."));
    EXPECT_ANY_THROW((void) fs.ls(".."));
    EXPECT_ANY_THROW((void) fs.read("../1.txt"));
    EXPECT_ANY_THROW((void) fs.openForReading("../1.txt"));
    EXPECT_ANY_THROW(fs.write("../1.txt", Blob()));
    EXPECT_ANY_THROW((void) fs.openForWriting("../1.txt"));
    EXPECT_ANY_THROW(fs.remove("../1.txt"));
    EXPECT_ANY_THROW(fs.rename("../1.txt", "2.txt"));
    EXPECT_ANY_THROW(fs.rename("1.txt", "../2.txt"));
}

UNIT_TEST(DirectoryFileSystem, EscapingDisplayPath) {
    DirectoryFileSystem fs("");

    EXPECT_TRUE(fs.displayPath("..").ends_with(".."));
}
