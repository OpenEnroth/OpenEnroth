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

static std::string noise(size_t size) {
    std::string result;
    uint32_t state = 1;
    for (size_t i = 0; i < size; i++) {
        state = state * 1103515245 + 12345;
        result += static_cast<char>(state >> 24);
    }
    return result;
}

static Palette testPalette() {
    Palette result;
    for (int i = 0; i < 256; i++)
        result.colors[i] = Color(i, i, i);
    return result;
}

static LodImage testImage() {
    LodImage result;
    result.image = GrayscaleImage::solid(0, 4, 2);
    for (int i = 0; i < 8; i++)
        result.image.pixels()[i] = i;
    result.palette = testPalette();
    return result;
}

static LodFont testFont() {
    LodFontHeader header;
    header.firstChar = 'A';
    header.lastChar = 'C';
    header.fontHeight = 32; // Glyphs this big make an MMZ font larger than the smallest possible MM6 font.

    LodFontAtlas atlas;
    int offset = 0;
    for (int c = 'A'; c <= 'C'; c++) {
        atlas.metrics[c].width = c - 'A' + 30;
        atlas.offsets[c] = offset;
        offset += header.fontHeight * atlas.metrics[c].width;
    }

    return LodFont(header, atlas, Blob::fromString(std::string(offset, '\xFF')));
}

UNIT_TEST(LodFormats, CompressedDataMm6) {
    // MM6 maps came out of decodeMaybeCompressed still compressed.
    Blob entry = lod::encodeCompressedDataMm6(Blob::fromString(testData));

    EXPECT_TRUE(lod::detectCompressedDataMm6(entry));
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).str(), testData);
}

UNIT_TEST(LodFormats, CompressedDataMm6_Stored) {
    // MM6 compressed data stored without compression came out of decodeMaybeCompressed with its header still on.
    LodCompressionHeader_MM6 header;
    header.dataSize = testData.size();
    header.decompressedSize = 0;
    Blob entry = Blob::concat(Blob::view(&header, sizeof(header)), Blob::fromString(testData));

    EXPECT_TRUE(lod::detectCompressedDataMm6(entry));
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).str(), testData);
}

UNIT_TEST(LodFormats, CompressedDataMm6_NearMisses) {
    // Data that only looks like MM6 compressed data comes out of decodeMaybeCompressed as is.
    Blob compressed = zlib::compress(Blob::fromString(testData));
    LodCompressionHeader_MM6 header;

    header.dataSize = compressed.size() + 1;
    header.decompressedSize = testData.size();
    Blob wrongDataSize = Blob::concat(Blob::view(&header, sizeof(header)), compressed);
    EXPECT_FALSE(lod::detectCompressedDataMm6(wrongDataSize));
    EXPECT_EQ(lod::decodeMaybeCompressed(wrongDataSize).str(), wrongDataSize.str());

    header.dataSize = testData.size();
    header.decompressedSize = testData.size();
    Blob notZlib = Blob::concat(Blob::view(&header, sizeof(header)), Blob::fromString(testData));
    EXPECT_FALSE(lod::detectCompressedDataMm6(notZlib));
    EXPECT_EQ(lod::decodeMaybeCompressed(notZlib).str(), notZlib.str());

    header.dataSize = testData.size() + 1;
    header.decompressedSize = 0;
    Blob storedWrongDataSize = Blob::concat(Blob::view(&header, sizeof(header)), Blob::fromString(testData));
    EXPECT_FALSE(lod::detectCompressedDataMm6(storedWrongDataSize));
    EXPECT_EQ(lod::decodeMaybeCompressed(storedWrongDataSize).str(), storedWrongDataSize.str());
}

UNIT_TEST(LodFormats, CompressedDataMm7) {
    // MM7 compressed data round trips through encodeCompressedDataMm7.
    Blob entry = lod::encodeCompressedDataMm7(Blob::fromString(testData));

    EXPECT_TRUE(lod::detectCompressedDataMm7(entry));
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).str(), testData);
}

UNIT_TEST(LodFormats, CompressedData_BrokenChecksum) {
    // Intact zlib data with a broken adler32 checksum, like English MM6's cd3.blv, failed to decode.
    Blob mm6 = lod::encodeCompressedDataMm6(Blob::fromString(testData));
    Blob mm7 = lod::encodeCompressedDataMm7(Blob::fromString(testData));

    for (const Blob *entry : {&mm6, &mm7}) {
        std::string broken(entry->str());
        broken.back() ^= 0xFF;

        EXPECT_EQ(lod::decodeMaybeCompressed(Blob::fromString(broken)).str(), testData);
    }
}

UNIT_TEST(LodFormats, CompressedDataMm7_WrongSize) {
    // Compressed data that didn't unpack into the size its header gives came back without complaint.
    for (int delta : {-1, 1}) {
        std::string entry(lod::encodeCompressedDataMm7(Blob::fromString(testData)).str());
        uint32_t decompressedSize = testData.size() + delta;
        memcpy(entry.data() + offsetof(LodCompressionHeader_MM7, decompressedSize), &decompressedSize, sizeof(decompressedSize));

        EXPECT_THROW((void) lod::decodeMaybeCompressed(Blob::fromString(entry)), Exception);
    }
}

