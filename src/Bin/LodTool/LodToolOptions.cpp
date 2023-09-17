#include "LodToolOptions.h"

#include <memory>

#include <CLI/CLI.hpp>

LodToolOptions LodToolOptions::parse(int argc, char **argv) {
    LodToolOptions result;
    std::unique_ptr<CLI::App> app = std::make_unique<CLI::App>();

    app->set_help_flag("-h,--help", "Print help and exit.");

    CLI::App *dump = app->add_subcommand("dump", "Dump a lod file.")->fallthrough();
    dump->add_option("LOD", result.lodPath, "Path to lod file.")->check(CLI::ExistingFile)->required()->option_text(" ");
    dump->callback([&] {
        result.subcommand = SUBCOMMAND_DUMP;
    });

    try {
        app->parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        if (app->get_help_ptr()->as<bool>() || dump->get_help_ptr()->as<bool>()) {
            app->exit(e);
            result.helpPrinted = true;
        } else {
            throw; // Genuine parse error => propagate.
        }
    }

    return result;
}
