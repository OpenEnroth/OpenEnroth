#include "GameOptions.h"

#include <memory>

#include <CLI/CLI.hpp>

#include "Utility/Format.h"

#include "GamePathResolver.h"

GameOptions GameOptions::Parse(int argc, char **argv) {
    GameOptions result;
    std::unique_ptr<CLI::App> app = std::make_unique<CLI::App>();

    app->add_option("--data-path", result.dataPath,
                    fmt::format("Path to MM7 data folder, default is taken from '{}' environment variable. "
                                "If neither this argument is supplied nor the environment variable is set, "
                                "then on Windows OpenEnroth will also try to read the path from registry. "
                                "If this also fails, then OpenEnroth will look for game data in the current folder.", mm7PathOverrideKey))->check(CLI::ExistingDirectory)->option_text("PATH");
    app->add_option("--config", result.configPath,
                    "Path to OpenEnroth config file, default is 'openenroth.ini' in data folder.")->option_text("PATH");
    app->add_flag("-v,--verbose", result.verbose,
                  "Enable verbose logging.");
    app->set_help_flag("-h,--help", "Print help and exit.");

    try {
        app->parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        if (app->get_help_ptr()->as<bool>()) {
            app->exit(e);
            result.helpPrinted = true;
        } else {
            throw; // Genuine parse error => propagate.
        }
    }

    return result;
}
