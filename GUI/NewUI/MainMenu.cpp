#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include "MainMenu.h"
#include "..\..\Engine/LOD.h"

bool MainMenuWindow::OnMouseLeftClick(int x, int y)
{
    if (UIControl::OnMouseLeftClick(x, y))
        return true;
    return false;
}

bool MainMenuWindow::OnMouseRightClick(int x, int y)
{
    if (UIControl::OnMouseRightClick(x, y))
        return true;
    return false;
}

bool MainMenuWindow::OnKey(int key)
{
    if (UIControl::OnKey(key))
        return true;
    return false;
}


void MainMenuWindow::Show() {}
bool MainMenuWindow::Focused() {return false;}

bool MainMenuWindow::Initialize()
{
    //RGBTexture* background_texture = new RGBTexture;
    //background_texture->Load("mm6title.pcx", 0);
    return true;
}

MainMenuWindow *MainMenuWindow::Create()
{
    MainMenuWindow* window = new MainMenuWindow;
    if (window)
        if (!window->Initialize())
        {
            delete window;
            window = nullptr;
        }
    return window;
}