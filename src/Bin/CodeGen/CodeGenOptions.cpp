#include "CodeGenOptions.h"

#include <memory>

#include "Library/Cli/CliApp.h"

CodeGenOptions CodeGenOptions::parse(int argc, char **argv) {
    CodeGenOptions result;
    result.useConfig = false; // CodeGen doesn't use external config.
    result.logLevel = LOG_CRITICAL; // CodeGen doesn't need logging.

    std::unique_ptr<CliApp> app = std::make_unique<CliApp>();

    app->add_option("--data-path", result.dataPath,
                    "Path to game data dir")->check(CLI::ExistingDirectory)->option_text("PATH");
    app->set_help_flag("-h,--help", "Print help and exit.");
    app->require_subcommand();

    app->add_subcommand("items", "Generate item ids enum.", result.subcommand, SUBCOMMAND_ITEM_ID)->fallthrough();
    app->add_subcommand("maps", "Generate map ids enum.", result.subcommand, SUBCOMMAND_MAP_ID)->fallthrough();
    app->add_subcommand("beacons", "Generate beacons mapping.", result.subcommand, SUBCOMMAND_BEACON_MAPPING)->fallthrough();
    app->add_subcommand("houses", "Generate house ids enum.", result.subcommand, SUBCOMMAND_HOUSE_ID)->fallthrough();
    app->add_subcommand("monsters", "Generate monster ids enum.", result.subcommand, SUBCOMMAND_MONSTER_ID)->fallthrough();
    app->add_subcommand("monster_types", "Generate monster types enum.", result.subcommand, SUBCOMMAND_MONSTER_TYPE)->fallthrough();
    app->add_subcommand("bounty_hunt", "Generate monster type / town hall table for bounty hunts.", result.subcommand, SUBCOMMAND_BOUNTY_HUNT)->fallthrough();
    app->add_subcommand("music", "Generate music ids enum.", result.subcommand, SUBCOMMAND_MUSIC)->fallthrough();
    app->add_subcommand("decorations", "Generate decorations ids enum.", result.subcommand, SUBCOMMAND_DECORATIONS)->fallthrough();

    app->parse(argc, argv, result.helpPrinted);
    return result;
}
