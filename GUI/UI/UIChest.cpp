#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Time.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"

#include "Engine/Objects/Chest.h"

#include "GUI/UI/UIChest.h"

#include "Platform/Api.h"



int pChestPixelOffsetX[8] = { 42, 18, 18, 42, 42, 42, 18, 42 };
int pChestPixelOffsetY[8] = { 34, 30, 30, 34, 34, 34, 30, 34 };
int pChestWidthsByType[8] = { 9, 9, 9, 9, 9, 9, 9, 9 };
int pChestHeightsByType[8] = { 9, 9, 9, 9, 9, 9, 9, 9 };



GUIWindow_Chest::GUIWindow_Chest(unsigned int chest_id) :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), chest_id)
{
    // --------------------------------------
    // 0041C432 GUIWindow::GUIWindow --- part
    CreateButton(61, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 1, '1', "");
    CreateButton(177, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 2, '2', "");
    CreateButton(292, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 3, '3', "");
    CreateButton(407, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 4, '4', "");
    CreateButton(0, 0, 0, 0, 1, 0, UIMSG_CycleCharacters, 0, 9, "");

    // --------------------------------------------------------
    // 0042041E bool Chest::Open( signed int uChestID ) -- part
    pBtn_ExitCancel = CreateButton(471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, 0, localization->GetString(79), { {ui_exit_cancel_button_background} });// Exit
    CreateButton(7, 8, 460, 343, 1, 0, UIMSG_CHEST_ClickItem, 0, 0, "");
    current_screen_type = SCREEN_CHEST;
    pEventTimer->Pause();
}



void GUIWindow_Chest::Update()
{
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    if (current_screen_type == SCREEN_CHEST_INVENTORY)
    {
        render->ClearZBuffer(0, 479);
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        render->DrawTextureAlphaNew(pBtn_ExitCancel->uX/640.0f, pBtn_ExitCancel->uY/480.0f, ui_exit_cancel_button_background);
    }
    else if (current_screen_type == SCREEN_CHEST)
    {
        //Chest::DrawChestUI(par1C);

        // -----------------------------------------------------
        // 0042092D void Chest::DrawChestUI(signed int uChestID)
        auto uChestID = (unsigned int)ptr_1C;

        int chestBitmapId; // eax@1
        //Image *v5; // eax@1
        int chest_item_index; // ecx@3
        //unsigned int item_texture_id; // eax@4
        //Texture_MM7 *item_texture; // esi@4
        signed int itemPixelWidth; // ecx@4
        signed int itemPixelHeght; // edx@4
        //    signed int v11; // eax@4
        int v12; // eax@6
        int v13; // eax@6
        unsigned int itemPixelPosX; // ST34_4@8
        int itemPixelPosY; // edi@8
        int *v16; // [sp+Ch] [bp-28h]@1
        //    int v17; // [sp+10h] [bp-24h]@4
        int chest_offs_y; // [sp+14h] [bp-20h]@1
        signed int chestHeghtCells; // [sp+18h] [bp-1Ch]@1
        int chest_offs_x; // [sp+1Ch] [bp-18h]@1
        signed int chestWidthCells; // [sp+20h] [bp-14h]@1
        signed int item_counter; // [sp+30h] [bp-4h]@1

        v16 = render->pActiveZBuffer;
        render->ClearZBuffer(0, 479);
        chestBitmapId = pChests[uChestID].uChestBitmapID;
        chest_offs_x = pChestPixelOffsetX[chestBitmapId];
        chest_offs_y = pChestPixelOffsetY[chestBitmapId];
        chestWidthCells = pChestWidthsByType[chestBitmapId];
        chestHeghtCells = pChestHeightsByType[chestBitmapId];

        auto chest_background = assets->GetImage_16BitColorKey(StringPrintf("chest%02d", pChestList->pChests[chestBitmapId].uTextureID), 0x7FF);
        render->DrawTextureAlphaNew(8/640.0f, 8/480.0f, chest_background);

        for (item_counter = 0; item_counter < chestWidthCells * chestHeghtCells; ++item_counter)
        {
            chest_item_index = pChests[uChestID].pInventoryIndices[item_counter];
            if (chest_item_index > 0)
            {
                auto item_texture = assets->GetImage_16BitColorKey(pChests[uChestID].igChestItems[chest_item_index - 1].GetIconName(), 0x7FF);
                itemPixelWidth = item_texture->GetWidth();
                itemPixelHeght = item_texture->GetHeight();

                if (itemPixelWidth < 14)
                    itemPixelWidth = 14;
                v12 = itemPixelWidth - 14;
                v12 = v12 & 0xFFFFFFE0;
                v13 = v12 + 32;
                if (itemPixelHeght < 14)
                    itemPixelHeght = 14;
                itemPixelPosX = chest_offs_x + 32 * (item_counter % chestWidthCells) + ((signed int)(v13 - itemPixelWidth) / 2);
                itemPixelPosY = chest_offs_y + 32 * (item_counter / chestHeghtCells) +
                    ((signed int)(((itemPixelHeght - 14) & 0xFFFFFFE0) + 32 - item_texture->GetHeight()) / 2);
                render->DrawTextureAlphaNew(itemPixelPosX/640.0f, itemPixelPosY/480.0f, item_texture);
                //ZBuffer_DoFill2(&v16[itemPixelPosX + pSRZBufferLineOffsets[itemPixelPosY]], item_texture, item_counter + 1);
            }
        }
        render->DrawTextureAlphaNew(pBtn_ExitCancel->uX/640.0f, pBtn_ExitCancel->uY/480.0f, ui_exit_cancel_button_background);
    }
}