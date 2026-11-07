#include "LodToolOptions.h"

#include <memory>

#include "Library/Cli/CliApp.h"

LodToolOptions LodToolOptions::parse(int argc, char **argv) {
    LodToolOptions result;
    std::unique_ptr<CliApp> app = std::make_unique<CliApp>(
        "Might & Magic lod/vid/snd archive manipulation tool.\n"
        "\n"
        "Paths can point into archives, as in 'data/games.lod/d.odm'.\n");

    app->set_help_flag("-h,--help", "Print help and exit.");
    app->require_subcommand();

    CLI::App *ls = app->add_subcommand("ls", "List an archive.", result.subcommand, SUBCOMMAND_LS)->fallthrough();
    ls->add_option("ARCHIVE", result.path, "Path to an archive.")->required()->option_text(" ");

    CLI::App *cat = app->add_subcommand("cat", "Write a file to stdout, decompressing it.", result.subcommand, SUBCOMMAND_CAT)->fallthrough();
    cat->add_flag("--raw", result.raw, "Don't decompress.");
    cat->add_option("PATH", result.path, "Path to a file.")->required()->option_text(" ");

    CLI::App *extract = app->add_subcommand("extract", "Extract an archive, decompressing its entries.", result.subcommand, SUBCOMMAND_EXTRACT)->fallthrough();
    extract->add_flag("--raw", result.raw, "Don't decompress.");
    extract->add_option("ARCHIVE", result.path, "Path to a LOD, VID or SND archive.")->required()->option_text(" ");
    extract->add_option("OUTPUT", result.output, "Directory to extract to, created if needed.")->required()->option_text(" ");

    CLI::App *convert = app->add_subcommand("convert", "Convert a file into a format given by the output extension, only png is supported.",
                                            result.subcommand, SUBCOMMAND_CONVERT)->fallthrough();
    convert->add_option("--palettes-lod-path", result.palettesLodPath, "Path to bitmaps.lod to use for sprite palettes.")->check(CLI::ExistingFile)->option_text("PATH");
    convert->add_option("PATH", result.path, "Path to an image, a sprite, a palette or a font.")->required()->option_text(" ");
    convert->add_option("OUTPUT", result.output, "Path to the output file.")->required()->option_text(" ");

    CLI::App *magic = app->add_subcommand("magic", "Print the format of a file.", result.subcommand, SUBCOMMAND_MAGIC)->fallthrough();
    magic->add_option("PATH", result.path, "Path to a file.")->required()->option_text(" ");

    app->parse(argc, argv, result.helpPrinted);
    return result;
}
