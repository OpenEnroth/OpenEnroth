#pragma once

#include <string_view>

#include "FSMEventHandler.h"

class FSMTransitionHandler;

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
    virtual void update() = 0;

    /**
     * @brief Invoked within FSM::update() when transitioning to this state as the new current state.
     */
    virtual void enter() = 0;

    /**
     * @brief Invoked within FSM::update() when this state is replaced by a new current state.
     */
    virtual void exit() = 0;

    /**
     * @brief Setter function used by the FSM to configure the state upon addition to the FSM.
     * Passing this information in the constructor would require additional boilerplate code for each new declared state.
     */
    void setTransitionHandler(FSMTransitionHandler *transitionHandler);

 protected:
    /**
     * @brief Internal function used to initiate transitions to other states. A state does not inherently know its next destination.
     * Its responsibility is limited, but it can schedule a transition that may be connected to another state when defining that state.
     * For example, a VideoState can schedule a "videoEnd" transition when the video playback is finished.
     * Transitions are not executed immediately; they are first evaluated for validity and executed during the next FSM::update() call.
     * Calling scheduleTransition outside the overridden FSMState::update() method is not permitted; an assertion is triggered within the FSM otherwise.
     */
    void _scheduleTransition(std::string_view transition);

 private:
    FSMTransitionHandler *_transitionHandler;
};
