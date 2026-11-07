#include <array>
#include <type_traits>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Font/Font.h"

static_assert(!std::is_copy_constructible_v<Font> && !std::is_copy_assignable_v<Font>);
static_assert(std::is_move_constructible_v<Font> && std::is_move_assignable_v<Font>);

UNIT_TEST(Font, Invalid) {
    Font font;

    EXPECT_FALSE(static_cast<bool>(font));
    EXPECT_TRUE(!font);
    EXPECT_EQ(font.height(), 0);
    EXPECT_EQ(font.size(), 0);
    EXPECT_FALSE(font.supports(U'A'));
    EXPECT_EQ(font.index(U'A'), -1);
}

UNIT_TEST(Font, FromData) {
    std::array<uint8_t, 6> pixels = {{7, /**/ 8, /**/ 1, 2, /**/ 3, 4}}; // 1x2 glyph, then a 2x2 glyph.

    Font font = Font::fromData(2, {U'A', U'Ы' + 0x10000}, {GlyphMetrics(0, 1, 0), GlyphMetrics(-1, 2, 3)}, {0, 2},
                               Blob::copy(pixels.data(), pixels.size()));

    EXPECT_TRUE(static_cast<bool>(font));
    EXPECT_EQ(font.height(), 2);
    EXPECT_EQ(font.size(), 2);
    EXPECT_EQ(font.index(U'A'), 0);
    EXPECT_EQ(font.index(U'Ы' + 0x10000), 1);
    EXPECT_EQ(font.index(U'B'), -1); // Shares a page with 'A'.
    EXPECT_EQ(font.index(U'☃'), -1); // Page doesn't exist.
    EXPECT_EQ(font.character(1), U'Ы' + 0x10000);
    EXPECT_EQ(font.metrics(1).leftSpacing, -1);
    EXPECT_EQ(font.metrics(1).rightSpacing, 3);

    GrayscaleImageView a = font.image(0);
    EXPECT_EQ(a.width(), 1);
    EXPECT_EQ(a.height(), 2);
    EXPECT_EQ(a[0][0], 7);
    EXPECT_EQ(a[1][0], 8);

    GrayscaleImageView b = font.image(1);
    EXPECT_EQ(b.width(), 2);
    EXPECT_EQ(b[0][1], 2);
    EXPECT_EQ(b[1][0], 3);
}

UNIT_TEST(Font, MoveKeepsPixels) {
    std::array<uint8_t, 2> pixels = {{1, 255}};
    Font font = Font::fromData(2, {U'A'}, {GlyphMetrics(0, 1, 0)}, {0}, Blob::copy(pixels.data(), pixels.size()));

    Font moved(std::move(font));
    EXPECT_EQ(moved.index(U'A'), 0);
    EXPECT_EQ(moved.image(0)[1][0], 255);
}
