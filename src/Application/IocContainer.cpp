#include "Game/IocContainer.h"

#include "Game/GameMenu.h"

using Game::IocContainer;
using Game::Menu;

Menu *IocContainer::menu = nullptr;
Menu *IocContainer::ResolveGameMenu() {
    if (!menu) {
        menu = new Menu();
    }
    return menu;
}
