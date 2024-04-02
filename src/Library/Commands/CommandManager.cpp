#include "CommandManager.h"
#include <sstream>
#include <utility>

#include "Engine/Party.h"

void test() {
    CommandManager cm;
    cm.addCommand("money1", std::make_unique<ChangeMoneyCommand>(), {});
    cm.addFunction("money2", [](int money) {
        if (pParty != nullptr) {
            pParty->SetGold(money);
        }
        return commandSuccess();
    }, {});
    cm.addFunction("money3", []() {
        if (pParty != nullptr) {
            pParty->SetGold(2000);
        }
        return commandSuccess();
    }, {});
    cm.execute("money 100");
}

ExecuteResult ChangeMoneyCommand::run(int money, float f, std::string t) {
    if (pParty != nullptr) {
        pParty->SetGold(money + f);
        return { t, true };
    }
    return { "", true };
}

ExecuteResult commandSuccess(const std::string& message) {
    return std::make_tuple(message, true);
}

ExecuteResult commandFailure(const std::string& message) {
    return std::make_tuple(message, false);
}

void CommandManager::addCommand(const std::string& commandName, std::unique_ptr<ICommand> command, const std::vector<std::string> &defaultValues) {
    _commands.emplace(std::make_pair(commandName, CommandEntry{ std::move(command), defaultValues }));
}

int CommandManager::onCommandExecuted(CommandExecutedCallback callback) {
    static int handle = 0;
    _onCommandExecutedCallbacks.insert(std::make_pair(++handle, callback));
    return handle;
}

void CommandManager::removeOnCommandExecutedCallback(int handle) {
    if (auto itr = _onCommandExecutedCallbacks.find(handle); itr != _onCommandExecutedCallbacks.end()) {
        _onCommandExecutedCallbacks.erase(itr);
    }
}

ExecuteResult CommandManager::execute(const std::string &commandLine) {
    ExecuteResult result;
    auto commandInfo = parseCommandLine(commandLine);
    if (const auto itr = _commands.find(std::get<0>(commandInfo)); itr != _commands.end()) {
        adjustDefaultParameters(std::get<1>(commandInfo), itr->second.defaultValues);
        result = (itr->second.command)->run(std::get<1>(commandInfo));
    } else {
        result = commandFailure("Can't find command: " + std::get<0>(commandInfo));
    }

    for (auto&& callbackPair : _onCommandExecutedCallbacks) {
        callbackPair.second(std::get<0>(result), commandLine, std::get<1>(result));
    }
    return result;
}

std::tuple<std::string, std::vector<std::string>> CommandManager::parseCommandLine(const std::string &str) {
    std::tuple<std::string, std::vector<std::string>> result;
    std::stringstream stream(str);
    std::string temp;
    int i = 0;
    while(std::getline(stream, temp, ' ')) {
        if (!std::isspace(temp[0])) {
            if (i == 0) {
                std::get<0>(result) = temp;
            } else {
                std::get<1>(result).push_back(temp);
            }
            ++i;
        }
    }

    return result;
}

void CommandManager::adjustDefaultParameters(std::vector<std::string> &parameters, const std::vector<std::string> &defaultValues) {
    for (int i = 0; i < defaultValues.size(); ++i) {
        if (i >= parameters.size()) {
            parameters.push_back(defaultValues[i]);
        } else if(parameters[i].empty()) {
            parameters[i] = defaultValues[i];
        }
    }
}
