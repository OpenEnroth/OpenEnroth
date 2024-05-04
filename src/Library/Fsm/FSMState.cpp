#include "FSMState.h"

#include "FSMTransitionHandler.h"

void FSMState::setTransitionHandler(FSMTransitionHandler *transitionHandler) {
    _transitionHandler = transitionHandler;
}

void FSMState::executeTransition(std::string_view transition) {
    _transitionHandler->executeTransition(transition);
}
