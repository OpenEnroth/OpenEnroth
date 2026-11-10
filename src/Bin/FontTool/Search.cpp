#include "Search.h"

#include <algorithm>
#include <atomic>
#include <bit>
#include <climits>
#include <cmath>
#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "Utility/Exception.h"
#include "Utility/String/Format.h"

#include "FontFiles.h"
#include "FreeType.h"

static constexpr int MAX_GLYPH_WIDTH = 64; // Glyph rows are single `uint64_t`s.
static constexpr int MAX_FACES_PER_FILE = 64; // Real collections have a handful of faces, more is a broken file.
static constexpr int MIN_PLACEHOLDER_COUNT = 3; // A glyph image shared by this many characters is a placeholder.
static constexpr int GRAY_THRESHOLDS[] = {64, 96, 128, 160, 192, 224};
static constexpr int EXHAUSTIVE_PPEM_RANGE = 12;
static constexpr int EXHAUSTIVE_MAX_OFFSET = 4;

struct Ink {
    std::vector<uint64_t> rows;
    int pixels = 0;
    double cx = 0; // Centroid.
    double cy = 0;
    int left = 0; // Leftmost ink column.
    int width = 0; // Width of the ink's bounding box.
    int height = 0;
};

/**
 * @param width                         Image width, at most `MAX_GLYPH_WIDTH`.
 * @param rows                          Image height.
 * @param pixel                         Callable `(x, y) -> bool`, whether the pixel is ink.
 * @return                              Ink of the image.
 */
template<class Pixel>
static Ink makeInk(int width, int rows, Pixel &&pixel) {
    Ink result;
    result.rows.assign(rows, 0);
    int minX = INT_MAX, maxX = -1, minY = INT_MAX, maxY = -1;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < width; x++) {
            if (!pixel(x, y))
                continue;
            result.rows[y] |= uint64_t(1) << x;
            result.pixels++;
            result.cx += x;
            result.cy += y;
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }
    }
    if (result.pixels > 0) {
        result.cx /= result.pixels;
        result.cy /= result.pixels;
        result.left = minX;
        result.width = maxX - minX + 1;
        result.height = maxY - minY + 1;
    }
    return result;
}

struct ReferenceGlyph {
    char32_t character = 0;
    Ink ink;
    int inkStart = 0; // From the pen position to the leftmost ink column.
    int advance = 0;
};

struct Reference {
    int height = 0;
    int inkPixels = 0;
    std::vector<ReferenceGlyph> glyphs;
};

struct CandidateGlyph {
    int top = 0;
    bool tooWide = false; // Wider than `MAX_GLYPH_WIDTH`, scored as all wrong.
    Ink ink;
    int inkStart = 0; // From the pen position to the leftmost ink column.
    int advance = 0;
};

/**
 * @param reference                     Reference font and the characters to match on.
 * @return                              Ink of the characters to match on. Characters with no ink, placeholder glyphs
 *                                      and glyphs wider than `MAX_GLYPH_WIDTH` are left out.
 */
static Reference prepareReference(const SearchReference &reference) {
    Reference result;
    result.height = reference.font.height();
    uint8_t color = inkColorIndex(reference.font);

    // Bitmap fonts draw the characters they don't have as a placeholder box, and those can't be matched.
    std::map<std::vector<uint8_t>, int> imageCounts;
    for (int i = 0; i < reference.font.size(); i++) {
        std::span<const uint8_t> pixels = reference.font.image(i).pixels();
        imageCounts[std::vector<uint8_t>(pixels.begin(), pixels.end())]++;
    }

    for (char32_t c : reference.characters) {
        int index = reference.font.index(c);
        if (index == -1)
            continue;
        GrayscaleImageView image = reference.font.image(index);
        if (image.width() > MAX_GLYPH_WIDTH)
            continue;
        if (imageCounts[std::vector<uint8_t>(image.pixels().begin(), image.pixels().end())] >= MIN_PLACEHOLDER_COUNT)
            continue;

        ReferenceGlyph glyph;
        glyph.character = c;
        glyph.ink = makeInk(image.width(), image.height(), [&](int x, int y) { return image[y][x] == color; });
        if (glyph.ink.pixels == 0)
            continue; // Spaces have nothing to match on.
        const GlyphMetrics &metrics = reference.font.metrics(index);
        glyph.inkStart = metrics.leftSpacing + glyph.ink.left;
        glyph.advance = metrics.leftSpacing + metrics.width + metrics.rightSpacing;

        result.inkPixels += glyph.ink.pixels;
        result.glyphs.push_back(std::move(glyph));
    }

    return result;
}

