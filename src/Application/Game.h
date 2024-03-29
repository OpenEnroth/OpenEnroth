#pragma once

#include <memory>
#include <string>

#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

using Io::Mouse;

class Engine;
class DecalBuilder;
class PlatformApplication;
class Platform;
class Menu;

class Game {
 public:
    Game(PlatformApplication *application, std::shared_ptr<GameConfig> config);
    ~Game();

    int run();

 private:
    bool loop();
    void processQueuedMessages();
    void gameLoop();
    void closeTargetedSpellWindow();
    void onEscape();
    void onPressSpace();

 private:
    PlatformApplication *_application = nullptr;
    std::shared_ptr<GameConfig> _config;
    DecalBuilder *_decalBuilder = nullptr;
    Menu *_menu = nullptr;
};

void initDataPath(Platform *platform, const std::string &dataPath);

extern class GraphicsImage *gamma_preview_image;  // 506E40
