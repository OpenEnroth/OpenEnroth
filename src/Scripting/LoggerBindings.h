#pragma once

#include "IBindings.h"

#include <functional>
#include <sol/sol.hpp>

class DistLogSink;

class LoggerBindings : public IBindings {
 public:
    explicit LoggerBindings(const sol::state_view &solState, DistLogSink& distLogSink);
    ~LoggerBindings();

    virtual sol::table getBindingTable() override;

 private:
    sol::state_view _solState;
    sol::table _bindingTable;
    std::function<void()> _unregisterLogSink;
};
