#pragma once

#include "IBindings.h"

class DebugViewBindings : public IBindings {
 public:
    explicit DebugViewBindings();
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

};
