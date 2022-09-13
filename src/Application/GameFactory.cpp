#include "GameFactory.h"

#include "Game.h"
#include "GameConfig.h"

using Application::Game;
using Application::GameConfig;
using Application::GameFactory;

std::shared_ptr<Game> GameFactory::CreateGame(const std::shared_ptr<GameConfig> &config) {
    auto game = std::make_shared<Game>();
    if (game) {
        if (game->Configure(config)) {
            return game;
        }
    }
    return nullptr;
}
