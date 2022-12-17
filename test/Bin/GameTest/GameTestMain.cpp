#include <gtest/gtest.h>

#include <thread>

#include "Application/Game.h"
#include "Application/GameFactory.h"

#include "Testing/Engine/TestState.h"
#include "Testing/Engine/TestStateHandle.h"
#include "Testing/Engine/TestPlatform.h"
#include "Testing/Game/GameWrapper.h"
#include "Testing/Game/GameTest.h"

void RunGameThread(TestState *unsafeState) {
    TestStateHandle state(GameSide, unsafeState);

    Log *log = EngineIoc::ResolveLogger();
    std::unique_ptr<Platform> platform = std::make_unique<TestPlatform>(Platform::CreateStandardPlatform(log), state);
    SetGlobalPlatform(platform.get());

    std::shared_ptr<Application::GameConfig> config = std::make_shared<Application::GameConfig>("");
    config->Startup(); // TODO(captainurist): Reads from womm.ini, not good for tests
    config->debug.NoIntro.Set(true);
    config->debug.NoLogo.Set(true);
    config->window.MouseGrab.Set(false);

    std::shared_ptr<Application::Game> game = Application::GameFactory().CreateGame(platform.get(), config);

    game->Run();
}

void RunTestThread(TestState *unsafeState, int *exitCode) {
    TestStateHandle state(TestSide, unsafeState);

    GameWrapper gameWrapper(state);
    GameTest::Init(&gameWrapper);
    gameWrapper.Tick(10); // Let the game thread initialize everything.

    *exitCode = RUN_ALL_TESTS();

    state->terminating = true;
}

int PlatformMain(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    int exitCode = 0;
    std::thread testThread;

    TestState state;
    state.terminationHandler = [&] {
        testThread.join();
        Engine_DeinitializeAndTerminate(exitCode);
    };

    testThread = std::thread(&RunTestThread, &state, &exitCode);
    RunGameThread(&state);

    assert(false); // should never get here.
    return exitCode;
}
