#pragma once

#include <Utility/TransparentFunctors.h>
#include <Library/Logger/LogCategory.h>

#include "FSMTransitionHandler.h"
#include "FSMState.h"

#include <memory>
#include <string_view>
#include <string>
#include <unordered_map>

using FSMTransitions = std::unordered_map<TransparentString, std::string, TransparentStringHash, TransparentStringEquals>;

class FSM : public FSMTransitionHandler {
 public:
    FSM();
    void setInitialState(std::string_view stateName);
    void update();
    [[nodiscard]] bool isDone() const;

    void addState(std::string_view name, std::unique_ptr<FSMState> state, FSMTransitions transitions);
    virtual void executeTransition(std::string_view transition) override;
    virtual void exitFromFSM() override;

 private:
    struct StateEntry {
        std::string name;
        std::unique_ptr<FSMState> state;
        FSMTransitions transitions;
    };

    StateEntry *_getStateByName(std::string_view stateName);

    std::unordered_map<TransparentString, std::unique_ptr<StateEntry>, TransparentStringHash, TransparentStringEquals> _states;
    StateEntry *_currentState{};
    StateEntry *_nextState{};
    StateEntry _nullState;
    bool _exitFromFSM{};

    static const LogCategory fsmLogCategory;
};
