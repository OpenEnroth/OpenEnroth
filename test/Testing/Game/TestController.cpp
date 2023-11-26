#include "TestController.h"

#include <utility>

#include "Application/GameKeyboardController.h"

#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Engine.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Platform/Application/PlatformApplication.h"

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
    resetConfigInternal();
    // Use frame time & rng from config defaults. If calling playTraceFromTestData next, these will be overridden
    // by whatever is saved in the trace file.
    prepareForNextTest(engine->config->debug.TraceFrameTimeMs.value(), engine->config->debug.TraceRandomEngine.value());
}

void TestController::prepareForNextTest(int frameTimeMs, RandomEngineType rngType) {
    resetConfigInternal();
    prepareForNextTestInternal(frameTimeMs, rngType);
}

void TestController::resetConfigInternal() {
    // These two lines bring the game config into the same state as if a trace playback was started with an empty
    // config patch. Mainly needed for tests that don't play back any traces.
    EngineTraceStateAccessor::prepareForPlayback(engine->config.get());
    EngineTraceStateAccessor::patchConfig(engine->config.get(), {});
    pAudioPlayer->UpdateVolumeFromConfig();
}

void TestController::prepareForNextTestInternal(int frameTimeMs, RandomEngineType rngType) {
    _tapeCallbacks.clear();
    ::application->get<GameKeyboardController>()->reset();
    ::application->get<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);
    _controller->goToMainMenu();
}

void TestController::runTapeCallbacks() {
    for (const auto &callback : _tapeCallbacks)
        callback();
}
