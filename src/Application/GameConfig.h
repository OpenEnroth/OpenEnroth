#pragma once
#include <memory>
#include <string>

#include "Engine/CommandLine.h"

using Engine_::CommandLine;

namespace Application {
    class GameConfig {
     public:
        explicit GameConfig(const std::shared_ptr<CommandLine> comamnd_line);

        std::shared_ptr<CommandLine> command_line;
        std::string renderer_name =
#ifdef _WINDOWS
            "DirectDraw";  //  "OpenGL";
#else
            "OpenGL";
#endif
        int display = 0;
        int fullscreen = 0;
        int borderless = 0;
        int window_width = 640;
        int window_height = 480;
        int window_x = -1;
        int window_y = -1;
    };
}  // namespace Application
