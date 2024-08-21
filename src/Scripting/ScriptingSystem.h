#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <sol/sol.hpp>

#include "Library/Logger/LogCategory.h"

class LogSink;
class DistLogSink;
class IBindings;
class PlatformApplication;
class ScriptLogSink;
class DistLogSink;

class ScriptingSystem {
 public:
    ScriptingSystem(std::string_view scriptFolder, std::string_view entryPointFile, PlatformApplication &platformApplication, DistLogSink &distLogSink);
    ~ScriptingSystem();

    void executeEntryPoint();

    template<typename TBindings, typename ...TArgs>
    void addBindings(std::string_view bindingTableName, TArgs &&... args) {
        auto bindings = std::make_unique<TBindings>(std::forward<TArgs>(args) ...);
        _bindings.insert({ "bindings." + std::string(bindingTableName), std::move(bindings)});
    }

    static LogCategory ScriptingLogCategory;

 private:
    void _initBaseLibraries();
    void _initPackageTable();
    void _initBindingFunction();

    std::unique_ptr<sol::state> _solState;
    std::unique_ptr<LogSink> _scriptingLogSink;
    std::unordered_map<std::string, std::unique_ptr<IBindings>> _bindings;
    std::string _scriptFolder;
    std::string _entryPointFile;
    PlatformApplication &_platformApplication;
    DistLogSink &_distLogSink;
};
