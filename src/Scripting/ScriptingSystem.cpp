#include "ScriptingSystem.h"
#include "IBindings.h"

#include <Utility/DataPath.h>
#include <Library/Logger/Logger.h>

#include <string>
#include <vector>
#include <memory>
#include <utility>

ScriptingSystem::ScriptingSystem(
    std::string_view scriptFolder,
    const std::vector<std::string> &entryPointFiles
) : _entryPointFiles(entryPointFiles)
  , _scriptFolder(scriptFolder) {
    _initBaseLibraries();
    _initPackageTable(scriptFolder);
    _bindRequireBindingsApi();
}

ScriptingSystem::~ScriptingSystem() {
}

std::unique_ptr<ScriptingSystem> ScriptingSystem::create(
    std::string_view scriptFolder,
    const std::vector<std::string> &entryPointFiles) {
    return std::make_unique<ScriptingSystem>(scriptFolder, entryPointFiles);
}

void ScriptingSystem::executeEntryPoints() {
    for (auto &&entryPointFile : _entryPointFiles) {
        try {
            _solState.safe_script_file(makeDataPath(_scriptFolder, entryPointFile));
        } catch (const sol::error &e) {
            logger->error("[Script] An unexpected error has occurred: ", e.what());
        }
    }
}

void ScriptingSystem::_initBaseLibraries() {
    _solState.open_libraries(
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
    _solState["package"]["path"] = makeDataPath(scriptFolder, "?.lua");
    _solState["package"]["cpath"] = ""; //Reset the path for any c loaders
}

void ScriptingSystem::_addBindings(std::string_view bindingTableName, std::unique_ptr<IBindings> bindings) {
    _solState["require" + std::string(bindingTableName)] = [bindingsPtr = bindings.get()]() {
        return bindingsPtr->getBindingTable();
    };
    _bindings.insert({ bindingTableName.data(), std::move(bindings) });
}

void ScriptingSystem::_bindRequireBindingsApi() {
    /* In lua we can request a binding table in two ways:
        requireBindings("Game") -- by providing the name
        requireGame() -- by calling the direct function */
    _solState["requireBindings"] = [this](std::string_view bindingTableName) {
        if (auto itr = _bindings.find(bindingTableName.data()); itr != _bindings.end()) {
            return sol::make_object(_solState, itr->second->getBindingTable());
        }
        return sol::make_object(_solState, sol::lua_nil);
    };
}
