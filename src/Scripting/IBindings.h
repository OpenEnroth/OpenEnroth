#pragma once

#include <sol/sol.hpp>

class IBindings {
 public:
    virtual ~IBindings() = default;
    virtual sol::table createBindingTable(sol::state_view &solState) const = 0;
};