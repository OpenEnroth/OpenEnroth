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
    using GameRoutine = std::function<void()>;

    /** Queue of control routines to run, these are added from the main thread and are run in control thread.
     * Routines are removed from the queue only once they're finished. */
    std::queue<ControlRoutine> controlRoutineQueue;

    /** Posted events, these are added by the control routine and are then delivered to the main thread. */
    std::queue<std::unique_ptr<PlatformEvent>> postedEvents;

    /** A hacky way to run some code in the main thread w/o really leaving the control routine.
     * If this function is valid, yielding execution from the control thread will run it w/o proceeding to the next
     * frame, and then switch right back into the control thread. */
    GameRoutine gameRoutine;

    /** Which thread is currently active. */
    EngineControlSide currentSide = SIDE_GAME;

    /** This mutex protects everything. Game thread & control thread can run ONLY after locking it, and thus cannot
     * run in parallel. */
    std::mutex mutex;

    /** Condvars to wake up control/game threads. */
    IndexedArray<std::condition_variable, SIDE_GAME, SIDE_CONTROL> wakeEvents;

    /** Flag denoting that `EngineControlPlugin` is being destroyed and it's time to terminate the control thread. */
    bool terminating = false;

    /** This exception is thrown from `EngineController::tick` to quickly leave the control routine on termination.
     * Intentionally not derived from `std::exception`. */
    struct TerminationException {};
};

