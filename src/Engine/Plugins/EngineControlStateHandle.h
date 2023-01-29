#pragma once

#include <cassert>
#include <memory>

#include "EngineControlState.h"

class EngineControlStateHandle {
 public:
    EngineControlStateHandle(EngineControlSide side, EngineControlState *state) :
        _data(std::make_shared<SharedData>(side, state))
    {}

    ~EngineControlStateHandle() = default;

    EngineControlStateHandle() = delete;
    EngineControlStateHandle(const EngineControlStateHandle &) = default;
    EngineControlStateHandle(EngineControlStateHandle &&) = default;

    EngineControlState *operator->() const {
        assert(_data->lock.owns_lock());

        return _data->state;
    }

    void yieldExecution() {
        _data->yieldExecutionInternal(true);
    }

 private:
    struct SharedData {
        SharedData(EngineControlSide side, EngineControlState *state): side(side), state(state), lock(state->mutex) {
            assert(state);
        }

        ~SharedData() {
            yieldExecutionInternal(false);
        }

        void yieldExecutionInternal(bool wait) {
            assert(lock.owns_lock());

            state->currentSide = otherSide();
            state->wakeEvents[otherSide()].notify_all();

            if (wait)
                state->wakeEvents[mySide()].wait(lock, [&] { return state->currentSide == mySide(); });
        }

        EngineControlSide mySide() const {
            return side;
        }

        EngineControlSide otherSide() const {
            return side == SIDE_GAME ? SIDE_CONTROL : SIDE_GAME;
        }

        EngineControlSide side;
        EngineControlState *state;
        std::unique_lock<std::mutex> lock;
    };

 private:
    // Storing a unique_lock in a shared_ptr is a bit retarded, but this makes this handle class copyable with exactly
    // the semantics that we want. Another option would be to manage the lock's lifetime externally, but this will just
    // pile more complexity where it's not really needed.
    std::shared_ptr<SharedData> _data;
};

