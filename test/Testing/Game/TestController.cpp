#include "TestController.h"
#include <utility>
#include "Library/Application/PlatformApplication.h"
#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Engine.h"
#include "Application/GameKeyboardController.h"
#include "Application/GameConfig.h"

TestController::TestController(EngineController *controller, const std::string &testDataPath):
    _controller(controller),
    _testDataPath(testDataPath) {}

std::string TestController::fullPathInTestData(const std::string &fileName) {
    return (_testDataPath / fileName).string();
}

void TestController::loadGameFromTestData(const std::string &name) {
    _controller->loadGame(fullPathInTestData(name));
}

void TestController::playTraceFromTestData(const std::string &saveName, const std::string &traceName, std::function<void()> postLoadCallback) {
    playTraceFromTestData(saveName, traceName, 0, std::move(postLoadCallback));
}

void TestController::playTraceFromTestData(const std::string &saveName, const std::string &traceName,
                                           EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback) {
    // TODO(captainurist): we need to overhaul our usage of path::string, path::u8string, path::generic_string,
    // pick one, and spell it out explicitly in HACKING
    ::application->get<EngineTracePlayer>()->playTrace(
        _controller,
        fullPathInTestData(saveName),
        fullPathInTestData(traceName),
        flags,
        std::move(postLoadCallback),
        [this] {
            runTapeCallbacks();
        }
    );
}

void TestController::prepareForNextTest() {
    engine->config->resetForTest();
    _tapeCallbacks.clear();

    // This is frame time for tests that are implemented by manually sending events from the test code.
    // For such tests, frame time value is taken from config defaults.
    restart(engine->config->debug.TraceFrameTimeMs.value());
    ::application->get<GameKeyboardController>()->reset();
    _controller->goToMainMenu();
}

void TestController::restart(int frameTimeMs) {
    ::application->get<EngineDeterministicComponent>()->restart(frameTimeMs);
}

void TestController::runTapeCallbacks() {
    for (const auto &callback : _tapeCallbacks)
        callback();
}
