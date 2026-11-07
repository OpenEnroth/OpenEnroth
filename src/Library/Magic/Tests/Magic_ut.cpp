#include <cstdint>
#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Library/Magic/Magic.h"

#include "Utility/Memory/Blob.h"

static std::string header(std::string_view signature, uint32_t a, uint32_t b, size_t size) {
    std::string result(size, '\0');
    result.replace(0, 4, signature);
    for (int i = 0; i < 4; i++) {
        result[4 + i] = static_cast<char>(a >> (8 * i));
        result[8 + i] = static_cast<char>(b >> (8 * i));
    }
    return result;
}

UNIT_TEST(Magic, Smacker) {
    // Smacker videos are recognized by their signature and a non-empty frame size.
    EXPECT_EQ(magic(Blob::fromString(header("SMK2", 640, 480, 104))), MAGIC_SMK);
    EXPECT_EQ(magic(Blob::fromString(header("SMK4", 640, 480, 104))), MAGIC_SMK);
    EXPECT_EQ(magic(Blob::fromString(header("SMK2", 0, 480, 104))), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(Blob::fromString(header("SMK2", 640, 480, 103))), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(Blob::fromString(header("SMK3", 640, 480, 104))), MAGIC_UNRECOGNIZED);
}

UNIT_TEST(Magic, Bink) {
    // Bink videos are recognized by their signature and a file size that matches the header.
    EXPECT_EQ(magic(Blob::fromString(header("BIKf", 92, 0, 100))), MAGIC_BIK);
    EXPECT_EQ(magic(Blob::fromString(header("BIKh", 92, 0, 100))), MAGIC_BIK);
    EXPECT_EQ(magic(Blob::fromString(header("BIKf", 93, 0, 100))), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(Blob::fromString(header("BIGf", 92, 0, 100))), MAGIC_UNRECOGNIZED);
}
