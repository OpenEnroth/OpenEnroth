#pragma once

#include <memory>
#include <string>

#include "src/Application/Game.h"
#include "src/Application/GameConfig.h"

#include "Engine/AssetsLocator.h"


namespace Application {

class GameFactory {
 public:
     std::shared_ptr<Game> CreateGame(
         const std::shared_ptr<const GameConfig> &config,
         std::shared_ptr<AssetsLocator> assets_locator
     );
};

}  // namespace Application