UNIT_TEST(LodFormats, CompressedPseudoImageMm8) {
    // Entries of MM8's EnglishT.lod and EnglishD.lod came out of decodeMaybeCompressed still compressed.
    std::string data = noise(1000);
    Blob mm6 = lod::encodeCompressedPseudoImageMm6(Blob::fromString(data));
    Blob mm8 = lod::encodeCompressedPseudoImageMm8(Blob::fromString(data));

    EXPECT_TRUE(lod::detectCompressedPseudoImageMm6(mm6));
    EXPECT_FALSE(lod::detectCompressedPseudoImageMm8(mm6));
    EXPECT_FALSE(lod::detectCompressedPseudoImageMm6(mm8));
    EXPECT_TRUE(lod::detectCompressedPseudoImageMm8(mm8));
    EXPECT_EQ(lod::decodeMaybeCompressed(mm6).str(), data);
    EXPECT_EQ(lod::decodeMaybeCompressed(mm8).str(), data);
}

UNIT_TEST(LodFormats, CompressedPseudoImageMm6_Empty) {
    // Empty pseudo images, like MM6's d4bm.evt, came out of decodeMaybeCompressed with their header still on.
    LodImageHeader_MM6 header = LodImageHeader_MM6();
    header.flags = 256;

    Blob entry;
    BlobOutputStream stream(&entry);
    serialize(header, &stream);
    stream.close();

    EXPECT_TRUE(lod::detectCompressedPseudoImageMm6(entry));
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).size(), 0);
}

UNIT_TEST(LodFormats, CompressedPseudoImageMm6_ShortName) {
    // A stored pseudo image whose name, read as a number, is its size minus 8 decodes as a pseudo image, not as MM6
    // compressed data.
    std::string data(25145, 'x');
    LodImageHeader_MM6 header = LodImageHeader_MM6();
    memcpy(header.name.data(), "ab", 2); // "ab" reads as 25185.
    header.dataSize = data.size();
    header.flags = 256;

    Blob entry;
    BlobOutputStream stream(&entry);
    serialize(header, &stream);
    stream.write(data);
    stream.close();

    ASSERT_EQ(entry.size(), 25185 + 8);
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).str(), data);
}

UNIT_TEST(LodFormats, ImageMm8) {
    // LOD images in MM8's EnglishD.lod didn't decode.
    Blob mm6 = lod::encodeImageMm6(testImage());
    Blob mm8 = lod::encodeImageMm8(testImage());

    EXPECT_TRUE(lod::detectImageMm6(mm6));
    EXPECT_FALSE(lod::detectImageMm8(mm6));
    EXPECT_FALSE(lod::detectImageMm6(mm8));
    EXPECT_TRUE(lod::detectImageMm8(mm8));

    for (const Blob *entry : {&mm6, &mm8}) {
        EXPECT_EQ(lod::decodeImageSize(*entry), Sizei(4, 2));

        LodImage decoded = lod::decodeImage(*entry);
        ASSERT_EQ(decoded.image.width(), 4);
        ASSERT_EQ(decoded.image.height(), 2);
        EXPECT_EQ(decoded.image[1][3], 7);
        EXPECT_EQ(decoded.palette.colors[5].r, 5);
    }
}

UNIT_TEST(LodFormats, PaletteMm6) {
    // A palette with its own number in the header's paletteId, like MM8's pal857, wasn't recognized as a palette.
    for (int16_t paletteId : {0, 857}) {
        std::string entry(lod::encodePaletteMm6(testPalette()).str());
        memcpy(entry.data() + offsetof(LodImageHeader_MM6, paletteId), &paletteId, sizeof(paletteId));

        EXPECT_TRUE(lod::detectPaletteMm6(Blob::fromString(entry)));
        EXPECT_EQ(lod::decodePalette(Blob::fromString(entry)).colors[5].g, 5);
    }
}

UNIT_TEST(LodFormats, FontMm6AndMmz) {
    // Fonts in the MM6 layout and in the MMZ layout of calig.fnt both decode, each in its own layout.
    Blob mm6 = lod::encodeFontMm6(testFont());
    Blob mmz = lod::encodeFontMmz(testFont());

    EXPECT_TRUE(lod::detectFontMm6(mm6));
    EXPECT_FALSE(lod::detectFontMmz(mm6));
    EXPECT_TRUE(lod::detectFontMmz(mmz));

    for (const Blob *entry : {&mm6, &mmz}) {
        LodFont font = lod::decodeFont(*entry);
        EXPECT_EQ(font.height(), 32);
        EXPECT_EQ(font.metrics('A').width, 30);
        EXPECT_EQ(font.metrics('C').width, 32);
    }
}

UNIT_TEST(LodFormats, FontMm6_TrailingData) {
    // Fonts with data after their glyphs, like the Russian MM6 and MM7 fonts with a stray byte, decode all the same.
    for (int trailingBytes : {1, 16}) {
        Blob entry = Blob::concat(lod::encodeFontMm6(testFont()), Blob::fromString(std::string(trailingBytes, '\xFF')));

        EXPECT_TRUE(lod::detectFontMm6(entry));
        EXPECT_EQ(lod::decodeFont(entry).metrics('C').width, 32);
    }
}