static CandidateGlyph makeCandidateGlyph(const InkMask &mask) {
    CandidateGlyph result;
    result.top = mask.top;
    result.advance = mask.advance;
    if (mask.width > MAX_GLYPH_WIDTH) {
        result.tooWide = true;
        result.ink.pixels = std::ranges::count(mask.pixels, 1);
        return result;
    }
    result.ink = makeInk(mask.width, mask.rows, [&](int x, int y) { return mask.pixels[y * mask.width + x] != 0; });
    result.inkStart = mask.left + result.ink.left;
    return result;
}

/**
 * @param reference                     Reference glyph ink, in cell coordinates.
 * @param candidate                     Candidate glyph ink, in bitmap coordinates.
 * @param dy                            Cell row of the candidate's first row.
 * @param dx                            Cell column of the candidate's first column.
 * @return                              Number of pixels that differ.
 */
static int distance(const Ink &reference, const Ink &candidate, int dy, int dx) {
    int overlap = 0;
    for (int y = 0; y < static_cast<int>(candidate.rows.size()); y++) {
        int cellY = y + dy;
        if (cellY < 0 || cellY >= static_cast<int>(reference.rows.size()))
            continue;
        uint64_t row = candidate.rows[y];
        if (dx >= 64 || dx <= -64) {
            row = 0;
        } else {
            row = dx >= 0 ? row << dx : row >> -dx;
        }
        overlap += std::popcount(reference.rows[cellY] & row);
    }
    return reference.pixels + candidate.pixels - 2 * overlap;
}

/**
 * Score of one rendering of a candidate face, at one size and binarization, against a whole reference.
 */
struct CellScore {
    int distance = INT_MAX;
    int baseline = 0;
    int exactWidths = 0;
    int inkOffset = 0; // Reference minus candidate ink start, in the most common spacing.
    int extraAdvance = 0; // Reference minus candidate advance, in the most common spacing.
    int exactSpacing = 0; // Glyphs spaced exactly that way.
};


/**
 * Scores one rendering of a candidate against a reference.
 *
 * @param reference                     Reference to score against.
 * @param candidates                    Candidate glyphs, one per reference glyph.
 * @param exhaustive                    Whether to try all baselines and offsets instead of the centroid ones.
 * @param bound                         Scoring stops once the distance can't get below this.
 * @return                              Score, `distance` is `INT_MAX` if scoring stopped early.
 */
