#include "FontMatchOptions.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <atomic>
#include <bit>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "Library/Font/Font.h"
#include "Library/Image/Image.h"
#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodFormats.h"

#include "Utility/Exception.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Format.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"
#include "Utility/UnicodeCrt.h"

namespace {

// Coverage thresholds the anti-aliased render is binarized at. `FT_LOAD_TARGET_MONO` isn't "render grey and
// round" - it runs a different hinting pass that snaps stems to whole pixels first - so the two pipelines put
// stems in different places and both have to be swept.
constexpr int aaThresholds[] = {64, 96, 128, 160, 192, 224};

constexpr int monoThreshold = 1; // The mono render is unpacked to 0 or 1 per pixel, so anything non-zero is ink.

constexpr uint8_t textColorIndex = 1; // Color index of the text layer. `0` is background, `2` is the shadow.

constexpr uint8_t shadowColorIndex = 2;

/**
 * Works out which color index carries the letterforms.
 *
 * `lod::decodeFont` maps the lod pixel value `1` to the shadow color, and every other non-zero value to the text
 * color. That's right for every face but `autonote`, which has no shadow and stores its letterforms as `1` - so
 * the whole face decodes into the shadow color and the text layer comes out empty. A font with no text pixels at
 * all is that case, and there its shadow color is really the text.
 */
uint8_t inkColorIndex(const Font &font) {
    for (int index = 0; index < font.size(); index++) {
        GrayscaleImageView image = font.image(index);
        for (int y = 0; y < image.height(); y++)
            for (int x = 0; x < image.width(); x++)
                if (image[y][x] == textColorIndex)
                    return textColorIndex;
    }
    return shadowColorIndex;
}

/**
 * A glyph of the reference font, text layer only, bit-packed row by row.
 */
struct ReferenceGlyph {
    char32_t character = 0;
    int width = 0; // Width of the original glyph box.
    int boxWidth = 0; // Width of the comparison box - `width` plus slack, so that a wider render is penalized.
    int stride = 0; // Words per row in `mask`.
    std::vector<uint64_t> mask; // Text layer, font height rows of `stride` words.
};

struct Reference {
    int height = 0;
    int inkPixels = 0; // Total text-layer pixels across all glyphs - the score of rendering nothing at all.
    std::vector<ReferenceGlyph> glyphs;
};

struct Match {
    int distance = 0; // Number of pixels that differ.
    int exactWidths = 0; // Number of glyphs rendered at exactly the original width.
    int ppem = 0;
    int baseline = 0;
    std::string mode;
    std::string family;
    std::string style;
};

struct Candidate {
    std::string path;
    std::optional<Match> match;
    std::string error; // Non-empty if the font couldn't be read.
};

void setBit(std::vector<uint64_t> &mask, size_t offset, int stride, int y, int x) {
    mask[offset + y * stride + (x >> 6)] |= uint64_t(1) << (x & 63);
}

Reference buildReference(const Font &font, std::string_view characters) {
    Reference result;
    result.height = font.height();
    uint8_t ink = inkColorIndex(font);

    for (char c : characters) {
        int index = font.index(static_cast<char32_t>(c));
        if (index == -1)
            continue;

        const GlyphMetrics &metrics = font.metrics(index);
        if (metrics.width <= 0)
            continue;

        ReferenceGlyph glyph;
        glyph.character = static_cast<char32_t>(c);
        glyph.width = metrics.width;
        glyph.boxWidth = metrics.width + 2;
        glyph.stride = (glyph.boxWidth + 63) / 64;
        glyph.mask.assign(static_cast<size_t>(result.height) * glyph.stride, 0);

        GrayscaleImageView image = font.image(index);
        bool blank = true;
        for (int y = 0; y < result.height; y++) {
            for (int x = 0; x < metrics.width; x++) {
                if (image[y][x] != ink)
                    continue;
                setBit(glyph.mask, 0, glyph.stride, y, x);
                result.inkPixels++;
                blank = false;
            }
        }

        if (blank)
            continue; // Spaces have no text layer, so there is nothing to match them on.

        result.glyphs.push_back(std::move(glyph));
    }

    return result;
}

/**
 * Renders every reference character at the current size into `coverage`, one byte per pixel.
 *
 * @return                              False if the font can't produce one of the glyphs, in which case it's not
 *                                      a candidate at this size.
 */
bool renderGlyphs(FT_Face face, const Reference &reference, int32_t target,
                  std::vector<std::vector<uint8_t>> *coverage, std::vector<int> *tops, std::vector<int> *widths,
                  std::vector<int> *rows) {
    bool mono = target == FT_LOAD_TARGET_MONO;

    for (size_t i = 0; i < reference.glyphs.size(); i++) {
        if (FT_Load_Char(face, reference.glyphs[i].character, FT_LOAD_RENDER | target) != 0)
            return false;

        const FT_Bitmap &bitmap = face->glyph->bitmap;
        std::vector<uint8_t> &pixels = (*coverage)[i];
        pixels.assign(static_cast<size_t>(bitmap.rows) * bitmap.width, 0);
        for (unsigned int y = 0; y < bitmap.rows; y++) {
            for (unsigned int x = 0; x < bitmap.width; x++) {
                const uint8_t *row = bitmap.buffer + y * bitmap.pitch;
                pixels[y * bitmap.width + x] = mono ? (row[x >> 3] >> (7 - (x & 7))) & 1 : row[x];
            }
        }

        (*tops)[i] = face->glyph->bitmap_top;
        (*widths)[i] = bitmap.width;
        (*rows)[i] = bitmap.rows;
    }

    return true;
}

/**
 * Scores the rendered glyphs against the reference, sweeping the baseline. The render is binarized at
 * `threshold` and packed once, then each baseline just offsets the row index into it.
 */
std::optional<Match> scoreGlyphs(const Reference &reference, const std::vector<std::vector<uint8_t>> &coverage,
                                 const std::vector<int> &tops, const std::vector<int> &widths,
                                 const std::vector<int> &rows, int threshold) {
    size_t glyphCount = reference.glyphs.size();

    std::vector<uint64_t> packed;
    std::vector<size_t> offsets(glyphCount);
    std::vector<int> totalInk(glyphCount, 0); // Candidate ink over the full bitmap, clipping included.
    std::vector<int> inkPrefix; // Per-row prefix sums of the packed (in-box) ink, `rows[i] + 1` entries per glyph.
    std::vector<size_t> inkPrefixOffsets(glyphCount);
    for (size_t i = 0; i < glyphCount; i++) {
        const ReferenceGlyph &glyph = reference.glyphs[i];
        offsets[i] = packed.size();
        packed.resize(packed.size() + static_cast<size_t>(rows[i]) * glyph.stride, 0);
        inkPrefixOffsets[i] = inkPrefix.size();
        inkPrefix.push_back(0);
        for (int y = 0; y < rows[i]; y++) {
            int rowInk = 0;
            for (int x = 0; x < widths[i]; x++) {
                if (coverage[i][y * widths[i] + x] < threshold)
                    continue;
                totalInk[i]++;
                if (x < glyph.boxWidth) {
                    setBit(packed, offsets[i], glyph.stride, y, x);
                    rowInk++;
                }
            }
            inkPrefix.push_back(inkPrefix.back() + rowInk);
        }
    }

    std::optional<Match> result;
    for (int baseline = reference.height / 2; baseline < reference.height + 2; baseline++) {
        int distance = 0;
        int exactWidths = 0;

        for (size_t i = 0; i < glyphCount; i++) {
            const ReferenceGlyph &glyph = reference.glyphs[i];
            int shift = baseline - tops[i];

            for (int y = 0; y < reference.height; y++) {
                int sourceY = y - shift;
                bool inside = sourceY >= 0 && sourceY < rows[i];
                for (int word = 0; word < glyph.stride; word++) {
                    uint64_t referenceWord = glyph.mask[y * glyph.stride + word];
                    uint64_t candidateWord =
                        inside ? packed[offsets[i] + sourceY * glyph.stride + word] : 0;
                    distance += std::popcount(referenceWord ^ candidateWord);
                }
            }

            // Candidate ink that never entered the comparison box - rows shifted outside it, and columns clipped
            // at packing time - is all extra ink, and counts as difference. Without this, an oversized render
            // sheds most of its ink for free and drifts towards the blank-render score.
            const int *prefix = inkPrefix.data() + inkPrefixOffsets[i];
            int visibleFrom = std::clamp(-shift, 0, rows[i]);
            int visibleTo = std::clamp(reference.height - shift, 0, rows[i]);
            distance += totalInk[i] - (prefix[std::max(visibleFrom, visibleTo)] - prefix[visibleFrom]);

            exactWidths += widths[i] == glyph.width;
        }

        if (!result || distance < result->distance)
            result = Match{distance, exactWidths, 0, baseline, {}, {}, {}};
    }

    return result;
}

/**
 * Scores a single candidate font over the full ppem x baseline x render mode matrix. No shortlist, no early exit -
 * a cheap filter is a short circuit, and the cheap signals have been wrong here before.
 */
Candidate scoreFont(FT_Library library, std::string path, const Reference &reference) {
    Candidate result;
    result.path = std::move(path);

    FT_Face face = nullptr;
    if (FT_Error error = FT_New_Face(library, result.path.c_str(), 0, &face); error != 0) {
        const char *message = FT_Error_String(error);
        result.error = message ? std::string(message) : fmt::format("FreeType error {}", error);
        return result;
    }

    size_t glyphCount = reference.glyphs.size();
    std::vector<std::vector<uint8_t>> coverage(glyphCount);
    std::vector<int> tops(glyphCount);
    std::vector<int> widths(glyphCount);
    std::vector<int> rows(glyphCount);

    auto consider = [&](std::optional<Match> match, int ppem, std::string_view mode) {
        if (!match || (result.match && result.match->distance <= match->distance))
            return;
        match->ppem = ppem;
        match->mode = mode;
        match->family = face->family_name ? face->family_name : "?";
        match->style = face->style_name ? face->style_name : "?";
        result.match = std::move(match);
    };

    for (int ppem = std::max(6, reference.height - 12); ppem < reference.height + 10; ppem++) {
        if (FT_Set_Pixel_Sizes(face, 0, ppem) != 0)
            continue;

        if (renderGlyphs(face, reference, FT_LOAD_TARGET_MONO, &coverage, &tops, &widths, &rows))
            consider(scoreGlyphs(reference, coverage, tops, widths, rows, monoThreshold), ppem, "mono");

        // The anti-aliased render doesn't depend on the threshold, so render once and binarize it six ways.
        if (renderGlyphs(face, reference, FT_LOAD_TARGET_NORMAL, &coverage, &tops, &widths, &rows))
            for (int threshold : aaThresholds)
                consider(scoreGlyphs(reference, coverage, tops, widths, rows, threshold), ppem,
                         fmt::format("aa{}", threshold));
    }

    FT_Done_Face(face);
    return result;
}

std::vector<std::string> readFontList(std::string_view path) {
    Blob blob = Blob::fromFile(path);

    std::vector<std::string> result;
    for (std::string_view line : split(blob.str()).by('\n'))
        if (std::string_view trimmed = trim(line); !trimmed.empty())
            result.emplace_back(trimmed);
    return result;
}

} // namespace

