#include <gtest/gtest.h>

#include "Application/GameStarter.h"
#include "Application/GameConfig.h"

#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"

#include "Testing/Game/GameTest.h"
#include "Testing/Game/TestController.h"

#include "Library/Platform/Application/PlatformApplication.h"

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
        GameTestOptions opts = GameTestOptions::parse(argc, argv);
        if (opts.helpPrinted) {
            fmt::print(stdout, "\n");
            printGoogleTestHelp(argv[0]);
            return 1;
        }

        testing::InitGoogleTest(&argc, argv);
        if (opts.listRequested)
            return RUN_ALL_TESTS();

        GameStarter starter(opts);
        starter.config()->resetForTest();

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
        // TODO(captainurist): we need a separate test that testing framework terminates correctly if the engine throws.
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
