#include <cstdint>
#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Library/Binary/CommonSerialization.h"
#include "Library/Magic/Magic.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"

static Blob header(std::string_view signature, uint32_t a, uint32_t b, size_t size) {
    Blob result;
    BlobOutputStream stream(&result);
    stream.write(signature);
    serialize(a, &stream);
    serialize(b, &stream);
    stream.write(std::string(size - 12, '\0'));
    stream.close();
    return result;
}

UNIT_TEST(Magic, Smacker) {
    // Smacker videos are recognized by their signature and a non-empty frame size.
    EXPECT_EQ(magic(header("SMK2", 640, 480, 104)), MAGIC_SMK);
    EXPECT_EQ(magic(header("SMK4", 640, 480, 104)), MAGIC_SMK);
    EXPECT_EQ(magic(header("SMK2", 0, 480, 104)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(header("SMK2", 640, 480, 103)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(header("SMK3", 640, 480, 104)), MAGIC_UNRECOGNIZED);
}

UNIT_TEST(Magic, Bink) {
    // Bink 1 and Bink 2 videos are recognized by their signature and a file size that matches the header.
    EXPECT_EQ(magic(header("BIKf", 92, 0, 100)), MAGIC_BIK);
    EXPECT_EQ(magic(header("BIKh", 92, 0, 100)), MAGIC_BIK);
    EXPECT_EQ(magic(header("BIKb", 92, 0, 100)), MAGIC_BIK);
    EXPECT_EQ(magic(header("KB2a", 92, 0, 100)), MAGIC_BIK);
    EXPECT_EQ(magic(header("KB2j", 92, 0, 100)), MAGIC_BIK);
    EXPECT_EQ(magic(header("BIK1", 92, 0, 100)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(header("KB2b", 92, 0, 100)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(header("BIGf", 92, 0, 100)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(header("BIKf", 93, 0, 100)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(header("BIKf", 35, 0, 43)), MAGIC_UNRECOGNIZED);
}
