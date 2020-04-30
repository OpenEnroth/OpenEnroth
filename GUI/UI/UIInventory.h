#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Inventory : public GUIWindow {
 public:
    GUIWindow_Inventory(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(WINDOW_CharacterWindow_Inventory, x, y, width, height, button, hint) {}
    virtual ~GUIWindow_Inventory() {}

    virtual void Update();
};

class GUIWindow_Inventory_CastSpell : public GUIWindow {
 public:
    GUIWindow_Inventory_CastSpell(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String());
    virtual ~GUIWindow_Inventory_CastSpell() {}

    virtual void Update();
};
