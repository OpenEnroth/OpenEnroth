#pragma once

class Menu;

// TODO(captainurist): this is a legacy class, drop.
class GameIocContainer {
 public:
     static Menu *ResolveGameMenu();

 private:
     static Menu *menu;
};
