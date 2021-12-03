#include "Engine/Engine.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIChest.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"

int pChestPixelOffsetX[8] = {42, 18, 18, 42, 42, 42, 18, 42};
int pChestPixelOffsetY[8] = {34, 30, 30, 34, 34, 34, 30, 34};
int pChestWidthsByType[8] = {9, 9, 9, 9, 9, 9, 9, 9};
int pChestHeightsByType[8] = {9, 9, 9, 9, 9, 9, 9, 9};

GUIWindow_Chest::GUIWindow_Chest(unsigned int chest_id)
    : GUIWindow(WINDOW_Chest, 0, 0, window->GetWidth(), window->GetHeight(), (GUIButton*)chest_id) {
    CreateButton(61, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 1, GameKey::Digit1);
    CreateButton(177, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 2, GameKey::Digit2);
    CreateButton(292, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 3, GameKey::Digit3);
    CreateButton(407, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 4, GameKey::Digit4);
    CreateButton(0, 0, 0, 0, 1, 0, UIMSG_CycleCharacters, 0, GameKey::Tab);

    pBtn_ExitCancel =
        CreateButton(471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, GameKey::None,
                     localization->GetString(LSTR_DIALOGUE_EXIT),
                     {{ui_exit_cancel_button_background}});
    CreateButton(7, 8, 460, 343, 1, 0, UIMSG_CHEST_ClickItem, 0);
    current_screen_type = CURRENT_SCREEN::SCREEN_CHEST;
    pEventTimer->Pause();
}

void GUIWindow_Chest::Update() {
    if (current_screen_type == CURRENT_SCREEN::SCREEN_CHEST_INVENTORY) {
        render->ClearZBuffer();
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        render->DrawTextureAlphaNew(pBtn_ExitCancel->uX / 640.0f,
                                    pBtn_ExitCancel->uY / 480.0f,
                                    ui_exit_cancel_button_background);
    } else if (current_screen_type == CURRENT_SCREEN::SCREEN_CHEST) {
        auto uChestID = (uint64_t)ptr_1C;

        int *v16 = render->pActiveZBuffer;
        render->ClearZBuffer();
        int chestBitmapId = vChests[uChestID].uChestBitmapID;
        int chest_offs_x = pChestPixelOffsetX[chestBitmapId];
        int chest_offs_y = pChestPixelOffsetY[chestBitmapId];
        int chestWidthCells = pChestWidthsByType[chestBitmapId];
        int chestHeghtCells = pChestHeightsByType[chestBitmapId];

        Image *chest_background = assets->GetImage_ColorKey(
            StringPrintf("chest%02d", pChestList->vChests[chestBitmapId].uTextureID), render->teal_mask_16);
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, chest_background);

        for (int item_counter = 0;
             item_counter < chestWidthCells * chestHeghtCells; ++item_counter) {
            int chest_item_index = vChests[uChestID].pInventoryIndices[item_counter];
            if (chest_item_index > 0) {
                auto item_texture = assets->GetImage_ColorKey(
                    vChests[uChestID].igChestItems[chest_item_index - 1].GetIconName(),
                    render->teal_mask_16);

                int itemPixelWidth = item_texture->GetWidth();
                int itemPixelHeght = item_texture->GetHeight();
                if (itemPixelWidth < 14) itemPixelWidth = 14;
                if (itemPixelHeght < 14) itemPixelHeght = 14;
                signed int X_offset = (((signed int)((itemPixelWidth - 14) & 0xFFFFFFE0) + 32) - itemPixelWidth) / 2;
                signed int Y_offset = (((signed int)((itemPixelHeght - 14) & 0xFFFFFFE0) + 32) - itemPixelHeght) / 2;
                int itemPixelPosX = chest_offs_x + 32 * (item_counter % chestWidthCells) + X_offset;
                int itemPixelPosY = chest_offs_y + 32 * (item_counter / chestHeghtCells) + Y_offset;
                render->DrawTextureAlphaNew(itemPixelPosX / 640.0f,
                                            itemPixelPosY / 480.0f,
                                            item_texture);
                //        ZBuffer_DoFill2(&v16[itemPixelPosX +
                //        pSRZBufferLineOffsets[itemPixelPosY]], item_texture,
                //        item_counter + 1);
            }
        }
        render->DrawTextureAlphaNew(pBtn_ExitCancel->uX / 640.0f,
                                    pBtn_ExitCancel->uY / 480.0f,
                                    ui_exit_cancel_button_background);
    }
}
