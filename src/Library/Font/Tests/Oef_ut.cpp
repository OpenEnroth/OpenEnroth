#include <algorithm>
#include <string>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Font/Oef.h"
#include "Library/Font/Font.h"
#include "Library/Image/Image.h"

#include "Utility/Memory/Blob.h"

static Font makeFont() {
    Font result(3);

    std::vector<uint8_t> a = {1, 2, /**/ 0, 1, /**/ 2, 0}; // 2x3.
    result.add(U'A', GlyphMetrics(1, 2, -1), GrayscaleImageView(a.data(), 2, 3));

    std::vector<uint8_t> b = {0, 0, 0}; // 1x3, blank.
    result.add(U' ', GlyphMetrics(0, 1, 0), GrayscaleImageView(b.data(), 1, 3));

    std::vector<uint8_t> c = {4, 3, 2, /**/ 1, 0, 4, /**/ 3, 2, 1}; // 3x3, uses all four colors.
    result.add(U'Ы', GlyphMetrics(-2, 3, 5), GrayscaleImageView(c.data(), 3, 3));

    return result;
}

UNIT_TEST(Oef, RoundTrip) {
    Font font = makeFont();
    Blob blob = oef::encode(font);

    EXPECT_TRUE(oef::detect(blob));

    Font decoded = oef::decode(blob);
    EXPECT_EQ(decoded.height(), font.height());
    EXPECT_EQ(decoded.size(), font.size());

    for (int i = 0; i < font.size(); i++) {
        char32_t c = font.character(i);
        int j = decoded.index(c);
        EXPECT_NE(j, -1);

        EXPECT_EQ(decoded.character(j), c);
        EXPECT_EQ(decoded.metrics(j).leftSpacing, font.metrics(i).leftSpacing);
        EXPECT_EQ(decoded.metrics(j).width, font.metrics(i).width);
        EXPECT_EQ(decoded.metrics(j).rightSpacing, font.metrics(i).rightSpacing);
        EXPECT_TRUE(std::ranges::equal(decoded.image(j).pixels(), font.image(i).pixels()));
    }
}

UNIT_TEST(Oef, DetectRejectsGarbage) {
    EXPECT_FALSE(oef::detect(Blob()));
    EXPECT_FALSE(oef::detect(Blob::fromString("OEF")));           // Too short.
    EXPECT_FALSE(oef::detect(Blob::fromString(std::string(64, '\0'))));
    EXPECT_FALSE(oef::detect(Blob::fromString("NOPE" + std::string(60, '\0')))); // Bad signature.

    // Right signature, wrong version.
    std::string wrongVersion = "OEFT" + std::string(60, '\0');
    wrongVersion[4] = 99;
    EXPECT_FALSE(oef::detect(Blob::fromString(wrongVersion)));

    EXPECT_ANY_THROW(oef::decode(Blob::fromString(std::string(64, '\0'))));
}

UNIT_TEST(Oef, DecodeRejectsTruncatedPayload) {
    Blob blob = oef::encode(makeFont());
    EXPECT_ANY_THROW(oef::decode(Blob::fromString(std::string(blob.str().substr(0, blob.size() - 4)))));
}
