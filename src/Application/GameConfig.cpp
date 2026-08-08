#include "GameConfig.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Library/Logger/Logger.h"

GameConfig::GameConfig() = default;
GameConfig::~GameConfig() = default;

GameConfig::CheatCommands::CheatCommands(GameConfig *config) : ConfigSection(config, "cheat_commands") {
    // All 40 slots are empty by default. Built-in commands are defined in
    // resources/scripts/dev/cheat_commands_list.lua so they can be reordered
    // without touching config or renumbering entries.
    static const int maxNumberOfCommands = 40;
    for (int i = 0; i < maxNumberOfCommands; ++i) {
        _addCommand(i, "");
    }
}

void GameConfig::CheatCommands::_addCommand(int commandIndex, const std::string& defaultValue) {
    std::string name = fmt::format("command{:02}", commandIndex + 1);
    auto item = std::make_unique<String>(this, name, defaultValue, fmt::format("Cheat command #{}.", commandIndex + 1));
    _commandList.push_back(std::move(item));
}
