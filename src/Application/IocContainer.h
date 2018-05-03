#pragma once

namespace Application {

class Menu;

class IocContainer {
 public:
     static Menu *ResolveGameMenu();

 private:
     static Menu *menu;
};

}  // namespace Application
