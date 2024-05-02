#include "FSM.h"

#include "NullState.h"
#include "ExitFromFSMState.h"

#include <Library/Logger/Logger.h>

#include <memory>
#include <utility>
#include <string>

const LogCategory FSM::fsmLogCategory("FSM");

FSM::FSM() {
    _nullState.name = "NullState";
    _nullState.state = std::make_unique<NullState>();
    _currentState = &_nullState;
    addState("ExitFSM", std::make_unique<ExitFromFSMState>(), {});
}

void FSM::setInitialState(std::string_view stateName) {
    if (StateEntry *entry = _getStateByName(stateName)) {
        _nextState = entry;
    } else {
        logger->warning(fsmLogCategory, "Can't start FSM with state [{}]. The state doesn't exists.", stateName);
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

bool FSM::isDone() const {
    return _exitFromFSM;
}

void FSM::addState(std::string_view name, std::unique_ptr<FSMState> state, FSMTransitions transitions) {
    state->setTransitionHandler(this);
    auto stateEntry = std::make_unique<StateEntry>();
    stateEntry->name = name;
    stateEntry->state = std::move(state);
    stateEntry->transitions = std::move(transitions);
    _states.insert({ name, std::move(stateEntry) });
}

void FSM::executeTransition(std::string_view transition) {
    auto &transitions = _currentState->transitions;
    if (auto itr = transitions.find(transition); itr != transitions.end()) {
        std::string &targetStateName = itr->second;
        StateEntry *stateEntry = _getStateByName(targetStateName);
        if (stateEntry) {
            _nextState = stateEntry;
        } else {
            logger->warning(fsmLogCategory, "Can't reach state [{}] while executing transition [{}] from state [{}].",
                targetStateName, transition, _currentState->name);
        }
    } else {
        logger->warning(fsmLogCategory, "Can't execute transition from state [{}]. Transition [{}] does not exist.",
            _currentState->name, transition);
    }
}

FSM::StateEntry *FSM::_getStateByName(std::string_view stateName) {
    if (auto itr = _states.find(stateName); itr != _states.end()) {
        return itr->second.get();
    }
    return nullptr;
}

void FSM::exitFromFSM() {
    _exitFromFSM = true;
    _nextState = &_nullState;
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
