#include "ScriptingSystem.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>

#include "Engine/EngineFileSystem.h"

#include "Library/Logger/Logger.h"
#include "Library/Logger/DistLogSink.h"
#include "Library/Platform/Application/PlatformApplication.h"

#include "Utility/String/Transformations.h"

#include "IBindings.h"
#include "InputScriptEventHandler.h"
#include "ScriptLogSink.h"

LogCategory ScriptingSystem::ScriptingLogCategory("script");

ScriptingSystem::ScriptingSystem(std::string_view scriptFolder, std::string_view entryPointFile, PlatformApplication &platformApplication, DistLogSink &distLogSink)
    : _scriptFolder(scriptFolder), _entryPointFile(entryPointFile), _platformApplication(platformApplication), _distLogSink(distLogSink) {
    _solState = std::make_unique<sol::state>();
    _scriptingLogSink = std::make_unique<ScriptLogSink>(*_solState);
    _platformApplication.installComponent(std::make_unique<InputScriptEventHandler>(*_solState));
    _distLogSink.addLogSink(_scriptingLogSink.get());

    _initBaseLibraries();
    _initPackageTable();
    _initBindingFunction();
}

ScriptingSystem::~ScriptingSystem() {
    _platformApplication.removeComponent<InputScriptEventHandler>();
    _distLogSink.removeLogSink(_scriptingLogSink.get());
}

void ScriptingSystem::executeEntryPoint() {
    // This will throw if we have script errors.
    _solState->script(dfs->read(fmt::format("{}/{}", _scriptFolder, _entryPointFile)).string_view());
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

void ScriptingSystem::_initPackageTable() {
    // Usage in Lua:
    // local gameBindings = require "bindings.game" -- If the module starts with 'bindings.' we try to load/create the binding table.
    // gameBindings.doSomething()
    //
    // Note that sol::as_function is required here because some compilers mangle (mis-mangle?) different lambdas
    // inside the same function using the same signature, and this leads to problems.
    // See https://sol2.readthedocs.io/en/latest/functions.html#working-with-callables-lambdas
    _solState->add_package_loader(sol::as_function([this](const std::string &module) {
        if (module.starts_with("bindings.")) {
            return _solState->load(fmt::format("return _createBindingTable('{}')", module), module).get<sol::object>();
        } else {
            // Note that "\n\t" is needed here so that the error message is properly formatted, see `searchpath`
            // function in lua sources.
            return sol::make_object(*_solState, fmt::format("\n\tno bindings module '{}'", module));
        }
    }));

    // Other scripts are loaded from our virtual FS.
    _solState->add_package_loader(sol::as_function([this](const std::string &module) {
        std::string path = fmt::format("{}/{}.lua", _scriptFolder, replaceAll(module, '.', '/'));
        if (dfs->exists(path)) {
            return _solState->load(dfs->read(path).string_view(), module).get<sol::object>();
        } else {
            return sol::make_object(*_solState, fmt::format("\n\tno file '{}'", dfs->displayPath(path)));
        }
    }));
}

void ScriptingSystem::_initBindingFunction() {
    (*_solState)["_createBindingTable"] = sol::as_function([this](std::string tableName) {
        if (auto itr = _bindings.find(tableName); itr != _bindings.end()) {
            return itr->second->createBindingTable(*_solState);
        }
        logger->warning(ScriptingLogCategory, "Can't find a binding table with name: {}", tableName);
        return _solState->create_table();
    });
}
