#include "Fsm.h"

#include <Library/Logger/Logger.h>

#include <utility>
#include <string>
#include <memory>

const LogCategory Fsm::fsmLogCategory("Fsm");
const std::string_view Fsm::exitState = "_Exit";

Fsm::Fsm(FsmStateEntries states, std::string_view startStateName) : _states(std::move(states)) {
    jumpToState(startStateName);
}

void Fsm::jumpToState(std::string_view stateName) {
    _hasReachedExitState = stateName == exitState;
    if (!_hasReachedExitState) {
        if (FsmStateEntry *entry = _getStateByName(stateName)) {
            _nextState = entry;
        } else {
            logger->warning(fsmLogCategory, "Cannot jump to state [{}]. The state does not exist.", stateName);
        }
    }
}

void Fsm::update() {
    if (_hasReachedExitState)
        return;

    _performPendingTransition();
    _updateCurrentState();

    //We can reach the end state while performing the transitions or update the current state
    if (_hasReachedExitState) {
        _currentState->state->exit();
    }
}

void Fsm::_performPendingTransition() {
    //We add a loop to resolve any pass-through transition
    while (_nextState) {
        if (_currentState) {
            _currentState->state->exit();
        }
        _currentState = _nextState;
        _nextState = nullptr;
        auto action = _currentState->state->enter();
        _performAction(action);
    }
}

void Fsm::_updateCurrentState() {
    auto action = _currentState->state->update();
    _performAction(action);
}

void Fsm::_performAction(FsmAction &action) {
    if (action) {
        _scheduleTransition(action.transitionName());
    }
}

bool Fsm::hasReachedExitState() const {
    return _hasReachedExitState;
}

void Fsm::_scheduleTransition(std::string_view transition) {
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

    jumpToState(transitionTarget->stateName);
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
