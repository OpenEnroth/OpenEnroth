#pragma once

#include <string>
#include <vector>

#include "Library/Font/Font.h"

#include "Render.h"

struct SearchReference {
    std::string name;
    Font font;
    std::u32string characters; // Characters to match on. Ones the font has no ink for are skipped.
};

struct SearchOptions {
    int top = 20; // Number of best matches to keep per reference.
    int ppemWindow = 2; // Sizes to try on either side of the estimated one.
    bool exhaustive = false; // Sweep all sizes, baselines and offsets instead of estimating them.
    bool oblique = false; // Also try slanted renders of every face.
    int threads = 0; // Zero means one per core.
};

struct SearchMatch {
    int distance = 0; // Number of pixels that differ.
    int exactWidths = 0; // Number of glyphs whose ink is exactly as wide as the reference's.
    std::string path;
    int faceIndex = 0;
    std::string family;
    std::string style;
    RenderSettings settings;
};

struct SearchResult {
    int glyphCount = 0; // Number of glyphs matched on.
    int inkPixels = 0; // Ink pixels in those glyphs, the distance a blank render scores.
    std::vector<SearchMatch> matches; // Best matches first.
};

/**
 * Searches outline and bitmap fonts for the ones that render into the reference fonts.
 *
 * Every candidate face is rendered at the sizes around the one estimated from glyph heights, in mono plain and
 * emboldened, and in gray at several thresholds, optionally slanted too. The rendered glyphs are aligned to the
 * reference glyphs by their ink centroids, the baseline once per font and the horizontal offset per glyph, and the
 * alignment is then refined by a pixel in each direction. The score is the number of pixels that differ.
 *
 * @param references                    Fonts to look for.
 * @param paths                         Candidate font files.
 * @param options                       Search options.
 * @param[out] failures                 Receives one line for every candidate that couldn't be read.
 * @return                              One result per reference.
 */
std::vector<SearchResult> searchFonts(const std::vector<SearchReference> &references, const std::vector<std::string> &paths,
                                      const SearchOptions &options, std::vector<std::string> *failures);
