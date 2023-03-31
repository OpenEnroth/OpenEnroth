#include "Engine/EngineFactory.h"

#include "Engine.h"

std::shared_ptr<Engine> EngineFactory::CreateEngine(std::shared_ptr<GameConfig> config) {
    return std::make_shared<Engine>(config);
}
