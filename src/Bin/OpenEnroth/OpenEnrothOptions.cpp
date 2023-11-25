#include "OpenEnrothOptions.h"

#include <memory>

#include "Application/GamePathResolver.h"

#include "Library/Cli/CliApp.h"

#include "Utility/Format.h"

OpenEnrothOptions OpenEnrothOptions::parse(int argc, char **argv) {
    OpenEnrothOptions result;
    std::unique_ptr<CliApp> app = std::make_unique<CliApp>();

    app->add_option(
        "--data-path", result.dataPath,
        fmt::format("Path to MM7 data folder, default is taken from '{}' environment variable. "
                    "If neither this argument is supplied nor the environment variable is set, "
                    "then on Windows OpenEnroth will also try to read the path from registry. "
                    "If this also fails, then OpenEnroth will look for game data in the current folder.", mm7PathOverrideKey))->check(CLI::ExistingDirectory)->option_text("PATH");
    app->add_option(
        "--config", result.configPath,
        "Path to OpenEnroth config file, default is 'openenroth.ini' in data folder.")->option_text("PATH");
    app->add_option(
        "--log-level", result.logLevel,
        "Log level, one of 'trace', 'debug', 'info', 'warning', 'error', 'critical'.")->option_text("LOG_LEVEL");
    app->add_flag_callback(
        "-v,--verbose", [&] { result.logLevel = LOG_TRACE; },
        "Set log level to 'trace'.");
    app->set_help_flag("-h,--help", "Print help and exit.");

    CLI::App *retrace = app->add_subcommand("retrace", "Retrace traces and exit.", result.subcommand, SUBCOMMAND_RETRACE)->fallthrough();
    retrace->add_flag(
        "--tracing-rng", result.tracingRng,
        "Use random number generators that print stack trace on each call.");
    retrace->add_option(
        "TRACE", result.retrace.traces,
        "Path to trace file(s) to retrace.")->check(CLI::ExistingFile)->required()->option_text("...");
    retrace->set_help_flag("-h,--help", "Print help and exit."); // This places --help last in the command list.

    app->parse(argc, argv, result.helpPrinted);

    if (result.subcommand == SUBCOMMAND_RETRACE)
        result.useConfig = false; // Don't use external config if retracing.

    return result;
}
