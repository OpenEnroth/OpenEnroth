#pragma once

#include <utility>

#include "Library/Font/Font.h"
#include "Library/Image/Image.h"

/**
 * How a font draws the shadow around its glyphs.
 */
enum class ShadowMode {
    SHADOW_NONE, // Font has no shadow.
    SHADOW_DROP, // Drop shadow, the text shifted by (1,1).
    SHADOW_OUTLINE, // Outline, the text dilated by its 4 neighbors.
};
using enum ShadowMode;

/**
 * Redraws a glyph's shadow, dropping whatever shadow it had and regenerating it from the text.
 *
 * The glyph box grows a pixel to fit the shadow, so a shadowed glyph is a pixel wider than the text alone.
 *
 * @param metrics                       Glyph metrics.
 * @param image                         Glyph image, color index `1` is the text and `2` is the shadow.
 * @param height                        Height of the resulting glyph. The source image can be of a different height.
 * @param shadow                        Shadow to draw.
 * @param raise                         How much to raise the glyph by, in pixels. Negative values lower it.
 * @param recolor                       Whether to treat the shadow color in the source image as the text color.
 * @return                              Metrics and image of the resulting glyph.
 */
std::pair<GlyphMetrics, GrayscaleImage> reshadow(const GlyphMetrics &metrics, GrayscaleImageView image, int height,
                                                ShadowMode shadow, int raise, bool recolor);
