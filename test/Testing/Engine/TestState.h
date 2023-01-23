#pragma once

#include <mutex>
#include <condition_variable>
#include <functional>

#include "Utility/IndexedArray.h"

class TestProxy;
class PlatformApplication;

enum class GameTestSide {
    GameSide,
    TestSide
};
using enum GameTestSide;

class TestState {
 public:
    explicit TestState(PlatformApplication *application) : application(application) {}

    PlatformApplication *application = nullptr;
    TestProxy *proxy = nullptr;

    bool terminating = false;
    std::function<void()> terminationHandler;

    GameTestSide currentSide;
    std::mutex mutex;
    IndexedArray<std::condition_variable, GameSide, TestSide> wakeEvents;
};

