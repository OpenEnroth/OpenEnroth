#include "Fsm.h"

#include <Library/Logger/Logger.h>

#include <utility>
#include <string>
#include <memory>

const LogCategory Fsm::fsmLogCategory("Fsm");
const std::string_view Fsm::exitState = "_Exit";

Fsm::Fsm(FsmStateEntries states, std::string_view startStateName) : _states(std::move(states)) {
    _goToState(startStateName);
}

void Fsm::update() {
    if (_hasReachedExitState)
        return;

    FsmAction action = _currentState->state->update();
    _performAction(action);
}

void Fsm::_goToState(std::string_view stateName) {
    FsmStateEntry *nextState = nullptr;
    if (!(nextState = _getStateByName(stateName))) {
        logger->warning(fsmLogCategory, "Cannot jump to state [{}]. The state does not exist.", stateName);
        return;
    }

    if (_currentState) {
        _currentState->state->exit();
    }
    _currentState = nextState;
    nextState = nullptr;
    FsmAction action = _currentState->state->enter();
    _performAction(action);
}

void Fsm::_performAction(FsmAction &action) {
    if (action) {
        _executeTransition(action.transitionName());
    }
}

bool Fsm::hasReachedExitState() const {
    return _hasReachedExitState;
}

void Fsm::_executeTransition(std::string_view transition) {
    // Look for the correct transition
    FsmTransitions &transitions = _currentState->transitions;
    auto itr = transitions.find(transition);
    if (itr == transitions.end()) {
        logger->warning(fsmLogCategory, "Cannot execute transition from state [{}]. Transition [{}] does not exist.",
            _currentState->name, transition);
        return;
    }

    // Check if there's at least one transition condition that evaluates to true
    FsmTransitionTarget *transitionTarget = nullptr;
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

    _hasReachedExitState = transitionTarget->stateName == exitState;
    if (_hasReachedExitState) {
        _currentState->state->exit();
        return;
    }

    _goToState(transitionTarget->stateName);
}

FsmStateEntry *Fsm::_getStateByName(std::string_view stateName) {
    if (auto itr = _states.find(stateName); itr != _states.end()) {
        return itr->second.get();
    }
    return nullptr;
}

bool Fsm::event(const PlatformEvent *event) {
    if (_currentState) {
        return _currentState->state->event(event);
    }
    return false;
}
