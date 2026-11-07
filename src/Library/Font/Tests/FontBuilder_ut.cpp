#include <array>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Library/Font/FontBuilder.h"

UNIT_TEST(FontBuilder, Empty) {
    Font font = FontBuilder(10).build();

    EXPECT_TRUE(static_cast<bool>(font));
    EXPECT_EQ(font.height(), 10);
    EXPECT_EQ(font.size(), 0);
    EXPECT_FALSE(font.supports(U'A'));
}

UNIT_TEST(FontBuilder, Add) {
    std::array<uint8_t, 4> aPixels = {{255, 0, 0, 255}};
    std::array<uint8_t, 2> bPixels = {{1, 255}};
    std::array<uint8_t, 2> spacePixels = {{0, 0}};

    FontBuilder builder(2);
    builder.add(U' ', GlyphMetrics(0, 1, 0), GrayscaleImageView(spacePixels.data(), 1, 2));
    builder.add(U'A', GlyphMetrics(1, 2, 1), GrayscaleImageView(aPixels.data(), 2, 2));
    builder.add(U'A' + 0x10000, GlyphMetrics(0, 1, 0), GrayscaleImageView(bPixels.data(), 1, 2));
    EXPECT_TRUE(builder.supports(U'A'));
    EXPECT_FALSE(builder.supports(U'B'));

    aPixels = {{0, 0, 0, 0}}; // Images are copied in.

    Font font = std::move(builder).build();
    EXPECT_EQ(font.height(), 2);
    EXPECT_EQ(font.size(), 3);
    EXPECT_EQ(font.index(U' '), 0);
    EXPECT_EQ(font.index(U'A'), 1);
    EXPECT_EQ(font.index(U'A' + 0x10000), 2);
    EXPECT_EQ(font.index(U'B'), -1);

    EXPECT_EQ(font.metrics(1).leftSpacing, 1);
    EXPECT_EQ(font.metrics(1).width, 2);
    EXPECT_EQ(font.metrics(1).rightSpacing, 1);

    GrayscaleImageView image = font.image(1);
    EXPECT_EQ(image.width(), 2);
    EXPECT_EQ(image.height(), 2);
    EXPECT_EQ(image[0][0], 255);
    EXPECT_EQ(image[0][1], 0);
    EXPECT_EQ(image[1][0], 0);
    EXPECT_EQ(image[1][1], 255);
    EXPECT_EQ(font.image(2)[1][0], 255);
}

UNIT_TEST(FontBuilder, Overwrite) {
    std::array<uint8_t, 2> oldPixels = {{1, 2}};
    std::array<uint8_t, 2> otherPixels = {{3, 4}};
    std::array<uint8_t, 4> newPixels = {{5, 6, 7, 8}};

    FontBuilder builder(2);
    builder.add(U'A', GlyphMetrics(0, 1, 0), GrayscaleImageView(oldPixels.data(), 1, 2));
    builder.add(U'B', GlyphMetrics(0, 1, 0), GrayscaleImageView(otherPixels.data(), 1, 2));
    builder.add(U'A', GlyphMetrics(1, 2, 1), GrayscaleImageView(newPixels.data(), 2, 2));

    Font font = std::move(builder).build();
    EXPECT_EQ(font.size(), 2);
    EXPECT_EQ(font.index(U'A'), 0);
    EXPECT_EQ(font.metrics(0).width, 2);
    EXPECT_EQ(font.image(0)[0][0], 5);
    EXPECT_EQ(font.image(0)[1][1], 8);
    EXPECT_EQ(font.index(U'B'), 1);
    EXPECT_EQ(font.image(1)[0][0], 3);
}

UNIT_TEST(FontBuilder, AddFont) {
    std::array<uint8_t, 2> aPixels = {{1, 1}};
    std::array<uint8_t, 2> bPixels = {{2, 2}};
    std::array<uint8_t, 2> newBPixels = {{3, 3}};

    FontBuilder first(2);
    first.add(U'A', GlyphMetrics(0, 1, 0), GrayscaleImageView(aPixels.data(), 1, 2));
    first.add(U'B', GlyphMetrics(0, 1, 0), GrayscaleImageView(bPixels.data(), 1, 2));
    Font firstFont = std::move(first).build();

    FontBuilder second(2);
    second.add(U'B', GlyphMetrics(0, 1, 0), GrayscaleImageView(newBPixels.data(), 1, 2));
    second.add(firstFont);
    Font font = std::move(second).build();

    EXPECT_EQ(font.size(), 2);
    EXPECT_EQ(font.index(U'B'), 0);
    EXPECT_EQ(font.index(U'A'), 1);
    EXPECT_EQ(font.image(0)[0][0], 2);
}
