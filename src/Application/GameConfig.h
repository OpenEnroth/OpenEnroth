#pragma once
#include <string>

#include "Engine/CommandLine.h"

using Engine_::CommandLine;

namespace Application {
    class GameConfig {
    public:
        GameConfig(const std::string& comamnd_line);

        std::shared_ptr<CommandLine> command_line;
        std::string renderer_name = "OpenGL"; // "DirectDraw"
        int game_window_width = 640;
        int game_window_height = 480;
    };
}
