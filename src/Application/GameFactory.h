#pragma once

#include <memory>
#include <string>

#include "src/Application/Game.h"
#include "src/Application/GameConfig.h"

namespace Application {

class GameFactory {
 public:
     std::shared_ptr<Game> CreateGame(const std::shared_ptr<const GameConfig> &config);
};

}  // namespace Application
