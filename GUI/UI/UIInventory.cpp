#include "GUI/UI/UIInventory.h"

#include "Engine/Localization.h"

#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/IRender.h"

#include "Engine/Objects/Actor.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIStatusBar.h"

#include "IO/Keyboard.h"
#include "IO/Mouse.h"

void GUIWindow_Inventory::Update() {
    DrawMessageBox(0);
    DrawText(pFontLucida, 10, 20, 0, "Making item number", 0, 0, 0);
    DrawText(pFontLucida, 10, 40, 0, pKeyActionMap->pPressedKeysBuffer, 0, 0, 0);
    if (!pKeyActionMap->field_204) {
        ItemGen ItemGen2;
        ItemGen2.Reset();
        Release();
        pEventTimer->Resume();
        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
        viewparams->bRedrawGameUI = 1;
        int v39 = atoi(pKeyActionMap->pPressedKeysBuffer);
        if (v39 > 0 && v39 < 800) {
            SpawnActor(v39);
        }
    }
}

// GUIWindow_Inventory_CastSpell

GUIWindow_Inventory_CastSpell::GUIWindow_Inventory_CastSpell(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint) :
    GUIWindow(WINDOW_CastSpell_InInventory, x, y, width, height, button, hint) {
    mouse->SetCursorImage("MICON2");
    pBtn_ExitCancel = CreateButton(392, 318, 75, 33, 1, 0, UIMSG_Escape, 0, 0, localization->GetString(34),  // Cancel
        { { ui_buttdesc2 } });
    GameUI_StatusBar_OnEvent(localization->GetString(39), 2);  // Choose target
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    current_screen_type = CURRENT_SCREEN::SCREEN_CASTING;
}

void GUIWindow_Inventory_CastSpell::Update() {
    render->ClearZBuffer(0, 479);
    draw_leather();
    CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
    CharacterUI_DrawPaperdoll(pPlayers[uActiveCharacter]);
    render->DrawTextureAlphaNew(pBtn_ExitCancel->uX / 640.0f, pBtn_ExitCancel->uY / 480.0f, dialogue_ui_x_x_u);
}
