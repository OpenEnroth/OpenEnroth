#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <memory>

#include "Utility/IndexedArray.h"

class PlatformEvent;
class EngineController;

enum class EngineControlSide {
    SIDE_GAME,
    SIDE_CONTROL
};
using enum EngineControlSide;

class EngineControlState {
 public:
    using ControlRoutine = std::function<void(EngineController *)>;

    std::queue<ControlRoutine> controlRoutineQueue;
    std::queue<std::unique_ptr<PlatformEvent>> postedEvents;

    EngineControlSide currentSide = SIDE_GAME;
    std::mutex mutex;
    IndexedArray<std::condition_variable, SIDE_GAME, SIDE_CONTROL> wakeEvents;

    bool terminating = false;
    struct TerminationException {}; // Intentionally not derived from std::exception.
};

