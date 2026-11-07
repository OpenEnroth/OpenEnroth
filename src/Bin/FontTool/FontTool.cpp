#include "FontToolOptions.h"

#include <algorithm>
#include <cstdio>
#include <exception>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Library/Font/FontBuilder.h"
#include "Library/Image/Png.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Format.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"
#include "Utility/System/Fs.h"
#include "Utility/Exception.h"
#include "Utility/UnicodeCrt.h"

#include "Diff.h"
#include "FontFiles.h"
#include "FreeType.h"
#include "Generate.h"
#include "Render.h"
#include "Search.h"
#include "Shadow.h"

static constexpr std::string_view FONT_EXTENSIONS[] = {".ttf", ".otf", ".ttc", ".otc", ".tt", ".pfb", ".pfa", ".fon", ".fnt", ".dfont", ".woff"};

static std::u32string range(char32_t first, char32_t last) {
    std::u32string result;
    for (char32_t c = first; c <= last; c++)
        result += c;
    return result;
}

static std::u32string printableAscii() {
    return range(U'!', U'~');
}

static std::u32string cyrillic() {
    return U"Ё" + range(U'А', U'я') + U"ё";
}

/**
 * @param text                          UTF-8 text from the command line, can be empty.
 * @param fallback                      Text to use if `text` is empty.
 * @return                              `text` as UTF-32, or `fallback`.
 */
static std::u32string textOr(const std::string &text, std::u32string fallback) {
    return text.empty() ? std::move(fallback) : txt::encodedToUtf32(text, ENCODING_UTF8);
}

static void writePng(PathView path, const RgbaImage &image) {
    FileOutputStream output(path);
    output.write(png::encode(image));
    output.close();
}

static int runConvert(const FontToolOptions &options) {
    writeFont(options.output, readFont(options.input, options.encoding));
    return 0;
}

static int runMerge(const FontToolOptions &options) {
    std::vector<Font> fonts;
    for (const std::string &input : options.inputs)
        fonts.push_back(readFont(input, options.encoding));

    FontBuilder builder(fonts[0].height());
    for (size_t i = 0; i < fonts.size(); i++) {
        if (fonts[i].height() != builder.height())
            throw Exception("Can't merge '{}' of height {} into a font of height {}", options.inputs[i], fonts[i].height(), builder.height());
        for (int j = 0; j < fonts[i].size(); j++)
            if (!builder.supports(fonts[i].character(j)))
                builder.add(fonts[i].character(j), fonts[i].metrics(j), fonts[i].image(j));
    }

    writeFont(options.output, std::move(builder).build());
    return 0;
}

static int runRender(const FontToolOptions &options) {
    FreeTypeLibrary library;
    FreeTypeFace face(library, options.input, options.faceIndex);

    if (face.isScalable()) {
        if (!face.setPixelSize(options.render.ppem))
            throw Exception("Can't render '{}' at {} pixels per em", options.input, options.render.ppem);
    } else {
        std::vector<int> strikes = face.strikeHeights();
        auto strike = std::ranges::find(strikes, options.render.ppem);
        if (strike == strikes.end() || !face.selectStrike(strike - strikes.begin()))
            throw Exception("Bitmap font '{}' has no strike of height {}", options.input, options.render.ppem);
    }

    std::u32string text = textOr(options.text, U" " + printableAscii() + range(0xA0, 0xFF) + cyrillic());
    Font font = renderFont(face, text, options.render);

    if (options.shadow != SHADOW_NONE) {
        FontBuilder builder(font.height());
        for (int i = 0; i < font.size(); i++) {
            auto [metrics, image] = reshadow(font.metrics(i), font.image(i), font.height(), options.shadow, 0, false);
            builder.add(font.character(i), metrics, image);
        }
        font = std::move(builder).build();
    }

    writeFont(options.output, font);
    return 0;
}

static int runDiff(const FontToolOptions &options) {
    Font a = readFont(options.input, options.encoding);
    Font b = readFont(options.secondInput, options.encoding);
    writePng(options.output, renderFontDiff(a, b, textOr(options.text, printableAscii()), options.scale));
    return 0;
}

static bool isFontFile(std::string_view name) {
    std::string lower = ascii::toLower(name);
    return std::ranges::any_of(FONT_EXTENSIONS, [&](std::string_view extension) { return lower.ends_with(extension); });
}

static void collectFonts(const Path &path, std::vector<std::string> *result) {
    if (fs::stat(path).type != FILE_DIRECTORY) {
        result->push_back(path.str());
        return;
    }

    for (const DirectoryEntry &entry : fs::ls(path)) {
        Path child = path / entry.name;
        if (entry.type == FILE_DIRECTORY) {
            collectFonts(child, result);
        } else if (isFontFile(entry.name)) {
            result->push_back(child.str());
        }
    }
}

static int runSearch(const FontToolOptions &options) {
    std::vector<SearchReference> references;
    for (const std::string &path : options.references) {
        SearchReference &reference = references.emplace_back();
        reference.name = path;
        reference.font = readFont(path, options.encoding);
        reference.characters = textOr(options.text, printableAscii() + cyrillic());
    }

    std::vector<std::string> paths;
    for (const std::string &input : options.inputs)
        collectFonts(Path(PathView(input)), &paths);
    if (!options.listPath.empty()) {
        for (std::string_view line : split(Blob::fromFile(options.listPath).str()).by('\n'))
            if (std::string_view path = trim(line); !path.empty())
                paths.emplace_back(path);
    }
    fmt::println(stderr, "Searching {} font files for {} reference fonts.", paths.size(), references.size());

    SearchOptions searchOptions;
    searchOptions.top = options.top;
    searchOptions.ppemWindow = options.ppemWindow;
    searchOptions.exhaustive = options.exhaustive;
    searchOptions.oblique = options.oblique;
    searchOptions.threads = options.threads;

    std::vector<std::string> failures;
    std::vector<SearchResult> results = searchFonts(references, paths, searchOptions, &failures);

    fmt::println(stderr, "{} of {} font files couldn't be read.", failures.size(), paths.size());
    for (const std::string &failure : failures | std::views::take(20))
        fmt::println(stderr, "  {}", failure);

    for (size_t r = 0; r < references.size(); r++) {
        const SearchResult &result = results[r];
        fmt::println("\n{}: {} glyphs, {} ink pixels", references[r].name, result.glyphCount, result.inkPixels);
        for (const SearchMatch &match : result.matches) {
            fmt::println("  {:6} {:3}/{:<3}  {:58}  {} / {}  {}{}", match.distance, match.exactWidths, result.glyphCount,
                         match.settings.toString(), match.family, match.style, match.path,
                         match.faceIndex > 0 ? fmt::format(" --face-index {}", match.faceIndex) : "");
        }
    }

    return 0;
}

static int runGenerate(const FontToolOptions &options) {
    generateFonts(options.enIconsLodPath, options.ruIconsLodPath, options.output);
    return 0;
}

int main(int argc, char **argv) {
    try {
        UnicodeCrt _(argc, argv);
        FontToolOptions options = FontToolOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        switch (options.subcommand) {
        case FontToolOptions::SUBCOMMAND_CONVERT: return runConvert(options);
        case FontToolOptions::SUBCOMMAND_MERGE: return runMerge(options);
        case FontToolOptions::SUBCOMMAND_RENDER: return runRender(options);
        case FontToolOptions::SUBCOMMAND_DIFF: return runDiff(options);
        case FontToolOptions::SUBCOMMAND_SEARCH: return runSearch(options);
        case FontToolOptions::SUBCOMMAND_GENERATE: return runGenerate(options);
        }
        return 1;
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
