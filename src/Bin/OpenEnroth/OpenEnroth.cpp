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

#include "Library/Application/PlatformApplication.h"
#include "Library/Trace/EventTrace.h"

#include "Utility/Format.h"

#include "OpenEnrothOptions.h"


int runRetrace(OpenEnrothOptions options) {
    GameStarter starter(options);
    starter.config()->resetForTest();

    starter.application()->get<EngineControlComponent>()->runControlRoutine([options, application = starter.application()] (EngineController *game) {
        game->tick(10); // Let the game thread initialize everything.

        EngineTraceSimplePlayer *player = application->get<EngineTraceSimplePlayer>();
        EngineTraceRecorder *recorder = application->get<EngineTraceRecorder>();

        for (const std::string &tracePath : options.retrace.traces) {
            std::string savePath = tracePath.substr(0, tracePath.length() - 5) + ".mm7";

            EventTrace oldTrace = EventTrace::loadFromFile(tracePath, application->window());
            EngineTraceStateAccessor::patchConfig(engine->config.get(), oldTrace.header.config);

            recorder->startRecording(game, savePath, tracePath, TRACE_RECORDING_LOAD_EXISTING_SAVE);
            engine->config->graphics.FPSLimit.setValue(0);
            player->playTrace(game, std::move(oldTrace.events), tracePath, TRACE_PLAYBACK_SKIP_RANDOM_CHECKS); // Don't skip time checks.
            recorder->finishRecording(game);
        }

        game->goToMainMenu();
        game->pressGuiButton("MainMenu_ExitGame");
    });
    starter.run();

    return 0;
}

int runOpenEnroth(OpenEnrothOptions options) {
    setErrorHandler([](const std::string &title, const std::string &message) {
        if (platform)
            platform->showMessageBox(title, message);
    });

    GameStarter(options).run();
    return 0;
}

int openEnrothMain(int argc, char **argv) {
    try {
        OpenEnrothOptions options = OpenEnrothOptions::parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        switch (options.subcommand) {
        case OpenEnrothOptions::SUBCOMMAND_GAME: return runOpenEnroth(std::move(options));
        case OpenEnrothOptions::SUBCOMMAND_RETRACE: return runRetrace(std::move(options));
        default:
            assert(false);
            return 1;
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