static CellScore scoreCell(const Reference &reference, const std::vector<const CandidateGlyph *> &candidates,
                           bool exhaustive, int bound) {
    std::vector<int> baselines;
    if (exhaustive) {
        for (int baseline = 0; baseline <= reference.height + 4; baseline++)
            baselines.push_back(baseline);
    } else {
        // A glyph's ink centroid sits `cy` rows below the cell top in the reference, and `cy` rows below the
        // bitmap top in the render, which is `top` rows above the baseline. All glyphs share the baseline.
        std::vector<double> estimates;
        for (size_t i = 0; i < candidates.size(); i++) {
            const CandidateGlyph &candidate = *candidates[i];
            if (!candidate.tooWide && candidate.ink.pixels > 0)
                estimates.push_back(reference.glyphs[i].ink.cy - candidate.ink.cy + candidate.top);
        }
        if (estimates.empty())
            return {};
        std::ranges::nth_element(estimates, estimates.begin() + estimates.size() / 2);
        int baseline = static_cast<int>(std::lround(estimates[estimates.size() / 2]));
        baselines = {baseline - 1, baseline, baseline + 1};
    }

    std::vector<int> partial(baselines.size(), 0);
    for (size_t i = 0; i < candidates.size(); i++) {
        const Ink &referenceInk = reference.glyphs[i].ink;
        const CandidateGlyph &candidate = *candidates[i];

        int firstDx = -EXHAUSTIVE_MAX_OFFSET;
        int lastDx = EXHAUSTIVE_MAX_OFFSET;
        if (!exhaustive) {
            int dx = static_cast<int>(std::lround(referenceInk.cx - candidate.ink.cx));
            firstDx = dx - 1;
            lastDx = dx + 1;
        }

        int best = INT_MAX;
        for (size_t k = 0; k < baselines.size(); k++) {
            int glyphDistance = referenceInk.pixels + candidate.ink.pixels;
            if (!candidate.tooWide && candidate.ink.pixels > 0) {
                for (int dx = firstDx; dx <= lastDx; dx++)
                    glyphDistance = std::min(glyphDistance, distance(referenceInk, candidate.ink, baselines[k] - candidate.top, dx));
            }
            partial[k] += glyphDistance;
            best = std::min(best, partial[k]);
        }

        if (best >= bound)
            return {};
    }

    size_t k = std::ranges::min_element(partial) - partial.begin();
    CellScore result;
    result.distance = partial[k];
    result.baseline = baselines[k];
    std::map<std::pair<int, int>, int> spacings; // (ink offset, extra advance) -> number of glyphs.
    for (size_t i = 0; i < candidates.size(); i++) {
        const CandidateGlyph &candidate = *candidates[i];
        if (candidate.tooWide || candidate.ink.pixels == 0)
            continue;
        result.exactWidths += candidate.ink.width == reference.glyphs[i].ink.width;
        spacings[{reference.glyphs[i].inkStart - candidate.inkStart, reference.glyphs[i].advance - candidate.advance}]++;
    }
    if (spacings.empty())
        return result;

    auto spacing = std::ranges::max_element(spacings, std::ranges::less(), [](const auto &pair) { return pair.second; });
    result.inkOffset = spacing->first.first;
    result.extraAdvance = spacing->first.second;
    result.exactSpacing = spacing->second;
    return result;
}

/**
 * Best matches for one reference, shared between the search threads.
 */
class Leaderboard {
 public:
    explicit Leaderboard(int size) : _size(size) {}

    [[nodiscard]] int bound() const {
        return _bound.load(std::memory_order_relaxed);
    }

    void add(SearchMatch match) {
        std::lock_guard lock(_mutex);
        _matches.push_back(std::move(match));
        std::ranges::stable_sort(_matches, {}, &SearchMatch::distance);
        if (_matches.size() > static_cast<size_t>(_size))
            _matches.resize(_size);
        if (_matches.size() == static_cast<size_t>(_size))
            _bound.store(_matches.back().distance, std::memory_order_relaxed);
    }

    [[nodiscard]] std::vector<SearchMatch> matches() const {
        std::lock_guard lock(_mutex);
        return _matches;
    }

 private:
    int _size = 0;
    std::atomic<int> _bound = INT_MAX;
    mutable std::mutex _mutex;
    std::vector<SearchMatch> _matches;
};

/**
 * How a face is rendered and thresholded into ink, everything about a rendering except the size.
 */
struct Binarization {
    RasterMode mode = RASTER_MONO;
    bool oblique = false;
    int threshold = 1;
    bool bold = false;
};

class Searcher {
 public:
    Searcher(const std::vector<Reference> &references, const SearchOptions &options) :
        _references(references), _options(options) {
        for (size_t i = 0; i < references.size(); i++)
            _leaderboards.push_back(std::make_unique<Leaderboard>(options.top));

        for (const Reference &reference : references) {
            std::vector<size_t> &indices = _characterIndices.emplace_back();
            for (const ReferenceGlyph &glyph : reference.glyphs) {
                auto pos = std::ranges::find(_characters, glyph.character);
                if (pos == _characters.end())
                    pos = _characters.insert(pos, glyph.character);
                indices.push_back(pos - _characters.begin());
            }
        }
    }

    void searchFile(const FreeTypeLibrary &library, const std::string &path) {
        FreeTypeFace first(library, path, 0);
        int faceCount = std::min(first.faceCount(), MAX_FACES_PER_FILE);
        searchFace(first, path, 0);
        for (int faceIndex = 1; faceIndex < faceCount; faceIndex++) {
            FreeTypeFace face(library, path, faceIndex);
            searchFace(face, path, faceIndex);
        }
    }

