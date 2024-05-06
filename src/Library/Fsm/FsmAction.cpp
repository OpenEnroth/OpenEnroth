#include "FsmAction.h"
#include "Fsm.h"

void FsmActionNone::execute(Fsm &fsm) {
}

FsmActionTransition::FsmActionTransition(std::string_view transitionName) : _transitionName(transitionName) {
}

void FsmActionTransition::execute(Fsm &fsm) {
    fsm.scheduleTransition(_transitionName);
}
