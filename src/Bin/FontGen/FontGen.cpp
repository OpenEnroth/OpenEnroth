#include "FontGenOptions.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

#include "Library/Image/Image.h"

#include "Library/Font/Oef.h"
#include "Library/Font/Font.h"
#include "Library/FileSystem/Directory/DirectoryFileSystem.h"
#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodFormats.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Format.h"
#include "Utility/UnicodeCrt.h"

namespace {

enum class MergeMode {
    MERGE_ENGLISH_ONLY, // Take the English font as is, ignoring the Russian one.
    MERGE_UNION, // English glyphs, plus the Cyrillic from the Russian font.
};
using enum MergeMode;

/**
 * How a font draws the shadow around its glyphs.
 */
enum class ShadowMode {
    SHADOW_NONE, // Font has no shadow.
    SHADOW_DROP, // Drop shadow - the text shifted by (1,1).
    SHADOW_OUTLINE, // Outline - the text dilated by its 4 neighbours.
};
using enum ShadowMode;

struct FontDesc {
    std::string_view name; // Base font name. `<name>.fnt` in `icons.lod` becomes `<name>.oef` in the output.
    MergeMode merge;
    ShadowMode shadow;
    int russianRaise; // How much to raise the Russian glyphs by to put them on the English baseline, in pixels.
                      // Negative values lower them.
    bool recolorRussian; // Whether the Russian font draws its glyphs in the shadow color while the English one
                         // uses the text color. Such glyphs are recolored to the text color on merge.
};

/**
 * Fonts to generate, and how to merge each one. `icons.lod` also has `calig.fnt`, which the engine doesn't use.
 *
 * `legal` is not localized - the Russian `icons.lod` ships the same font, so its 0xC0-0xFF glyphs are Latin-1
 * accents rather than Cyrillic. It is taken from the English side only, and ends up without Cyrillic.
 */
constexpr FontDesc fonts[] = {
    {"arrus",    MERGE_UNION,        SHADOW_DROP,     1, false},
    {"autonote", MERGE_UNION,        SHADOW_NONE,    -2, false},
    {"book",     MERGE_UNION,        SHADOW_OUTLINE,  0, false},
    {"book2",    MERGE_UNION,        SHADOW_OUTLINE, -1, false},
    {"cchar",    MERGE_UNION,        SHADOW_DROP,    -2, false},
    {"comic",    MERGE_UNION,        SHADOW_DROP,     0, false},
    {"create",   MERGE_UNION,        SHADOW_DROP,     1, false},
    {"endgame",  MERGE_UNION,        SHADOW_NONE,    -1, true },
    {"legal",    MERGE_ENGLISH_ONLY, SHADOW_OUTLINE,  0, false}, // Not localized.
    {"lucida",   MERGE_UNION,        SHADOW_DROP,     0, false},
    {"quick",    MERGE_UNION,        SHADOW_OUTLINE,  1, false},
    {"smallnum", MERGE_UNION,        SHADOW_DROP,     0, false},
    {"spell",    MERGE_UNION,        SHADOW_NONE,     0, false},
};

/**
 * Redraws a glyph's shadow, dropping whatever shadow it had and regenerating it from the text.
 *
 * The glyph box grows a pixel to fit the shadow, so a shadowed glyph is a pixel wider than the text alone.
 *
 * @param metrics                       Glyph metrics.
 * @param image                         Glyph image, color index `1` is the text and `2` is the shadow.
 * @param height                        Height of the resulting glyph. The source image can be of a different height.
 * @param shadow                        Shadow to draw.
 * @param raise                         How much to raise the glyph by, in pixels. Negative values lower it.
 * @param recolor                       Whether to treat the shadow color in the source image as the text color.
 * @return                              Metrics and image of the resulting glyph.
 */
std::pair<GlyphMetrics, GrayscaleImage> reshadow(const GlyphMetrics &metrics, GrayscaleImageView image,
                                                int height, ShadowMode shadow, int raise, bool recolor) {
    auto sourcePixel = [&](int y, int x) -> uint8_t {
        int sourceY = y + raise;
        if (sourceY < 0 || sourceY >= image.height())
            return 0;
        uint8_t result = image[sourceY][x];
        return recolor && result == 2 ? 1 : result;
    };

    // Only the text is carried over - whatever shadow the glyph had is redrawn from scratch below.
    int textLeft = metrics.width;
    int textRight = -1;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < metrics.width; x++) {
            if (sourcePixel(y, x) != 1)
                continue;
            textLeft = std::min(textLeft, x);
            textRight = std::max(textRight, x);
        }
    }

    // A glyph with no text needs no shadow, and is taken as is. Spaces are the only such glyphs, and shrinking
    // them to fit a shadow that isn't there would run the words together. Note that `autonote` is drawn entirely
    // in the shadow color and reaches this function only with `SHADOW_NONE`, so its pixels are passed through.
    if (shadow == SHADOW_NONE || textRight < 0) {
        GrayscaleImage result = GrayscaleImage::solid(0, metrics.width, height);
        for (int y = 0; y < height; y++)
            for (int x = 0; x < metrics.width; x++)
                result[y][x] = sourcePixel(y, x);
        return {metrics, std::move(result)};
    }

    // The shadow needs a pixel of room around the text - to the right for a drop shadow, and on both sides for an
    // outline. Grow the glyph box if the room isn't already there, which is the case for the fonts that ship
    // without a shadow. Growing to the left shifts the text inside the box, so the left spacing compensates and
    // the glyph stays where it was.
    int growLeft = shadow == SHADOW_OUTLINE ? std::max(0, 1 - textLeft) : 0;
    int growRight = std::max(0, textRight + 2 - metrics.width);
    int width = metrics.width + growLeft + growRight;

    GrayscaleImage result = GrayscaleImage::solid(0, width, height);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < metrics.width; x++)
            if (sourcePixel(y, x) == 1)
                result[y][x + growLeft] = 1;

    if (shadow == SHADOW_DROP) {
        for (int y = height - 1; y >= 1; y--)
            for (int x = width - 1; x >= 1; x--)
                if (result[y][x] == 0 && result[y - 1][x - 1] == 1)
                    result[y][x] = 2; // Text shifted by (1,1), only where there's no text.
    } else {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (result[y][x] != 0)
                    continue;
                bool neighbor = (y > 0 && result[y - 1][x] == 1) || (y + 1 < height && result[y + 1][x] == 1) ||
                                (x > 0 && result[y][x - 1] == 1) || (x + 1 < width && result[y][x + 1] == 1);
                if (neighbor)
                    result[y][x] = 2;
            }
        }
    }

    return {GlyphMetrics(metrics.leftSpacing - growLeft, width, metrics.rightSpacing), std::move(result)};
}

