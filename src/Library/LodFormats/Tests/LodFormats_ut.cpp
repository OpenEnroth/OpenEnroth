#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/LodFormats/LodFormats.h"
#include "Library/LodFormats/LodFormatSnapshots.h"

#include "Utility/Exception.h"
#include "Utility/Memory/Blob.h"

static const std::string testData = std::string(1000, 'x') + "map data";

UNIT_TEST(LodFormats, Mm7CompressedData) {
    // MM7 compressed data round trips through encodeMm7CompressedData.
    Blob entry = lod::encodeMm7CompressedData(Blob::fromString(testData));

    EXPECT_TRUE(lod::detectMm7CompressedData(entry));
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).str(), testData);
}

UNIT_TEST(LodFormats, Mm7CompressedData_BrokenChecksum) {
    // A broken adler32 checksum after intact zlib data doesn't stop the data from decoding in full.
    std::string entry(lod::encodeMm7CompressedData(Blob::fromString(testData)).str());
    entry.back() ^= 0xFF;

    EXPECT_EQ(lod::decodeMaybeCompressed(Blob::fromString(entry)).str(), testData);
}

UNIT_TEST(LodFormats, Mm7CompressedData_WrongSize) {
    // Data that doesn't unpack into the size the header gives is an error, not a silently short result.
    std::string entry(lod::encodeMm7CompressedData(Blob::fromString(testData)).str());
    uint32_t decompressedSize = testData.size() + 1;
    memcpy(entry.data() + offsetof(LodCompressionHeader_MM7, decompressedSize), &decompressedSize, sizeof(decompressedSize));

    EXPECT_THROW((void) lod::decodeMaybeCompressed(Blob::fromString(entry)), Exception);
}
