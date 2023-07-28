#pragma once

#include <memory>

#include "GameStarterOptions.h"

class Platform;
class PlatformLogger;
class PlatformApplication;
class GameConfig;
class Game;

class GameStarter {
 public:
    explicit GameStarter(GameStarterOptions options);
    ~GameStarter();

    PlatformApplication *application() const {
        return _application.get();
    }

    GameConfig *config() const {
        return _config.get();
    }

    void run();

 private:
    static void resolveDefaults(Platform *platform, GameStarterOptions* options);

 private:
    GameStarterOptions _options;
    std::unique_ptr<PlatformLogger> _logger;
    std::unique_ptr<Platform> _platform;
    std::unique_ptr<PlatformApplication> _application;
    std::shared_ptr<GameConfig> _config;
    std::shared_ptr<Game> _game;
};
