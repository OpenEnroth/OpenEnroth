#pragma once

#include <unordered_map>
#include <vector>
#include <sol/sol.hpp>

#include "Library/Platform/Filters/PlatformEventFilter.h"

class InputScriptEventHandler : public PlatformEventFilter {
 public:
    InputScriptEventHandler();
    bool keyPressEvent(const PlatformKeyEvent *event) override;

    typedef std::function<sol::function(std::string_view)> ScriptFunctionProvider;
    void setScriptFunctionProvider(const ScriptFunctionProvider &scriptFunctionProvider);

 private:
    ScriptFunctionProvider _scriptFunctionProvider;
};
