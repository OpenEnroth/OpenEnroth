#pragma once

#include <memory>
#include <string_view>

#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

using Io::Mouse;

class Engine;
struct DecalBuilder;
class PlatformApplication;
class Platform;
class Menu;
class UiSystem;

class Game {
 public:
    Game(PlatformApplication *application, UiSystem &uiSystem, std::shared_ptr<GameConfig> config);
    ~Game();

    int run();

 private:
    bool loop();
    void processQueuedMessages();
    void gameLoop();
    void closeTargetedSpellWindow();
    void onEscape();
    void onPressSpace();
    void registerInGameCommands();

 private:
    PlatformApplication *_application = nullptr;
    std::shared_ptr<GameConfig> _config;
    DecalBuilder *_decalBuilder = nullptr;
    Menu *_menu = nullptr;
    UiSystem &_uiSystem;
};

void initDataPath(Platform *platform, std::string_view dataPath);

extern GraphicsImage *gamma_preview_image;  // 506E40
