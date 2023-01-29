#include "TestController.h"

#include <utility>

#include "Library/Application/PlatformApplication.h"

#include "Engine/Plugins/EngineTracer.h"
#include "Engine/Plugins/EngineController.h"
#include "Engine/Plugins/EngineDeterministicPlugin.h"
#include "Engine/EngineGlobals.h"

#include "Application/GameKeyboardController.h"

TestController::TestController(EngineController *controller, const std::string &testDataPath):
    _controller(controller),
    _testDataPath(testDataPath) {}

void TestController::loadGameFromTestData(const std::string &name) {
    _controller->loadGame((_testDataPath / name).string());
}

void TestController::playTraceFromTestData(const std::string &saveName, const std::string &traceName, std::function<void()> postLoadCallback) {
    ::application->get<EngineTracer>()->playTrace(
        _controller,
        (_testDataPath / saveName).string(),
        (_testDataPath / traceName).string(),
        std::move(postLoadCallback)
    );
}

void TestController::prepareForNextTest() {
    ::application->get<EngineDeterministicPlugin>()->resetDeterministicState();
    ::application->get<GameKeyboardController>()->reset();
}
