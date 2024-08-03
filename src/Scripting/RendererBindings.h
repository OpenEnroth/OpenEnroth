#pragma once

#include "IBindings.h"

class RendererBindings : public IBindings {
 public:
    virtual sol::table createBindingTable(sol::state_view &solState) const override;
};
