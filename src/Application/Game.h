#pragma once

#include <memory>

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
    explicit Game(PlatformApplication *app);
    ~Game();

    bool Configure(std::shared_ptr<GameConfig> config) {
        this->config = config;
        return true;
    }

    int Run();

 private:
    bool Loop();
    void EventLoop();
    void GameLoop();
    void CloseTargetedSpellWindow();
    void OnEscape();
    void OnPressSpace();

    PlatformApplication *app = nullptr;
    std::shared_ptr<GameConfig> config;
    std::unique_ptr<GameWindowHandler> windowHandler;
    std::unique_ptr<NuklearEventHandler> nuklearHandler;
    std::shared_ptr<Engine> engine;
    std::shared_ptr<IRender> render;
    std::shared_ptr<Mouse> mouse = nullptr;
    Logger *log = nullptr;
    DecalBuilder *decal_builder = nullptr;
    Vis *vis = nullptr;
    Menu *menu = nullptr;
    std::shared_ptr<Nuklear> nuklear = nullptr;
};

void AutoInitDataPath(Platform *platform);

extern class Image *gamma_preview_image;  // 506E40
