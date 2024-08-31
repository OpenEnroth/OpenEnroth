#include "OpenEnrothOptions.h"

#include <memory>
#include <utility>
#include <ranges>
#include <vector>
#include <string>

#include "Application/GamePathResolver.h"

#include "Library/Cli/CliApp.h"

#include "Utility/Exception.h"
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
    // TODO(captainurist): to print default value here we'll need to pass in Environment.
    app->add_option(
        "--user-path", result.userPath,
        "Path to OpenEnroth user data folder.")->check(CLI::ExistingDirectory)->option_text("PATH");
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

    std::string traceDir;
    CLI::App *retrace = app->add_subcommand("retrace", "Retrace traces and exit.", result.subcommand, SUBCOMMAND_RETRACE)->fallthrough();
    app->add_flag(
        "--headless", result.headless,
        "Run in headless mode.");
    retrace->add_flag(
        "--tracing-rng", result.tracingRng,
        "Use random number generators that print stack trace on each call.");
    retrace->add_flag(
        "--check-canonical", result.retrace.checkCanonical,
        "Check whether all passed traces are stored in canonical representation and return an error if not. Don't overwrite the actual trace files.");
    retrace->add_option(
        "--ls", traceDir,
        "Directory to look for traces to retrace."); // This is here so that we don't have to jump through hoops in cmake.
    retrace->add_option(
        "TRACE", result.retrace.traces,
        "Path to trace file(s) to retrace.")->option_text("...");
    retrace->set_help_flag("-h,--help", "Print help and exit."); // This places --help last in the command list.

    app->parse(argc, argv, result.helpPrinted);

    if (result.subcommand == SUBCOMMAND_RETRACE) {
        result.ramFsUserData = true; // No config & no user data if retracing.

        if (!traceDir.empty()) {
            for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(traceDir))
                if (entry.path().extension() == ".json")
                    result.retrace.traces.push_back(entry.path().generic_string());
            std::ranges::sort(result.retrace.traces); // NOLINT: This is ranges::sort. We want a fixed order.
        }

        if (result.retrace.traces.empty())
            throw Exception("No trace files to retrace.");
    }

    if (result.subcommand == SUBCOMMAND_PLAY)
        result.ramFsUserData = true; // No config & no user data if playing a trace.

    return result;
}
