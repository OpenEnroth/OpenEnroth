#pragma once

#include <Utility/String/TransparentFunctors.h>
#include <Library/Logger/LogCategory.h>

#include "FsmEventHandler.h"
#include "FsmTypes.h"

#include <string>
#include <memory>

class Fsm : public FsmEventHandler {
 public:
    Fsm(FsmStateEntries states, std::string_view startStateName);

    /**
     * @brief Updates the current state of the Fsm or executes any pending transitions.
     */
    void update();

    /*
    * @brief Check if the Fsm has reached its internal Fsm::exitState.
    */
    [[nodiscard]] bool hasReachedExitState() const;

    /**
     * @brief Sets the next state to be reached in the Fsm. The transition does not occur immediately but will be executed during the next Fsm::update() call.
     * The jumpToState function allows unconditional transitions without requiring a previously defined transition connecting the current state to the target state.
     * Subsequent calls to Fsm::jumpToState or Fsm::scheduleTransition will overwrite the target state since the actual transition happens during the next Fsm::update() call.
     * @param stateName The name of the state to transition to. This name must belong to a state that has been previously added through Fsm::addState.
     */
    void jumpToState(std::string_view stateName);

    /**
     * Global string used to specify the state used to exit from the Fsm.
     * When the Fsm jumps to this state, Fsm::hasReachedExitState() will return true.
     */
    static const std::string_view exitState;

    static const LogCategory fsmLogCategory;

 private:
    // FsmEventHandler implementation
    virtual bool event(const PlatformEvent *event) override;

    void _performPendingTransition();
    void _updateCurrentState();
    void _performAction(FsmAction &action);
    void _scheduleTransition(std::string_view transition);
    [[nodiscard]] FsmStateEntry *_getStateByName(std::string_view stateName);

    FsmStateEntries _states;
    FsmStateEntry *_currentState{};
    FsmStateEntry *_nextState{};

    // By default, when the Fsm has no states, it's treated as if it reached already the Fsm::exitState state
    bool _hasReachedExitState{true};
};
