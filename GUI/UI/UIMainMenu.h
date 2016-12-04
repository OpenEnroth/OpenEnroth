#pragma once
#include "GUI/GUIWindow.h"



extern struct GUIButton *pMainMenu_BtnExit;
extern struct GUIButton *pMainMenu_BtnCredits;
extern struct GUIButton *pMainMenu_BtnLoad;
extern struct GUIButton *pMainMenu_BtnNew;



struct GUIWindow_MainMenu: public GUIWindow
{
             GUIWindow_MainMenu();
    virtual ~GUIWindow_MainMenu() {}

    virtual void Update();
};
