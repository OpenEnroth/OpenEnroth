#pragma once

#include <string_view>

class FSMTransitionHandler {
 public:
    virtual void scheduleTransition(std::string_view transitionName) = 0;
};
