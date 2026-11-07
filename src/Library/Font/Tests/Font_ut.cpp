#include <array>
#include <string>
#include <type_traits>
#include <utility>

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

UNIT_TEST(Font, ValidEmpty) {
    Font font(10);

    EXPECT_TRUE(static_cast<bool>(font));
    EXPECT_FALSE(!font);
    EXPECT_EQ(font.height(), 10);
    EXPECT_EQ(font.size(), 0);
    EXPECT_FALSE(font.supports(U'A'));
    EXPECT_EQ(font.index(U'A'), -1);
}

UNIT_TEST(Font, Add) {
    std::array<uint8_t, 4> aPixels = {{255, 0, 0, 255}};
    std::array<uint8_t, 2> bPixels = {{1, 255}};
    std::array<uint8_t, 2> spacePixels = {{0, 0}};

    Font font(2);
    EXPECT_EQ(font.add(U' ', GlyphMetrics(0, 1, 0), GrayscaleImageView(spacePixels.data(), 1, 2)), 0);
    EXPECT_EQ(font.add(U'A', GlyphMetrics(1, 2, 1), GrayscaleImageView(aPixels.data(), 2, 2)), 1);
    EXPECT_EQ(font.add(U'A' + 0x10000, GlyphMetrics(0, 1, 0), GrayscaleImageView(bPixels.data(), 1, 2)), 2);

    EXPECT_EQ(font.height(), 2);
    EXPECT_EQ(font.size(), 3);

    EXPECT_TRUE(font.supports(U' '));
    EXPECT_TRUE(font.supports(U'A'));
    EXPECT_TRUE(font.supports(U'A' + 0x10000));
    EXPECT_FALSE(font.supports(U'B')); // Not added, but shares a page with 'A'.
    EXPECT_FALSE(font.supports(U'☃')); // Not added, page doesn't exist.
    EXPECT_FALSE(font.supports(U'\0'));

    EXPECT_EQ(font.index(U' '), 0);
    EXPECT_EQ(font.index(U'A'), 1);
    EXPECT_EQ(font.index(U'A' + 0x10000), 2);
    EXPECT_EQ(font.index(U'B'), -1);

    EXPECT_EQ(font.metrics(font.index(U'A')).leftSpacing, 1);
    EXPECT_EQ(font.metrics(font.index(U'A')).width, 2);
    EXPECT_EQ(font.metrics(font.index(U'A')).rightSpacing, 1);

    GrayscaleImageView image = font.image(font.index(U'A'));
    EXPECT_EQ(image.width(), 2);
    EXPECT_EQ(image.height(), 2);
    EXPECT_EQ(image[0][0], 255);
    EXPECT_EQ(image[0][1], 0);
    EXPECT_EQ(image[1][0], 0);
    EXPECT_EQ(image[1][1], 255);

    // Images are copied in.
    aPixels = {{0, 0, 0, 0}};
    EXPECT_EQ(image[0][0], 255);
}

UNIT_TEST(Font, Overwrite) {
    std::array<uint8_t, 2> oldPixels = {{1, 2}};
    std::array<uint8_t, 2> otherPixels = {{3, 4}};
    std::array<uint8_t, 4> newPixels = {{5, 6, 7, 8}};

    Font font(2);
    EXPECT_EQ(font.add(U'A', GlyphMetrics(0, 1, 0), GrayscaleImageView(oldPixels.data(), 1, 2)), 0);
    EXPECT_EQ(font.add(U'B', GlyphMetrics(0, 1, 0), GrayscaleImageView(otherPixels.data(), 1, 2)), 1);
    EXPECT_EQ(font.add(U'A', GlyphMetrics(1, 2, 1), GrayscaleImageView(newPixels.data(), 2, 2)), 0); // Same index.

    EXPECT_EQ(font.size(), 2); // Overwriting doesn't add glyphs.
    EXPECT_EQ(font.index(U'A'), 0);
    EXPECT_EQ(font.metrics(0).width, 2);
    EXPECT_EQ(font.image(0)[0][0], 5);
    EXPECT_EQ(font.image(0)[1][1], 8);

    // 'B' is untouched.
    EXPECT_EQ(font.index(U'B'), 1);
    EXPECT_EQ(font.image(1)[0][0], 3);
}

UNIT_TEST(Font, Move) {
    std::array<uint8_t, 2> pixels = {{1, 255}};

    Font font(2);
    font.add(U'A', GlyphMetrics(0, 1, 0), GrayscaleImageView(pixels.data(), 1, 2));

    Font movedFont(std::move(font));
    EXPECT_EQ(movedFont.height(), 2);
    EXPECT_EQ(movedFont.size(), 1);
    EXPECT_EQ(movedFont.index(U'A'), 0);
    EXPECT_EQ(movedFont.image(0)[1][0], 255); // Image data pointers stay valid after a move.
}

UNIT_TEST(Font, ImageStorageIsPointerStable) {
    // A single glyph is 16x16=256 bytes, and 64 glyphs is 16KB, spanning several storage chunks. Check that the
    // images stored first don't get invalidated as the storage grows.
    Font font(16);
    for (int i = 1; i <= 64; i++) {
        std::string pixels(256, static_cast<char>(i));
        font.add(static_cast<char32_t>(0x2600 + i), GlyphMetrics(0, 16, 0),
                 GrayscaleImageView(reinterpret_cast<const uint8_t *>(pixels.data()), 16, 16));
    }

    EXPECT_EQ(font.size(), 64);
    for (int i = 1; i <= 64; i++) {
        GrayscaleImageView image = font.image(font.index(static_cast<char32_t>(0x2600 + i)));
        EXPECT_EQ(image.width(), 16);
        EXPECT_EQ(image.height(), 16);
        EXPECT_EQ(image[0][0], i);
        EXPECT_EQ(image[15][15], i);
    }
}
