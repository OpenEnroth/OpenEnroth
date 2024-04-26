#include "ScriptingSystem.h"
#include "IBindings.h"

#include <Utility/DataPath.h>
#include <Library/Logger/Logger.h>

#include <string>
#include <vector>
#include <memory>
#include <utility>

static LogCategory scriptingLogCategory("Script");

ScriptingSystem::ScriptingSystem(
    std::string_view scriptFolder,
    std::string_view entryPointFile
) : _scriptFolder(scriptFolder)
  , _entryPointFile(entryPointFile) {
    _initBaseLibraries();
    _initPackageTable(scriptFolder);
}

void ScriptingSystem::executeEntryPoints() {
    try {
        _solState.script_file(makeDataPath(_scriptFolder, _entryPointFile));
    } catch (const sol::error &e) {
        logger->error(scriptingLogCategory, "An unexpected error has occurred: {}", e.what());
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
    _solState["require" + std::string(bindingTableName) + "Bindings"] = [bindingsPtr = bindings.get()]() {
        return bindingsPtr->getBindingTable();
    };
    _bindings.insert({ bindingTableName.data(), std::move(bindings) });
}
