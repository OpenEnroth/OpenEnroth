#include <cstdio>
#include <cassert>
#include <utility>
#include <ranges>
#include <vector>
#include <string>
#include <algorithm>

#include "Application/GameStarter.h"

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Trace/EngineTraceSimplePlayer.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Engine.h"

#include "Library/StackTrace/StackTraceOnCrash.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Trace/EventTrace.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/String/Format.h"
#include "Utility/UnicodeCrt.h"
#include "Utility/String/Transformations.h"
#include "Utility/String/Split.h"
#include "Utility/Types.h"

#include "OpenEnrothOptions.h"

static std::string readTextFile(std::string_view path) {
    // Normalize to UNIX line endings. Need this b/c git on Windows checks out CRLF line endings.
    std::string result = replaceAll(FileInputStream(path).readAll(), "\r\n", "\n");

    // Also drop trailing newlines. Vim always adds a newline, but retracing removes it.
    while (result.ends_with('\n'))
        result.pop_back();

    return result;
}

static void printLines(const std::vector<std::string_view> &lines, ssize_t line, ssize_t delta) {
    for (size_t i = std::max(static_cast<ssize_t>(0), line - delta); i < std::min(std::ssize(lines), line + delta + 1); i++)
        fmt::println(stderr, "{:>5}: {}", i + 1, lines[i]);
}

static void printTraceDiff(std::string_view current, std::string_view canonical) {
    assert(canonical != current);

    size_t pos = *std::ranges::find_if(std::views::iota(0), [&] (size_t i) { return canonical[i] != current[i]; });
    size_t line = std::ranges::count(std::string_view(canonical.data(), pos), '\n'); // 0-indexed.

    std::vector<std::string_view> canonicalLines = split(canonical, '\n');
    std::vector<std::string_view> currentLines = split(current, '\n');

    fmt::println(stderr, "Canonical:");
    printLines(canonicalLines, line, 2);
    fmt::println(stderr, "Current:");
    printLines(currentLines, line, 2);
}

int runRetrace(const OpenEnrothOptions &options) {
    GameStarter starter(options);

    int status = 0;

    starter.runInstrumented([&status, options, application = starter.application()] (EngineController *game) {
        EngineTraceSimplePlayer *player = application->component<EngineTraceSimplePlayer>();
        EngineTraceRecorder *recorder = application->component<EngineTraceRecorder>();

        for (const std::string &tracePath : options.retrace.traces) {
            fmt::println(stderr, "Retracing '{}'...", tracePath);

            std::string oldTraceJson;
            if (options.retrace.checkCanonical)
                oldTraceJson = readTextFile(tracePath);

            std::string savePath = tracePath.substr(0, tracePath.length() - 5) + ".mm7";

            EventTrace oldTrace = EventTrace::loadFromFile(tracePath, application->window());
            EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), oldTrace.header.config);

            recorder->startRecording(game, savePath, tracePath, TRACE_RECORDING_LOAD_EXISTING_SAVE);
            engine->config->graphics.FPSLimit.setValue(0);
            player->playTrace(game, std::move(oldTrace.events), tracePath, TRACE_PLAYBACK_SKIP_RANDOM_CHECKS | TRACE_PLAYBACK_SKIP_STATE_CHECKS);
            recorder->finishRecording(game);

            if (options.retrace.checkCanonical) {
                std::string newTraceJson = readTextFile(tracePath);
                if (oldTraceJson != newTraceJson) {
                    fmt::println(stderr, "Trace '{}' is not in canonical representation.", tracePath);
                    printTraceDiff(oldTraceJson, newTraceJson);
                    status = 1;
                }
            }
        }
    });

    if (options.retrace.checkCanonical && status == 0)
        fmt::println(stderr, "All traces are in canonical representation.");

    return status;
}

int runPlay(const OpenEnrothOptions &options) {
    GameStarter starter(options);

    starter.runInstrumented([options, application = starter.application()] (EngineController *game) {
        EngineTracePlayer *player = application->component<EngineTracePlayer>();

        for (const std::string &tracePath : options.play.traces) {
            fmt::println(stderr, "Playing back '{}'...", tracePath);

            std::string savePath = tracePath.substr(0, tracePath.length() - 5) + ".mm7";
            player->playTrace(game, savePath, tracePath, TRACE_PLAYBACK_SKIP_RANDOM_CHECKS | TRACE_PLAYBACK_SKIP_STATE_CHECKS , [&] {
                int fps = options.play.speed * 1000 / engine->config->debug.TraceFrameTimeMs.value();
                engine->config->graphics.FPSLimit.setValue(std::max(1, fps));
            });
        }
    });

    return 0;
}

int runOpenEnroth(const OpenEnrothOptions &options) {
    GameStarter(options).run();
    return 0;
}

int openEnrothMain(int argc, char **argv) {
    try {
        StackTraceOnCrash st;
        UnicodeCrt _(argc, argv);
        OpenEnrothOptions options = OpenEnrothOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        switch (options.subcommand) {
        default: assert(false); [[fallthrough]];
        case OpenEnrothOptions::SUBCOMMAND_GAME: return runOpenEnroth(options);
        case OpenEnrothOptions::SUBCOMMAND_PLAY: return runPlay(options);
        case OpenEnrothOptions::SUBCOMMAND_RETRACE: return runRetrace(options);
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}

int platformMain(int argc, char **argv) {
    int result = openEnrothMain(argc, argv);

#ifdef _WINDOWS
    // SDL on Windows creates a separate console window, and we want to be able to actually read the error message
    // before that window closes.
    if (result != 0) {
        printf("[Press any key to close this window]");
        getchar();
    }
#elif __ANDROID__
    // TODO: on android without this it won't close application properly until it finishes music track?!
    // Something is not closing and preventing proper teardown?
    exit(0);
#endif

    return result;
}
