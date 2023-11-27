#include <cstdio>
#include <utility>

#include "Application/GameStarter.h"

#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Trace/EngineTraceSimplePlayer.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/StackTrace/StackTraceOnCrash.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Trace/EventTrace.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Format.h"
#include "Utility/UnicodeCrt.h"
#include "Utility/String.h"

#include "OpenEnrothOptions.h"

static std::string readTextFile(const std::string &path) {
    // Normalize to UNIX line endings. Need this b/c git on Windows checks out CRLF line endings.
    return replaceAll(FileInputStream(path).readAll(), "\r\n", "\n");
}

int runRetrace(OpenEnrothOptions options) {
    GameStarter starter(options);
    EngineTraceStateAccessor::prepareForPlayback(starter.config(), pAudioPlayer.get()); // These settings are not changed by EngineTraceStateAccessor::patchConfig.

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
            EngineTraceStateAccessor::patchConfig(engine->config.get(), oldTrace.header.config);

            recorder->startRecording(game, savePath, tracePath, TRACE_RECORDING_LOAD_EXISTING_SAVE);
            engine->config->graphics.FPSLimit.setValue(0);
            player->playTrace(game, std::move(oldTrace.events), tracePath, TRACE_PLAYBACK_SKIP_RANDOM_CHECKS); // Don't skip time checks.
            recorder->finishRecording(game);

            if (options.retrace.checkCanonical) {
                std::string newTraceJson = readTextFile(tracePath);
                if (oldTraceJson != newTraceJson) {
                    fmt::println(stderr, "Trace '{}' is not in canonical representation.", tracePath);
                    status = 1;
                }
            }
        }
    });

    if (options.retrace.checkCanonical && status == 0)
        fmt::println(stderr, "All traces are in canonical representation.");

    return status;
}

int runOpenEnroth(OpenEnrothOptions options) {
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
        case OpenEnrothOptions::SUBCOMMAND_GAME: return runOpenEnroth(std::move(options));
        case OpenEnrothOptions::SUBCOMMAND_RETRACE: return runRetrace(std::move(options));
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
