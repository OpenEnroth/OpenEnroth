#include <gtest/gtest.h>

#include <thread>

#include "Application/Game.h"
#include "Application/GameFactory.h"

#include "Testing/Engine/TestState.h"
#include "Testing/Engine/TestStateHandle.h"
#include "Testing/Engine/TestProxy.h"
#include "Testing/Game/GameWrapper.h"
#include "Testing/Game/GameTest.h"

#include "Library/Application/PlatformApplication.h"

#include "Utility/Random/Random.h"
#include "Utility/Random/NonRandomEngine.h"
#include "Utility/ScopeGuard.h"

#include "GameTestOptions.h"

class GameThread {
 public:
    explicit GameThread(const GameTestOptions& options) {
        SetGlobalRandomEngine(std::make_unique<NonRandomEngine>());

        _logger = PlatformLogger::CreateStandardLogger(WinEnsureConsoleOption);
        _logger->SetLogLevel(ApplicationLog, LogInfo);
        _logger->SetLogLevel(PlatformLog, LogError);
        EngineIoc::ResolveLogger()->SetBaseLogger(_logger.get());
        auto guard = ScopeGuard([] { EngineIoc::ResolveLogger()->SetBaseLogger(nullptr); });
        Engine::LogEngineBuildInfo();

        _application = std::make_unique<PlatformApplication>(_logger.get());

        _unsafeState = std::make_unique<TestState>(_application.get());

        _proxy = std::make_unique<TestProxy>(TestStateHandle(GameSide, _unsafeState.get())); // State is locked here
        _application->installProxy(_proxy.get());

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

    void run() {
        _game->Run();
    }

    TestState *unsafeState() const {
        return _unsafeState.get();
    }

 private:
    std::unique_ptr<PlatformLogger> _logger;
    std::unique_ptr<PlatformApplication> _application;
    std::unique_ptr<TestState> _unsafeState;
    std::unique_ptr<TestProxy> _proxy;
    std::shared_ptr<Application::GameConfig> _config;
    std::shared_ptr<Application::Game> _game;
};

class TestThread {
 public:
    TestThread(const GameTestOptions& options, TestState *unsafeState, int *exitCode) : _options(options), _unsafeState(unsafeState), _exitCode(exitCode) {}

    void run() {
        TestStateHandle state(TestSide, _unsafeState);

        GameWrapper gameWrapper(state, _options.testDataDir);
        GameTest::Init(&gameWrapper);
        gameWrapper.Tick(10); // Let the game thread initialize everything.

        *_exitCode = RUN_ALL_TESTS();

        state->terminating = true;
    }

 private:
    GameTestOptions _options;
    TestState *_unsafeState = nullptr;
    int *_exitCode;
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
    TestThread testThread(opts, gameThread.unsafeState(), &exitCode);

    std::thread thread;
    gameThread.unsafeState()->terminationHandler = [&] {
        thread.join();
        Engine_DeinitializeAndTerminate(exitCode);
    };

    thread = std::thread(&TestThread::run, &testThread);
    gameThread.run();

    assert(false); // should never get here.
    return exitCode;
}
