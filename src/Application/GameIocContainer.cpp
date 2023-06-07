#include "GameIocContainer.h"

#include "GameMenu.h"
#include "GameWindowHandler.h"

Menu *GameIocContainer::menu = nullptr;
Menu *GameIocContainer::ResolveGameMenu() {
    if (!menu) {
        menu = new Menu();
    }
    return menu;
}
