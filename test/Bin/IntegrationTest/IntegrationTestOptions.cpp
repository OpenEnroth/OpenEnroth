#include "IntegrationTestOptions.h"

#include <memory>
#include <optional>
#include <string>

#include "Library/Cli/CliApp.h"

IntegrationTestOptions IntegrationTestOptions::parse(int argc, char **argv) {
    IntegrationTestOptions result;
    std::optional<std::string> binaryPath;

    std::unique_ptr<CliApp> app = std::make_unique<CliApp>();

    std::string requiredOptions = "Required Options";
    std::string otherOptions = "Other Options";

    auto binaryPathOption = app->add_option("--binary-path", binaryPath,
                                            "Path to the OpenEnroth binary to test.")->check(CLI::ExistingFile)->option_text("PATH")->group(requiredOptions);
    app->set_help_flag("-h,--help", "Print help and exit.")->group(otherOptions);
    app->add_flag(
        "--gtest_list_tests", result.listRequested,
        "List the names of all tests instead of running them.")->group(""); // group("") hides the option. It's shown in gtest's help.
    app->allow_extras();

    app->parse(argc, argv, result.helpPrinted);

    if (!result.listRequested && !result.helpPrinted && !binaryPath)
        throw CLI::RequiredError(binaryPathOption->get_name());
    result.binaryPath = NativePath::fromWtf8(binaryPath.value_or(""));

    return result;
}
