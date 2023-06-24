#include "GameIocContainer.h"

#include "GameMenu.h"

Menu *GameIocContainer::menu = nullptr;
Menu *GameIocContainer::ResolveGameMenu() {
    if (!menu) {
        menu = new Menu();
    }
    return menu;
}
