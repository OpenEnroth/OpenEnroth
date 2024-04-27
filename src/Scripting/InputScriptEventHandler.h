#pragma once

#include <Library/Platform/Filters/PlatformEventFilter.h>
#include <memory>
#include <sol/sol.hpp>

class InputScriptEventHandler : public PlatformEventFilter {
 public:
    explicit InputScriptEventHandler(std::weak_ptr<sol::state_view> solState);
    bool keyPressEvent(const PlatformKeyEvent *event) override;
 private:
    std::weak_ptr<sol::state_view> _solState;
};
