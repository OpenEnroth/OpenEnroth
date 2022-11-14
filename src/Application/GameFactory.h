#pragma once

#include <memory>
#include <string>

#include "Game.h"
#include "GameConfig.h"

class Platform;

namespace Application {

class GameFactory {
 public:
     std::shared_ptr<Game> CreateGame(Platform *platform, const std::shared_ptr<GameConfig> &config);
};

}  // namespace Application
