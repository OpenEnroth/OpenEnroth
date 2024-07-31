#pragma once

#include <string>

#include "Overlay.h"

#include <sol/sol.hpp>

class ScriptedOverlay: public Overlay {
 public:
    ScriptedOverlay(std::string_view name, sol::state_view &solState, sol::table luaOverlay);
    virtual ~ScriptedOverlay() override;
    virtual void update() override;

 private:
    void _addFunctionToRegistry(sol::table &table, sol::reference& ref, std::string_view functionName);
    sol::protected_function _prepareFunction(sol::reference &functionReference);
    void _setErrorHandler(sol::protected_function &function);
    void _logMissingFunctionWarning(std::string_view functionName);

    std::string _name;
    sol::state_view _solState;
    sol::reference _updateFunctionReference;
    sol::reference _closeFunctionReference;
};
