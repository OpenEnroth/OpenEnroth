#pragma once

#include <string>
#include <vector>

#include "Library/Font/Font.h"

#include "FreeType.h"

/**
 * How to turn an outline font into a bitmap font.
 */
struct RenderSettings {
    int ppem = 0; // Pixels per em, or the pixel height of the strike to use for a bitmap font.
    RasterMode mode = RASTER_MONO;
    int threshold = 128; // Coverage at or above which a pixel becomes ink.
    bool bold = false; // Emboldens the way GDI simulates bold: the glyph is drawn again one pixel to the right.
    bool oblique = false; // Slants the outlines the way FreeType and GDI simulate italics.
    int baseline = 0; // Row of the baseline in the glyph cell. Rows above it are `[0, baseline)`.
    int height = 0; // Height of the glyph cell.

    /**
     * @return                          The settings as `FontTool render` options.
     */
    [[nodiscard]] std::string toString() const;
};

/**
 * Pixels of a rendered glyph that make the cut, row by row.
 */
struct InkMask {
    int left = 0; // Same as in `RenderedGlyph`.
    int top = 0;
    int width = 0;
    int rows = 0;
    int advance = 0;
    std::vector<uint8_t> pixels; // `rows` rows of `width` pixels, 1 for ink and 0 otherwise.
};

/**
 * @param glyph                         Rendered glyph.
 * @param threshold                     Coverage at or above which a pixel becomes ink.
 * @param bold                          Whether to embolden, see `RenderSettings::bold`.
 * @return                              Ink mask of the glyph.
 */
InkMask makeInkMask(const RenderedGlyph &glyph, int threshold, bool bold);

/**
 * @param face                          Face to render, with the size already set.
 * @param characters                    Characters to render. The ones the face doesn't have are skipped.
 * @param settings                      Render settings, `ppem` is not used.
 * @return                              Bitmap font with the text in color `1`.
 */
Font renderFont(const FreeTypeFace &face, const std::u32string &characters, const RenderSettings &settings);
