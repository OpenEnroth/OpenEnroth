#include "FontSheet.h"

#include <algorithm>

#include "Library/Font/Font.h"

RgbaImage renderFontSheet(const Font &font) {
    int cellWidth = 1;
    for (int i = 0; i < font.size(); i++)
        cellWidth = std::max(cellWidth, font.metrics(i).width);

    // Laid out by glyph index, a sheet laid out by character would be the size of the unicode range.
    int columns = 16;
    int rows = std::max(1, (font.size() + columns - 1) / columns);
    RgbaImage image = RgbaImage::solid(Color(0, 0, 0, 255), cellWidth * columns, font.height() * rows);

    for (int i = 0; i < font.size(); i++) {
        int x0 = (i % columns) * cellWidth;
        int y0 = (i / columns) * font.height();

        GrayscaleImageView glyph = font.image(i);
        for (int y = 0; y < glyph.height(); y++) {
            for (int x = 0; x < glyph.width(); x++) {
                int gray = 0;
                switch (glyph[y][x]) {
                case 0: break;
                case 1: gray = 255; break;
                case 2: gray = 128; break;
                default: gray = 64; break;
                }
                image[y0 + y][x0 + x] = Color(gray, gray, gray, 255);
            }
        }
    }

    return image;
}
