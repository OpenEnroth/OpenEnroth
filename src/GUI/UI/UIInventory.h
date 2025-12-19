#pragma once

#include <string>

#include "GUI/GUIWindow.h"
#include "GUI/UI/UISpell.h"

struct CastSpellInfo;

class GUIWindow_Inventory_CastSpell : public TargetedSpellUI {
 public:
    GUIWindow_Inventory_CastSpell(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint = {});

    virtual void Update() override;
};
