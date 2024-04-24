#pragma once

#include "IBindings.h"

#include <sol/sol.hpp>

class Nuklear;

class NuklearBindings : public IBindings {
 public:
    NuklearBindings(const sol::state_view &solState, Nuklear *nuklear);
    sol::table getBindingTable() override;

 private:
    Nuklear *_nuklear;
    sol::table _bindingTable;
};
