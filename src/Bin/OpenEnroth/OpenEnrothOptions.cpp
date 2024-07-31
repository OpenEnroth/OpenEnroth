#include "OpenEnrothOptions.h"

#include <memory>
#include <utility>
#include <ranges>
#include <vector>
#include <string>

#include <glob/glob.hpp> // NOLINT: not a C system header.

#include "Application/GamePathResolver.h"

#include "Library/Cli/CliApp.h"

#include "Utility/String/Format.h"

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

    CLI::App *play = app->add_subcommand("play", "Play provided traces.", result.subcommand, SUBCOMMAND_PLAY)->fallthrough();
    play->add_option(
        "--speed", result.play.speed,
        "Playback speed, default is '1.0'.")->option_text("SPEED");
    play->add_option(
        "TRACE", result.play.traces,
        "Path to trace file(s) to play.")->required()->option_text("...");
    play->set_help_flag("-h,--help", "Print help and exit."); // This places --help last in the command list.

    bool globTraces = false;
    CLI::App *retrace = app->add_subcommand("retrace", "Retrace traces and exit.", result.subcommand, SUBCOMMAND_RETRACE)->fallthrough();
    app->add_flag(
        "--headless", result.headless,
        "Run in headless mode.");
    retrace->add_flag(
        "--tracing-rng", result.tracingRng,
        "Use random number generators that print stack trace on each call.");
    retrace->add_flag(
        "--check-canonical", result.retrace.checkCanonical,
        "Check whether all passed traces are stored in canonical representation and return an error if not.");
    retrace->add_flag(
        "--glob", globTraces,
        "Glob passed trace paths.")->group(""); // group("") hides this option. It's here so that we don't have to jump through hoops in cmake.
    retrace->add_option(
        "TRACE", result.retrace.traces,
        "Path to trace file(s) to retrace.")->required()->option_text("...");
    retrace->set_help_flag("-h,--help", "Print help and exit."); // This places --help last in the command list.

    app->parse(argc, argv, result.helpPrinted);

    if (result.subcommand == SUBCOMMAND_RETRACE) {
        result.useConfig = false; // Don't use external config if retracing.

        if (globTraces) {
            std::vector<std::string> patterns = std::move(result.retrace.traces);
            result.retrace.traces.clear();
            for (const std::string &pattern : patterns)
                for (const std::filesystem::path &path : glob::glob(pattern))
                    result.retrace.traces.push_back(path.string());
            std::ranges::sort(result.retrace.traces); // NOLINT: This is ranges::sort. We want a fixed order.
        }
    }

    if (result.subcommand == SUBCOMMAND_PLAY)
        result.useConfig = false; // Don't use external config if playing a trace.

    return result;
}
