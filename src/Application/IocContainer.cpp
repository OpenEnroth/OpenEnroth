#include "Application/IocContainer.h"

#include "Application/GameMenu.h"

using Application::IocContainer;
using Application::Menu;

Menu *IocContainer::menu = nullptr;
Menu *IocContainer::ResolveGameMenu() {
    if (!menu) {
        menu = new Menu();
    }
    return menu;
}
