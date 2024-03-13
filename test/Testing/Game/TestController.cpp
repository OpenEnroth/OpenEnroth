#include "TestController.h"

#include <algorithm>
#include <utility>

#include "Application/GameKeyboardController.h"

#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Engine.h"
#include "Engine/Spells/CastSpellInfo.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Platform/Application/PlatformApplication.h"

TestController::TestController(EngineController *controller, const std::string &testDataPath, float playbackSpeed):
    _controller(controller),
    _testDataPath(testDataPath),
    _playbackSpeed(playbackSpeed)
{}

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
    ::application->component<EngineTracePlayer>()->playTrace(
        _controller,
        fullPathInTestData(saveName),
        fullPathInTestData(traceName),
        flags,
        [this, postLoadCallback = std::move(postLoadCallback)] {
            if (postLoadCallback)
                postLoadCallback();

            // FPS are unlimited by default, and speed over x1000 isn't really distinguishable from unlimited FPS.
            if (_playbackSpeed < 1000.0f) {
                int fps = _playbackSpeed * 1000 / engine->config->debug.TraceFrameTimeMs.value();
                engine->config->graphics.FPSLimit.setValue(std::max(1, fps));
            }
        },
        [this] {
            runTapeCallbacks();
        }
    );
}

void TestController::prepareForNextTest() {
    // Use frame time & rng from config defaults. However, if calling playTraceFromTestData next, these will be
    // overridden by whatever is saved in the trace file.
    EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), {});
    prepareForNextTestInternal();
}

void TestController::prepareForNextTest(int frameTimeMs, RandomEngineType rngType) {
    EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), {});
    engine->config->debug.TraceFrameTimeMs.setValue(frameTimeMs);
    engine->config->debug.TraceRandomEngine.setValue(rngType);
    prepareForNextTestInternal();
}

void TestController::prepareForNextTestInternal() {
    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();
    RandomEngineType rngType = engine->config->debug.TraceRandomEngine.value();

    _tapeCallbacks.clear();
    ::application->component<GameKeyboardController>()->reset();
    ::application->component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);

    // Clear the spell/message queue to fix #1535
    CastSpellInfoHelpers::clearSpellQueue();
    engine->_messageQueue->clear();

    _controller->goToMainMenu();
}

void TestController::runTapeCallbacks() {
    for (const auto &callback : _tapeCallbacks)
        callback();
}
