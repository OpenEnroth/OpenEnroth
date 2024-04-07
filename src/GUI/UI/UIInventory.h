#pragma once

#include <string>

#include "GUI/GUIWindow.h"
#include "GUI/UI/UISpell.h"

struct CastSpellInfo;

class GUIWindow_Inventory : public GUIWindow {
 public:
    GUIWindow_Inventory(Pointi position, Sizei dimensions, std::string_view hint = {}) :
        GUIWindow(WINDOW_CharacterWindow_Inventory, position, dimensions, hint) {}
    virtual ~GUIWindow_Inventory() {}

    virtual void Update() override;
};

class GUIWindow_Inventory_CastSpell : public TargetedSpellUI {
 public:
    GUIWindow_Inventory_CastSpell(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint = {});

    virtual void Update() override;
};
