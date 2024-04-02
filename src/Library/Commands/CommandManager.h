#pragma once

#include "Command.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <tuple>
#include <utility>
#include <functional>

struct CommandConfig {
    std::vector<std::string> defaultValues;
};

ExecuteResult commandSuccess(const std::string &message = "");
ExecuteResult commandFailure(const std::string &message = "");

typedef std::function<void(const std::string&, const std::string&, bool)> CommandExecutedCallback;

class CommandManager {
 public:
    void addCommand(const std::string &commandName, std::unique_ptr<ICommand> command, const std::vector<std::string> &defaultValues);
    template<typename Func>
    void addFunction(const std::string &commandName, Func &&func, const std::vector<std::string> &defaultValues) {
        addCommand(commandName, make_command_func(std::forward<Func>(func), &Func::operator()), defaultValues);
    }

    ExecuteResult execute(const std::string &commandLine);
    int onCommandExecuted(CommandExecutedCallback callback);
    void removeOnCommandExecutedCallback(int handle);

 private:
    std::tuple<std::string, std::vector<std::string>> parseCommandLine(const std::string &str);
    void adjustDefaultParameters(std::vector<std::string> &parameters, const std::vector<std::string> &defaultValues);

    template<typename Func, typename Ret, typename Class, typename... Args>
    std::unique_ptr<ICommand> make_command_func(Func &&func, Ret(Class::*)(Args...) const) {
        return std::make_unique<TCommandFunc<Func, Args...>>(std::forward<Func>(func));
    }

    struct CommandEntry {
        std::unique_ptr<ICommand> command;
        std::vector<std::string> defaultValues;
    };

    std::unordered_map<std::string, CommandEntry> _commands;
    std::unordered_map<int, CommandExecutedCallback> _onCommandExecutedCallbacks;
};
