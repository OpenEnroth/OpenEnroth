#pragma once

#include "IBindings.h"

#include <vector>

class OverlaySystem;

class OverlayBindings : public IBindings {
 public:
    explicit OverlayBindings(OverlaySystem &overlaySystem);
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    OverlaySystem &_overlaySystem;
};
