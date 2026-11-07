#pragma once

#include <string>

#include "Library/Font/Font.h"
#include "Library/Image/Image.h"

/**
 * Draws two fonts glyph by glyph for comparing them by eye. There are three rows of glyph cells. The first two hold
 * the glyphs of each font, text in white and the other colors in grays. The third overlays the letterforms - white
 * where both fonts have ink, red where only the first one does, and cyan where only the second one does. Below them
 * the text is set in each font with the fonts' own spacing.
 *
 * @param a                             First font.
 * @param b                             Second font.
 * @param text                          Characters to draw. Ones neither font has are skipped.
 * @param scale                         Integer upscaling factor.
 * @return                              Rendered image.
 */
RgbaImage renderFontDiff(const Font &a, const Font &b, const std::u32string &text, int scale);
