#include "StartState.h"

FsmAction StartState::enter() {
    return FsmAction::transition("proceed");
}

FsmAction StartState::update() {
    return FsmAction::none();
}

void StartState::exit() {
}
