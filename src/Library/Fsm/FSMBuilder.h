#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "FSM.h"

class FSMBuilder {
 public:
    template<typename TState, typename ...TArgs>
    FSMBuilder &state(std::string_view stateName, TArgs &&... args) {
        auto stateInstace = std::make_unique<TState>(std::forward<TArgs>(args) ...);
        state(stateName, std::move(stateInstace));
        return *this;
    }

    FSMBuilder &state(std::string_view stateName, std::unique_ptr<FSMState> state);
    FSMBuilder &on(std::string_view transitionName);
    FSMBuilder &jumpTo(std::string_view targetState);
    FSMBuilder &jumpTo(std::function<bool()> condition, std::string_view targetState);

    std::unique_ptr<FSM> build();

 private:
    std::vector<std::unique_ptr<FSM::StateEntry>> _states;
    std::string _latestOnTransition;
};
