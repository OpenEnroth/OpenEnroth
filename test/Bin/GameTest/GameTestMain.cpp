#include <gtest/gtest.h>

#include "Application/GameStarter.h"
#include "Application/GameConfig.h"

#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"

#include "Testing/Game/GameTest.h"
#include "Testing/Game/TestController.h"

#include "Library/Application/PlatformApplication.h"

#include "Utility/Format.h"

#include "GameTestOptions.h"

void printGoogleTestHelp(char *app) {
    int argc = 2;
    char help[] = "--help";
    char *argv[] = { app, help, nullptr };
    testing::InitGoogleTest(&argc, argv);
}

int platformMain(int argc, char **argv) {
    try {
        // TODO(captainurist): just listing tests doesn't work now because --test-path is required.
        GameTestOptions opts = GameTestOptions::parse(argc, argv);
        if (opts.helpRequested) {
            fmt::print(stdout, "\n");
            printGoogleTestHelp(argv[0]);
            return 1;
        }

        testing::InitGoogleTest(&argc, argv);

        GameStarter starter(opts);
        starter.config()->resetForTest();
        if (opts.renderer)
            starter.config()->graphics.Renderer.setValue(*opts.renderer);

        int exitCode = 0;
        starter.application()->get<EngineControlComponent>()->runControlRoutine([&] (EngineController *game) {
            TestController test(game, opts.testPath);

            GameTest::init(game, &test);
            game->tick(10); // Let the game thread initialize everything.

            exitCode = RUN_ALL_TESTS();

            game->goToMainMenu();
            game->pressGuiButton("MainMenu_ExitGame");
        });
        starter.run();

        return exitCode;
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
