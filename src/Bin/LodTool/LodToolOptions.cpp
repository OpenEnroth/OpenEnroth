#include "LodToolOptions.h"

#include <memory>

#include "Library/Cli/CliApp.h"

LodToolOptions LodToolOptions::parse(int argc, char **argv) {
    LodToolOptions result;
    std::unique_ptr<CliApp> app = std::make_unique<CliApp>();

    app->set_help_flag("-h,--help", "Print help and exit.");
    app->require_subcommand();

    CLI::App *dump = app->add_subcommand("dump", "Dump a lod file.", result.subcommand, SUBCOMMAND_DUMP)->fallthrough();
    dump->add_option("LOD", result.lodPath, "Path to lod file.")->check(CLI::ExistingFile)->required()->option_text(" ");

    app->parse(argc, argv, result.helpPrinted);
    return result;
}
