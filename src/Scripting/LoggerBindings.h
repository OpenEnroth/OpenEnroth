#pragma once

#include "IBindings.h"

class LoggerBindings : public IBindings {
 public:
    virtual sol::table createBindingTable(sol::state_view &solState) const override;
};
