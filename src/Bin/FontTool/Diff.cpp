#include "Diff.h"

#include <algorithm>

#include "FontFiles.h"

static constexpr Color BACKGROUND = Color(32, 32, 48, 255);
static constexpr Color CELL = Color(0, 0, 0, 255);
static constexpr Color BOTH = Color(255, 255, 255, 255);
static constexpr Color ONLY_A = Color(255, 64, 64, 255);
static constexpr Color ONLY_B = Color(64, 224, 255, 255);
static constexpr int GAP = 2;

static Color glyphColor(uint8_t index) {
    switch (index) {
    case 0: return CELL;
    case 1: return BOTH;
    case 2: return Color(128, 128, 128, 255);
    default: return Color(96, 96, 96, 255);
    }
}

static void fill(RgbaImage &image, int x, int y, int scale, Color color) {
    for (int dy = 0; dy < scale; dy++)
        for (int dx = 0; dx < scale; dx++)
            image[y * scale + dy][x * scale + dx] = color;
}

static void drawGlyph(RgbaImage &image, int x0, int y0, int scale, GrayscaleImageView glyph) {
    for (int y = 0; y < glyph.height(); y++)
        for (int x = 0; x < glyph.width(); x++)
            fill(image, x0 + x, y0 + y, scale, glyphColor(glyph[y][x]));
}

/**
 * @param font                          Font to set the text in.
 * @param text                          Text to set.
 * @return                              Rightmost column the text reaches, glyphs can reach past the final pen
 *                                      position.
 */
static int textWidth(const Font &font, const std::u32string &text) {
    int pen = 0;
    int result = 0;
    for (char32_t c : text) {
        int index = font.index(c);
        if (index == -1)
            continue;
        const GlyphMetrics &metrics = font.metrics(index);
        result = std::max(result, pen + metrics.leftSpacing + metrics.width);
        pen += metrics.leftSpacing + metrics.width + metrics.rightSpacing;
        result = std::max(result, pen);
    }
    return result;
}

static void drawText(RgbaImage &image, int x0, int y0, int scale, const Font &font, const std::u32string &text) {
    int x = x0;
    for (char32_t c : text) {
        int index = font.index(c);
        if (index == -1)
            continue;
        const GlyphMetrics &metrics = font.metrics(index);
        x += metrics.leftSpacing;
        GrayscaleImageView glyph = font.image(index);
        for (int y = 0; y < glyph.height(); y++)
            for (int gx = 0; gx < glyph.width(); gx++)
                if (glyph[y][gx] != 0 && x + gx >= 0)
                    fill(image, x + gx, y0 + y, scale, glyphColor(glyph[y][gx]));
        x += metrics.width + metrics.rightSpacing;
    }
}

RgbaImage renderFontDiff(const Font &a, const Font &b, const std::u32string &text, int scale) {
    uint8_t inkA = inkColorIndex(a);
    uint8_t inkB = inkColorIndex(b);
    int height = std::max(a.height(), b.height());

    std::u32string glyphs;
    int cellsWidth = 0;
    for (char32_t c : text) {
        if (!a.supports(c) && !b.supports(c))
            continue;
        glyphs += c;
        int widthA = a.supports(c) ? a.metrics(a.index(c)).width : 0;
        int widthB = b.supports(c) ? b.metrics(b.index(c)).width : 0;
        cellsWidth += std::max(widthA, widthB) + GAP;
    }

    int width = GAP + std::max({cellsWidth, textWidth(a, glyphs) + GAP, textWidth(b, glyphs) + GAP});
    int rows = GAP + 5 * (height + GAP);
    RgbaImage image = RgbaImage::solid(BACKGROUND, width * scale, rows * scale);

    int x = GAP;
    for (char32_t c : glyphs) {
        int indexA = a.index(c);
        int indexB = b.index(c);
        int widthA = indexA == -1 ? 0 : a.metrics(indexA).width;
        int widthB = indexB == -1 ? 0 : b.metrics(indexB).width;
        int cellWidth = std::max(widthA, widthB);

        for (int row = 0; row < 3; row++)
            for (int y = 0; y < height; y++)
                for (int cx = 0; cx < cellWidth; cx++)
                    fill(image, x + cx, GAP + row * (height + GAP) + y, scale, CELL);

        if (indexA != -1)
            drawGlyph(image, x, GAP, scale, a.image(indexA));
        if (indexB != -1)
            drawGlyph(image, x, GAP + height + GAP, scale, b.image(indexB));

        for (int y = 0; y < height; y++) {
            for (int cx = 0; cx < cellWidth; cx++) {
                bool hasA = indexA != -1 && cx < widthA && y < a.height() && a.image(indexA)[y][cx] == inkA;
                bool hasB = indexB != -1 && cx < widthB && y < b.height() && b.image(indexB)[y][cx] == inkB;
                if (hasA || hasB)
                    fill(image, x + cx, GAP + 2 * (height + GAP) + y, scale, hasA && hasB ? BOTH : hasA ? ONLY_A : ONLY_B);
            }
        }

        x += cellWidth + GAP;
    }

    drawText(image, GAP, GAP + 3 * (height + GAP), scale, a, glyphs);
    drawText(image, GAP, GAP + 4 * (height + GAP), scale, b, glyphs);
    return image;
}
