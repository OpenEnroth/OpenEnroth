#include "RetraceTestOptions.h"

#include <memory>
#include <optional>
#include <string>

#include "Library/Cli/CliApp.h"

RetraceTestOptions RetraceTestOptions::parse(int argc, char **argv) {
    RetraceTestOptions result;
    result.ramFsUserData = true; // We want reproducible tests, so shouldn't depend on external user data.
    result.quickStart = true;
    std::optional<std::string> testPath;

    std::unique_ptr<CliApp> app = std::make_unique<CliApp>();

    std::string requiredOptions = "Required Options";
    std::string otherOptions = "Other Options";

    auto testPathOption = app->add_option("--test-path", testPath,
                                          "Path to test data dir.")->check(CLI::ExistingDirectory)->option_text("PATH")->group(requiredOptions);
    app->add_option(
        "--data-path", result.dataPath,
        "Path to game data dir.")->check(CLI::ExistingDirectory)->option_text("PATH")->group(otherOptions);
    app->add_flag(
        "--headless", result.headless,
        "Run in headless mode.")->group(otherOptions);
    app->add_option(
        "--log-level", result.logLevel,
        "Log level, one of 'none', 'trace', 'debug', 'info', 'warning', 'error', 'critical'.")->option_text("LOG_LEVEL");
    app->add_flag_callback(
        "-v,--verbose", [&] { result.logLevel = LOG_TRACE; },
        "Set log level to 'trace'.");
    app->set_help_flag("-h,--help", "Print help and exit.")->group(otherOptions);
    app->add_flag(
        "--gtest_list_tests", result.listRequested,
        "List the names of all tests instead of running them.")->group(""); // group("") hides the option. It's shown in gtest's help.
    app->allow_extras();

    app->parse(argc, argv, result.helpPrinted);

    if (!result.listRequested && !result.helpPrinted && !testPath)
        throw CLI::RequiredError(testPathOption->get_name());
    result.testPath = Path(testPath.value_or(""));

    return result;
}
