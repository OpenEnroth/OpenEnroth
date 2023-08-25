#include "GUI/UI/UIInventory.h"

#include "Engine/Engine.h"
#include "Engine/Localization.h"

#include "Engine/AssetsManager.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Party.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

void GUIWindow_Inventory::Update() {
    DrawMessageBox(0);
    DrawText(assets->pFontLucida.get(), {10, 20}, colorTable.White, "Making item number");
    DrawText(assets->pFontLucida.get(), {10, 40}, colorTable.White, keyboardInputHandler->GetTextInput());

    // a hack to capture end of user input (enter) while avoiding listening to UI message handler
    // redo this in a more clean way
    __debugbreak();

    // if (userInputHandler->inputType == TextInputType::None) {
    //    ItemGen ItemGen2;
    //    ItemGen2.Reset();
    //    Release();
    //    pEventTimer->Resume();
    //    current_screen_type = SCREEN_GAME;
    //    int v39 = atoi(userInputHandler->GetTextInput().c_str());
    //    if (v39 > 0 && v39 < 800) {
    //        // SpawnActor(v39);
    //    }
    // }
}


GUIWindow_Inventory_CastSpell::GUIWindow_Inventory_CastSpell(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, const std::string &hint) :
    GUIWindow(WINDOW_CastSpell_InInventory, position, dimensions, spellInfo, hint) {
    mouse->SetCursorImage("MICON2");
    pBtn_ExitCancel = CreateButton({392, 318}, {75, 33}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid, localization->GetString(LSTR_CANCEL), {ui_buttdesc2});
    engine->_statusBar->setEvent(LSTR_CHOOSE_TARGET);
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
