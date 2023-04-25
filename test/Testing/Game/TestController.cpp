#include "TestController.h"

#include <utility>

#include "Library/Application/PlatformApplication.h"

#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Engine.h"

#include "Application/GameKeyboardController.h"

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
        std::move(postLoadCallback)
    );
}

void TestController::prepareForNextTest() {
    engine->config->resetForTest();
    startDeterministicSegment(engine->config->debug.TraceFrameTimeMs.value());
    ::application->get<GameKeyboardController>()->reset();
    _controller->goToMainMenu();
}

void TestController::startDeterministicSegment(int frameTimeMs) {
    ::application->get<EngineDeterministicComponent>()->startDeterministicSegment(frameTimeMs);
}
