#pragma once

#include <memory>

class Engine;
class GameConfig;

class EngineFactory {
 public:
    std::shared_ptr<Engine> CreateEngine(std::shared_ptr<GameConfig> config);
};
