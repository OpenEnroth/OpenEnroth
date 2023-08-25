#pragma once

#include <filesystem>
#include <string>
#include <functional>
#include <utility>
#include <vector>
#include <memory>
#include <type_traits>
#include "Engine/Components/Trace/EngineTraceEnums.h"
#include "TestTape.h"

class EngineController;

class TestController {
 public:
    TestController(EngineController *controller, const std::string &testDataPath);

    std::string fullPathInTestData(const std::string &fileName);

    void loadGameFromTestData(const std::string &name);
    void playTraceFromTestData(const std::string &saveName, const std::string &traceName, std::function<void()> postLoadCallback = {});
    void playTraceFromTestData(const std::string &saveName, const std::string &traceName, EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback = {});

    void prepareForNextTest();

    void restart(int frameTimeMs); // TODO(captainurist): need a better name here.

 private:
    friend class CharacterTapeRecorder;
    friend class CommonTapeRecorder;

    // Accessed by tape recorders.
    template<class Callback, class T = std::invoke_result_t<Callback>>
    TestTape<T> recordTape(Callback callback) {
        auto state = std::make_shared<detail::TestTapeState<T>>(std::move(callback));
        _tapeCallbacks.push_back([state] { state->tick(); });
        return TestTape<T>(std::move(state));
    }

    void runTapeCallbacks();

 private:
    EngineController *_controller;
    std::filesystem::path _testDataPath;
    std::vector<std::function<void()>> _tapeCallbacks;
};
