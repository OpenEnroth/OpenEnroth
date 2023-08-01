#include "GameTestOptions.h"

#include <memory>

#include <CLI/CLI.hpp>

#include "Application/GamePathResolver.h"


GameTestOptions GameTestOptions::parse(int argc, char **argv) {
    GameTestOptions result;
    result.useConfig = false; // Tests don't need an external config.
    std::optional<std::string> testPath;

    std::unique_ptr<CLI::App> app = std::make_unique<CLI::App>();

    std::string requiredOptions = "Required Options";
    std::string otherOptions = "Other Options";

    auto testPathOption = app->add_option("--test-path", testPath,
                                          "Path to test data dir.")->check(CLI::ExistingDirectory)->option_text("PATH")->group(requiredOptions);
    app->add_option("--data-path", result.dataPath,
                    "Path to game data dir.")->check(CLI::ExistingDirectory)->option_text("PATH")->group(otherOptions);
    app->add_flag("--headless", result.headless,
                  "Run in headless mode.")->group(otherOptions);
    app->add_flag("--gtest_list_tests", result.listRequested,
                  "List the names of all tests instead of running them.")->group(""); // group("") hides the option.
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

    if (!result.listRequested && !result.helpPrinted && !testPath)
        throw CLI::RequiredError(testPathOption->get_name());

    result.testPath = testPath.value_or("");
    return result;
}
