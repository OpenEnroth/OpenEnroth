#include "FSMAction.h"
#include "FSM.h"

void FSMActionNone::execute(FSM &fsm) {
}

FSMActionTransition::FSMActionTransition(std::string_view transitionName) : _transitionName(transitionName) {
}

void FSMActionTransition::execute(FSM &fsm) {
    fsm.scheduleTransition(_transitionName);
}
