#pragma once

#include <string_view>

#include "Utility/String/Encoding.h"

/**
 * Detects the encoding of the strings stored in a legacy Might & Magic VII savegame.
 *
 * MM7 shipped in English, German, French and Russian, so the only options are windows-1251 and windows-1252.
 * Russian names are written in Cyrillic, which is entirely non-ASCII in windows-1251, while the Western
 * localizations spell names in ASCII with the occasional accented letter. So the share of non-ASCII letters is what
 * tells the two apart.
 *
 * Charset detection libraries don't work here - they need running prose to match their language models against, and
 * a handful of names is nowhere near enough. Feeding uchardet the whole English `history.txt` makes it answer
 * "Macedonian".
 *
 * @param names                         Names stored in the savegame, concatenated. Raw bytes, not decoded.
 * @return                              `ENCODING_WINDOWS_1251` or `ENCODING_WINDOWS_1252`. Note that the two agree
 *                                      on all of ASCII, so the answer doesn't matter for names that have no
 *                                      letters outside it.
 */
TextEncoding detectLegacySaveEncoding(std::string_view names);
