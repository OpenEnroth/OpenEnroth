#include "ScriptingSystem.h"

#include <Utility/DataPath.h>
#include <Library/Logger/Logger.h>
#include <Library/Logger/DistLogSink.h>
#include <Library/Platform/Application/PlatformApplication.h>

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "IBindings.h"
#include "InputScriptEventHandler.h"
#include "ScriptLogSink.h"

LogCategory ScriptingSystem::ScriptingLogCategory("Script");

ScriptingSystem::ScriptingSystem(std::string_view scriptFolder, std::string_view entryPointFile, PlatformApplication &platformApplication, DistLogSink &distLogSink)
    : _scriptFolder(scriptFolder), _entryPointFile(entryPointFile), _platformApplication(platformApplication) {
    _solState = std::make_shared<sol::state>();
    _platformApplication.installComponent(std::make_unique<InputScriptEventHandler>(_solState));

    distLogSink.addLogSink(std::make_unique<ScriptLogSink>(_solState));

    _initBaseLibraries();
    _initPackageTable(scriptFolder);
}

ScriptingSystem::~ScriptingSystem() {
    _platformApplication.removeComponent<InputScriptEventHandler>();
}

void ScriptingSystem::executeEntryPoint() {
    try {
        _solState->script_file(makeDataPath(_scriptFolder, _entryPointFile));
    } catch (const sol::error &e) {
        logger->error(ScriptingLogCategory, "An unexpected error has occurred: {}", e.what());
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
}

void ScriptingSystem::_addBindings(std::string_view bindingTableName, std::unique_ptr<IBindings> bindings) {
    (*_solState)[bindingTableName] = bindings->createBindingTable(*_solState);
    (*_solState)["require" + std::string(bindingTableName) + "Bindings"] = [this, bindingTableName]() -> sol::table {
        return (*_solState)[bindingTableName];
    };
    _bindings.insert({ bindingTableName.data(), std::move(bindings) });
}
