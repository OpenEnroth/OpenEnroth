#include "FontGenOptions.h"

#include <memory>

#include "Library/Cli/CliApp.h"

FontGenOptions FontGenOptions::parse(int argc, char **argv) {
    FontGenOptions result;
    std::unique_ptr<CliApp> app = std::make_unique<CliApp>(
        "Generates OpenEnroth font files from Might & Magic localization data.\n"
        "\n"
        "The English and Russian fonts ship the same letterforms for some faces, but the English ones carry a\n"
        "baked-in shadow and the Latin-1 accents, while only the Russian ones carry Cyrillic. This tool merges\n"
        "them into single Unicode-keyed fonts.\n");

    app->set_help_flag("-h,--help", "Print help and exit.");

    app->add_option("EN_ICONS_LOD", result.enLodPath, "Path to icons.lod from an English localization.")
        ->check(CLI::ExistingFile)->required()->option_text(" ");
    app->add_option("RU_ICONS_LOD", result.ruLodPath, "Path to icons.lod from a Russian localization.")
        ->check(CLI::ExistingFile)->required()->option_text(" ");
    app->add_option("OUTPUT", result.outputPath, "Directory to write the generated fonts into.")
        ->required()->option_text(" ");

    app->parse(argc, argv, result.helpPrinted);
    return result;
}
