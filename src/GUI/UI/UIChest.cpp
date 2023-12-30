#include "UIChest.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Time/Timer.h"
#include "Engine/Party.h"
#include "Engine/AssetsManager.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/ItemGrid.h"

int pChestPixelOffsetX[8] = {42, 18, 18, 42, 42, 42, 18, 42};
int pChestPixelOffsetY[8] = {34, 30, 30, 34, 34, 34, 30, 34};
int pChestWidthsByType[8] = {9, 9, 9, 9, 9, 9, 9, 9};
int pChestHeightsByType[8] = {9, 9, 9, 9, 9, 9, 9, 9};

GUIWindow_Chest::GUIWindow_Chest(int chestId) : GUIWindow(WINDOW_Chest, {0, 0}, render->GetRenderDimensions()), _chestId(chestId) {
    CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1);
    CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2);
    CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3);
    CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4);
    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, Io::InputAction::CharCycle);

    pBtn_ExitCancel = CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
                                   localization->GetString(LSTR_DIALOGUE_EXIT), {ui_exit_cancel_button_background});
    CreateButton({7, 8}, {460, 343}, 1, 0, UIMSG_CHEST_ClickItem, 0);
    current_screen_type = SCREEN_CHEST;
    pEventTimer->setPaused(true);
}

void GUIWindow_Chest::Update() {
    if (current_screen_type == SCREEN_CHEST_INVENTORY) {
        render->ClearZBuffer();
        draw_leather();
        CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);
        render->DrawTextureNew(pBtn_ExitCancel->uX / 640.0f, pBtn_ExitCancel->uY / 480.0f, ui_exit_cancel_button_background);
    } else if (current_screen_type == SCREEN_CHEST) {
        render->ClearZBuffer();

        int uChestID = _chestId;
        int chestBitmapId = vChests[uChestID].uChestBitmapID;
        int chest_offs_x = pChestPixelOffsetX[chestBitmapId];
        int chest_offs_y = pChestPixelOffsetY[chestBitmapId];
        int chestWidthCells = pChestWidthsByType[chestBitmapId];
        int chestHeghtCells = pChestHeightsByType[chestBitmapId];

        GraphicsImage *chest_background = assets->getImage_ColorKey(fmt::format("chest{:02}", pChestList->vChests[chestBitmapId].uTextureID));
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, chest_background);

        for (int item_counter = 0; item_counter < chestWidthCells * chestHeghtCells; ++item_counter) {
            int chest_item_index = vChests[uChestID].pInventoryIndices[item_counter];
            if (chest_item_index > 0) {
                auto item_texture = assets->getImage_ColorKey(vChests[uChestID].igChestItems[chest_item_index - 1].GetIconName());
                int X_offset = itemOffset(item_texture->width());
                int Y_offset = itemOffset(item_texture->height());
                int itemPixelPosX = chest_offs_x + 32 * (item_counter % chestWidthCells) + X_offset;
                int itemPixelPosY = chest_offs_y + 32 * (item_counter / chestHeghtCells) + Y_offset;

                assert(0 < itemPixelPosX && itemPixelPosX < 640);
                assert(0 < itemPixelPosY && itemPixelPosY < 480);
                render->DrawTextureNew(itemPixelPosX / 640.0f, itemPixelPosY / 480.0f, item_texture);
            }
        }
        render->DrawTextureNew(pBtn_ExitCancel->uX / 640.0f, pBtn_ExitCancel->uY / 480.0f, ui_exit_cancel_button_background);
    }
}
