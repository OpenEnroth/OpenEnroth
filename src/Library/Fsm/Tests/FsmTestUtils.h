#pragma once

#include <gmock/gmock.h>

#include <Library/Fsm/Fsm.h>

#include <functional>
#include <memory>
#include <utility>

class FsmTest : public testing::Test {
 protected:
    void _addState(std::string_view name, std::unique_ptr<FsmState> state) {
        auto entry = std::make_unique<FsmStateEntry>();
        entry->name = name;
        entry->state = std::move(state);
        _states.insert({ name, std::move(entry) });
    }

    std::unique_ptr<Fsm> _createFsm(std::string_view startingStateName) {
        return std::make_unique<Fsm>(std::move(_states), startingStateName);
    }

    FsmStateEntries _states;
};

class MockFsmState : public FsmState {
 public:
     MOCK_METHOD(FsmAction, enter, (), (override));
     MOCK_METHOD(FsmAction, update, (), (override));
     MOCK_METHOD(void, exit, (), (override));
};
