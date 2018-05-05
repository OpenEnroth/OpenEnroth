#include "Application/GameFactory.h"

#include "Application/Configuration.h"
#include "Application/Game.h"

using Application::GameFactory;
using Application::Configuration;
using Application::Game;

std::shared_ptr<const Configuration> GameFactory::CreateConfiguration(const std::string &command_line) {
    auto config = std::make_shared<Configuration>();
    config->command_line = command_line;

    return config;
}

std::shared_ptr<Game> GameFactory::CreateGame(const std::string &command_line) {
    auto config = CreateConfiguration(command_line);

    auto game = std::make_shared<Game>();
    if (game) {
        if (game->Configure(config)) {
            return game;
        }
    }
    return nullptr;
}
