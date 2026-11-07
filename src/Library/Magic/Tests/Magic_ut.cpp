#include <cstdint>
#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Library/Binary/CommonSerialization.h"
#include "Library/Magic/Magic.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"

static Blob smackerHeader(std::string_view signature, uint32_t width, uint32_t height, size_t size) {
    Blob result;
    BlobOutputStream stream(&result);
    stream.write(signature);
    serialize(width, &stream);
    serialize(height, &stream);
    stream.write(std::string(size - 12, '\0'));
    stream.close();
    return result;
}

static Blob binkHeader(std::string_view signature, uint32_t sizeMinus8, uint32_t frameCount, uint32_t width, uint32_t height, uint32_t fps) {
    Blob result;
    BlobOutputStream stream(&result);
    stream.write(signature);
    serialize(sizeMinus8, &stream);
    serialize(frameCount, &stream);
    serialize(uint32_t(0), &stream); // Largest frame size.
    serialize(uint32_t(0), &stream);
    serialize(width, &stream);
    serialize(height, &stream);
    serialize(fps, &stream);
    serialize(uint32_t(1), &stream); // Frame rate denominator.
    stream.write(std::string(64, '\0'));
    stream.close();
    return result;
}

UNIT_TEST(Magic, Smacker) {
    // Smacker videos are recognized by their signature and a non-empty frame size.
    EXPECT_EQ(magic(smackerHeader("SMK2", 640, 480, 104)), MAGIC_SMK);
    EXPECT_EQ(magic(smackerHeader("SMK4", 640, 480, 104)), MAGIC_SMK);
    EXPECT_EQ(magic(smackerHeader("SMK2", 0, 480, 104)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(smackerHeader("SMK2", 640, 480, 103)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(smackerHeader("SMK3", 640, 480, 104)), MAGIC_UNRECOGNIZED);
}

UNIT_TEST(Magic, Bink) {
    // Bink 1 and Bink 2 videos are recognized by their signature and a header that holds up.
    EXPECT_EQ(magic(binkHeader("BIKf", 92, 10, 640, 480, 15)), MAGIC_BIK);
    EXPECT_EQ(magic(binkHeader("BIKh", 92, 10, 640, 480, 15)), MAGIC_BIK);
    EXPECT_EQ(magic(binkHeader("BIKb", 92, 10, 640, 480, 15)), MAGIC_BIK);
    EXPECT_EQ(magic(binkHeader("KB2a", 92, 10, 640, 480, 15)), MAGIC_BIK);
    EXPECT_EQ(magic(binkHeader("KB2j", 92, 10, 640, 480, 15)), MAGIC_BIK);
    EXPECT_EQ(magic(binkHeader("BIKf", 92, 10, 7680, 4800, 15)), MAGIC_BIK);

    EXPECT_EQ(magic(binkHeader("BIK1", 92, 10, 640, 480, 15)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(binkHeader("KB2b", 92, 10, 640, 480, 15)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(binkHeader("BIGf", 92, 10, 640, 480, 15)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(binkHeader("BIKf", 93, 10, 640, 480, 15)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(binkHeader("BIKf", 92, 0, 640, 480, 15)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(binkHeader("BIKf", 92, 10, 0, 480, 15)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(binkHeader("BIKf", 92, 10, 7681, 480, 15)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(binkHeader("BIKf", 92, 10, 640, 4801, 15)), MAGIC_UNRECOGNIZED);
    EXPECT_EQ(magic(binkHeader("BIKf", 92, 10, 640, 480, 0)), MAGIC_UNRECOGNIZED);
}
