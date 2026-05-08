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
    CreateCharacterButtons();
    CreateButton({0, 0}, {0, 0}, BUTTON_TYPE_NORMAL, 0, UIMSG_CycleCharacters, 0, INPUT_ACTION_NEXT_CHAR);

    pBtn_ExitCancel = CreateButton({471, 445}, {169, 35}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID,
                                   localization->str(LSTR_EXIT_DIALOGUE), {ui_exit_cancel_button_background});
    CreateButton({7, 8}, {460, 343}, BUTTON_TYPE_NORMAL, 0, UIMSG_CHEST_ClickItem, 0);
    current_screen_type = SCREEN_CHEST;
    pEventTimer->setPaused(true);
}

void GUIWindow_Chest::Update() {
    if (current_screen_type == SCREEN_CHEST_INVENTORY && pParty->hasActiveCharacter()) {
        draw_leather();
        CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);
        render->DrawQuad2D(ui_exit_cancel_button_background, pBtn_ExitCancel->rect.topLeft());
    } else if (current_screen_type == SCREEN_CHEST) {
        int uChestID = _chestId;
        int chestDescId = vChests[uChestID].chestTypeId;
        int chest_offs_x = chestTable[chestDescId].inventoryOffset.x;
        int chest_offs_y = chestTable[chestDescId].inventoryOffset.y;
        int chestWidthCells = chestTable[chestDescId].size.w;
        int chestHeghtCells = chestTable[chestDescId].size.h;

        GraphicsImage *chest_background = assets->getImage_ColorKey(chestTable[chestDescId].textureName);
        render->DrawQuad2D(chest_background, {8, 8});

        render->SetUIClipRect({ chest_offs_x, chest_offs_y, 32 * chestWidthCells, 32 * chestHeghtCells });
        CharacterUI_DrawPickedItemUnderlay({ chest_offs_x, chest_offs_y });
        render->ResetUIClipRect();

        for (InventoryEntry entry : vChests[uChestID].inventory.entries()) {
            if (entry.zone() != INVENTORY_ZONE_GRID)
                continue;

            auto item_texture = assets->getImage_ColorKey(entry->GetIconName());
            int X_offset = itemOffset(item_texture->width());
            int Y_offset = itemOffset(item_texture->height());
            int itemPixelPosX = chest_offs_x + 32 * entry.geometry().x + X_offset;
            int itemPixelPosY = chest_offs_y + 32 * entry.geometry().y + Y_offset;

            assert(0 < itemPixelPosX && itemPixelPosX < 640);
            assert(0 < itemPixelPosY && itemPixelPosY < 480);
            render->DrawQuad2D(item_texture, {itemPixelPosX, itemPixelPosY});
        }

        render->DrawQuad2D(ui_exit_cancel_button_background, pBtn_ExitCancel->rect.topLeft());
    }
}
