#pragma once

#include "Utility/System/Path.h"

/**
 * Generates OpenEnroth fonts from MM7 localization data.
 *
 * The English and Russian fonts ship the same letterforms for some faces, but the English ones carry a baked-in
 * shadow and the Latin-1 accents, while only the Russian ones carry Cyrillic. The two are merged into single
 * Unicode-keyed fonts, `<name>.oef` for every `<name>.fnt` the engine uses.
 *
 * @param enIconsLodPath                Path to `icons.lod` from an English localization.
 * @param ruIconsLodPath                Path to `icons.lod` from a Russian localization.
 * @param outputPath                    Directory to write the fonts into.
 */
void generateFonts(PathView enIconsLodPath, PathView ruIconsLodPath, PathView outputPath);
