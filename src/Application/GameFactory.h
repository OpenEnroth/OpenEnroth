#pragma once

#include <memory>
#include <string>

#include "Game.h"
#include "GameConfig.h"

namespace Application {

class GameFactory {
 public:
     std::shared_ptr<Game> CreateGame(const std::shared_ptr<GameConfig> &config);
};

}  // namespace Application
