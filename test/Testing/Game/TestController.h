#pragma once

#include <filesystem>
#include <string>
#include <functional>
#include <utility>
#include <vector>
#include <memory>

#include "Engine/Components/Trace/EngineTraceEnums.h"
#include "Engine/Objects/Character.h"
#include "Engine/Party.h"

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

    /**
     * Creates a tape object that will record changes in a value computed by the provided callback throughout the
     * execution of a trace.
     *
     * The callback is called on every frame, and also before and after running the trace, and unique values are stored
     * on a tape. This effectively means that if, for example, the provided callback always returns the same value,
     * then you'll get a tape of size 1.
     *
     * A typical use case is to create a tape, call `playTraceFromTestData` to play a trace, then check the data
     * stored on the tape.
     *
     * @param callback                  Callback that will calculate the values to store on a tape.
     * @return                          Tape object.
     */
    template<class Callback, class T = std::invoke_result_t<Callback>>
    TestTape<T> tape(Callback callback) {
        auto state = std::make_shared<detail::TestTapeState<T>>(std::move(callback));
        _tapeCallbacks.push_back([state] { state->tick(); });
        return TestTape<T>(std::move(state));
    }

    /**
     * Like the other `tape` overload, but records per-character values, and produces a tape that has vectors
     * as individual values.
     *
     * @param callback                  Callback taking a `const Character &` reference that will
     *                                  calculate per-character values to store on a tape.
     * @return                          Tape object.
     */
    template<class Callback, class T = std::invoke_result_t<Callback, const Character &>>
    TestTape<TestVector<T>> tape(Callback callback) {
        return tape([callback = std::move(callback)] {
            TestVector<T> result;
            for (const Character &character : pParty->pCharacters)
                result.push_back(callback(character));
            return result;
        });
    }

 private:
    void runTapeCallbacks();

 private:
    EngineController *_controller;
    std::filesystem::path _testDataPath;
    std::vector<std::function<void()>> _tapeCallbacks;
};
