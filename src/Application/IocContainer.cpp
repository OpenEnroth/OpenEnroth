#include "IocContainer.h"

#include "GameMenu.h"
#include "GameWindowHandler.h"


using Application::IocContainer;
using Application::Menu;
using Application::GameWindowHandler;

Menu *IocContainer::menu = nullptr;
Menu *IocContainer::ResolveGameMenu() {
    if (!menu) {
        menu = new Menu();
    }
    return menu;
}

GameWindowHandler *IocContainer::gameWindowHandler = nullptr;
GameWindowHandler *IocContainer::ResolveGameWindowHandler() {
    if (!gameWindowHandler) {
        gameWindowHandler = new GameWindowHandler();
    }
    return gameWindowHandler;
}
