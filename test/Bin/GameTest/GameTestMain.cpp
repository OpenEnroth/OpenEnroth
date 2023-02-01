#include <gtest/gtest.h>

#include "Application/Game.h"
#include "Application/GameFactory.h"

#include "Engine/Plugins/EngineTracer.h"
#include "Engine/Plugins/EngineControlPlugin.h"
#include "Engine/Plugins/EngineController.h"
#include "Engine/Plugins/EngineDeterministicPlugin.h"

#include "Testing/Game/GameTest.h"

#include "Library/Application/PlatformApplication.h"

#include "Utility/ScopeGuard.h"

#include "GameTestOptions.h"

class GameThread {
 public:
    explicit GameThread(const GameTestOptions& options) {
        _logger = PlatformLogger::CreateStandardLogger(WinEnsureConsoleOption);
        _logger->SetLogLevel(ApplicationLog, LogInfo);
        _logger->SetLogLevel(PlatformLog, LogError);
        EngineIoc::ResolveLogger()->SetBaseLogger(_logger.get());
        Engine::LogEngineBuildInfo();

        _application = std::make_unique<PlatformApplication>(_logger.get());

        if (options.gameDataDir.empty()) {
            Application::AutoInitDataPath(_application->platform());
        } else {
            SetDataPath(options.gameDataDir);
        }

        _config = std::make_shared<Application::GameConfig>();
        _config->LoadConfiguration(); // TODO(captainurist): Reads from openenroth.ini, not good for tests
        _config->debug.NoVideo.Set(true);
        _config->window.MouseGrab.Set(false);
        _config->graphics.FPSLimit.Set(0); // Unlimited

        _game = Application::GameFactory().CreateGame(_application.get(), _config);
    }

    ~GameThread() {
        EngineIoc::ResolveLogger()->SetBaseLogger(nullptr);
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
    std::shared_ptr<Application::GameConfig> _config;
    std::shared_ptr<Application::Game> _game;
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

int PlatformMain(int argc, char **argv) {
    GameTestOptions opts;
    int exitCode = parseOptions(argc, argv, &opts);
    if (exitCode != 0)
        return exitCode;

    GameThread gameThread(opts);

    gameThread.app()->get<EngineControlPlugin>()->runControlRoutine([&] (EngineController *game) {
        TestController test(game, opts.testDataDir);

        GameTest::init(game, &test);
        gameThread.app()->get<EngineDeterministicPlugin>()->enterDeterministicMode(); // And never leave it.
        game->tick(10); // Let the game thread initialize everything.

        exitCode = RUN_ALL_TESTS();

        game->goToMainMenu();
        game->pressGuiButton("MainMenu_ExitGame");
    });

    gameThread.run();

    return exitCode;
}
