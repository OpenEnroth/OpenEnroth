#include <filesystem>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileOutputStream.h"

UNIT_TEST(Blob, FromFile) {
    std::string fileName = "abcdefghijklmnopqrstuvwxyz.tmp";
    std::string fileContents = "abcd";

    auto cleanup = [&] {
        std::error_code ec;
        std::filesystem::remove(fileName, ec);
    };
    cleanup(); // Just in case.

    Blob blob;
    EXPECT_THROW(blob = Blob::fromFile(fileName), std::runtime_error);

    FileOutputStream output(fileName);
    output.write(fileContents);
    output.close();

    blob = Blob::fromFile(fileName);
    EXPECT_EQ(blob.string_view(), fileContents);

    cleanup();
}

UNIT_TEST(Blob, SharedFromFile) {
    std::string fileName = "abcdefghijklmnopqrstuvwxyz1.tmp";
    std::string fileContents = "0123456789";

    auto cleanup = [&] {
        std::error_code ec;
        std::filesystem::remove(fileName, ec);
    };
    cleanup(); // Just in case.

    FileOutputStream output(fileName);
    output.write(fileContents);
    output.close();

    Blob blob = Blob::fromFile(fileName);
    EXPECT_EQ(blob.string_view(), fileContents);

    Blob subBlob = blob.subBlob(5, 5);
    EXPECT_EQ(subBlob.string_view(), "56789");

    blob = Blob(); // Release original blob.
    EXPECT_EQ(subBlob.string_view(), "56789");

    cleanup();
}
