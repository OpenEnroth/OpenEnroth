#pragma once

#include <string_view>

#include "FSMEventHandler.h"
#include "FSMAction.h"

/**
 * @brief Implement this abstract class to create a new State that can be added to an FSM.
 * FSMState derives from FSMEventHandler, enabling reaction to platform application events (such as input, window events, etc.) 
 * by overriding the appropriate functions.
 */
class FSMState: public FSMEventHandler {
 public:
    virtual ~FSMState() = default;

    /*
    * @brief Called during each FSM::update() iteration if this state is the current state in the FSM.
    */
    virtual FSMAction update() = 0;

    /**
     * @brief Invoked within FSM::update() when transitioning to this state as the new current state.
     */
    virtual FSMAction enter() = 0;

    /**
     * @brief Invoked within FSM::update() when this state is replaced by a new current state.
     */
    virtual void exit() = 0;
};
