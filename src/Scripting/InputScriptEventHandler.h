#pragma once

#include <sol/sol.hpp>

#include "Library/Platform/Filters/PlatformEventFilter.h"

class InputScriptEventHandler : public PlatformEventFilter {
 public:
    explicit InputScriptEventHandler(sol::state_view &solState);
    bool keyPressEvent(const PlatformKeyEvent *event) override;
 private:
    sol::state_view _solState;
};
