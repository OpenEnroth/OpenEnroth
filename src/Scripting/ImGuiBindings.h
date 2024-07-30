#pragma once

#include <sol/sol.hpp>

class ImGuiBindings {
 public:
    static void Init(sol::state_view &solState, sol::table &table);
};
