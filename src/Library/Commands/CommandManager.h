#pragma once

#include "Command.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <tuple>
#include <utility>

struct CommandConfig {
    std::vector<std::string> defaultValues;
};

class CommandManager {
 public:
    void addCommand(const std::string& commandName, std::unique_ptr<ICommand> command);
    template<typename Func>
    void addFunction(const std::string& commandName, Func&& func) {
        addCommand(commandName, make_command_func(std::forward<Func>(func), &Func::operator()));
    }
    std::string execute(const std::string &commandLine);

 private:
    std::tuple<std::string, std::vector<std::string>> parseCommandLine(const std::string& str);

    template<typename Func, typename Ret, typename Class, typename... Args>
    std::unique_ptr<ICommand> make_command_func(Func&& func, Ret(Class::*)(Args...) const) {
        return std::make_unique<TCommandFunc<Func, Args...>>(std::forward<Func>(func));
    }

    std::unordered_map<std::string, std::unique_ptr<ICommand>> _commands;
};
