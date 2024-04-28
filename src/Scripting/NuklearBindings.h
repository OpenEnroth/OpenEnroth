#pragma once

#include "IBindings.h"

#include <sol/sol.hpp>

class Nuklear;

class NuklearBindings : public IBindings {
 public:
    explicit NuklearBindings(Nuklear *nuklear);
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    Nuklear *_nuklear;
};
