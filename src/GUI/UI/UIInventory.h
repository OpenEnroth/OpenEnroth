#pragma once

#include <string>

#include "GUI/GUIWindow.h"
#include "GUI/UI/UISpell.h"

struct CastSpellInfo;

class GUIWindow_Inventory : public GUIWindow {
 public:
    GUIWindow_Inventory(Pointi position, Sizei dimensions, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_CharacterWindow_Inventory, position, dimensions, hint) {}
    virtual ~GUIWindow_Inventory() {}

    virtual void Update() override;
};

class GUIWindow_Inventory_CastSpell : public TargetedSpellUI {
 public:
    GUIWindow_Inventory_CastSpell(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, const std::string &hint = std::string());

    virtual void Update() override;
};
