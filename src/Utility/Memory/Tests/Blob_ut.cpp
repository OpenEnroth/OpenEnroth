#include <gtest/gtest.h>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
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
