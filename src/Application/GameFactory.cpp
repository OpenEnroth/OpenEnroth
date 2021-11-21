#include "src/Application/GameFactory.h"

#include "src/Application/Game.h"
#include "src/Application/GameConfig.h"

using Application::Game;
using Application::GameConfig;
using Application::GameFactory;

std::shared_ptr<Game> GameFactory::CreateGame(
    const std::shared_ptr<const GameConfig> &config,
    std::shared_ptr<AssetLocator> asset_locator
) {
    auto game = std::make_shared<Game>();
    if (game) {
        if (game->Configure(config, asset_locator)) {
            return game;
        }
    }
    return nullptr;
}