    [[nodiscard]] std::vector<SearchMatch> matches(size_t reference) const {
        return _leaderboards[reference]->matches();
    }

 private:
    struct FaceState {
        std::vector<bool> active; // Reference index -> whether the face has all its characters.
        std::vector<std::optional<SearchMatch>> best; // Reference index -> best match in this face.
    };

    void searchFace(FreeTypeFace &face, const std::string &path, int faceIndex) {
        FaceState state;
        state.active.assign(_references.size(), false);
        state.best.resize(_references.size());
        bool anyActive = false;
        for (size_t r = 0; r < _references.size(); r++) {
            state.active[r] = std::ranges::all_of(_references[r].glyphs, [&](const ReferenceGlyph &glyph) {
                return face.hasGlyph(glyph.character);
            });
            anyActive |= state.active[r];
        }
        if (!anyActive)
            return;

        if (face.isScalable()) {
            searchScalable(face, &state);
        } else {
            std::vector<int> strikes = face.strikeHeights();
            for (int strike = 0; strike < static_cast<int>(strikes.size()); strike++) {
                if (!face.selectStrike(strike))
                    continue;
                std::vector<size_t> all;
                for (size_t r = 0; r < _references.size(); r++)
                    if (state.active[r])
                        all.push_back(r);
                searchSize(face, strikes[strike], all, &state,
                           {{.mode = RASTER_MONO, .threshold = 1}, {.mode = RASTER_MONO, .threshold = 1, .bold = true}});
            }
        }

        for (size_t r = 0; r < _references.size(); r++) {
            if (!state.best[r])
                continue;
            state.best[r]->path = path;
            state.best[r]->faceIndex = faceIndex;
            state.best[r]->family = face.family();
            state.best[r]->style = face.style();
            _leaderboards[r]->add(std::move(*state.best[r]));
        }
    }

    void searchScalable(FreeTypeFace &face, FaceState *state) {
        std::vector<std::vector<int>> sizes(_references.size()); // Reference index -> sizes to try.
        for (size_t r = 0; r < _references.size(); r++) {
            if (!state->active[r])
                continue;

            const Reference &reference = _references[r];
            int first = std::max(4, reference.height - EXHAUSTIVE_PPEM_RANGE);
            int last = reference.height + EXHAUSTIVE_PPEM_RANGE;
            if (!_options.exhaustive) {
                // Ink heights scale with the size, so the reference's ink heights over the outlines' heights in font
                // units is the size in pixels per font unit.
                long long pixels = 0;
                long long units = 0;
                for (const ReferenceGlyph &glyph : reference.glyphs) {
                    std::optional<int> height = face.outlineHeight(glyph.character);
                    if (!height)
                        continue;
                    pixels += glyph.ink.height;
                    units += *height;
                }
                if (units <= 0)
                    continue;
                double estimate = static_cast<double>(pixels) * face.unitsPerEm() / units;
                int ppem = static_cast<int>(std::lround(std::clamp(estimate, 4.0, 4.0 * reference.height)));
                first = std::max(4, ppem - _options.ppemWindow);
                last = ppem + _options.ppemWindow;
            }
            for (int ppem = first; ppem <= last; ppem++)
                sizes[r].push_back(ppem);
        }

        std::vector<int> allSizes;
        for (const std::vector<int> &referenceSizes : sizes)
            allSizes.insert(allSizes.end(), referenceSizes.begin(), referenceSizes.end());
        std::ranges::sort(allSizes);
        auto [tail, end] = std::ranges::unique(allSizes);
        allSizes.erase(tail, end);

        std::vector<Binarization> binarizations;
        for (bool oblique : {false, true}) {
            if (oblique && !_options.oblique)
                continue;
            binarizations.push_back({.mode = RASTER_MONO, .oblique = oblique, .threshold = 1});
            binarizations.push_back({.mode = RASTER_MONO, .oblique = oblique, .threshold = 1, .bold = true});
            for (int threshold : GRAY_THRESHOLDS)
                binarizations.push_back({.mode = RASTER_GRAY, .oblique = oblique, .threshold = threshold});
        }

        for (int ppem : allSizes) {
            std::vector<size_t> references;
            for (size_t r = 0; r < _references.size(); r++)
                if (std::ranges::contains(sizes[r], ppem))
                    references.push_back(r);
            if (face.setPixelSize(ppem))
                searchSize(face, ppem, references, state, binarizations);
        }
    }

