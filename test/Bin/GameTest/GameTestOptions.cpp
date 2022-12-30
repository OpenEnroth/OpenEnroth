#include "GameTestOptions.h"

#include <memory>

#include <CLI/CLI.hpp>

bool GameTestOptions::Parse(int argc, char **argv) {
    std::unique_ptr<CLI::App> app = std::make_unique<CLI::App>();

    std::string generalOptions = "General Options";
    std::string gameOptions = "Game Options";
    std::string windowOptions = "Window Options";

    app->set_help_flag("-h,--help", "Print help and exit");
    app->add_option("-d,--test-data", testDataDir, "Path to test data dir")->check(CLI::ExistingDirectory)->required();
    app->add_option("-g,--game-data", gameDataDir, "Path to game data dir")->check(CLI::ExistingDirectory);

    try {
        app->parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        app->exit(e);
        return false;
    }

    return true;
}
