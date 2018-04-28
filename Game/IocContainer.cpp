#include "Game/IocContainer.h"

#include "Game/GameMenu.h"

using namespace Game;

Menu *IocContainer::menu = nullptr;
Menu *IocContainer::ResolveGameMenu() {
    if (!menu) {
        menu = new Menu();
    }
    return menu;
}