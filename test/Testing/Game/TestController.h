#pragma once

#include <string>
#include <functional>
#include <utility>
#include <vector>
#include <memory>

#include "Engine/Components/Trace/EngineTraceEnums.h"

#include "Engine/Random/RandomEnums.h"

#include "TestTape.h"
#include "TestCallObserver.h"

class FileSystem;
class EngineController;

class TestController {
 public:
    TestController(EngineController *controller, FileSystem *tfs, float playbackSpeed);
    ~TestController();

    void loadGameFromTestData(std::string_view name);
    void playTraceFromTestData(std::string_view saveName, std::string_view traceName, std::function<void()> postLoadCallback = {});
    void playTraceFromTestData(std::string_view saveName, std::string_view traceName, EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback = {});

    void prepareForNextTest();
    void prepareForNextTest(int frameTimeMs, RandomEngineType rngType);

    bool isTaping() const;
    void startTaping();
    void stopTaping();

 private:
    friend class TestControllerTickCallback;
    friend class CharacterTapeRecorder;
    friend class CommonTapeRecorder;
    friend class ActorTapeRecorder;

    void prepareForNextTestInternal();
    void adjustMaxFps();

    // Accessed by tape recorders.
    template<class Callback, class T = std::invoke_result_t<Callback>>
    TestTape<T> recordTape(Callback callback) {
        auto state = std::make_shared<detail::TestTapeState<T>>(std::move(callback));
        _tapeCallbacks.push_back([state] { state->tick(); });
        return TestTape<T>(std::move(state));
    }

    template<class T>
    TestMultiTape<T> recordFunctionTape(EngineCall call) {
        return recordTape(_callObserver.record<T>(call));
    }

 private:
    EngineController *_controller;
    FileSystem *_tfs;
    float _playbackSpeed;
    TestCallObserver _callObserver;
    std::vector<std::function<void()>> _tapeCallbacks;
};
