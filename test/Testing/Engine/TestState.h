#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "Utility/IndexedArray.h"

class TestWindow;
class TestEventLoop;

enum class GameTestSide {
    GameSide,
    TestSide
};
using enum GameTestSide;


class TestState {
 public:
    TestWindow *window = nullptr;
    TestEventLoop *eventLoop = nullptr;

    bool terminating = false;
    std::function<void()> terminationHandler;

    GameTestSide currentSide;
    std::mutex mutex;
    IndexedArray<std::condition_variable, GameSide, TestSide> wakeEvents;
};

