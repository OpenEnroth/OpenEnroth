#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <sol/sol.hpp>

class IBindings;

class ScriptingSystem {
 public:
    ScriptingSystem(
        std::string_view scriptFolder,
        const std::vector<std::string> &entryPointFiles
    );
    ~ScriptingSystem();

    void start();

    template<typename TBindings, typename ...TArgs>
    void addBindings(TArgs &&... args) {
        auto bindings = std::make_unique<TBindings>(_solState, std::forward<TArgs>(args) ...);
        _bindings.push_back(std::move(bindings));
    }

 private:
    void _initBaseLibraries();
    void _initRequireTable(std::string_view scriptFolder);
    void _bindSetupFunction();
    void _runEntryPoints();

    sol::state _solState;
    std::vector<std::unique_ptr<IBindings>> _bindings;
    std::vector<std::string> _entryPointFiles;
};
