#include <memory>
#include <string>

#include "src/Application/Game.h"
#include "src/Application/GameConfig.h"
#include "src/Application/GameFactory.h"

using Application::Game;
using Application::GameConfig;
using Application::GameFactory;

int MM_Main(const std::string &command_line) {
    std::shared_ptr<GameConfig> gameConfig = std::make_shared<GameConfig>(command_line);
    auto game = GameFactory().CreateGame(gameConfig);
    int result = game->Run();

#ifdef _WINDOWS
    // SDL on Windows creates a separate console window, and we want to be able to actually read the error message
    // before that window closes.
    if (result != 0) {
        printf("[Press any key to close this window]");
        getchar();
    }
#endif

    return result;
}
