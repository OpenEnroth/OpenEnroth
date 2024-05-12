#include "ScriptedOverlay.h"

#include "Library/Logger/Logger.h"
#include "Scripting/ScriptingSystem.h"

ScriptedOverlay::ScriptedOverlay(std::string_view name, sol::state_view &solState, sol::table luaOverlay)
    : _name(name), _solState(solState) {
    _addFunctionToRegistry(luaOverlay, _closeFunctionReference, "close");
    _addFunctionToRegistry(luaOverlay, _updateFunctionReference, "update");
    sol::protected_function initFunction = luaOverlay["init"];
    if (initFunction.valid()) {
        _setErrorHandler(initFunction);
        initFunction();
    } else {
        _logMissingFunctionWarning("init");
    }
}

ScriptedOverlay::~ScriptedOverlay() {
    auto closeFunction = _prepareFunction(_closeFunctionReference);
    if (closeFunction.valid()) {
        closeFunction();
    }
}

void ScriptedOverlay::update() {
    auto updateFunction = _prepareFunction(_updateFunctionReference);
    if (updateFunction.valid()) {
        updateFunction();
    }
}

void ScriptedOverlay::_addFunctionToRegistry(sol::table &table, sol::reference &ref, std::string_view functionName) {
    if (sol::protected_function function = table[functionName]; function.valid()) {
        ref = sol::make_reference(_solState, function);
    } else {
        _logMissingFunctionWarning(functionName);
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

void ScriptedOverlay::_logMissingFunctionWarning(std::string_view functionName) {
    logger->warning(ScriptingSystem::ScriptingLogCategory, "Missing [{}] function for the Scripted Overlay: {}", functionName, _name);
}
