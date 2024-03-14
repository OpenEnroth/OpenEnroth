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

#include "Media/Audio/AudioPlayer.h"

#include "Library/Platform/Application/PlatformApplication.h"

class TestControllerTickCallback : public ProxyOpenGLContext {
 public:
    virtual void swapBuffers() override {
        // This code goes first because we FIRST write the tapes, THEN jump into the control thread, and return
        // from EngineController::tick().
        if (_controller && _controller->isTaping())
            for (const auto &callback : _controller->_tapeCallbacks)
                callback();

        ProxyOpenGLContext::swapBuffers();
    }

    void setController(TestController *controller) {
        _controller = controller;
    }

    TestController *controller() const {
        return _controller;
    }

 private:
    TestController *_controller = nullptr;
};

TestController::TestController(EngineController *controller, const std::string &testDataPath, float playbackSpeed) {
    _controller = controller;
    _testDataPath = testDataPath;
    _playbackSpeed = playbackSpeed;

    assert(engine->callObserver == nullptr);
    engine->callObserver = &_callObserver;

    std::unique_ptr<TestControllerTickCallback> tickCallback = std::make_unique<TestControllerTickCallback>();
    tickCallback->setController(this);
    _tickCallback = tickCallback.get();
    application->installComponent(std::move(tickCallback));
}

TestController::~TestController() {
    _callObserver.reset();
    assert(engine->callObserver == &_callObserver);
    engine->callObserver = nullptr;

    // Application is alive at this point, thus tick callback is also alive.
    _tickCallback->setController(nullptr);
}

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

            adjustMaxFps();
            startTaping();
        }
    );
    stopTaping();
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

bool TestController::isTaping() const {
    return _callObserver.isEnabled();
}

void TestController::startTaping() {
    assert(!isTaping());
    _callObserver.setEnabled(true);
}

void TestController::stopTaping() {
    assert(isTaping());
    _callObserver.setEnabled(false);
}

void TestController::prepareForNextTestInternal() {
    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();
    RandomEngineType rngType = engine->config->debug.TraceRandomEngine.value();

    _callObserver.reset();
    _tapeCallbacks.clear();
    ::application->component<GameKeyboardController>()->reset();
    ::application->component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);

    _controller->goToMainMenu();
    adjustMaxFps(); // Set max fps AFTER going to the main menu, so that the latter one is done quickly.
}

void TestController::adjustMaxFps() {
    // FPS are unlimited by default, and speed over x1000 isn't really distinguishable from unlimited FPS.
    if (_playbackSpeed < 1000.0f) {
        int fps = _playbackSpeed * 1000 / engine->config->debug.TraceFrameTimeMs.value();
        engine->config->graphics.FPSLimit.setValue(std::max(1, fps));
    }
}
