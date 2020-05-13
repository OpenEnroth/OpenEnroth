#include "src/Application/GameConfig.h"

using Application::GameConfig;

GameConfig::GameConfig(const std::string& command_line_str) {
    command_line = std::make_shared<CommandLine>(command_line_str);

    std::shared_ptr<std::string> value;
    if (command_line->TryGetValue("render", &value)) {
        renderer_name = *value;
    }
    if (command_line->TryGetValue("game_window_width", &value)) {
        game_window_width = std::stoi(*value);
    }
    if (command_line->TryGetValue("game_window_height", &value)) {
        game_window_height = std::stoi(*value);
    }
}
