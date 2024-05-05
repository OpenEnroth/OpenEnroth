#include "FSMState.h"

#include "FSMTransitionHandler.h"

void FSMState::setTransitionHandler(FSMTransitionHandler *transitionHandler) {
    _transitionHandler = transitionHandler;
}

void FSMState::_scheduleTransition(std::string_view transition) {
    _transitionHandler->scheduleTransition(transition);
}
