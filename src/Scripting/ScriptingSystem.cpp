#include "ScriptingSystem.h"
#include "IBindings.h"

#include <Utility/DataPath.h>

ScriptingSystem::ScriptingSystem(std::string_view scriptFolder) {
    _initBaseLibraries();
    _initRequireTable(scriptFolder);
    _bindSetupFunction();
}

ScriptingSystem::~ScriptingSystem() {
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
    _solState["package"]["cpath"] = ""; //Reset the path for c loader. We're not interested in c lib right now.
}

void ScriptingSystem::_bindSetupFunction() {
    _solState["setupBindings"] = [this]() {
        for (auto &&bindings : _bindings) {
            bindings->init();
        }
    };
}
