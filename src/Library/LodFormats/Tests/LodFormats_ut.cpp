#include <cstdint>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/Binary/CommonSerialization.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/LodFormats/LodFormatSnapshots.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"

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
