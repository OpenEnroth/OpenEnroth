#pragma once
#include <memory>

#include "Engine/CommandLine.h"
#include "Engine/Engine.h"

namespace Engine_ {
    class EngineFactory {
     public:
        std::shared_ptr<Engine> CreateEngine(std::shared_ptr<CommandLine> command_line);
    };
}  // Engine_
