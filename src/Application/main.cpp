#include <cstdio>
#include <utility>

#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Trace/EngineTraceComponent.h"

#include "Library/Application/PlatformApplication.h"
#include "Library/Trace/EventTrace.h"

#include "Utility/Format.h"

#include "GameStarter.h"
#include "GameOptions.h"
#include "GameConfig.h"


int runRetrace(GameOptions options) {
    GameStarter starter(options);
    starter.config()->resetForTest();

    starter.application()->get<EngineControlComponent>()->runControlRoutine([application = starter.application(), tracePaths = options.retrace.traces] (EngineController *game) {
        game->tick(10); // Let the game thread initialize everything.

        for (const std::string &tracePath : tracePaths) {
            std::string savePath = tracePath.substr(0, tracePath.length() - 5) + ".mm7";

            game->goToMainMenu();

            EventTrace trace = EventTrace::loadFromFile(tracePath, application->window());
            application->get<EngineTracePlayer>()->prepareTrace(game, savePath, tracePath);
            application->get<EngineTraceComponent>()->start();
            application->get<EngineTracePlayer>()->playPreparedTrace(game, TRACE_PLAYBACK_SKIP_RANDOM_CHECKS);
            trace.events = application->get<EngineTraceComponent>()->finish();
            EventTrace::saveToFile(tracePath, trace);
        }

        game->goToMainMenu();
        game->pressGuiButton("MainMenu_ExitGame");
    });
    starter.run();

    return 0;
}

int runOpenEnroth(GameOptions options) {
    GameStarter(options).run();
    return 0;
}

int openEnrothMain(int argc, char **argv) {
    try {
        GameOptions options = GameOptions::Parse(argc, argv);
        if (options.helpPrinted)
            return 1;

        switch (options.subcommand) {
        case GameOptions::SUBCOMMAND_GAME: return runOpenEnroth(std::move(options));
        case GameOptions::SUBCOMMAND_RETRACE: return runRetrace(std::move(options));
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
