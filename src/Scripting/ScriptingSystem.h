#pragma once

#include <Library/Logger/LogCategory.h>

#include <string_view>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <sol/sol.hpp>

class LogSink;
class DistLogSink;
class IBindings;
class PlatformApplication;
class ScriptLogSink;

class ScriptingSystem {
 public:
    ScriptingSystem(std::string_view scriptFolder, std::string_view entryPointFile, PlatformApplication &platformApplication);
    ~ScriptingSystem();

    LogSink *scriptingLogSink() const;

    void executeEntryPoint();

    template<typename TBindings, typename ...TArgs>
    void addBindings(std::string_view bindingTableName, TArgs &&... args) {
        auto bindings = std::make_unique<TBindings>(std::forward<TArgs>(args) ...);
        _bindings.insert({ "bindings." + std::string(bindingTableName), std::move(bindings)});
    }

    static LogCategory ScriptingLogCategory;

 private:
    void _initBaseLibraries();
    void _initPackageTable(std::string_view scriptFolder);
    void _initBindingFunction();

    std::unique_ptr<sol::state> _solState;
    std::unique_ptr<LogSink> _scriptingLogSink;
    std::unordered_map<std::string, std::unique_ptr<IBindings>> _bindings;
    std::string _scriptFolder;
    std::string _entryPointFile;
    PlatformApplication &_platformApplication;
};
