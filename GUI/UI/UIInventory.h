#pragma once

#include <string>

#include "GUI/GUIWindow.h"

struct CastSpellInfo;

class GUIWindow_Inventory : public GUIWindow {
 public:
    GUIWindow_Inventory(unsigned int x, unsigned int y, unsigned int width, unsigned int height, WindowData data, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_CharacterWindow_Inventory, x, y, width, height, data, hint) {}
    virtual ~GUIWindow_Inventory() {}

    virtual void Update();
};

class GUIWindow_Inventory_CastSpell : public GUIWindow {
 public:
    GUIWindow_Inventory_CastSpell(unsigned int x, unsigned int y, unsigned int width, unsigned int height, CastSpellInfo *spellInfo, const std::string &hint = std::string());
    virtual ~GUIWindow_Inventory_CastSpell() {}

    virtual void Update();
};
