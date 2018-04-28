#pragma once

namespace Game {

class Menu;

class IocContainer {
 public:

     static Menu *ResolveGameMenu();

 private:
     static Menu *menu;
};

}  // namespace Game
