#pragma once

#include <memory>
#include <string>

#include "GameConfig.h"
#include "GameMenu.h"
#include "GameIocContainer.h"

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"

#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"


using Io::Mouse;

class IRender;
class Platform;
class PlatformApplication;
class EngineTracer;
class GameTraceHandler;
class NuklearEventHandler;

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
    std::unique_ptr<GameWindowHandler> _windowHandler;
    std::unique_ptr<NuklearEventHandler> _nuklearHandler;
    std::shared_ptr<Engine> _engine;
    std::shared_ptr<IRender> _render;
    std::shared_ptr<Mouse> _mouse = nullptr;
    Logger *_log = nullptr;
    DecalBuilder *_decalBuilder = nullptr;
    Vis *_vis = nullptr;
    Menu *_menu = nullptr;
    std::shared_ptr<Nuklear> _nuklear = nullptr;
};

void initDataPath(const std::string &dataPath);

extern class Image *gamma_preview_image;  // 506E40
