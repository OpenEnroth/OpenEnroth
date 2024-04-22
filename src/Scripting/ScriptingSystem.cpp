#include "ScriptingSystem.h"
#include "IBindings.h"

#include <Utility/DataPath.h>
#include <Library/Logger/Logger.h>

#include <string>
#include <vector>

ScriptingSystem::ScriptingSystem(
    std::string_view scriptFolder,
    const std::vector<std::string> &entryPointFiles
) {
    _initBaseLibraries();
    _initRequireTable(scriptFolder);
}

ScriptingSystem::~ScriptingSystem() {
}

void ScriptingSystem::start() {
    _bindSetupFunction();
    _runEntryPoints();
}

void ScriptingSystem::_runEntryPoints() {
    for (auto &&entryPointFile : _entryPointFiles) {
        try {
            _solState.safe_script_file(makeDataPath("scripts", entryPointFile));
            sol::function init = _solState["init"];
            init();
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

void ScriptingSystem::_initRequireTable(std::string_view scriptFolder) {
    _solState["package"]["path"] = makeDataPath(scriptFolder, "?.lua");
    _solState["package"]["cpath"] = ""; //Reset the path for any c loaders
}

void ScriptingSystem::_bindSetupFunction() {
    _solState["setupBindings"] = [this]() {
        for (auto &&bindings : _bindings) {
            bindings->init();
        }
    };
}
