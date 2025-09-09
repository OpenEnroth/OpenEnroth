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

    // Main thread -> control thread communication.

    /** Queue of control routines to run, these are added from the main thread and are consumed & run in control thread.
     * Routines are removed from the queue only once they're finished. */
    std::queue<ControlRoutine> controlRoutineQueue;

    /** Flag denoting that `EngineControlComponent` is being destroyed and it's time to terminate the control thread.
     * It is set from the main thread. */
    bool terminating = false;

    /** This exception is thrown from `EngineController::tick` to quickly leave the control routine on termination.
     * Intentionally not derived from `std::exception`. */
    struct TerminationException {};

    // Control thread -> main thread communication.

    /** Posted events, these are added from the control thread and are then consumed in the main thread. */
    std::queue<std::unique_ptr<PlatformEvent>> postedEvents;

    /** A way to run some code in the main thread w/o really leaving the control routine.
     * If this function is valid, yielding execution from the control thread will run it w/o proceeding to the next
     * frame, and then switch right back into the control thread. It is set in control thread and cleared in the
     * main thread.
     *
     * Exception propagation from the control thread to the main thread is done with a game routine. */
    GameRoutine gameRoutine;

    // Synchronization primitives.

    /** Which thread is currently active. */
    EngineControlSide currentSide = SIDE_GAME;

    /** This mutex protects everything. Game thread & control thread can run ONLY after locking it, and thus cannot
     * run in parallel. */
    std::mutex mutex;

    /** Condvars to wake up control/game threads. */
    IndexedArray<std::condition_variable, SIDE_GAME, SIDE_CONTROL> wakeEvents;
};

