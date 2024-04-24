#pragma once

#include "IBindings.h"

#include <sol/sol.hpp>

class DistLogSink;

class LoggerBindings : public IBindings {
 public:
    explicit LoggerBindings(const sol::state_view &solState, DistLogSink& distLogSink);

    sol::table getBindingTable() override;

 private:
    sol::state_view _solState;
    sol::table _bindingTable;
};
