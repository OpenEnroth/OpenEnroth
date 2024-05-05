#include "FSM.h"

#include <Library/Logger/Logger.h>

#include <memory>
#include <utility>
#include <string>

const LogCategory FSM::fsmLogCategory("FSM");
const std::string_view FSM::exitState = "_Exit";

void FSM::jumpToState(std::string_view stateName) {
    _hasReachedExitState = stateName == exitState;
    if (!_hasReachedExitState) {
        if (StateEntry *entry = _getStateByName(stateName)) {
            _nextState = entry;
        } else {
            logger->warning(fsmLogCategory, "Cannot jump to state [{}]. The state does not exist.", stateName);
        }
    }
}

void FSM::update() {
    if (_hasReachedExitState)
        return;

    _performPendingTransition();
    _updateCurrentState();

    //We can reach the end state while performing the transitions or update the current state
    if (_hasReachedExitState) {
        _currentState->state->exit();
    }
}

void FSM::_performPendingTransition() {
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

void FSM::_updateCurrentState() {
    auto action = _currentState->state->update();
    _performAction(action);
}

void FSM::_performAction(FSMAction &action) {
    std::visit([this](auto &&action) { action.execute(*this); }, action);
}

bool FSM::hasReachedExitState() const {
    return _hasReachedExitState;
}

void FSM::addState(std::unique_ptr<StateEntry> stateEntry) {
    _states.insert({ stateEntry->name, std::move(stateEntry) });
}

void FSM::scheduleTransition(std::string_view transition) {
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

    jumpToState(transitionTarget->stateName);
}

FSM::StateEntry *FSM::_getStateByName(std::string_view stateName) {
    if (auto itr = _states.find(stateName); itr != _states.end()) {
        return itr->second.get();
    }
    return nullptr;
}

bool FSM::event(const PlatformEvent *event) {
    if (_currentState) {
        return _currentState->state->event(event);
    }
    return false;
}
