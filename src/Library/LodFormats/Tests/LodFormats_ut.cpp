#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/Binary/CommonSerialization.h"
#include "Library/Compression/Compression.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/LodFormats/LodFormatSnapshots.h"

#include "Utility/Exception.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"

static const std::string testData = std::string(1000, 'x') + "map data";

static Blob mm6CompressedData(const Blob &compressed, uint32_t dataSize, uint32_t decompressedSize) {
    Blob result;
    BlobOutputStream stream(&result);
    serialize(dataSize, &stream);
    serialize(decompressedSize, &stream);
    stream.write(compressed);
    stream.close();
    return result;
}

UNIT_TEST(LodFormats, Mm6CompressedData) {
    // MM6 maps carry only the packed and unpacked sizes in front of the zlib data, and decode like MM7's signed ones.
    Blob compressed = zlib::compress(Blob::fromString(testData));
    Blob entry = mm6CompressedData(compressed, compressed.size(), testData.size());

    EXPECT_TRUE(lod::detectMm6CompressedData(entry));
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).str(), testData);
}

UNIT_TEST(LodFormats, Mm6CompressedData_NearMisses) {
    // Without a signature, the sizes and the zlib header are all that tells MM6 compressed data apart from other data.
    Blob compressed = zlib::compress(Blob::fromString(testData));

    Blob wrongDataSize = mm6CompressedData(compressed, compressed.size() + 1, testData.size());
    EXPECT_FALSE(lod::detectAnyCompressedData(wrongDataSize));
    EXPECT_EQ(lod::decodeMaybeCompressed(wrongDataSize).str(), wrongDataSize.str());

    Blob notZlib = mm6CompressedData(Blob::fromString(testData), testData.size(), testData.size());
    EXPECT_FALSE(lod::detectAnyCompressedData(notZlib));
    EXPECT_EQ(lod::decodeMaybeCompressed(notZlib).str(), notZlib.str());
}

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
