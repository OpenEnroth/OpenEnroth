#pragma once

#include <memory>

class Menu;
class GameWindowHandler;

class GameIocContainer {
 public:
     static Menu *ResolveGameMenu();
     static GameWindowHandler *ResolveGameWindowHandler();

 private:
     static Menu *menu;
     static GameWindowHandler *gameWindowHandler;
};
