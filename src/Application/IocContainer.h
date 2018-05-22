#pragma once

namespace Application {

class Menu;
class GameWindowHandler;

class IocContainer {
 public:
     static Menu *ResolveGameMenu();
     static GameWindowHandler *ResolveGameWindowHandler();

 private:
     static Menu *menu;
     static GameWindowHandler *gameWindowHandler;
};

}  // namespace Application