    void searchSize(FreeTypeFace &face, int ppem, const std::vector<size_t> &references, FaceState *state,
                    const std::vector<Binarization> &binarizations) {
        std::vector<std::optional<RenderedGlyph>> rendered[2][2]; // Indexed by `RasterMode` and obliqueness.
        std::vector<CandidateGlyph> candidates(_characters.size());

        for (const Binarization &binarization : binarizations) {
            std::vector<std::optional<RenderedGlyph>> &glyphs = rendered[static_cast<int>(binarization.mode)][binarization.oblique];
            if (glyphs.empty())
                for (char32_t c : _characters)
                    glyphs.push_back(face.render(c, binarization.mode, binarization.oblique));

            for (size_t i = 0; i < _characters.size(); i++)
                candidates[i] = glyphs[i] ? makeCandidateGlyph(makeInkMask(*glyphs[i], binarization.threshold, binarization.bold)) : CandidateGlyph();

            for (size_t r : references) {
                const Reference &reference = _references[r];
                std::vector<const CandidateGlyph *> cell;
                for (size_t index : _characterIndices[r])
                    cell.push_back(&candidates[index]);

                int bound = _leaderboards[r]->bound();
                if (state->best[r])
                    bound = std::min(bound, state->best[r]->distance);

                CellScore score = scoreCell(reference, cell, _options.exhaustive, bound);
                if (score.distance == INT_MAX)
                    continue;

                SearchMatch match;
                match.distance = score.distance;
                match.exactWidths = score.exactWidths;
                match.exactSpacing = score.exactSpacing;
                match.settings.inkOffset = score.inkOffset;
                match.settings.extraAdvance = score.extraAdvance;
                match.settings.ppem = ppem;
                match.settings.mode = binarization.mode;
                match.settings.threshold = binarization.threshold;
                match.settings.bold = binarization.bold;
                match.settings.oblique = binarization.oblique;
                match.settings.baseline = score.baseline;
                match.settings.height = reference.height;
                state->best[r] = std::move(match);
            }
        }
    }

    const std::vector<Reference> &_references;
    const SearchOptions &_options;
    std::vector<std::unique_ptr<Leaderboard>> _leaderboards;
    std::vector<char32_t> _characters; // Union of the references' characters.
    std::vector<std::vector<size_t>> _characterIndices; // Reference index -> glyph index -> index in `_characters`.
};

std::vector<SearchResult> searchFonts(const std::vector<SearchReference> &references, const std::vector<std::string> &paths,
                                      const SearchOptions &options, std::vector<std::string> *failures) {
    std::vector<Reference> prepared;
    for (const SearchReference &reference : references) {
        prepared.push_back(prepareReference(reference));
        if (prepared.back().glyphs.empty())
            throw Exception("Reference font '{}' has no glyphs to match on", reference.name);
    }

    Searcher searcher(prepared, options);
    std::atomic<size_t> next = 0;
    std::mutex failuresMutex;
    int threadCount = options.threads > 0 ? options.threads : std::max(1u, std::thread::hardware_concurrency());

    std::vector<std::unique_ptr<FreeTypeLibrary>> libraries;
    for (int t = 0; t < threadCount; t++)
        libraries.push_back(std::make_unique<FreeTypeLibrary>());

    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++) {
        threads.emplace_back([&, &library = *libraries[t]] {
            for (size_t i = next++; i < paths.size(); i = next++) {
                if (i % 10000 == 0)
                    fmt::println(stderr, "{}/{} fonts searched", i, paths.size());
                try {
                    searcher.searchFile(library, paths[i]);
                } catch (const std::exception &e) {
                    std::lock_guard lock(failuresMutex);
                    failures->push_back(e.what());
                }
            }
        });
    }
    for (std::thread &thread : threads)
        thread.join();

    std::vector<SearchResult> result;
    for (size_t r = 0; r < prepared.size(); r++) {
        SearchResult &referenceResult = result.emplace_back();
        referenceResult.glyphCount = prepared[r].glyphs.size();
        referenceResult.inkPixels = prepared[r].inkPixels;
        referenceResult.matches = searcher.matches(r);
    }
    return result;
}
