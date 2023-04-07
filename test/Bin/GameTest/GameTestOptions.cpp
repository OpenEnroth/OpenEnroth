#include "GameTestOptions.h"

#include <memory>

#include <CLI/CLI.hpp>

#include "Application/GamePathResolver.h"


GameTestOptions GameTestOptions::Parse(int argc, char **argv) {
    GameTestOptions result;
    result.verbose = true; // Logging in tests is always verbose.
    result.configPath = "openenroth_test.ini"; // Default config is different from the one used by the game.
    result.resetConfig = true; // Tests start with a default config.

    std::unique_ptr<CLI::App> app = std::make_unique<CLI::App>();

    std::string requiredOptions = "Required Options";
    std::string otherOptions = "Other Options";

    app->add_option("--test-path", result.testPath,
                    "Path to test data dir")->check(CLI::ExistingDirectory)->option_text("PATH")->required()->group(requiredOptions);
    app->add_option("--data-path", result.dataPath,
                    "Path to game data dir")->check(CLI::ExistingDirectory)->option_text("PATH")->group(otherOptions);
    app->set_help_flag("-h,--help", "Print help and exit.")->group(otherOptions);
    app->allow_extras();

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
