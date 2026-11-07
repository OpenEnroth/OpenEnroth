#include "Generate.h"

#include <string>
#include <string_view>
#include <utility>

#include "Library/Font/FontBuilder.h"

#include "Utility/String/Format.h"
#include "Utility/System/Fs.h"

#include "FontFiles.h"
#include "Shadow.h"

enum class MergeMode {
    MERGE_ENGLISH_ONLY, // Take the English font as is, ignoring the Russian one.
    MERGE_UNION, // English glyphs, plus the Cyrillic from the Russian font.
};
using enum MergeMode;

struct FontDesc {
    std::string_view name; // Base font name. `<name>.fnt` in `icons.lod` becomes `<name>.oef` in the output.
    MergeMode merge;
    ShadowMode shadow;
    /** How much to raise the Russian glyphs by to put them on the English baseline, in pixels. Negative values
     * lower them. */
    int russianRaise;
    /** Whether the Russian font draws its glyphs in the shadow color while the English one uses the text color.
     * Such glyphs are recolored to the text color on merge. */
    bool recolorRussian;
};

/**
 * Fonts to generate, and how to merge each one. `icons.lod` also has `calig.fnt`, which the engine doesn't use.
 *
 * `legal` is not localized - the Russian `icons.lod` ships the same font, so its 0xC0-0xFF glyphs are Latin-1
 * accents rather than Cyrillic. It is taken from the English side only, and ends up without Cyrillic.
 */
static constexpr FontDesc FONTS[] = {
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
 * Merges the Russian font into the English one, code point by code point.
 *
 * Only the English font has the Latin-1 accents, and only the Russian one has Cyrillic, so the result is the union
 * of the two. English glyphs win for the code points that both fonts have.
 *
 * @param enFont                        English font.
 * @param ruFont                        Russian font, can be empty.
 * @param desc                          How to merge.
 * @return                              Merged font, of the English font's height.
 */
static Font mergeFonts(const Font &enFont, const Font &ruFont, const FontDesc &desc) {
    // The result is of the English font's height. Some Russian fonts are taller (`quick` is 21 vs 20, `smallnum`
    // is 15 vs 14), but the extra height is cell padding - their glyphs still fit once baseline-aligned.
    FontBuilder result(enFont.height());

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

    return std::move(result).build();
}

void generateFonts(PathView enIconsLodPath, PathView ruIconsLodPath, PathView outputPath) {
    fs::mkdirs(outputPath);

    for (const FontDesc &desc : FONTS) {
        std::string lodName = fmt::format("{}.fnt", desc.name);
        Font enFont = readFont(enIconsLodPath / lodName, ENCODING_WINDOWS_1252);
        Font ruFont = FontBuilder(enFont.height()).build(); // Stays empty if we're not merging.
        if (desc.merge != MERGE_ENGLISH_ONLY)
            ruFont = readFont(ruIconsLodPath / lodName, ENCODING_WINDOWS_1251);

        Font merged = mergeFonts(enFont, ruFont, desc);
        std::string outputName = fmt::format("{}.oef", desc.name);
        writeFont(outputPath / outputName, merged);
        fmt::println("{}: {} glyphs", outputName, merged.size());
    }
}
