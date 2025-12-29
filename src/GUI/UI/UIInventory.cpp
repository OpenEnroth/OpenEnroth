#include "GUI/UI/UIInventory.h"

#include "Engine/Localization.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Party.h"

#include "GUI/GUIButton.h"

#include "Io/KeyboardInputHandler.h"

GUIWindow_Inventory_CastSpell::GUIWindow_Inventory_CastSpell(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint) :
    TargetedSpellUI(WINDOW_CastSpell_InInventory, position, dimensions, spellInfo, hint) {
    pBtn_ExitCancel = CreateButton({392, 318}, {75, 33}, 1, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_CANCEL), {ui_buttdesc2});
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    current_screen_type = SCREEN_CASTING;
}

void GUIWindow_Inventory_CastSpell::Update() {
    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);
    CharacterUI_DrawPaperdoll(&pParty->activeCharacter());
    render->DrawQuad2D(dialogue_ui_x_x_u, pBtn_ExitCancel->rect.topLeft());
}
