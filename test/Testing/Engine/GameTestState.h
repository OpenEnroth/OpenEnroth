#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>

#include "Utility/IndexedArray.h"

class GameTestWindow;
class GameTestEventLoop;

enum class GameTestSide {
    GameSide,
    TestSide
};
using enum GameTestSide;


class GameTestState {
 public:
    GameTestWindow *window = nullptr;
    GameTestEventLoop *eventLoop = nullptr;

    GameTestSide currentSide;
    std::mutex mutex;
    IndexedArray<std::condition_variable, GameSide, TestSide> wakeEvents;
};

