#pragma once

#include <memory>
#include <string>

#include "Engine/Engine.h"
#include "Engine/EngineConfig.h"

namespace Engine_ {

class EngineFactory {
 public:
    std::shared_ptr<Engine> CreateEngine(const std::string &command_line);
};

}  // Engine_
