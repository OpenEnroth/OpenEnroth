#pragma once

#include <memory>
#include <string>

#include "Engine/Engine.h"
#include "Engine/Configuration.h"

namespace Engine_ {

class EngineFactory {
 public:
    std::shared_ptr<Engine> CreateEngine(const std::string &command_line);

 private:
    std::shared_ptr<Configuration> CreateDefaultConfiguration();
    std::shared_ptr<Configuration> MergeCommandLine(
        std::shared_ptr<Configuration> config,
        const std::string &command_line
    );
};

}  // Engine_
