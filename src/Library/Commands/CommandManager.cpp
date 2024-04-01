#include "CommandManager.h"
#include <sstream>
#include <utility>

#include "Engine/Party.h"

void test() {
    CommandManager cm;
    cm.addCommand("money1", std::make_unique<ChangeMoneyCommand>());
    cm.addFunction("money2", [](int money) {
        if (pParty != nullptr) {
            pParty->SetGold(money);
        }
        return "";
    });
    cm.addFunction("money3", []() {
        if (pParty != nullptr) {
            pParty->SetGold(2000);
        }
        return "";
    });
    cm.execute("money 100");
}

std::string ChangeMoneyCommand::run(int money, float f, std::string t) {
    if (pParty != nullptr) {
        pParty->SetGold(money + f);
        return t;
    }
    return "";
}

void CommandManager::addCommand(const std::string& commandName, std::unique_ptr<ICommand> command) {
    _commands.emplace(std::make_pair(commandName, std::move(command)));
}

std::string CommandManager::execute(const std::string & commandLine) {
    auto commandInfo = parseCommandLine(commandLine);
    if (const auto itr = _commands.find(std::get<0>(commandInfo)); itr != _commands.end()) {
        return (itr->second)->run(std::get<1>(commandInfo));
    }

    return "Can't find command: " + std::get<0>(commandInfo);
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
