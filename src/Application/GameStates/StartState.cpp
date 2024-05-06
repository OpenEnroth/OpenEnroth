#include "StartState.h"

FsmAction StartState::enter() {
    return FsmActionTransition("proceed");
}

FsmAction StartState::update() {
    return FsmActionNone();
}

void StartState::exit() {
}
