#include "CodeGenOptions.h"

#include <memory>

#include <CLI/CLI.hpp>

#include "Utility/Format.h"

CodeGenOptions CodeGenOptions::parse(int argc, char **argv) {
    CodeGenOptions result;
    result.useConfig = false; // CodeGen doesn't use external config.
    result.logLevel = LOG_CRITICAL; // CodeGen doesn't need logging.

    std::unique_ptr<CLI::App> app = std::make_unique<CLI::App>();

    app->add_option("--data-path", result.dataPath,
                    "Path to game data dir")->check(CLI::ExistingDirectory)->option_text("PATH");
    app->set_help_flag("-h,--help", "Print help and exit.");
    app->require_subcommand();

    CLI::App *items = app->add_subcommand("items", "Generate item ids enum.")->fallthrough();
    items->callback([&] { result.subcommand = SUBCOMMAND_ITEM_ID; });

    CLI::App *maps = app->add_subcommand("maps", "Generate map ids enum.")->fallthrough();
    maps->callback([&] { result.subcommand = SUBCOMMAND_MAP_ID; });

    CLI::App *beacons = app->add_subcommand("beacons", "Generate beacons mapping.")->fallthrough();
    beacons->callback([&] { result.subcommand = SUBCOMMAND_BEACON_MAPPING; });

    CLI::App *houses = app->add_subcommand("houses", "Generate house ids enum.")->fallthrough();
    houses->callback([&] { result.subcommand = SUBCOMMAND_HOUSE_ID; });

    try {
        app->parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        if (app->get_help_ptr()->as<bool>() || items->get_help_ptr()->as<bool>() || maps->get_help_ptr()->as<bool>() || beacons->get_help_ptr()->as<bool>()) {
            app->exit(e);
            result.helpPrinted = true;
        } else {
            throw; // Genuine parse error => propagate.
        }
    }

    return result;
}
