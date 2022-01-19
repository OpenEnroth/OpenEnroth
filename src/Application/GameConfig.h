#pragma once
#include <memory>
#include <string>

#include "Engine/CommandLine.h"

using Engine_::CommandLine;

namespace Application {
    class GameConfig {
     public:
        explicit GameConfig(const std::string& comamnd_line);

        std::shared_ptr<CommandLine> command_line;
    };
}  // namespace Application
