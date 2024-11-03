#include "TestController.h"

#include <algorithm>
#include <utility>
#include <memory>
#include <string>

#include "Application/GameKeyboardController.h"

#include "Engine/Components/Trace/EngineTracePlayer.h"
#include "Engine/Components/Trace/EngineTraceStateAccessor.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Engine.h"
#include "Engine/EngineFileSystem.h"

#include "GUI/GUIMessageQueue.h"

#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/Platform/Application/PlatformApplication.h"

class TestControllerTickCallback : public ProxyOpenGLContext {
 public:
    explicit TestControllerTickCallback(TestController *controller): _controller(controller) {}

    virtual void swapBuffers() override {
        // This code goes first because we FIRST write the tapes, THEN jump into the control thread, and return
        // from EngineController::tick().
        if (_controller->isTaping())
            for (const auto &callback : _controller->_tapeCallbacks)
                callback();

        // This call potentially destroys `this`. Which is totally safe X). See `TestController::~TestController`.
        ProxyOpenGLContext::swapBuffers();
    }

 private:
    TestController *_controller = nullptr;
};

TestController::TestController(EngineController *controller, FileSystem *tfs, float playbackSpeed) {
    assert(controller);
    assert(tfs);

    _controller = controller;
    _tfs = tfs;
    _playbackSpeed = playbackSpeed;

    assert(engine->callObserver == nullptr);
    engine->callObserver = &_callObserver;

    application->installComponent(std::make_unique<TestControllerTickCallback>(this));
}

TestController::~TestController() {
    _callObserver.reset();
    assert(engine->callObserver == &_callObserver);
    engine->callObserver = nullptr;

    // This call destroys the `TestControllerTickCallback`. There is a slight problem with that - the fact that we are
    // here means that the main thread is currently paused inside the `swapBuffers` call, somewhere up the stack
    // is a frame for `TestControllerTickCallback::swapBuffers`, and we're destroying the object it's called on.
    // `TestControllerTickCallback::swapBuffers` is tail-calling, so this should work.
    application->removeComponent<TestControllerTickCallback>();
}

void TestController::loadGameFromTestData(std::string_view name) {
    _controller->loadGame(_tfs->read(name));
}

void TestController::playTraceFromTestData(std::string_view saveName, std::string_view traceName, std::function<void()> postLoadCallback) {
    playTraceFromTestData(saveName, traceName, 0, std::move(postLoadCallback));
}

void TestController::playTraceFromTestData(std::string_view saveName, std::string_view traceName,
                                           EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback) {
    EngineTraceRecording recording;
    recording.save = _tfs->read(saveName);
    recording.trace = _tfs->read(traceName);

    ::application->component<EngineTracePlayer>()->playTrace(
        _controller,
        recording,
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

    for (const DirectoryEntry &entry : ufs->ls(""))
        ufs->remove(entry.name);

    _callObserver.reset();
    _tapeCallbacks.clear();
    ::application->component<GameKeyboardController>()->reset();
    ::application->component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);

    // Clear the message queue, otherwise spells can roll over between test runs.
    // TODO(captainurist): this should really happen somewhere in the main loop. When new game is started, or a save is loaded.
    engine->_messageQueue->clear();

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
