#include "OpenEnrothOptions.h"

#include <memory>
#include <utility>
#include <ranges>
#include <vector>
#include <string>

#include "Application/Startup/PathResolver.h"

#include "Library/Cli/CliApp.h"
#include "Library/Environment/Interface/Environment.h"

#include "Utility/Exception.h"
#include "Utility/String/Format.h"

OpenEnrothOptions OpenEnrothOptions::parse(int argc, char **argv) {
    // Note that it's OK to create a temporary `Environment` here.
    std::unique_ptr<Environment> env = Environment::createStandardEnvironment();

    OpenEnrothOptions result;
    std::unique_ptr<CliApp> app = std::make_unique<CliApp>();

    std::optional<bool> portable;
    app->add_option(
        "--data-path", result.dataPath,
        fmt::format("Path to MM7 data folder, default is taken from '{}' environment variable. "
                    "If neither this argument is supplied nor the environment variable is set, "
                    "then OpenEnroth will try to look for game data in the current folder, "
                    "then on Windows it will also try to read the path from registry, "
                    "and on MacOS it will also try to look in '~/Library/Application Support/OpenEnroth'.", mm7PathOverrideKey))->check(CLI::ExistingDirectory)->option_text("PATH");
    app->add_option(
        "--user-path", result.userPath,
        fmt::format("Path to OpenEnroth user data folder. Default is '{}'.",
                    resolveMm7UserPath(env.get())))->check(CLI::ExistingDirectory)->option_text("PATH");
    app->add_flag(
        "--portable", portable,
        "Run in portable mode, game & user data paths will default to current folder. "
        "If '.portable' file exists in the current folder, then this parameter defaults to 'true'.");
    app->add_option(
        "--log-level", result.logLevel,
        "Log level, one of 'none', 'trace', 'debug', 'info', 'warning', 'error', 'critical'.")->option_text("LOG_LEVEL");
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

    if (!portable && std::filesystem::exists(".portable"))
        portable = true;
    if (portable && *portable) {
        if (result.userPath.empty())
            result.userPath = std::filesystem::current_path().generic_string();
        if (result.dataPath.empty())
            result.dataPath = std::filesystem::current_path().generic_string();
    }

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

        if (!result.logLevel)
            result.logLevel = LOG_ERROR; // Default log level for retracing is LOG_ERROR.
    }

    if (result.subcommand == SUBCOMMAND_PLAY)
        result.ramFsUserData = true; // No config & no user data if playing a trace.

    return result;
}
