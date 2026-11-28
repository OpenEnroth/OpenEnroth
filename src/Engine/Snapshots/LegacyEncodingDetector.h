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
 * @param names                         Names stored in the savegame, concatenated. Raw bytes, not decoded.
 * @param fallback                      Encoding to return when the names have no letters outside ASCII and so give
 *                                      nothing to go on. Everything written into the save afterwards uses it.
 * @return                              `ENCODING_WINDOWS_1251`, `ENCODING_WINDOWS_1252`, or `fallback`.
 */
TextEncoding detectLegacySaveEncoding(std::string_view names, TextEncoding fallback);
