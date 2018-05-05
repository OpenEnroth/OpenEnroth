#include "Engine/EngineFactory.h"

#include "Engine/EngineConfigFactory.h"


using Engine_::EngineFactory;
using Engine_::EngineConfig;
using Engine_::EngineConfigFactory;


std::shared_ptr<Engine> EngineFactory::CreateEngine(const std::string &command_line) {
    EngineConfigFactory engineConfigFactory;
    auto config = engineConfigFactory.CreateFromCommandLine(command_line);

    auto engine = std::make_shared<Engine>();
    if (engine) {
        engine->Configure(config);
    }
    return engine;
}
