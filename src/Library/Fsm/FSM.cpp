#include "FSM.h"

#include "NullState.h"
#include "ExitFromFSMState.h"

#include <Library/Logger/Logger.h>

#include <memory>
#include <utility>
#include <string>

const LogCategory FSM::fsmLogCategory("FSM");

FSM::FSM() {
    _createDefaultStates();
    // By default, the FSM attempts to reach the _Exit state.
    // This occurs if an FSM has no custom states or the user doesn't provide a different state
    // Although it's very unlikely to happen we still need to account for this possibility.
    jumpToState("_Exit");
}

void FSM::jumpToState(std::string_view stateName) {
    if (StateEntry *entry = _getStateByName(stateName)) {
        _nextState = entry;
    } else {
        logger->warning(fsmLogCategory, "Cannot jump to state [{}]. The state does not exist.", stateName);
    }
}

void FSM::update() {
    if (_nextState) {
        _currentState->state->exit();
        _currentState = _nextState;
        _nextState = nullptr;
        _currentState->state->enter();
    }

    _currentState->state->update();
}

bool FSM::hasReachedExitState() const {
    return _hasReachedExitState;
}

void FSM::addState(std::unique_ptr<StateEntry> stateEntry) {
    stateEntry->state->setTransitionHandler(this);
    _states.insert({ stateEntry->name, std::move(stateEntry) });
}

void FSM::executeTransition(std::string_view transition) {
    // Look for the correct transition
    FSMTransitions &transitions = _currentState->transitions;
    auto itr = transitions.find(transition);
    if (itr == transitions.end()) {
        logger->warning(fsmLogCategory, "Cannot execute transition from state [{}]. Transition [{}] does not exist.",
            _currentState->name, transition);
        return;
    }

    // Check if there's at least one transition condition that evaluates to true
    FSMTransitionTarget *transitionTarget = nullptr;
    for (auto &target : itr->second) {
        if (!target.condition || target.condition()) {
            transitionTarget = &target;
            break;
        }
    }
    if (!transitionTarget) {
        logger->warning(fsmLogCategory, "Cannot execute transition [{}] from state [{}]. No condition evaluated to true",
            transition, _currentState->name);
        return;
    }

    // Find the next state now that we have a name cadidate
    std::string &targetStateName = transitionTarget->stateName;
    StateEntry *stateEntry = _getStateByName(targetStateName);
    if (stateEntry) {
        _nextState = stateEntry;
    } else {
        logger->warning(fsmLogCategory, "Failed to transition to state [{}] during execution of transition [{}] from state [{}]. The target state does not exist.",
            targetStateName, transition, _currentState->name);
    }
}

FSM::StateEntry *FSM::_getStateByName(std::string_view stateName) {
    if (auto itr = _states.find(stateName); itr != _states.end()) {
        return itr->second.get();
    }
    return nullptr;
}

void FSM::exitFromFSM() {
    _hasReachedExitState = true;
    jumpToState("_Null");
}

bool FSM::keyPressEvent(const PlatformKeyEvent *event) {
    return _currentState->state->keyPressEvent(event);
}

bool FSM::keyReleaseEvent(const PlatformKeyEvent *event) {
    return _currentState->state->keyReleaseEvent(event);
}

bool FSM::mouseMoveEvent(const PlatformMouseEvent *event) {
    return _currentState->state->mouseMoveEvent(event);
}

bool FSM::mousePressEvent(const PlatformMouseEvent *event) {
    return _currentState->state->mousePressEvent(event);
}

bool FSM::mouseReleaseEvent(const PlatformMouseEvent *event) {
    return _currentState->state->mouseReleaseEvent(event);
}

bool FSM::wheelEvent(const PlatformWheelEvent *event) {
    return _currentState->state->wheelEvent(event);
}

bool FSM::moveEvent(const PlatformMoveEvent *event) {
    return _currentState->state->moveEvent(event);
}

bool FSM::resizeEvent(const PlatformResizeEvent *event) {
    return _currentState->state->resizeEvent(event);
}

bool FSM::activationEvent(const PlatformWindowEvent *event) {
    return _currentState->state->activationEvent(event);
}

bool FSM::closeEvent(const PlatformWindowEvent *event) {
    return _currentState->state->closeEvent(event);
}

bool FSM::gamepadConnectionEvent(const PlatformGamepadEvent *event) {
    return _currentState->state->gamepadConnectionEvent(event);
}

bool FSM::gamepadKeyPressEvent(const PlatformGamepadKeyEvent *event) {
    return _currentState->state->gamepadKeyPressEvent(event);
}

bool FSM::gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *event) {
    return _currentState->state->gamepadKeyReleaseEvent(event);
}

bool FSM::gamepadAxisEvent(const PlatformGamepadAxisEvent *event) {
    return _currentState->state->gamepadAxisEvent(event);
}

bool FSM::nativeEvent(const PlatformNativeEvent *event) {
    return _currentState->state->nativeEvent(event);
}

bool FSM::textInputEvent(const PlatformTextInputEvent *event) {
    return _currentState->state->textInputEvent(event);
}

void FSM::_createDefaultStates() {
    // Internal default states - They are prefixed with an underscore to denote them as internal.
    auto nullStateEntry = std::make_unique<FSM::StateEntry>();
    nullStateEntry->name = "_Null";
    nullStateEntry->state = std::make_unique<NullState>();
    _currentState = nullStateEntry.get();
    addState(std::move(nullStateEntry));

    auto exitStateEntry = std::make_unique<FSM::StateEntry>();
    exitStateEntry->name = "_Exit";
    exitStateEntry->state = std::make_unique<ExitFromFSMState>();
    addState(std::move(exitStateEntry));
}
