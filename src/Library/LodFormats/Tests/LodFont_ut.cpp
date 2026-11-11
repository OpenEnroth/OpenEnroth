#include <map>
#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/LodFormats/LodFormats.h"
#include "Library/LodFormats/LodFormatSnapshots.h"
#include "Library/Snapshots/CommonSnapshots.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"

namespace {
struct Glyph {
    int leftSpacing = 0;
    int width = 0;
    int rightSpacing = 0;
    std::vector<uint8_t> pixels; // width * height, row-major. LOD values: 0 background, 1 shadow, 255 text.
};

// Builds an MM7-format LOD font blob. Every character in `[firstChar, lastChar]` must have a glyph.
Blob makeFontBlob(int firstChar, int lastChar, int height, const std::map<int, Glyph> &glyphs) {
    LodFontHeader_MM7 header = {};
    header.firstChar = static_cast<uint8_t>(firstChar);
    header.lastChar = static_cast<uint8_t>(lastChar);
    header.field_3 = 8;
    header.height = static_cast<uint8_t>(height);

    LodFontAtlas_MM7 atlas = {};
    std::string pixels;
    for (const auto &[c, glyph] : glyphs) {
        atlas.metrics[c].leftSpacing = glyph.leftSpacing;
        atlas.metrics[c].width = glyph.width;
        atlas.metrics[c].rightSpacing = glyph.rightSpacing;
        atlas.offsets[c] = static_cast<uint32_t>(pixels.size());
        pixels.append(reinterpret_cast<const char *>(glyph.pixels.data()), glyph.pixels.size());
    }

    Blob result;
    BlobOutputStream stream(&result);
    serialize(header, &stream);
    serialize(atlas, &stream);
    stream.write(pixels);
    stream.close();
    return result;
}
} // namespace

UNIT_TEST(DecodeFont, Basic) {
    std::map<int, Glyph> glyphs;
    glyphs['A'] = {1, 2, 1, {255, 1, /**/ 0, 0, /**/ 0, 0, /**/ 0, 0}}; // 2x4: text at [0][0], shadow at [0][1].
    glyphs['B'] = {0, 1, 0, {255, 255, 255, 255}};                      // 1x4: all text.
    Font font = lod::decodeFont(makeFontBlob('A', 'B', 4, glyphs), ENCODING_ASCII);

    EXPECT_EQ(font.height(), 4);
    EXPECT_EQ(font.size(), 2);
    EXPECT_TRUE(font.supports(U'A'));
    EXPECT_TRUE(font.supports(U'B'));
    EXPECT_FALSE(font.supports(U'C')); // Outside [firstChar, lastChar].

    const GlyphMetrics &a = font.metrics(font.index(U'A'));
    EXPECT_EQ(a.leftSpacing, 1);
    EXPECT_EQ(a.width, 2);
    EXPECT_EQ(a.rightSpacing, 1);

    // LOD pixel values are remapped to color indices: 255 (text) -> 1, 1 (shadow) -> 2.
    GrayscaleImageView image = font.image(font.index(U'A'));
    EXPECT_EQ(image.width(), 2);
    EXPECT_EQ(image.height(), 4);
    EXPECT_EQ(image[0][0], 1);
    EXPECT_EQ(image[0][1], 2);
    EXPECT_EQ(image[1][0], 0);
}

UNIT_TEST(DecodeFont, EncodingKeying) {
    std::map<int, Glyph> glyphs;
    glyphs[0xC8] = {0, 1, 0, {255, 255, 255, 255}};
    glyphs[0xC9] = {0, 1, 0, {255, 255, 255, 255}};
    Blob blob = makeFontBlob(0xC8, 0xC9, 4, glyphs);

    // The encoding maps glyph bytes to code points, so the same font keys differently per codepage.
    Font cyrillic = lod::decodeFont(blob, ENCODING_WINDOWS_1251);
    EXPECT_TRUE(cyrillic.supports(0x0418)); // CYRILLIC CAPITAL LETTER I (windows-1251 0xC8).
    EXPECT_TRUE(cyrillic.supports(0x0419)); // CYRILLIC CAPITAL LETTER SHORT I.
    EXPECT_FALSE(cyrillic.supports(0x00C8));

    Font latin = lod::decodeFont(blob, ENCODING_ISO_8859_1);
    EXPECT_TRUE(latin.supports(0x00C8)); // LATIN CAPITAL LETTER E WITH GRAVE (identity byte mapping).
    EXPECT_TRUE(latin.supports(0x00C9));
    EXPECT_FALSE(latin.supports(0x0418));
}

UNIT_TEST(DecodeFont, BlankGlyphsAreUnsupportedExceptSpace) {
    std::map<int, Glyph> glyphs;
    glyphs[' '] = {0, 3, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}; // Blank space, 3x4.
    glyphs['!'] = {0, 2, 0, {0, 0, 0, 0, 0, 0, 0, 0}};             // Blank non-space, 2x4.
    Font font = lod::decodeFont(makeFontBlob(' ', '!', 4, glyphs), ENCODING_ASCII);

    EXPECT_TRUE(font.supports(U' ')); // A blank space glyph is kept.
    EXPECT_FALSE(font.supports(U'!')); // A blank non-space glyph is treated as unsupported.
}

UNIT_TEST(DecodeFont, RejectsNonFont) {
    EXPECT_ANY_THROW(lod::decodeFont(Blob::fromString(std::string(2000, '\0')), ENCODING_ASCII));
}
