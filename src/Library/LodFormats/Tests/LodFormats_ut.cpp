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

template<class Header>
static Blob compressedPseudoImage(const Blob &data) {
    Blob compressed = zlib::compress(data);
    Header header = Header();
    header.dataSize = compressed.size();
    header.decompressedSize = data.size();
    header.flags = 256;

    Blob result;
    BlobOutputStream stream(&result);
    serialize(header, &stream);
    stream.write(compressed);
    stream.close();
    return result;
}

template<class Header>
static Blob image(int width, int height) {
    Header header = Header();
    header.size = width * height;
    header.dataSize = width * height;
    header.width = width;
    header.height = height;

    std::string pixels;
    for (int i = 0; i < width * height; i++)
        pixels += static_cast<char>(i);
    std::string palette;
    for (int i = 0; i < 0x300; i++)
        palette += static_cast<char>(i / 3);

    Blob result;
    BlobOutputStream stream(&result);
    serialize(header, &stream);
    stream.write(pixels);
    stream.write(palette);
    stream.close();
    return result;
}

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

UNIT_TEST(LodFormats, Mm8CompressedPseudoImage) {
    // MM8's EnglishT.lod and EnglishD.lod put a 64-byte name in front of the pseudo image header, MM6 a 16-byte one,
    // and each layout is told apart from the other.
    Blob data = Blob::fromString(testData);
    Blob mm6 = compressedPseudoImage<LodImageHeader_MM6>(data);
    Blob mm8 = compressedPseudoImage<LodImageHeader_MM8>(data);

    EXPECT_TRUE(lod::detectMm6CompressedPseudoImage(mm6));
    EXPECT_FALSE(lod::detectMm8CompressedPseudoImage(mm6));
    EXPECT_FALSE(lod::detectMm6CompressedPseudoImage(mm8));
    EXPECT_TRUE(lod::detectMm8CompressedPseudoImage(mm8));
    EXPECT_EQ(lod::decodeMaybeCompressed(mm6).str(), testData);
    EXPECT_EQ(lod::decodeMaybeCompressed(mm8).str(), testData);
}

UNIT_TEST(LodFormats, Mm8Image) {
    // MM8's EnglishD.lod stores LOD images with a 64-byte name in front of the header, MM6 with a 16-byte one, and both
    // decode.
    Blob mm6 = image<LodImageHeader_MM6>(4, 2);
    Blob mm8 = image<LodImageHeader_MM8>(4, 2);

    EXPECT_TRUE(lod::detectMm6Image(mm6));
    EXPECT_FALSE(lod::detectMm8Image(mm6));
    EXPECT_FALSE(lod::detectMm6Image(mm8));
    EXPECT_TRUE(lod::detectMm8Image(mm8));

    for (const Blob *entry : {&mm6, &mm8}) {
        EXPECT_EQ(lod::decodeAnyImageSize(*entry), Sizei(4, 2));

        LodImage decoded = lod::decodeAnyImage(*entry);
        ASSERT_EQ(decoded.image.width(), 4);
        ASSERT_EQ(decoded.image.height(), 2);
        EXPECT_EQ(decoded.image[1][3], 7);
        EXPECT_EQ(decoded.palette.colors[5].r, 5);
    }
}
