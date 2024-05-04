#pragma once

#include <string_view>

class FSMTransitionHandler {
 public:
    virtual void executeTransition(std::string_view transitionName) = 0;
};
