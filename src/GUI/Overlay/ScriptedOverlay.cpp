#include "ScriptedOverlay.h"

#include <Library/Logger/Logger.h>
#include <Scripting/ScriptingSystem.h>

#include <nuklear_config.h> // NOLINT: not a C system header.

ScriptedOverlay::ScriptedOverlay(sol::state_view &solState, sol::table luaOverlay)
    : _solState(solState) {
    _addFunctionToRegistry(luaOverlay, _closeFunctionReference, "close");
    _addFunctionToRegistry(luaOverlay, _updateFunctionReference, "update");
    sol::protected_function initFunction = luaOverlay["init"];
    if (initFunction.valid()) {
        _setErrorHandler(initFunction);
        initFunction();
    }
}

ScriptedOverlay::~ScriptedOverlay() {
    auto closeFunction = _prepareFunction(_closeFunctionReference);
    if (closeFunction.valid()) {
        closeFunction();
    }
}

void ScriptedOverlay::update(nk_context &context) {
    auto updateFunction = _prepareFunction(_updateFunctionReference);
    if (updateFunction.valid()) {
        updateFunction(&context);
    }
}

void ScriptedOverlay::_addFunctionToRegistry(sol::table &table, sol::reference &ref, const char *functionName) {
    if (sol::protected_function function = table[functionName]; function.valid()) {
        ref = sol::make_reference(_solState, function);
    }
}

sol::protected_function ScriptedOverlay::_prepareFunction(sol::reference &functionReference) {
    sol::table registry = _solState.registry();
    sol::protected_function updateFunction = registry[functionReference.registry_index()];
    _setErrorHandler(updateFunction);
    return updateFunction;
}

void ScriptedOverlay::_setErrorHandler(sol::protected_function &function) {
    sol::function errorHandler = _solState["_globalErrorHandler"];
    if (errorHandler.valid()) {
        function.set_error_handler(errorHandler);
    }
}
