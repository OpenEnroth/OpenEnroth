#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "Utility/IndexedArray.h"

class TestPlatform;
class TestWindow;
class TestEventLoop;
class TestGamepad;

enum class GameTestSide {
    GameSide,
    TestSide
};
using enum GameTestSide;


class TestState {
 public:
    TestPlatform *platform = nullptr;
    TestWindow *window = nullptr;
    TestEventLoop *eventLoop = nullptr;

    int64_t time = 0; // In milliseconds, for TestPlatform::TickCount

    bool terminating = false;
    std::function<void()> terminationHandler;

    GameTestSide currentSide;
    std::mutex mutex;
    IndexedArray<std::condition_variable, GameSide, TestSide> wakeEvents;
};

