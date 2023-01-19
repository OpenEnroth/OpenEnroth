#include <gtest/gtest.h>

#include <thread>

#include "Application/Game.h"
#include "Application/GameFactory.h"

#include "Testing/Engine/TestState.h"
#include "Testing/Engine/TestStateHandle.h"
#include "Testing/Engine/TestPlatform.h"
#include "Testing/Game/GameWrapper.h"
#include "Testing/Game/GameTest.h"

#include "Utility/Random/Random.h"
#include "Utility/Random/NonRandomEngine.h"
#include "Utility/ScopeGuard.h"

#include "GameTestOptions.h"

void RunGameThread(const GameTestOptions& opts, TestState *unsafeState) {
    TestStateHandle state(GameSide, unsafeState);

    SetGlobalRandomEngine(std::make_unique<NonRandomEngine>());

    std::unique_ptr<PlatformLogger> logger = PlatformLogger::CreateStandardLogger(WinEnsureConsoleOption);
    logger->SetLogLevel(ApplicationLog, LogInfo);
    logger->SetLogLevel(PlatformLog, LogError);
    EngineIoc::ResolveLogger()->SetBaseLogger(logger.get());
    auto guard = ScopeGuard([] { EngineIoc::ResolveLogger()->SetBaseLogger(nullptr); });
    Engine::LogEngineBuildInfo();

    std::unique_ptr<Platform> platform = std::make_unique<TestPlatform>(Platform::CreateStandardPlatform(logger.get()), state);

    if (opts.gameDataDir.empty()) {
        Application::AutoInitDataPath(platform.get());
    } else {
        SetDataPath(opts.gameDataDir);
    }

    std::shared_ptr<Application::GameConfig> config = std::make_shared<Application::GameConfig>();
    config->LoadConfiguration(); // TODO(captainurist): Reads from openenroth.ini, not good for tests
    config->debug.NoVideo.Set(true);
    config->window.MouseGrab.Set(false);
    config->graphics.FPSLimit.Set(0); // Unlimited

    std::shared_ptr<Application::Game> game = Application::GameFactory().CreateGame(platform.get(), config);

    game->Run();
}

void RunTestThread(const GameTestOptions& opts, TestState *unsafeState, int *exitCode) {
    TestStateHandle state(TestSide, unsafeState);

    GameWrapper gameWrapper(state, opts.testDataDir);
    GameTest::Init(&gameWrapper);
    gameWrapper.Tick(10); // Let the game thread initialize everything.

    *exitCode = RUN_ALL_TESTS();

    state->terminating = true;
}

void PrintGoogleTestHelp(char *app) {
    int argc = 2;
    char help[] = "--help";
    char *argv[] = { app, help, nullptr };
    testing::InitGoogleTest(&argc, argv);
}

int PlatformMain(int argc, char **argv) {
    GameTestOptions opts;
    int exitCode = opts.Parse(argc, argv) ? 0 : 1;

    if (opts.helpRequested) {
        std::cout << std::endl;
        PrintGoogleTestHelp(argv[0]);
    } else {
        testing::InitGoogleTest(&argc, argv);
    }

    if (exitCode != 0)
        return exitCode;

    std::thread testThread;

    TestState state;
    state.terminationHandler = [&] {
        testThread.join();
        Engine_DeinitializeAndTerminate(exitCode);
    };

    testThread = std::thread(&RunTestThread, opts, &state, &exitCode);
    RunGameThread(opts, &state);

    assert(false); // should never get here.
    return exitCode;
}
