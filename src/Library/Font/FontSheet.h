#pragma once

#include "Library/Image/Image.h"

class Font;

/**
 * Renders all glyphs of a font into a grid, sixteen to a row, in glyph index order. Text is drawn white, the shadow
 * gray, and the other colors dark gray, on a black background.
 *
 * @param font                          Font to render.
 * @return                              Rendered glyph sheet.
 */
RgbaImage renderFontSheet(const Font &font);
