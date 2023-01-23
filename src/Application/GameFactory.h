#pragma once

#include <memory>
#include <string>

#include "Game.h"
#include "GameConfig.h"

class PlatformApplication;

namespace Application {

class GameFactory {
 public:
     std::shared_ptr<Game> CreateGame(PlatformApplication *app, const std::shared_ptr<GameConfig> &config);
};

}  // namespace Application
