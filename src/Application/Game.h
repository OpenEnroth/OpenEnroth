#pragma once
#include <memory>

#include "GameConfig.h"
#include "GameMenu.h"
#include "IocContainer.h"

#include "Engine/Engine.h"
#include "Engine/IocContainer.h"

#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"


using EngineIoc = Engine_::IocContainer;
using GameIoc = Application::IocContainer;
using Io::Mouse;

class IRender;
class Platform;
class PlatformWindow;
class PlatformEventLoop;

namespace Application {

class Game {
 public:
     Game(Platform *platform);
     ~Game();

     bool Configure(std::shared_ptr<GameConfig> config) {
         this->config = config;
         return true;
     }

     int Run();

 private:
     void UpdateWindowFromConfig();
     void UpdateConfigFromWindow();
     bool Loop();
     void EventLoop();
     void GameLoop();
     void CloseTargetedSpellWindow();
     void OnEscape();
     void OnPressSpace();

     Platform *platform = nullptr;
     std::shared_ptr<GameConfig> config;
     std::unique_ptr<GameWindowHandler> windowHandler;
     std::shared_ptr<Engine> engine;
     std::unique_ptr<PlatformWindow> window;
     std::unique_ptr<PlatformEventLoop> eventLoop;
     std::shared_ptr<IRender> render;
     std::shared_ptr<Mouse> mouse = nullptr;
     Log *log = nullptr;
     DecalBuilder *decal_builder = nullptr;
     Vis *vis = nullptr;
     Menu *menu = nullptr;
     std::shared_ptr<Nuklear> nuklear = nullptr;
};

}  // namespace Application

extern class Image *gamma_preview_image;  // 506E40
