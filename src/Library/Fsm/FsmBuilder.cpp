#include "FsmBuilder.h"
#include "Fsm.h"

#include <Library/Logger/Logger.h>

#include <memory>
#include <utility>
#include <string>
#include <cassert>

FsmBuilder &FsmBuilder::state(std::string_view stateName, std::unique_ptr<FsmState> state) {
    _latestOnTransition.clear();
    auto entry = std::make_unique<FsmStateEntry>();
    entry->name = stateName;
    entry->state = std::move(state);
    _latestState = entry.get();
    _states.insert({ stateName, std::move(entry) });
    return *this;
}

FsmBuilder &FsmBuilder::on(std::string_view transitionName) {
    _latestOnTransition.clear();
    if (!_latestState) {
        logger->error(Fsm::fsmLogCategory, "Can't create a transition with name [{}]. No state has been setup in the FsmBuilder",
            transitionName);
        assert(false && "Can't create a transition without a declared state");
        return *this;
    }

    if (_latestState->transitions.contains(transitionName)) {
        logger->error(Fsm::fsmLogCategory, "Can't create a transition with the same name [{}]. State [{}]",
            transitionName, _latestState->name);
        assert(false && "Can't create a transition with the same name");
        return *this;
    }

    _latestState->transitions.insert({ transitionName, {} });
    _latestOnTransition = transitionName;
    return *this;
}

FsmBuilder &FsmBuilder::jumpTo(std::string_view targetState) {
    return jumpTo(nullptr, targetState);
}

FsmBuilder &FsmBuilder::exitFsm() {
    return jumpTo(nullptr, Fsm::exitState);
}

FsmBuilder &FsmBuilder::jumpTo(std::function<bool()> condition, std::string_view targetState) {
    if (!_latestState) {
        logger->error(Fsm::fsmLogCategory, "Can't add a target state to jumpTo. No state has been setup in the FsmBuilder. TargetState [{}]",
            targetState);
        assert(false && "Can't add a target state to jumpTo without a valid state.");
        return *this;
    }

    if (_latestOnTransition.empty()) {
        logger->error(Fsm::fsmLogCategory, "Can't add a target state to jumpTo. No 'on' event has been defined yet. State [{}], TargetState [{}]",
            _latestState->name, targetState);
        assert(false && "Can't add a target state to jumpTo without a valid event.");
        return *this;
    }

    _latestState->transitions[_latestOnTransition].push_back(FsmTransitionTarget{ std::string(targetState), condition });
    return *this;
}

std::unique_ptr<Fsm> FsmBuilder::build(std::string_view startStateName) {
    auto fsm = std::make_unique<Fsm>(std::move(_states), startStateName);
    _states.clear();
    _latestOnTransition.clear();
    return std::move(fsm);
}
