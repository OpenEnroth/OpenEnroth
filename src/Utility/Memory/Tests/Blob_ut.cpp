#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Testing/TestExistingFile.h"
#include "Utility/Testing/TestNonExistingFile.h"

UNIT_TEST(Blob, FromFile) {
    std::string fileName = "abcdefghijklmnopqrstuvwxyz.tmp";
    std::string fileContents = "abcd";

    TestNonExistingFile tmp(fileName);

    Blob blob;
    EXPECT_THROW(blob = Blob::fromFile(fileName), std::runtime_error);

    FileOutputStream output(fileName);
    output.write(fileContents);
    output.close();

    blob = Blob::fromFile(fileName);
    EXPECT_EQ(blob.string_view(), fileContents);
}

UNIT_TEST(Blob, FromEmptyFile) {
    TestExistingFile tmp("1.txt", "");

    Blob blob = Blob::fromFile("1.txt"); // Shouldn't throw.
    EXPECT_EQ(blob.size(), 0);
    EXPECT_TRUE(!blob);
}

UNIT_TEST(Blob, SharedFromFile) {
    std::string fileName = "abcdefghijklmnopqrstuvwxyz1.tmp";
    std::string fileContents = "0123456789";

    TestExistingFile tmp(fileName, fileContents);

    Blob blob = Blob::fromFile(fileName);
    EXPECT_EQ(blob.string_view(), fileContents);

    Blob subBlob = blob.subBlob(5, 5);
    EXPECT_EQ(subBlob.string_view(), "56789");

    blob = Blob(); // Release original blob.
    EXPECT_EQ(subBlob.string_view(), "56789");
}

UNIT_TEST(Blob, DisplayPathCopyShare) {
    Blob blob = Blob::fromString("123").withDisplayPath("1.bin");

    EXPECT_EQ(blob.displayPath(), "1.bin");
    EXPECT_EQ(Blob::copy(blob).displayPath(), "1.bin");
    EXPECT_EQ(Blob::share(blob).displayPath(), "1.bin");
}

UNIT_TEST(Blob, DisplayPathFromFile) {
    TestExistingFile tmp("1.bin", "123");

    EXPECT_EQ(Blob::fromFile("1.bin").displayPath(), "1.bin");
}

UNIT_TEST(Blob, DisplayPathFromStream) {
    TestExistingFile tmp("1.bin", "123");

    FileInputStream in("1.bin");
    EXPECT_EQ(Blob::read(&in, 2).displayPath(), "1.bin");
}
