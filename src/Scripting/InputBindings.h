#pragma once

#include "IBindings.h"

#include <sol/sol.hpp>

class InputScriptEventHandler;

class InputBindings : public IBindings {
 public:
    explicit InputBindings(const sol::state_view &solState, InputScriptEventHandler &inputScriptEventHandler);

    void init() override;

 private:
    sol::state_view _solState;
    InputScriptEventHandler &_inputScriptEventHandler;
};
