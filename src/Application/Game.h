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

class Engine;
class GameConfig;
class Logger;
class Menu;
class Nuklear;
class Vis;
namespace Io {
class Mouse;
}  // namespace Io
struct DecalBuilder;


using Io::Mouse;

class IRender;
class Platform;
class PlatformApplication;
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
    std::unique_ptr<NuklearEventHandler> _nuklearHandler;
    std::unique_ptr<Engine> _engine;
    std::shared_ptr<IRender> _render;
    std::shared_ptr<Mouse> _mouse = nullptr;
    Logger *_log = nullptr;
    DecalBuilder *_decalBuilder = nullptr;
    Vis *_vis = nullptr;
    Menu *_menu = nullptr;
    std::shared_ptr<Nuklear> _nuklear = nullptr;
};

void initDataPath(const std::string &dataPath);

extern class GraphicsImage *gamma_preview_image;  // 506E40
