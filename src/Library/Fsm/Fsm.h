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
     * @brief Updates the current state of the Fsm and executes any transition.
     */
    void update();

    /*
    * @brief Check if the Fsm has reached its internal Fsm::exitState.
    */
    [[nodiscard]] bool hasReachedExitState() const;

    /**
     * Global string used to specify the state used to exit from the Fsm.
     * When the Fsm jumps to this state, Fsm::hasReachedExitState() will return true.
     */
    static const std::string_view exitState;

    static const LogCategory fsmLogCategory;

 private:
    // FsmEventHandler implementation
    virtual bool event(const PlatformEvent *event) override;

    void _goToState(std::string_view stateName);
    void _performAction(FsmAction &action);
    void _executeTransition(std::string_view transition);
    [[nodiscard]] FsmStateEntry *_getStateByName(std::string_view stateName);

    FsmStateEntries _states;
    FsmStateEntry *_currentState{};

    bool _hasReachedExitState{};
};
