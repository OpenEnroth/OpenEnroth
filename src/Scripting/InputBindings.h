#pragma once

#include "IBindings.h"

class InputScriptEventHandler;

class InputBindings : public IBindings {
 public:
    explicit InputBindings(InputScriptEventHandler &inputScriptEventHandler);
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    void _fillTableWithEnums(sol::table &table) const;

    InputScriptEventHandler &_inputScriptEventHandler;
};
