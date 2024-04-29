#pragma once

#include "Overlay.h"
#include <sol/sol.hpp>

class ScriptedOverlay: public Overlay {
 public:
    ScriptedOverlay(sol::state_view &solState, sol::table luaOverlay);
    virtual ~ScriptedOverlay() override;
    virtual void update(nk_context &context) override;

 private:
    void _addFunctionToRegistry(sol::table &table, sol::reference& ref, const char* functionName);
    sol::protected_function _prepareFunction(sol::reference &functionReference);
    void _setErrorHandler(sol::protected_function &function);

    sol::state_view _solState;
    sol::reference _updateFunctionReference;
    sol::reference _closeFunctionReference;
};
