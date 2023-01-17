#include "GameTestOptions.h"

#include <memory>

#include <CLI/CLI.hpp>

bool GameTestOptions::Parse(int argc, char **argv) {
    std::unique_ptr<CLI::App> app = std::make_unique<CLI::App>();

    std::string requiredOptions = "Required Options";
    std::string otherOptions = "Other Options";

    app->add_option("--test-data", testDataDir, "Path to test data dir")->check(CLI::ExistingDirectory)->option_text("PATH")->required()->group(requiredOptions);
    app->add_option("--game-data", gameDataDir, "Path to game data dir")->check(CLI::ExistingDirectory)->option_text("PATH")->group(otherOptions);
    app->set_help_flag("-h,--help", "Print help and exit")->group(otherOptions);
    app->allow_extras();

    try {
        app->parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        app->exit(e);
        helpRequested = app->get_help_ptr()->as<bool>();
        return false;
    }

    return true;
}
