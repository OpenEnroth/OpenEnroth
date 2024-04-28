#pragma once

#include "IBindings.h"

class PlatformApplication;

class PlatformBindings : public IBindings {
 public:
    explicit PlatformBindings(PlatformApplication &platformApplication);
    ~PlatformBindings();

    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    PlatformApplication &_platformApplication;
};
