#include <gtest/gtest.h>

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
    explicit GameThread(const GameTestOptions& options) {
        _logger = PlatformLogger::createStandardLogger(WIN_ENSURE_CONSOLE_OPTION);
        _logger->setLogLevel(APPLICATION_LOG, LOG_INFO);
        _logger->setLogLevel(PLATFORM_LOG, LOG_ERROR);
        EngineIocContainer::ResolveLogger()->setBaseLogger(_logger.get());
        Engine::LogEngineBuildInfo();

        _application = std::make_unique<PlatformApplication>(_logger.get());

        if (options.gameDataDir.empty()) {
            AutoInitDataPath(_application->platform());
        } else {
            setDataPath(options.gameDataDir);
        }

        _config = std::make_shared<GameConfig>();
        ResetTestConfig(_config.get());
        _game = GameFactory().CreateGame(_application.get(), _config);
    }

    ~GameThread() {
        EngineIocContainer::ResolveLogger()->setBaseLogger(nullptr);
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

int parseOptions(int argc, char **argv, GameTestOptions *opts) {
    int exitCode = opts->Parse(argc, argv) ? 0 : 1;

    if (opts->helpRequested) {
        std::cout << std::endl;
        printGoogleTestHelp(argv[0]);
    } else {
        testing::InitGoogleTest(&argc, argv);
    }

    return exitCode;
}

int platformMain(int argc, char **argv) {
    GameTestOptions opts;
    int exitCode = parseOptions(argc, argv, &opts);
    if (exitCode != 0)
        return exitCode;

    GameThread gameThread(opts);

    gameThread.app()->get<EngineControlComponent>()->runControlRoutine([&] (EngineController *game) {
        TestController test(game, opts.testDataDir);

        GameTest::init(game, &test);
        gameThread.app()->get<EngineDeterministicComponent>()->enterDeterministicMode(); // And never leave it.
        game->tick(10); // Let the game thread initialize everything.

        exitCode = RUN_ALL_TESTS();

        game->goToMainMenu();
        game->pressGuiButton("MainMenu_ExitGame");
    });

    gameThread.run();

    return exitCode;
}
