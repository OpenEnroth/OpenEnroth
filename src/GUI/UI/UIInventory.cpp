#include "GUI/UI/UIInventory.h"

#include "Engine/Localization.h"

#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Party.h"

#include "GUI/GUIButton.h"

#include "Io/KeyboardInputHandler.h"

void GUIWindow_Inventory::Update() {
    DrawMessageBox(0);
    DrawText(assets->pFontLucida.get(), {10, 20}, colorTable.White, "Making item number");
    DrawText(assets->pFontLucida.get(), {10, 40}, colorTable.White, keyboardInputHandler->GetTextInput());

    // a hack to capture end of user input (enter) while avoiding listening to UI message handler
    // redo this in a more clean way
    assert(false);

    // if (userInputHandler->inputType == TextInputType::None) {
    //    ItemGen ItemGen2;
    //    ItemGen2.Reset();
    //    Release();
    //    pEventTimer->setPaused(false);
    //    current_screen_type = SCREEN_GAME;
    //    int v39 = atoi(userInputHandler->GetTextInput().c_str());
    //    if (v39 > 0 && v39 < 800) {
    //        // SpawnActor(v39);
    //    }
    // }
}


GUIWindow_Inventory_CastSpell::GUIWindow_Inventory_CastSpell(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, std::string_view hint) :
    TargetedSpellUI(WINDOW_CastSpell_InInventory, position, dimensions, spellInfo, hint) {
    pBtn_ExitCancel = CreateButton({392, 318}, {75, 33}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid, localization->GetString(LSTR_CANCEL), {ui_buttdesc2});
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    current_screen_type = SCREEN_CASTING;
}

void GUIWindow_Inventory_CastSpell::Update() {
    render->ClearZBuffer();
    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);
    CharacterUI_DrawPaperdoll(&pParty->activeCharacter());
    render->DrawTextureNew(pBtn_ExitCancel->uX / 640.0f, pBtn_ExitCancel->uY / 480.0f, dialogue_ui_x_x_u);
}
