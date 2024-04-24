#pragma once

#include <sol/sol.hpp>

class IBindings {
 public:
    virtual ~IBindings() = default;
    virtual sol::table getBindingTable() = 0;
};