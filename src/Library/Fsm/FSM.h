#pragma once

#include <Utility/TransparentFunctors.h>
#include <Library/Logger/LogCategory.h>

#include <memory>
#include <string_view>
#include <string>
#include <unordered_map>

class FSMTransitionHandler {
 public:
    virtual void executeTransition(std::string_view transitionName) = 0;
    virtual void exitFromFSM() = 0;
};

class FSMState {
 public:
    virtual ~FSMState() = default;
    virtual void update() = 0;
    virtual void enter() = 0;
    virtual void exit() = 0;

    void setTransitionHandler(FSMTransitionHandler *transitionHandler) {
        _transitionHandler = transitionHandler;
    }

 protected:
    void executeTransition(std::string_view transition) {
        _transitionHandler->executeTransition(transition);
    }

    void exitFromFSM() {
        _transitionHandler->exitFromFSM();
    }

 private:
    FSMTransitionHandler *_transitionHandler;
};

using FSMTransitions = std::unordered_map<TransparentString, std::string, TransparentStringHash, TransparentStringEquals>;

class NullState : public FSMState {
 public:
    virtual void enter() {}
    virtual void exit() {}
    virtual void update() {}
};

class FSM : public FSMTransitionHandler {
 public:
    FSM();
    void start(std::string_view stateName);
    bool update();

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

class ExitFromFSMState : public FSMState {
 public:
    virtual void update() override {}
    virtual void enter() override { exitFromFSM(); }
    virtual void exit() override {}
};
