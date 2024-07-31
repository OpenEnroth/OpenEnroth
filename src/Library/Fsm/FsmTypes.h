#pragma once

#include <Utility/String/TransparentFunctors.h>

#include "FsmState.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

struct FsmTransitionTarget {
    std::string stateName;
    std::function<bool()> condition{};
};

using FsmTransitions = std::unordered_map<TransparentString, std::vector<FsmTransitionTarget>, TransparentStringHash, TransparentStringEquals>;

struct FsmStateEntry {
    std::string name;
    std::unique_ptr<FsmState> state;
    FsmTransitions transitions;
};

using FsmStateEntries = std::unordered_map<TransparentString, std::unique_ptr<FsmStateEntry>, TransparentStringHash, TransparentStringEquals>;
