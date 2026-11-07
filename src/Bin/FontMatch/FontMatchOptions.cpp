#include "FontMatchOptions.h"

#include <memory>

#include "Library/Cli/CliApp.h"

FontMatchOptions FontMatchOptions::parse(int argc, char **argv) {
    FontMatchOptions result;
    std::unique_ptr<CliApp> app = std::make_unique<CliApp>(
        "Finds the typeface a Might & Magic bitmap font was rendered from.\n"
        "\n"
        "Renders every candidate font across the full ppem x baseline x render mode matrix, and compares the\n"
        "result to the reference font pixel by pixel. Prints the best matches, sorted by pixel distance.\n"
        "\n"
        "Scores are bimodal - a real match scores below ~50, a non-match scores 900 or more, and there is\n"
        "nothing in between. Ordering within the noise band is meaningless. Exact widths is the number of\n"
        "glyphs whose rendered width matches the original exactly, and it can identify a face that pixel\n"
        "distance rejects - a hand-tuned bitmap keeps the widths but loses the pixels.\n");

    app->set_help_flag("-h,--help", "Print help and exit.");

    app->add_option("ICONS_LOD", result.lodPath, "Path to icons.lod to read the reference font from.")
        ->check(CLI::ExistingFile)->required()->option_text(" ");
    app->add_option("FONT", result.fontName, "Name of the font inside the lod, e.g. \"autonote.fnt\".")
        ->required()->option_text(" ");
    app->add_option("FONT_LIST", result.listPath, "Path to a file listing candidate font paths, one per line.")
        ->check(CLI::ExistingFile)->required()->option_text(" ");

    // Matching on a subset is a footgun - capitals alone are too few and too generic, and rank false positives
    // that the full charset knocks out. Narrow this only to debug a face, never to decide one.
    app->add_option("--chars", result.characters,
                    "Characters to match on. Defaults to all printable ASCII, and you should leave it that way.")
        ->option_text("STR");
    app->add_option("--top", result.top, "Number of best matches to print.")
        ->check(CLI::PositiveNumber)->option_text("N");

    app->parse(argc, argv, result.helpPrinted);

    if (result.characters.empty()) {
        for (char c = '!'; c <= '~'; c++)
            result.characters += c;
    }

    return result;
}
