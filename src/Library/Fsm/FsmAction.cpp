#include "FsmAction.h"

#include <string>

FsmAction FsmAction::none() {
    return {};
}

FsmAction FsmAction::transition(std::string_view transitionName) {
    FsmAction result;
    result._transitionName = transitionName;
    return result;
}

FsmAction::operator bool() const {
    return !_transitionName.empty();
}

bool FsmAction::operator!() const {
    return _transitionName.empty();
}

const std::string &FsmAction::transitionName() const {
    assert(!!*this);
    return _transitionName;
}
