#pragma once

#include <cstdint>

#include "Library/Font/Font.h"

#include "Utility/String/Encoding.h"
#include "Utility/System/Path.h"

/**
 * @param path                          Path to a LOD font or an OpenEnroth font. Can point into an archive, as in
 *                                      `"icons.lod/arrus.fnt"`.
 * @param encoding                      Encoding of a LOD font's glyph bytes. Not used for OpenEnroth fonts.
 * @return                              Decoded font.
 * @throws Exception                    If the file doesn't exist or isn't a font.
 */
Font readFont(PathView path, TextEncoding encoding);

/**
 * @param path                          Path to write to. A `.png` path gets a glyph sheet, an `.oef` path gets an
 *                                      OpenEnroth font.
 * @param font                          Font to write.
 * @throws Exception                    If the extension is neither.
 */
void writeFont(PathView path, const Font &font);

/**
 * LOD fonts store text as color `1` and the shadow as color `2`, but a shadowless LOD font can store its letterforms
 * in the shadow color. MM7's `autonote` is such a font.
 *
 * @param font                          Font to look at.
 * @return                              Color index that carries the letterforms.
 */
uint8_t inkColorIndex(const Font &font);
