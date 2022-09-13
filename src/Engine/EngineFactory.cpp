#include "Engine/EngineFactory.h"

using Engine_::EngineFactory;

std::shared_ptr<Engine> EngineFactory::CreateEngine(std::shared_ptr<Application::GameConfig> config) {
    return std::make_shared<Engine>(config);
}
