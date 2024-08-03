#include "ScriptingSystem.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>

#include "Library/Logger/Logger.h"
#include "Library/Logger/DistLogSink.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Utility/DataPath.h"

#include "IBindings.h"
#include "InputScriptEventHandler.h"
#include "ScriptLogSink.h"

LogCategory ScriptingSystem::ScriptingLogCategory("Script");

ScriptingSystem::ScriptingSystem(std::string_view scriptFolder, std::string_view entryPointFile, PlatformApplication &platformApplication, DistLogSink &distLogSink)
    : _scriptFolder(scriptFolder), _entryPointFile(entryPointFile), _platformApplication(platformApplication), _distLogSink(distLogSink) {
    _solState = std::make_unique<sol::state>();
    _scriptingLogSink = std::make_unique<ScriptLogSink>(*_solState);
    _platformApplication.installComponent(std::make_unique<InputScriptEventHandler>(*_solState));
    _distLogSink.addLogSink(_scriptingLogSink.get());

    _initBaseLibraries();
    _initPackageTable(scriptFolder);
    _initBindingFunction();
}

ScriptingSystem::~ScriptingSystem() {
    _platformApplication.removeComponent<InputScriptEventHandler>();
    _distLogSink.removeLogSink(_scriptingLogSink.get());
}

void ScriptingSystem::executeEntryPoint() {
    try {
        _solState->script_file(makeDataPath(_scriptFolder, _entryPointFile));
    } catch (const sol::error &e) {
        logger->warning(ScriptingLogCategory, "An unexpected error has occurred: {}", e.what());
    }
}

void ScriptingSystem::_initBaseLibraries() {
    _solState->open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::os,
        sol::lib::package,
        sol::lib::table,
        sol::lib::math,
        sol::lib::string,
        sol::lib::debug,
        sol::lib::bit32,
        sol::lib::jit
    );
}

void ScriptingSystem::_initPackageTable(std::string_view scriptFolder) {
    sol::table packageTable = (*_solState)["package"];
    packageTable["path"] = makeDataPath(scriptFolder, "?.lua");
    packageTable["cpath"] = ""; //Reset the path for any c loaders

    // Usage in Lua:
    // local gameBindings = require "bindings.game" -- If the module starts with 'bindings.' we try to load/create the binding table.
    // gameBindings.doSomething()
    _solState->add_package_loader([this](const std::string &module) {
        if (module.starts_with("bindings.")) {
            return _solState->load(fmt::format("return _createBindingTable('{}')", module), module).get<sol::object>();
        } else {
            // Note that "\n\t" is needed here so that the error message is properly formatted, see `searchpath`
            // function in lua sources.
            return sol::make_object(*_solState, fmt::format("\n\tno bindings module '{}'", module));
        }
    });
}

void ScriptingSystem::_initBindingFunction() {
    (*_solState)["_createBindingTable"] = [this](std::string tableName) {
        if (auto itr = _bindings.find(tableName); itr != _bindings.end()) {
            return itr->second->createBindingTable(*_solState);
        }
        logger->warning(ScriptingLogCategory, "Can't find a binding table with name: {}", tableName);
        return _solState->create_table();
    };
}
