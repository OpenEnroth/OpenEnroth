#pragma once

#include "IBindings.h"

#include <sol/sol.hpp>

class LoggerBindings : public IBindings {
 public:
    explicit LoggerBindings(const sol::state_view &solState);

    void init() override;

 private:
    sol::state_view _solState;
};
