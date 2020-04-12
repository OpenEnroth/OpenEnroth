#pragma once

#include <memory>

#include "src/Application/Configuration.h"
#include "src/Application/GameMenu.h"
#include "src/Application/IocContainer.h"

#include "Engine/Engine.h"
#include "Engine/IocContainer.h"

#include "IO/Keyboard.h"
#include "IO/UserInputHandler.h"


using EngineIoc = Engine_::IocContainer;
using GameIoc = Application::IocContainer;


namespace Application {

class Game {
 public:
     inline Game() {
         this->log = EngineIoc::ResolveLogger();
         this->mouse = EngineIoc::ResolveMouse();
         this->decal_builder = EngineIoc::ResolveDecalBuilder();
         this->vis = EngineIoc::ResolveVis();
         this->menu = GameIoc::ResolveGameMenu();
         //this->keyboardController = std::make_shared<KeyboardController>(/*std::make_shared<Keyboard>()*/nullptr);
         //::keyboardController = keyboardController.get();

         //this->keyboardActionMapping = std::make_shared<KeyboardActionMapping>();
         //this->userInputHandler = std::make_shared<UserInputHandler>(/* get input provider from OI window */);
     }

     bool Configure(std::shared_ptr<const Configuration> config);
     void Run();

 private:
     bool Loop();
     void EventLoop();
     void GameLoop();
     void CloseTargetedSpellWindow();
     void OnEscape();
     void OnPressSpace();
     void ProcessInputActions();


     std::shared_ptr<const Configuration> config;
     std::shared_ptr<Engine> engine;
     Log *log = nullptr;
     Mouse *mouse = nullptr;

     //std::shared_ptr<KeyboardActionMapping> keyboardActionMapping;
     //std::shared_ptr<UserInputHandler> userInputHandler;
     DecalBuilder *decal_builder = nullptr;
     Vis *vis = nullptr;

     Menu *menu = nullptr;

};

}  // namespace Application

extern class Image *gamma_preview_image;  // 506E40
