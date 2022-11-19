#pragma once

#include <mutex>
#include <memory>

#include "GameTestState.h"

class GameTestStateHandle {
 public:
    GameTestStateHandle(GameTestSide side, GameTestState *state) :
        state_(state),
        data_(std::make_shared<Data>(side, state->mutex))
    {}

    GameTestStateHandle() = delete;
    GameTestStateHandle(const GameTestStateHandle &) = default;
    GameTestStateHandle(GameTestStateHandle &&) = default;

    GameTestState *operator->() const {
        assert(data_->lock.owns_lock());

        return state_;
    }

    void SwitchSide() {
        assert(data_->lock.owns_lock());

        state_->currentSide = OtherSide();
        state_->wakeEvents[OtherSide()].notify_all();
        state_->wakeEvents[MySide()].wait(data_->lock, [&] { return state_->currentSide == MySide(); });
    }

 private:
    struct Data {
        GameTestSide side;
        std::unique_lock<std::mutex> lock;

        Data(GameTestSide side, std::mutex &mutex) : side(side), lock(mutex) {}
    };

    GameTestSide MySide() const {
        return data_->side;
    }

    GameTestSide OtherSide() const {
        return data_->side == TestSide ? GameSide : TestSide;
    }

 private:
    GameTestState *state_;
    std::shared_ptr<Data> data_;
};

