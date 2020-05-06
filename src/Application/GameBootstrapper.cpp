#include <memory>
#include <string>

#include "src/Application/Game.h"
#include "src/Application/GameConfig.h"
#include "src/Application/GameFactory.h"

using Application::Game;
using Application::GameConfig;
using Application::GameFactory;

int MM_Main(const std::string &command_line) {
    std::shared_ptr<const GameConfig> gameConfig = std::make_shared<GameConfig>(command_line);
    auto game = GameFactory().CreateGame(gameConfig);
    game->Run();

    return 0;
}
