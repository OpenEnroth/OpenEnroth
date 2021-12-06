#include "src/Application/GameConfig.h"

using Application::GameConfig;

GameConfig::GameConfig(const std::string& command_line_str) {
    command_line = std::make_shared<CommandLine>(command_line_str);

}