/**
 * Merges the Russian font into the English one, code point by code point.
 *
 * Only the English font has the Latin-1 accents, and only the Russian one has Cyrillic, so the result is the union
 * of the two. English glyphs win for the code points that both fonts have.
 */
Font mergeFonts(const Font &enFont, const Font &ruFont, const FontDesc &desc) {
    // The result is of the English font's height. Some Russian fonts are taller (`quick` is 21 vs 20, `smallnum`
    // is 15 vs 14), but the extra height is cell padding - their glyphs still fit once baseline-aligned.
    Font result(enFont.height());

    for (int i = 0; i < enFont.size(); i++) {
        auto [metrics, image] = reshadow(enFont.metrics(i), enFont.image(i), enFont.height(), desc.shadow, 0, false);
        result.add(enFont.character(i), metrics, image);
    }

    for (int i = 0; i < ruFont.size(); i++) {
        char32_t c = ruFont.character(i);
        if (enFont.supports(c))
            continue; // English wins.

        auto [metrics, image] = reshadow(ruFont.metrics(i), ruFont.image(i), enFont.height(), desc.shadow, desc.russianRaise,
                                        desc.recolorRussian);
        result.add(c, metrics, image);
    }

    return result;
}

/**
 * @return                              Contents of the LOD entry, decompressed if it was compressed.
 */
Blob readEntry(const LodReader &reader, std::string_view name) {
    return lod::decodeMaybeCompressed(reader.read(name));
}

} // namespace

int runFontGen(const FontGenOptions &options) {
    LodReader enLod(options.enLodPath);
    LodReader ruLod(options.ruLodPath);
    DirectoryFileSystem output(options.outputPath);

    for (const FontDesc &desc : fonts) {
        std::string lodName = fmt::format("{}.fnt", desc.name);
        Font enFont = lod::decodeFont(readEntry(enLod, lodName), ENCODING_WINDOWS_1252);
        Font ruFont(enFont.height()); // Stays empty if we're not merging.

        // The Russian font can be of a different height, its glyphs are then baseline-aligned into the English
        // font's cell.
        if (desc.merge != MERGE_ENGLISH_ONLY)
            ruFont = lod::decodeFont(readEntry(ruLod, lodName), ENCODING_WINDOWS_1251);

        Font merged = mergeFonts(enFont, ruFont, desc);
        std::string outputName = fmt::format("{}.oef", desc.name);
        output.write(outputName, oef::encode(merged));
        fmt::println("{}: {} glyphs", outputName, merged.size());
    }

    return 0;
}

int main(int argc, char **argv) {
    try {
        UnicodeCrt _(argc, argv);
        FontGenOptions options = FontGenOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        return runFontGen(options);
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
