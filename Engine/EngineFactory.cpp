#include "Engine/EngineFactory.h"

#include "Engine/EngineConfigFactory.h"


using Engine_::CommandLine;
using Engine_::EngineFactory;
using Engine_::EngineConfig;
using Engine_::EngineConfigFactory;


std::shared_ptr<Engine> EngineFactory::CreateEngine(std::shared_ptr<CommandLine> command_line) {
    EngineConfigFactory engineConfigFactory;
    auto config = engineConfigFactory.Create(command_line);

    auto engine = std::make_shared<Engine>();
    if (engine) {
        if (engine->Configure(config)) {
            return engine;
        }
    }
    return nullptr;
}
