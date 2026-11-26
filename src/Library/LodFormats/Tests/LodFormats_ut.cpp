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

static Blob compressedDataMm6(const Blob &compressed, uint32_t dataSize, uint32_t decompressedSize) {
    Blob result;
    BlobOutputStream stream(&result);
    serialize(dataSize, &stream);
    serialize(decompressedSize, &stream);
    stream.write(compressed);
    stream.close();
    return result;
}

static Blob paletteMm6(int16_t paletteId) {
    LodImageHeader_MM6 header = LodImageHeader_MM6();
    header.paletteId = paletteId;

    std::string palette;
    for (int i = 0; i < 0x300; i++)
        palette += static_cast<char>(i / 3);

    Blob result;
    BlobOutputStream stream(&result);
    serialize(header, &stream);
    stream.write(palette);
    stream.close();
    return result;
}

static LodFontHeader_MM6 fontHeader() {
    LodFontHeader_MM6 header = LodFontHeader_MM6();
    header.firstChar = 'A';
    header.lastChar = 'C';
    header.field_3 = 8;
    header.height = 4;
    return header;
}

static Blob fontMm6(int trailingBytes) {
    LodFontAtlas_MM6 atlas = LodFontAtlas_MM6();
    int offset = 0;
    for (int c = 'A'; c <= 'C'; c++) {
        atlas.metrics[c].width = c - 'A' + 1;
        atlas.offsets[c] = offset;
        offset += 4 * atlas.metrics[c].width;
    }

    Blob result;
    BlobOutputStream stream(&result);
    serialize(fontHeader(), &stream);
    serialize(atlas, &stream);
    stream.write(std::string(offset + trailingBytes, '\xFF'));
    stream.close();
    return result;
}

static Blob fontMmz() {
    LodFontAtlas_MMZ atlas = LodFontAtlas_MMZ();
    int offset = 0;
    for (int c = 'A'; c <= 'C'; c++) {
        atlas.widths[c] = c - 'A' + 1;
        atlas.offsets[c] = offset;
        offset += 4 * atlas.widths[c];
    }

    Blob result;
    BlobOutputStream stream(&result);
    serialize(fontHeader(), &stream);
    serialize(atlas, &stream);
    stream.write(std::string(offset, '\xFF'));
    stream.close();
    return result;
}

UNIT_TEST(LodFormats, CompressedDataMm6) {
    // MM6 maps came out of decodeMaybeCompressed still compressed.
    Blob compressed = zlib::compress(Blob::fromString(testData));
    Blob entry = compressedDataMm6(compressed, compressed.size(), testData.size());

    EXPECT_TRUE(lod::detectCompressedDataMm6(entry));
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).str(), testData);
}

UNIT_TEST(LodFormats, CompressedDataMm6_NearMisses) {
    // Without a signature, the sizes and the zlib header are all that tells MM6 compressed data apart from other data.
    Blob compressed = zlib::compress(Blob::fromString(testData));

    Blob wrongDataSize = compressedDataMm6(compressed, compressed.size() + 1, testData.size());
    EXPECT_FALSE(lod::detectCompressedData(wrongDataSize));
    EXPECT_EQ(lod::decodeMaybeCompressed(wrongDataSize).str(), wrongDataSize.str());

    Blob notZlib = compressedDataMm6(Blob::fromString(testData), testData.size(), testData.size());
    EXPECT_FALSE(lod::detectCompressedData(notZlib));
    EXPECT_EQ(lod::decodeMaybeCompressed(notZlib).str(), notZlib.str());
}

UNIT_TEST(LodFormats, CompressedDataMm7) {
    // MM7 compressed data round trips through encodeCompressedDataMm7.
    Blob entry = lod::encodeCompressedDataMm7(Blob::fromString(testData));

    EXPECT_TRUE(lod::detectCompressedDataMm7(entry));
    EXPECT_EQ(lod::decodeMaybeCompressed(entry).str(), testData);
}

UNIT_TEST(LodFormats, CompressedDataMm7_BrokenChecksum) {
    // Compressed data with a broken adler32 checksum after intact zlib data failed to decode.
    std::string entry(lod::encodeCompressedDataMm7(Blob::fromString(testData)).str());
    entry.back() ^= 0xFF;

    EXPECT_EQ(lod::decodeMaybeCompressed(Blob::fromString(entry)).str(), testData);
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

UNIT_TEST(LodFormats, PaletteMm6) {
    // A palette with its own number in the header's paletteId, like MM8's pal857, wasn't recognized as a palette.
    for (int16_t paletteId : {0, 857}) {
        Blob entry = paletteMm6(paletteId);

        EXPECT_TRUE(lod::detectPaletteMm6(entry));
        EXPECT_EQ(lod::decodePalette(entry).colors[5].g, 5);
    }
}

UNIT_TEST(LodFormats, FontMm6AndMmz) {
    // Fonts in the MM6 layout and in the MMZ layout of calig.fnt both decode, and neither is taken for the other.
    Blob mm6 = fontMm6(0);
    Blob mmz = fontMmz();

    EXPECT_TRUE(lod::detectFontMm6(mm6));
    EXPECT_FALSE(lod::detectFontMmz(mm6));
    EXPECT_FALSE(lod::detectFontMm6(mmz));
    EXPECT_TRUE(lod::detectFontMmz(mmz));

    for (const Blob *entry : {&mm6, &mmz}) {
        LodFont font = lod::decodeFont(*entry);
        EXPECT_EQ(font.height(), 4);
        EXPECT_EQ(font.metrics('C').width, 3);
    }
}

UNIT_TEST(LodFormats, FontMm6_TrailingByte) {
    // Some Russian MM6 and MM7 fonts have a stray byte after their glyphs, and they decode all the same.
    Blob entry = fontMm6(1);

    EXPECT_TRUE(lod::detectFontMm6(entry));
    EXPECT_EQ(lod::decodeFont(entry).metrics('C').width, 3);
}
