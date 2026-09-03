#include <string>
#include <filesystem>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System/Fs.h"

UNIT_TEST(Blob, FromFile) {
    Path fileName = Path("abcdefghijklmnopqrstuvwxyz.tmp");
    std::string fileContents = "abcd";

    ScopedTestFileSlot tmp(fileName);

    Blob blob;
    EXPECT_THROW(blob = Blob::fromFile(fileName), std::runtime_error);

    FileOutputStream output(fileName);
    output.write(fileContents);
    output.close();

    blob = Blob::fromFile(fileName);
    EXPECT_EQ(blob.str(), fileContents);
}

UNIT_TEST(Blob, FromEmptyFile) {
    Path fileName = Path("1.txt");
    ScopedTestFile tmp(fileName, "");

    Blob blob = Blob::fromFile(fileName); // Shouldn't throw.
    EXPECT_EQ(blob.size(), 0);
    EXPECT_TRUE(!blob);
}

UNIT_TEST(Blob, SharedFromFile) {
    Path fileName = Path("abcdefghijklmnopqrstuvwxyz1.tmp");
    std::string fileContents = "0123456789";

    ScopedTestFile tmp(fileName, fileContents);

    Blob blob = Blob::fromFile(fileName);
    EXPECT_EQ(blob.str(), fileContents);

    Blob subBlob = blob.subBlob(5, 5);
    EXPECT_EQ(subBlob.str(), "56789");

    blob = Blob(); // Release original blob.
    EXPECT_EQ(subBlob.str(), "56789");
}

UNIT_TEST(Blob, DisplayPathCopyShare) {
    Blob blob = Blob::fromString("123").withDisplayPath("1.bin");

    EXPECT_EQ(blob.displayPath(), "1.bin");
    EXPECT_EQ(Blob::copy(blob).displayPath(), "1.bin");
    EXPECT_EQ(Blob::share(blob).displayPath(), "1.bin");
}

UNIT_TEST(Blob, DisplayPathFromFile) {
    Path fileName = Path("1.bin");
    ScopedTestFile tmp(fileName, "123");

    std::string displayPath = Blob::fromFile(fileName).displayPath();
    EXPECT_TRUE(displayPath.ends_with("1.bin"));
    EXPECT_TRUE(std::filesystem::path(displayPath).is_absolute());
}

UNIT_TEST(Blob, DisplayPathFromEmptyFile) {
    Path fileName = Path("1.txt");
    ScopedTestFile tmp(fileName, "");

    std::string displayPath = Blob::fromFile(fileName).displayPath();
    EXPECT_TRUE(displayPath.ends_with("1.txt"));
    EXPECT_TRUE(std::filesystem::path(displayPath).is_absolute());
}

UNIT_TEST(Blob, DisplayPathFromStream) {
    Path fileName = Path("1.bin");
    ScopedTestFile tmp(fileName, "123");

    FileInputStream in(fileName);
    std::string displayPath = Blob::read(&in, 2).displayPath();
    EXPECT_TRUE(displayPath.ends_with("1.bin"));
    EXPECT_TRUE(std::filesystem::path(displayPath).is_absolute());
}

UNIT_TEST(Blob, ExceptionMessages) {
    Path fileName = Path("lknjdfgsbiuherqbhvdfnjkkvsdhjkweqguy.txt");

    EXPECT_FALSE(fs::exists(fileName));
    EXPECT_THROW_MESSAGE((void) Blob::fromFile(fileName), fileName.string());
}
