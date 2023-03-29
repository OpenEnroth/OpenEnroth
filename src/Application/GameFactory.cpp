#include "GameFactory.h"

#include "Game.h"
#include "GameConfig.h"

std::shared_ptr<Game> GameFactory::CreateGame(PlatformApplication *app, const std::shared_ptr<GameConfig> &config) {
    auto game = std::make_shared<Game>(app);
    if (game) {
        if (game->Configure(config)) {
            return game;
        }
    }
    return nullptr;
}
