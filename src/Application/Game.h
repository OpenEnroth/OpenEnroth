#pragma once
#include <memory>

#include "src/Application/GameConfig.h"
#include "src/Application/GameMenu.h"
#include "src/Application/IocContainer.h"

#include "Engine/Engine.h"
#include "Engine/IocContainer.h"

#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"


using EngineIoc = Engine_::IocContainer;
using GameIoc = Application::IocContainer;
using Io::Mouse;


namespace Application {

class Game {
 public:
     inline Game() {
         this->log = EngineIoc::ResolveLogger();
         this->decal_builder = EngineIoc::ResolveDecalBuilder();
         this->vis = EngineIoc::ResolveVis();
         this->menu = GameIoc::ResolveGameMenu();
     }

     bool Configure(std::shared_ptr<const GameConfig> config) {
         this->config = config;
         return true;
     }

     void Run();

 private:
     bool Loop();
     void EventLoop();
     void GameLoop();
     void CloseTargetedSpellWindow();
     void OnEscape();
     void OnPressSpace();
     void ProcessInputActions();


     std::shared_ptr<const GameConfig> config;
     std::shared_ptr<Engine> engine;
     std::shared_ptr<OSWindow> window;
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
