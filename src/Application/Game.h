#pragma once

#include "Application/Configuration.h"
#include "Application/GameMenu.h"
#include "Application/IocContainer.h"

#include "Engine/IocContainer.h"


using EngineIoc = Engine_::IocContainer;
using GameIoc = Application::IocContainer;


namespace Application {

class Game {
 public:
     inline Game() {
         this->log = EngineIoc::ResolveLogger();
         this->mouse = EngineIoc::ResolveMouse();
         this->keyboard = EngineIoc::ResolveKeyboard();
         this->decal_builder = EngineIoc::ResolveDecalBuilder();
         this->vis = EngineIoc::ResolveVis();
         this->menu = GameIoc::ResolveGameMenu();
     }

     void Configure(Configuration *);
     void Run();

 private:
     bool Loop();
     void EventLoop();
     void GameLoop();
     void CloseTargetedSpellWindow();
     void OnEscape();
     void OnPressSpace();


     Configuration *config = nullptr;
     Log *log = nullptr;
     Mouse *mouse = nullptr;
     Keyboard *keyboard = nullptr;
     DecalBuilder *decal_builder = nullptr;
     Vis *vis = nullptr;

     Menu *menu = nullptr;
};

}  // namespace Game

extern class Image *gamma_preview_image;  // 506E40
