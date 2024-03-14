#pragma once

#include <filesystem>
#include <string>
#include <functional>
#include <utility>
#include <vector>
#include <memory>

#include "Engine/Components/Trace/EngineTraceEnums.h"

#include "Engine/Random/RandomEnums.h"

#include "TestTape.h"
#include "TestCallObserver.h"

class EngineController;
class TestControllerTickCallback;

class TestController {
 public:
    TestController(EngineController *controller, const std::string &testDataPath, float playbackSpeed);
    ~TestController();

    std::string fullPathInTestData(const std::string &fileName);

    void loadGameFromTestData(const std::string &name);
    void playTraceFromTestData(const std::string &saveName, const std::string &traceName, std::function<void()> postLoadCallback = {});
    void playTraceFromTestData(const std::string &saveName, const std::string &traceName, EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback = {});

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
    std::filesystem::path _testDataPath;
    float _playbackSpeed;
    TestCallObserver _callObserver;
    TestControllerTickCallback *_tickCallback = nullptr;
    std::vector<std::function<void()>> _tapeCallbacks;
};
