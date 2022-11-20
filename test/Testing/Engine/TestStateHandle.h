#pragma once

#include <mutex>
#include <memory>

#include "TestState.h"

class TestStateHandle {
 public:
    TestStateHandle(GameTestSide side, TestState *state) :
        state_(state),
        data_(std::make_shared<Data>(side, state->mutex))
    {}

    ~TestStateHandle() {
        if (data_)
            YieldExecutionInternal(false); // unlock will get called in Data destructor.
    }

    TestStateHandle() = delete;
    TestStateHandle(const TestStateHandle &) = default;
    TestStateHandle(TestStateHandle &&) = default;

    TestState *operator->() const {
        assert(data_->lock.owns_lock());

        return state_;
    }

    void YieldExecution() {
        YieldExecutionInternal(true);
    }

 private:
    struct Data {
        GameTestSide side;
        std::unique_lock<std::mutex> lock;

        Data(GameTestSide side, std::mutex &mutex) : side(side), lock(mutex) {}
    };

    void YieldExecutionInternal(bool wait) {
        assert(data_->lock.owns_lock());

        state_->currentSide = OtherSide();
        state_->wakeEvents[OtherSide()].notify_all();

        if (wait)
            state_->wakeEvents[MySide()].wait(data_->lock, [&] { return state_->currentSide == MySide(); });
    }

    GameTestSide MySide() const {
        return data_->side;
    }

    GameTestSide OtherSide() const {
        return data_->side == TestSide ? GameSide : TestSide;
    }

 private:
    TestState *state_;
    std::shared_ptr<Data> data_;
};

