#pragma once
#include <memory>

#include "Engine/Engine.h"

namespace Engine_ {
    class EngineFactory {
     public:
        std::shared_ptr<Engine> CreateEngine(std::shared_ptr<Application::GameConfig> config);
    };
}  // Engine_
