#include "src/Application/GameConfig.h"

using Application::GameConfig;

GameConfig::GameConfig(const std::string& command_line_str) {
    command_line = std::make_shared<CommandLine>(command_line_str);

    std::shared_ptr<std::string> value;
    if (command_line->TryGetValue("render", &value)) {
        renderer_name = *value;
    }
    if (command_line->TryGetValue("display", &value)) {
        display = std::stoi(*value);
    }
    if (command_line->TryGetValue("window_width", &value)) {
        window_width = std::stoi(*value);
    }
    if (command_line->TryGetValue("window_height", &value)) {
        window_height = std::stoi(*value);
    }
    if (command_line->TryGetValue("window_x", &value)) {
        window_x = std::stoi(*value);
    }
    if (command_line->TryGetValue("window_y", &value)) {
        window_y = std::stoi(*value);
    }
    if (command_line->TryGetValue("fullscreen", &value)) {
        fullscreen = std::stoi(*value);
    }
    if (command_line->TryGetValue("borderless", &value)) {
        borderless = std::stoi(*value);
    }
}
