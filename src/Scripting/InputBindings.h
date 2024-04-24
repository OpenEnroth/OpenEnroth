#pragma once

#include "IBindings.h"

class InputScriptEventHandler;

class InputBindings : public IBindings {
 public:
    explicit InputBindings(const sol::state_view &solState, InputScriptEventHandler &inputScriptEventHandler);
    sol::table getBindingTable() override;

 private:
    void _fillTableWithEnums();

    sol::state_view _solState;
    sol::table _bindingTable;
    InputScriptEventHandler &_inputScriptEventHandler;
};
