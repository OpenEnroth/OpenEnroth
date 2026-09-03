#include <cstdio>
#include <cassert>
#include <utility>
#include <ranges>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <unordered_set>

#include "Application/Startup/GameStarter.h"

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Trace/EngineTraceSimplePlayer.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Engine.h"

#include "Io/KeyboardActionMapping.h"
#include "Io/InputEnumFunctions.h"

#include "Core/Trace/EventTrace.h"
#include "Core/Trace/EventTraceMigrations.h"

#include "Library/StackTrace/StackTraceOnCrash.h"
#include "Library/Platform/Application/PlatformApplication.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Format.h"
#include "Utility/UnicodeCrt.h"
#include "Utility/String/Transformations.h"

#include "OpenEnrothOptions.h"

void migrateTrace(OpenEnrothOptions::Migration migration, EventTrace *trace) {
    std::unordered_set<PlatformKey> continuousKeys, onceKeys;
    for (InputAction inputAction : allInputActions())
        (triggerModeForInputAction(inputAction) == TRIGGER_ONCE ? onceKeys : continuousKeys).insert(keyboardActionMapping->keyFor(inputAction));
    erase_if(onceKeys, [&] (PlatformKey key) { return continuousKeys.contains(key); });

    switch (migration) {
    default: assert(false); [[fallthrough]];
    case OpenEnrothOptions::MIGRATION_NONE:
        return;
    case OpenEnrothOptions::MIGRATION_DROP_REDUNDANT_KEY_EVENTS:
        return trace::migrateDropRedundantKeyEvents(trace);
    case OpenEnrothOptions::MIGRATION_DROP_PRESS_RELEASE_FOR_CONTINUOUS_ACTIONS:
        return trace::migrateDropKeyPressReleaseEvents(continuousKeys, trace);
    case OpenEnrothOptions::MIGRATION_DROP_PAINT_AFTER_ACTIVATE:
        return trace::migrateDropPaintAfterActivate(trace);
    case OpenEnrothOptions::MIGRATION_TIGHTEN_KEY_EVENTS_FOR_ONCE_ACTIONS:
        return trace::migrateTightenKeyEvents(onceKeys, trace);
    }
}

int runRetrace(const OpenEnrothOptions &options) {
    GameStarter starter(options);

    starter.runInstrumented([options, application = starter.application()] (EngineController *game) {
        EngineTraceSimplePlayer *player = application->component<EngineTraceSimplePlayer>();
        EngineTraceRecorder *recorder = application->component<EngineTraceRecorder>();

        for (const Path &tracePath : options.retrace.traces) {
            fmt::println(stderr, "Retracing '{}'...", tracePath);
            auto startTime = std::chrono::steady_clock::now();

            Path savePath = tracePath.withExtension(".mm7");
            Blob oldTraceBlob = Blob::fromFile(tracePath);
            Blob oldSaveBlob = Blob::fromFile(savePath);

            EventTrace oldTrace = EventTrace::fromJsonBlob(oldTraceBlob, application->window());
            migrateTrace(options.retrace.migration, &oldTrace);

            EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), oldTrace.header.config);
            recorder->startRecording(game, oldSaveBlob);
            engine->config->graphics.FPSLimit.setValue(0);
            player->playTrace(game, std::move(oldTrace.events), tracePath.string(), TRACE_PLAYBACK_SKIP_RANDOM_CHECKS | TRACE_PLAYBACK_SKIP_STATE_CHECKS);
            EngineTraceRecording recording = recorder->finishRecording(game);

            auto endTime = std::chrono::steady_clock::now();
            fmt::println(stderr, "Retraced in {}ms.", std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());

            std::string oldTraceJson = EventTrace::normalizeJson(oldTraceBlob.str());
            std::string newTraceJson = EventTrace::normalizeJson(recording.trace.str());
            if (oldTraceJson != newTraceJson) {
                oldTraceBlob = Blob(); // Close old trace file
                FileOutputStream(tracePath).write(recording.trace);
            }
        }
    });

    return 0;
}

int runPlay(const OpenEnrothOptions &options) {
    GameStarter starter(options);

    starter.runInstrumented([options, application = starter.application()] (EngineController *game) {
        EngineTracePlayer *player = application->component<EngineTracePlayer>();

        for (const Path &tracePath : options.play.traces) {
            fmt::println(stderr, "Playing back '{}'...", tracePath);

            Path savePath = tracePath.withExtension(".mm7");

            EngineTraceRecording recording;
            recording.save = Blob::fromFile(savePath);
            recording.trace = Blob::fromFile(tracePath);

            player->playTrace(game, recording, TRACE_PLAYBACK_SKIP_RANDOM_CHECKS | TRACE_PLAYBACK_SKIP_STATE_CHECKS , [&] {
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

#ifdef _WINDOWS
static void waitForAnyKey() {
    printf("[Press any key to close this window]");
    getchar();
}
#endif

int openEnrothMain(int argc, char **argv) {
    try {
#ifdef _WINDOWS
        StackTraceOnCrash st(&waitForAnyKey);
#else
        StackTraceOnCrash st;
#endif
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
    if (result != 0)
        waitForAnyKey();
#elif __ANDROID__
    // TODO: on android without this it won't close application properly until it finishes music track?!
    // Something is not closing and preventing proper teardown?
    exit(0);
#endif

    return result;
}
