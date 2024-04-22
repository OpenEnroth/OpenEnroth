#pragma once

#include <unordered_map>
#include <vector>
#include <sol/sol.hpp>

#include "Library/Platform/Filters/PlatformEventFilter.h"

class InputScriptEventHandler : public PlatformEventFilter {
 public:
    InputScriptEventHandler();
    bool keyPressEvent(const PlatformKeyEvent *event) override;
    void registerKeyPress(PlatformKey key, const sol::function &callback);
    void unregisterKeyPress(PlatformKey key, const sol::function &callbackToRemove);

 private:
    std::unordered_map<PlatformKey, std::vector<sol::function>> _keyPressCallbacks;
};
