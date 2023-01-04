#pragma once

#include <string>

#include "GUI/GUIWindow.h"

struct CastSpellInfo;

class GUIWindow_Inventory : public GUIWindow {
 public:
    GUIWindow_Inventory(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_CharacterWindow_Inventory, position, dimensions, data, hint) {}
    virtual ~GUIWindow_Inventory() {}

    virtual void Update();
};

class GUIWindow_Inventory_CastSpell : public GUIWindow {
 public:
    GUIWindow_Inventory_CastSpell(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, const std::string &hint = std::string());
    virtual ~GUIWindow_Inventory_CastSpell() {}

    virtual void Update();
};
