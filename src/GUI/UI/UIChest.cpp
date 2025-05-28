#include "UIChest.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Time/Timer.h"
#include "Engine/Party.h"
#include "Engine/AssetsManager.h"
#include "Engine/Tables/ChestTable.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/ItemGrid.h"

GUIWindow_Chest::GUIWindow_Chest(int chestId) : GUIWindow(WINDOW_Chest, {0, 0}, render->GetRenderDimensions()), _chestId(chestId) {
    CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1);
    CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2);
    CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3);
    CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4);
    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, Io::InputAction::CharCycle);

    pBtn_ExitCancel = CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
                                   localization->GetString(LSTR_EXIT_DIALOGUE), {ui_exit_cancel_button_background});
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
        int uChestID = _chestId;
        int chestDescId = vChests[uChestID].chestTypeId;
        int chest_offs_x = chestTable[chestDescId].inventoryOffset.x;
        int chest_offs_y = chestTable[chestDescId].inventoryOffset.y;
        int chestWidthCells = chestTable[chestDescId].size.w;
        int chestHeghtCells = chestTable[chestDescId].size.h;

        GraphicsImage *chest_background = assets->getImage_ColorKey(chestTable[chestDescId].textureName);
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, chest_background);

        render->SetUIClipRect({ chest_offs_x, chest_offs_y, 32 * chestWidthCells, 32 * chestHeghtCells });
        CharacterUI_DrawPickedItemUnderlay({ chest_offs_x, chest_offs_y });
        render->ResetUIClipRect();

        for (int item_counter = 0; item_counter < chestWidthCells * chestHeghtCells; ++item_counter) {
            int chest_item_index = vChests[uChestID].inventoryMatrix[item_counter];
            if (chest_item_index > 0) {
                auto item_texture = assets->getImage_ColorKey(vChests[uChestID].items[chest_item_index - 1].GetIconName());
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
