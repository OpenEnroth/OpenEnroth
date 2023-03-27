#include "GUI/UI/UIInventory.h"

#include "Engine/Localization.h"

#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/IRender.h"

#include "Engine/Objects/Actor.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

void GUIWindow_Inventory::Update() {
    DrawMessageBox(0);
    DrawText(pFontLucida, {10, 20}, 0, "Making item number", 0, 0, 0);
    DrawText(pFontLucida, {10, 40}, 0, keyboardInputHandler->GetTextInput().c_str(), 0, 0, 0);

    // a hack to capture end of user input (enter) while avoiding listening to UI message handler
    // redo this in a more clean way
    __debugbreak();

    // if (userInputHandler->inputType == TextInputType::None) {
    //    ItemGen ItemGen2;
    //    ItemGen2.Reset();
    //    Release();
    //    pEventTimer->Resume();
    //    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
    //    int v39 = atoi(userInputHandler->GetTextInput().c_str());
    //    if (v39 > 0 && v39 < 800) {
    //        SpawnActor(v39);
    //    }
    // }
}


GUIWindow_Inventory_CastSpell::GUIWindow_Inventory_CastSpell(Pointi position, Sizei dimensions, CastSpellInfo *spellInfo, const std::string &hint) :
    GUIWindow(WINDOW_CastSpell_InInventory, position, dimensions, spellInfo, hint) {
    mouse->SetCursorImage("MICON2");
    pBtn_ExitCancel = CreateButton({392, 318}, {75, 33}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_CANCEL), {ui_buttdesc2});
    GameUI_SetStatusBar(LSTR_CHOOSE_TARGET);
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    current_screen_type = CURRENT_SCREEN::SCREEN_CASTING;
}

void GUIWindow_Inventory_CastSpell::Update() {
    render->ClearZBuffer();
    draw_leather();
    CharacterUI_InventoryTab_Draw(pPlayers[pParty->getActiveCharacter()], true);
    CharacterUI_DrawPaperdoll(pPlayers[pParty->getActiveCharacter()]);
    render->DrawTextureNew(pBtn_ExitCancel->uX / 640.0f, pBtn_ExitCancel->uY / 480.0f, dialogue_ui_x_x_u);
}
