#pragma once

#include <sol/state_view.hpp>

#include "Library/Platform/Filters/PlatformEventFilter.h"

class InputScriptEventHandler : public PlatformEventFilter {
 public:
    explicit InputScriptEventHandler(sol::state_view solState);

    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;

 private:
    sol::state_view _solState;
};