int runFontMatch(const FontMatchOptions &options) {
    LodReader lod(options.lodPath);
    Font font = lod::decodeFont(lod::decodeMaybeCompressed(lod.read(options.fontName)), ENCODING_WINDOWS_1252);
    Reference reference = buildReference(font, options.characters);
    if (reference.glyphs.empty())
        throw Exception("Font '{}' has no glyphs for any of the requested characters", options.fontName);

    std::vector<std::string> paths = readFontList(options.listPath);
    fmt::println("{}: height {}, {} glyphs to match, {} ink pixels, {} candidates.", options.fontName,
                 reference.height, reference.glyphs.size(), reference.inkPixels, paths.size());

    std::vector<Candidate> candidates(paths.size());
    std::atomic<size_t> next = 0;
    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < std::max(1u, std::thread::hardware_concurrency()); i++) {
        threads.emplace_back([&] {
            FT_Library library = nullptr;
            if (FT_Init_FreeType(&library) != 0)
                return;
            for (size_t j = next++; j < paths.size(); j = next++)
                candidates[j] = scoreFont(library, paths[j], reference);
            FT_Done_FreeType(library);
        });
    }
    for (std::thread &thread : threads)
        thread.join();

    // A font we couldn't read is a font we didn't search, so say so - restricting the candidate pool is how the
    // right answer gets missed, and a silent skip is just a restriction we didn't notice making.
    std::vector<const Candidate *> failed;
    std::vector<const Candidate *> matched;
    for (const Candidate &candidate : candidates)
        (candidate.match ? matched : failed).push_back(&candidate);

    if (!failed.empty()) {
        fmt::println(stderr, "\n{} of {} candidates could not be read:", failed.size(), candidates.size());
        for (const Candidate *candidate : failed)
            fmt::println(stderr, "  {}: {}", candidate->path, candidate->error);
    }

    // A candidate whose best cell can't beat rendering nothing at all - a distance at or above the total ink of the
    // reference - carries no signal; near-blank renders (thin faces, ink-erasing thresholds) all pile up at exactly
    // this floor. Dropping them keeps the tail of the ranking meaningful: everything printed genuinely overlaps the
    // reference.
    size_t floored = matched.size();
    std::erase_if(matched, [&](const Candidate *candidate) { return candidate->match->distance >= reference.inkPixels; });
    floored -= matched.size();
    if (floored > 0)
        fmt::println("{} candidates dropped for scoring at or above the blank-render floor of {} pixels.", floored,
                     reference.inkPixels);

    std::ranges::sort(matched, {}, [](const Candidate *candidate) { return candidate->match->distance; });

    fmt::println("\n  {:>6} {:>7} {:>4} {:>3} {:>6}  {:28} {}", "pixels", "widths", "ppem", "bl", "mode", "family",
                 "style");
    for (const Candidate *candidate : matched | std::views::take(options.top)) {
        const Match &match = *candidate->match;
        fmt::println("  {:6} {:3}/{:<3} {:4} {:3} {:>6}  {:28} {}", match.distance, match.exactWidths,
                     reference.glyphs.size(), match.ppem, match.baseline, match.mode, match.family, match.style);
    }

    return 0;
}

int main(int argc, char **argv) {
    try {
        UnicodeCrt _(argc, argv);
        FontMatchOptions options = FontMatchOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        return runFontMatch(options);
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
