#pragma once

#include "IBindings.h"

class PlatformApplication;

class PlatformBindings : public IBindings {
 public:
    explicit PlatformBindings(const sol::state_view &solState, PlatformApplication &platformApplication);
    ~PlatformBindings();

    sol::table getBindingTable() override;

 private:
    sol::state_view _solState;
    sol::table _bindingTable;
    PlatformApplication &_platformApplication;
};
