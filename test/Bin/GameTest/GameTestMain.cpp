#include <gtest/gtest.h>

#include <filesystem>

#include "Application/Game.h"
#include "Application/GameFactory.h"

#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"

#include "Testing/Game/GameTest.h"
#include "Testing/Game/TestConfig.h"
#include "Testing/Game/TestController.h"

#include "Library/Application/PlatformApplication.h"

#include "GameTestOptions.h"

class GameThread {
 public:
    explicit GameThread(GameTestOptions &options) {
        _logger = PlatformLogger::createStandardLogger(WIN_ENSURE_CONSOLE_OPTION);
        _logger->setLogLevel(APPLICATION_LOG, LOG_VERBOSE);
        _logger->setLogLevel(PLATFORM_LOG, LOG_VERBOSE);
        EngineIocContainer::ResolveLogger()->setBaseLogger(_logger.get());
        Engine::LogEngineBuildInfo();

        _application = std::make_unique<PlatformApplication>(_logger.get());
        options.ResolveDefaults(_application->platform());

        initDataPath(options.dataPath);

        _config = std::make_shared<GameConfig>("openenroth_test.ini");
        ResetTestConfig(_config.get());
        _game = GameFactory().CreateGame(_application.get(), _config);
    }

    ~GameThread() {
        EngineIocContainer::ResolveLogger()->setBaseLogger(nullptr);
        std::filesystem::remove(MakeDataPath("openenroth_test.ini"));
    }

    PlatformApplication *app() const {
        return _application.get();
    }

    void run() {
        _game->Run();
    }

 private:
    std::unique_ptr<PlatformLogger> _logger;
    std::unique_ptr<PlatformApplication> _application;
    std::shared_ptr<GameConfig> _config;
    std::shared_ptr<Game> _game;
};

void printGoogleTestHelp(char *app) {
    int argc = 2;
    char help[] = "--help";
    char *argv[] = { app, help, nullptr };
    testing::InitGoogleTest(&argc, argv);
}

int platformMain(int argc, char **argv) {
    try {
        GameTestOptions opts = GameTestOptions::Parse(argc, argv);
        if (opts.helpPrinted) {
            fmt::print(stdout, "\n");
            printGoogleTestHelp(argv[0]);
            return 1;
        }

        testing::InitGoogleTest(&argc, argv);

        GameThread gameThread(opts);

        int exitCode = 0;
        gameThread.app()->get<EngineControlComponent>()->runControlRoutine([&] (EngineController *game) {
            TestController test(game, opts.testPath);

            GameTest::init(game, &test);
            gameThread.app()->get<EngineDeterministicComponent>()->enterDeterministicMode(); // And never leave it.
            game->tick(10); // Let the game thread initialize everything.

            exitCode = RUN_ALL_TESTS();

            game->goToMainMenu();
            game->pressGuiButton("MainMenu_ExitGame");
        });
        gameThread.run();

        return exitCode;
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
