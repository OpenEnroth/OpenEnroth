#include "Shadow.h"

#include <algorithm>
#include <utility>

std::pair<GlyphMetrics, GrayscaleImage> reshadow(const GlyphMetrics &metrics, GrayscaleImageView image,
                                                int height, ShadowMode shadow, int raise, bool recolor) {
    auto sourcePixel = [&](int y, int x) -> uint8_t {
        int sourceY = y + raise;
        if (sourceY < 0 || sourceY >= image.height())
            return 0;
        uint8_t result = image[sourceY][x];
        return recolor && result == 2 ? 1 : result;
    };

    int textLeft = metrics.width;
    int textRight = -1;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < metrics.width; x++) {
            if (sourcePixel(y, x) != 1)
                continue;
            textLeft = std::min(textLeft, x);
            textRight = std::max(textRight, x);
        }
    }

    // A glyph with no text needs no shadow, and is taken as is. Spaces are the only such glyphs, and shrinking
    // them to fit a shadow that isn't there would run the words together. Note that `autonote` is drawn entirely
    // in the shadow color and reaches this function only with `SHADOW_NONE`, so its pixels are passed through.
    if (shadow == SHADOW_NONE || textRight < 0) {
        GrayscaleImage result = GrayscaleImage::solid(0, metrics.width, height);
        for (int y = 0; y < height; y++)
            for (int x = 0; x < metrics.width; x++)
                result[y][x] = sourcePixel(y, x);
        return {metrics, std::move(result)};
    }

    // The shadow needs a pixel of room around the text, to the right for a drop shadow, and on both sides for an
    // outline. Grow the glyph box if the room isn't already there, which is the case for the fonts that ship
    // without a shadow. Growing to the left shifts the text inside the box, so the left spacing compensates and
    // the glyph stays where it was.
    int growLeft = shadow == SHADOW_OUTLINE ? std::max(0, 1 - textLeft) : 0;
    int growRight = std::max(0, textRight + 2 - metrics.width);
    int width = metrics.width + growLeft + growRight;

    GrayscaleImage result = GrayscaleImage::solid(0, width, height);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < metrics.width; x++)
            if (sourcePixel(y, x) == 1)
                result[y][x + growLeft] = 1;

    if (shadow == SHADOW_DROP) {
        for (int y = height - 1; y >= 1; y--)
            for (int x = width - 1; x >= 1; x--)
                if (result[y][x] == 0 && result[y - 1][x - 1] == 1)
                    result[y][x] = 2; // Text shifted by (1,1), only where there's no text.
    } else {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (result[y][x] != 0)
                    continue;
                bool neighbor = (y > 0 && result[y - 1][x] == 1) || (y + 1 < height && result[y + 1][x] == 1) ||
                                (x > 0 && result[y][x - 1] == 1) || (x + 1 < width && result[y][x + 1] == 1);
                if (neighbor)
                    result[y][x] = 2;
            }
        }
    }

    return {GlyphMetrics(metrics.leftSpacing - growLeft, width, metrics.rightSpacing), std::move(result)};
}
