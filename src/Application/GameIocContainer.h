#pragma once

class Menu;

class GameIocContainer {
 public:
     static Menu *ResolveGameMenu();

 private:
     static Menu *menu;
};
