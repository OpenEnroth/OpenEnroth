#include "LodToolOptions.h"

#include <memory>

#include "Library/Cli/CliApp.h"

LodToolOptions LodToolOptions::parse(int argc, char **argv) {
    LodToolOptions result;
    std::unique_ptr<CliApp> app = std::make_unique<CliApp>("Might & Magic lod/vid/snd archive manipulation tool.\n");

    app->set_help_flag("-h,--help", "Print help and exit.");
    app->require_subcommand();

    CLI::App *ls = app->add_subcommand("ls", "List an archive file", result.subcommand, SUBCOMMAND_LS)->fallthrough();
    ls->add_option("ARCHIVE", result.path, "Path to archive file.")->check(CLI::ExistingFile)->required()->option_text(" ");

    CLI::App *dump = app->add_subcommand("dump", "Dump an archive file.", result.subcommand, SUBCOMMAND_DUMP)->fallthrough();
    dump->add_option("ARCHIVE", result.path, "Path to archive file.")->check(CLI::ExistingFile)->required()->option_text(" ");

    CLI::App *cat = app->add_subcommand("cat", "Write contents of a single archive entry to stdout.", result.subcommand, SUBCOMMAND_CAT)->fallthrough();
    cat->add_flag("--raw", result.raw, "Don't decompress compressed entries & don't convert images to png.");
    cat->add_option("--palettes-lod-path", result.palettesLodPath, "Path to bitmaps.lod to use for sprite palettes.")->check(CLI::ExistingFile)->option_text("PATH");
    cat->add_option("ARCHIVE", result.path, "Path to archive file.")->check(CLI::ExistingFile)->required()->option_text(" ");
    cat->add_option("ENTRY", result.cat.entry, "Name of the entry to print.")->required()->option_text(" ");

    CLI::App *extract = app->add_subcommand("extract", "Extract everything from an archive file.", result.subcommand, SUBCOMMAND_EXTRACT)->fallthrough();
    extract->add_flag("--raw", result.raw, "Don't decompress compressed entries & don't convert images to png.");
    extract->add_option("--palettes-lod-path", result.palettesLodPath, "Path to bitmaps.lod to use for sprite palettes.")->check(CLI::ExistingFile)->option_text("PATH");
    extract->add_option("ARCHIVE", result.path, "Path to archive file.")->check(CLI::ExistingFile)->required()->option_text(" ");
    extract->add_option("OUTPUT", result.extract.output, "Directory to extract the entries to.")->required()->option_text(" ");

    app->parse(argc, argv, result.helpPrinted);
    return result;
}
