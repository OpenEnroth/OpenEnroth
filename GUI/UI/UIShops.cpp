#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Events2D.h"
#include "Engine/MapInfo.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"

#include "Engine/Objects/Items.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UiStatusBar.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIHouses.h"

#include "IO/Mouse.h"

#include "Media/Audio/AudioPlayer.h"



Image *shop_ui_background = nullptr;

std::array<Image *, 12> shop_ui_items_in_store;


//----- (004B910F) --------------------------------------------------------
void WeaponShopDialog()
{
    int phrases_id; // eax@16
    int v19; // edi@25
    GUIButton *pButton; // esi@27
    unsigned int pColorText; // ax@27
    signed int v32; // esi@41
    int all_text_height; // esi@96
    ItemGen *item;
    int pItemID;
    GUIWindow dialog_window; // [sp+7Ch] [bp-7Ch]@1
    __int32 v103; // [sp+D4h] [bp-24h]@25
    int pPrice; // [sp+ECh] [bp-Ch]@26
    unsigned int v109;
    int pNumString;
    int item_X;

    memcpy(&dialog_window, window_SpeakInHouse, sizeof(dialog_window));
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 148;
    dialog_window.uFrameZ = 334;

    Point mouse = pMouse->GetCursorPos();

    switch (dialog_menu_id)
    {
        case HOUSE_DIALOGUE_MAIN:
        {
            if (HouseUI_CheckIfPlayerCanInteract())
            {
                pShopOptions[0] = localization->GetString(134);
                pShopOptions[1] = localization->GetString(152);
                pShopOptions[2] = localization->GetString(159);
                pShopOptions[3] = localization->GetString(160);

                all_text_height = 0;
                for (int i = 0; i < 4; ++i)
                    all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialog_window, 0);
                v103 = (174 - all_text_height) / 4;
                v19 = (174 - 4 * (174 - all_text_height) / 4 - all_text_height) / 2 - (174 - all_text_height) / 4 / 2 + 138;
                pNumString = 0;
                for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
                {
                    pButton = pDialogueWindow->GetControl(i);
                    pButton->uY = v103 + v19;
                    pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0);
                    v19 = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0) - 1;
                    pButton->uW = v19;
                    pColorText = Color16(0xE1u, 0xCDu, 0x23u);
                    if (pDialogueWindow->pCurrentPosActiveItem != i)
                        pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
                    dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
                    ++pNumString;
                }
            }
            break;
        }

    case HOUSE_DIALOGUE_SHOP_BUY_STANDARD:
    {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        item_X = 0;
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
            {
                render->DrawTextureAlphaNew(
                    ((60 - ((signed int)shop_ui_items_in_store[i]->GetWidth() / 2)) + item_X) / 640.0f,
                    (weapons_Ypos[i] + 30) / 480.0f,
                    shop_ui_items_in_store[i]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[(60 - ((signed int)shop_ui_items_in_store[i]->uTextureWidth / 2)) + item_X + window->GetWidth() * (weapons_Ypos[i] + 30)], shop_ui_items_in_store[i], i + 1);
            }
            item_X += 70;
        }
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            uint item_num = 0;
            for (uint i = 0; i < 6; ++i)
            {
                if (pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
                    ++item_num;
            }

            if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
                GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0); // Steal item  /  Украсть предмет
            else
                GameUI_StatusBar_DrawImmediate(localization->GetString(195), 0); // Buy item  /  Выберите предмет для покупки

            if (item_num)
            { // this shoudl go into func??
				int testx = (mouse.x-30 ) / 70;
				// testx limits check
				if (testx >= 0 && testx < 6) {

					item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
					
					if (item->uItemID) // item picking
					{
						//could check x coords imits here?
						if (mouse.y >= weapons_Ypos[testx]+30 && mouse.y < (weapons_Ypos[testx] +30+ shop_ui_items_in_store[testx]->GetHeight())) {

							//item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];

							String str;
							if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal())
							{
								str = BuildDialogueString(
									pMerchantsBuyPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_WeaponShop, (int)window_SpeakInHouse->ptr_1C, 2)],
									uActiveCharacter - 1,
									item,
									(char *)window_SpeakInHouse->ptr_1C,
									2
								);
							}
							else
							{
								str = BuildDialogueString(
									localization->GetString(181),
									uActiveCharacter - 1,
									item,
									(char *)window_SpeakInHouse->ptr_1C,
									2
								);
							}
							dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
						}
					}
				}
            }
            else
            {
                dialog_window.DrawShops_next_generation_time_string(
                    pParty->PartyTimes.Shops_next_generation_time[(unsigned int)window_SpeakInHouse->ptr_1C] - pParty->GetPlayingTime()
                );//Приходите через 7 дней
            }
        }
        break;
    }

    case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL:
    {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        item_X = 0;
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
            {
                render->DrawTextureAlphaNew(
                    ((60 - ((signed int)shop_ui_items_in_store[i]->GetWidth() / 2)) + item_X) / 640.0f,
                    (weapons_Ypos[i] + 30) / 480.0f,
                    shop_ui_items_in_store[i]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[(60 - ((signed int)shop_ui_items_in_store[i]->uTextureWidth / 2)) + item_X + window->GetWidth() * (weapons_Ypos[i] + 30)], shop_ui_items_in_store[i], i + 1);
            }
            item_X += 70;
        }
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            uint item_num = 0;
            for (uint i = 0; i < 6; ++i)
            {
                if (pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
                    ++item_num;
            }

            if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
                GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0); // Steal item  /  Украсть предмет
            else
                GameUI_StatusBar_DrawImmediate(localization->GetString(196), 0); // Buy item  /  Выберите предмет для покупки

            if (item_num)
            {
				int testx = (mouse.x-30) / 70;
				// testx limits check
				if (testx >= 0 && testx < 6) {

					item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];

					if (item->uItemID) // item picking
					{
						//could check x coords imits here?
						if (mouse.y >= weapons_Ypos[testx] + 30 && mouse.y < (weapons_Ypos[testx] + 30 + shop_ui_items_in_store[testx]->GetHeight())) {
							// if (render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF)
							 //{
							   //  item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];

							String str;
							if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal())
							{
								str = BuildDialogueString(
									pMerchantsBuyPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_WeaponShop, (int)window_SpeakInHouse->ptr_1C, 2)],
									uActiveCharacter - 1,
									item,
									(char *)window_SpeakInHouse->ptr_1C,
									2
								);
							}
							else
							{
								str = BuildDialogueString(
									localization->GetString(181),
									uActiveCharacter - 1,
									item,
									(char *)window_SpeakInHouse->ptr_1C,
									2
								);
							}
							dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
						}
					}
					}
            }
            else
                dialog_window.DrawShops_next_generation_time_string(
                    pParty->PartyTimes.Shops_next_generation_time[(unsigned int)window_SpeakInHouse->ptr_1C] - pParty->GetPlayingTime()
                );
        }
        break;
    }

    case HOUSE_DIALOGUE_SHOP_SELL:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(199), 0);
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;

        v109 = ((mouse.x + 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
        if (mouse.x <= 13 || mouse.x >= 462)
            return;
        pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v109);
        if (!pItemID)
            return;
        item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
        phrases_id = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_WeaponShop, (int)window_SpeakInHouse->ptr_1C, 3);
        auto str = BuildDialogueString(
            pMerchantsSellPhrases[phrases_id],
            uActiveCharacter - 1,
            item,
            (char *)window_SpeakInHouse->ptr_1C,
            3
        );
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
        break;
    }
    case HOUSE_DIALOGUE_SHOP_IDENTIFY:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(197), 0);
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;

        v109 = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
        if (mouse.x <= 13 || mouse.x >= 462)
            return;
        pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v109);
        if (!pItemID)
            return;
        item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];

        String str;
        if (!item->IsIdentified())
        {
            phrases_id = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_WeaponShop, (int)window_SpeakInHouse->ptr_1C, 4);
            str = BuildDialogueString(
                pMerchantsIdentifyPhrases[phrases_id],
                uActiveCharacter - 1,
                item,
                (char *)window_SpeakInHouse->ptr_1C,
                4
            );
        }
        else
        {
            str = BuildDialogueString(
                "%24",
                uActiveCharacter - 1,
                item,
                (char *)window_SpeakInHouse->ptr_1C,
                4
            );
        }
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
        break;
    }

    case HOUSE_DIALOGUE_SHOP_REPAIR:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(198), 0);
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;

        v109 = ((mouse.x - 14) >> 5) + 14 * (mouse.y - 17) >> 5;
        if ((mouse.x <= 13) || mouse.x >= 462)
            return;
        pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v109);
        if (!pItemID || (!(pPlayers[uActiveCharacter]->pOwnItems[pItemID - 1].uAttributes & 2)))
            return;
        item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
        phrases_id = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_ArmorShop, (int)window_SpeakInHouse->ptr_1C, 5);
        auto str = BuildDialogueString(
            pMerchantsRepairPhrases[phrases_id],
            uActiveCharacter - 1,
            item,
            (char *)window_SpeakInHouse->ptr_1C,
            5
        );
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
        return;
    }
    break;

    case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        pShopOptions[0] = localization->GetString(200); //sell
        pShopOptions[1] = localization->GetString(113); //identify
        pShopOptions[2] = localization->GetString(179); //repair
        all_text_height = 0;
        for (int i = 0; i < 3; ++i)
            all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialog_window, 0);
        v103 = (174 - all_text_height) / 3;
        v19 = (3 * (58 - (signed int)v103) - all_text_height) / 2 - ((174 - all_text_height) / 3) / 2 + 138;
        pNumString = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
        {
            pButton = pDialogueWindow->GetControl(i);
            pButton->uY = v103 + v19;
            pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0);
            v19 = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0) - 1;
            pButton->uW = v19;
            pColorText = Color16(0xE1u, 0xCDu, 0x23u);
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
            dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
            ++pNumString;
        }
        break;
    }

    case HOUSE_DIALOGUE_LEARN_SKILLS:
    {
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;
        uint item_num = 0;
        all_text_height = 0;
        v32 = (signed __int64)(p2DEvents[(signed int)window_SpeakInHouse->ptr_1C - 1].flt_24 * 500.0);
        pPrice = v32 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
        if (pPrice < v32 / 3)
            pPrice = v32 / 3;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
        {
            if (byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pDialogueWindow->GetControl(i)->msg_param - 36]
                && !pPlayers[uActiveCharacter]->pActiveSkills[pDialogueWindow->GetControl(i)->msg_param - 36])
            {
                all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(pDialogueWindow->GetControl(i)->msg_param - 36), &dialog_window, 0);
                item_num++;
            }
        }
        if (item_num)
        {
            auto str = localization->FormatString(401, pPrice); // Skill price: %lu   /   Стоимость навыка: %lu
            dialog_window.DrawTitleText(pFontArrus, 0, 0x92u, 0, str, 3);
            v103 = (149 - all_text_height) / item_num;
            if ((149 - all_text_height) / item_num > 32)
                v103 = 32;
            v19 = (149 - item_num * v103 - all_text_height) / 2 - v103 / 2 + 162;
            for (uint i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
            {
                pButton = pDialogueWindow->GetControl(i);
                if (!byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pButton->msg_param - 36] || pPlayers[uActiveCharacter]->pActiveSkills[pButton->msg_param - 36])
                {
                    pButton->uW = 0;
                    pButton->uHeight = 0;
                    pButton->uY = 0;
                }
                else
                {
                    pButton->uY = v103 + v19;
                    pButton->uHeight = pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialog_window, 0);
                    v19 = pButton->uY + pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialog_window, 0) - 1;
                    pButton->uW = v19;
                    pColorText = Color16(0xE1u, 0xCDu, 0x23u);
                    if (pDialogueWindow->pCurrentPosActiveItem != i)
                        pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
                    dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, localization->GetSkillName(pButton->msg_param - 36), 3);
                }
            }
            break;
        }

        auto str =
            localization->FormatString(
                544,
                pPlayers[uActiveCharacter]->pName,
                localization->GetClassName(pPlayers[uActiveCharacter]->classType)
            )
            + "\n \n"
            + localization->GetString(528);//Больше ничего не могу предложить.

        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0, 0)) / 2 + 138, Color16(0xE1u, 0xCDu, 0x23u), str, 3);
        return;
    }
    default:
    {
        __debugbreak();
        break;
    }
    }
}

//----- (004BA928) --------------------------------------------------------
void ArmorShopDialog()
{
    int phrases_id; // eax@20
    int all_text_height; // ebx@22
    int v146; // [sp+E4h] [bp-20h]@24
    int v23; // ebx@24
    GUIButton *pButton; // eax@26
    signed int v38; // esi@42
    int v59; // eax@68
    ItemGen *selected_item; // ecx@99
    int pNumString; // edx@109
    unsigned __int16 pTextColor; // ax@109
    GUIWindow dialog_window; // [sp+8Ch] [bp-78h]@1
    __int32 pItemCount; // [sp+F0h] [bp-14h]@8
    int item_x; // [sp+100h] [bp-4h]@44
    int pPrice;
    unsigned int v153;

    memcpy(&dialog_window, window_SpeakInHouse, sizeof(dialog_window));
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 148;
    dialog_window.uFrameZ = 334;

    Point mouse = pMouse->GetCursorPos();
    switch (dialog_menu_id)
    {
        case HOUSE_DIALOGUE_MAIN:
        {
            if (!HouseUI_CheckIfPlayerCanInteract())
                return;
            pShopOptions[0] = localization->GetString(134); // Buy Standard
            pShopOptions[1] = localization->GetString(152); // Buy Special
            pShopOptions[2] = localization->GetString(159); // Display Inventory
            pShopOptions[3] = localization->GetString(160);
            all_text_height = 0;
            for (int i = 0; i < 4; ++i)
                all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialog_window, 0);
            v146 = (174 - all_text_height) / 4;
            v23 = (174 - 4 * (174 - all_text_height) / 4 - all_text_height) / 2 - (174 - all_text_height) / 4 / 2 + 138;
            pNumString = 0;
            for (
                int i = pDialogueWindow->pStartingPosActiveItem;
                i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem;
                ++i
            )
            {
                pButton = pDialogueWindow->GetControl(i);
                pButton->uY = v146 + v23;
                pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0);
                v23 = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0) - 1;
                pButton->uW = v23;
                pTextColor = Color16(0xE1u, 0xCDu, 0x23u);
                if (pDialogueWindow->pCurrentPosActiveItem != i)
                    pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
                dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor, pShopOptions[pNumString], 3);
                ++pNumString;
            }
            break;
        }

        case HOUSE_DIALOGUE_SHOP_BUY_STANDARD:
        {
            render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
            item_x = 0;
            for (int i = 0; i < 8; ++i)// разместить вещи
            {
                if (pParty->StandartItemsInShops[window_SpeakInHouse->par1C][i].uItemID)
                {
                    if (i >= 4)  //low row
                    {
                        render->DrawTextureAlphaNew(
                            ((90 - (shop_ui_items_in_store[i]->GetWidth() / 2)) + item_x - 420) / 640.0f,
                            126 / 480.0f,
                            shop_ui_items_in_store[i]);
                        v59 = (90 - (shop_ui_items_in_store[i]->GetWidth() / 2)) + item_x + 80220;
                    }
                    else
                    {
                        render->DrawTextureAlphaNew(
                            ((86 - (shop_ui_items_in_store[i]->GetWidth() / 2)) + item_x) / 640.0f,
                            (98 - shop_ui_items_in_store[i]->GetHeight()) / 480.0f,
                            shop_ui_items_in_store[i]);
                        v59 = item_x + (86 - (shop_ui_items_in_store[i]->GetWidth() / 2)) + window->GetWidth() * (98 - shop_ui_items_in_store[i]->GetHeight());
                    }
                    //ZBuffer_DoFill(&render->pActiveZBuffer[v59], shop_ui_items_in_store[i], i + 1);
                }
                item_x += 105;
            }
            if (!HouseUI_CheckIfPlayerCanInteract())
                return;
            pItemCount = 0;
            for (int i = 0; i < 8; ++i)
            {
                if (pParty->StandartItemsInShops[window_SpeakInHouse->par1C][i].uItemID)
                    ++pItemCount;
            }

            if (OS_IfCtrlPressed() == 0 || pPlayers[uActiveCharacter]->CanSteal() == 0)
                GameUI_StatusBar_DrawImmediate(localization->GetString(195), 0); // Select the Item to Buy
            else
                GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0); // Steal item

            if (pItemCount)
            { // this should go into func??

				int testx = (mouse.x-40 ) / 105;
				// testx limits check
				if (testx >= 0 && testx < 4) {
					if (mouse.y >= 126){
						testx += 4;
					}
					//if (!(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF))
					//	return;
					//selected_item = &pParty->StandartItemsInShops[window_SpeakInHouse->par1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];
					selected_item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
					
					if (selected_item->uItemID) // item picking
					{
						//could check x coords imits here?
						if ( (mouse.y >= 126 && mouse.y < (126 +shop_ui_items_in_store[testx]->GetHeight())) || 
							( mouse.y <= 98 && mouse.y >= (98-shop_ui_items_in_store[testx]->GetHeight() ) ) ) {
							// y is 126 to 126 + height low or 98-height to 98

							//item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];

							String str;
				         if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal())
					         {
				             str = BuildDialogueString(pMerchantsBuyPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(selected_item, BuildingType_ArmorShop, window_SpeakInHouse->par1C, 2)],
			                uActiveCharacter - 1, selected_item, (char *)window_SpeakInHouse->ptr_1C, 2);
					        }
				        else
				       {
				           str = BuildDialogueString(localization->GetString(181), uActiveCharacter - 1, selected_item, (char *)window_SpeakInHouse->ptr_1C, 2); //"Steal %24"
				      }
					      dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
						}
					}
				}
                

                
                return;
            }
            dialog_window.DrawShops_next_generation_time_string(
                pParty->PartyTimes.Shops_next_generation_time[window_SpeakInHouse->par1C] - pParty->GetPlayingTime()
            );

            return;
        }

        case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL: // could special and standard be collapsed into 1 very similar codes ??
        {
            render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
            item_x = 0;
            for (int i = 0; i < 8; ++i)
            {
                if (pParty->SpecialItemsInShops[window_SpeakInHouse->par1C][i].uItemID)
                {
                    if (i >= 4)
                    {
                        render->DrawTextureAlphaNew(
                            (item_x + (90 - ((signed int)shop_ui_items_in_store[i]->GetWidth() / 2)) - 420) / 640.0f,
                            126 / 480.0f,
                            shop_ui_items_in_store[i]);
                        v59 = item_x + (90 - ((signed int)shop_ui_items_in_store[i]->GetWidth() / 2)) + 80220;
                    }
                    else
                    {
                        render->DrawTextureAlphaNew(
                            (86 - ((signed int)shop_ui_items_in_store[i]->GetWidth() / 2) + item_x) / 640.0f,
                            (98 - shop_ui_items_in_store[i]->GetHeight()) / 480.0f,
                            shop_ui_items_in_store[i]);
                        v59 = (86 - ((signed int)shop_ui_items_in_store[i]->GetWidth() / 2)) + item_x + window->GetWidth() * (98 - shop_ui_items_in_store[i]->GetHeight());
                    }
                    //ZBuffer_DoFill(&render->pActiveZBuffer[v59], shop_ui_items_in_store[i], i + 1);
                }
                item_x += 105;
            }
            if (!HouseUI_CheckIfPlayerCanInteract())
                return;
            pItemCount = 0;
            for (uint i = 0; i < 6; ++i)
            {
                if (pParty->SpecialItemsInShops[window_SpeakInHouse->par1C][i].uItemID)
                    ++pItemCount;
            }
            if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal())
                GameUI_StatusBar_DrawImmediate(localization->GetString(196), 0); //Select the Special Item to Buy
            else
                GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0);

            if (pItemCount)
            {
                if (!(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF))
                    return;
                pItemCount = (render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1;
                if (dialog_menu_id == 2)
                    selected_item = &pParty->StandartItemsInShops[window_SpeakInHouse->par1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];
                else
                    selected_item = &pParty->SpecialItemsInShops[window_SpeakInHouse->par1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];

                String str;
                if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal())
                {
                    str = BuildDialogueString(pMerchantsBuyPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(selected_item, BuildingType_ArmorShop, window_SpeakInHouse->par1C, 2)],
                        uActiveCharacter - 1, selected_item, (char *)window_SpeakInHouse->ptr_1C, 2);
                }
                else
                {
                    str = BuildDialogueString(localization->GetString(181), uActiveCharacter - 1, selected_item, (char *)window_SpeakInHouse->ptr_1C, 2);//"Steal %24"
                }
                dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
                return;
            }
            dialog_window.DrawShops_next_generation_time_string(
                pParty->PartyTimes.Shops_next_generation_time[window_SpeakInHouse->par1C] - pParty->GetPlayingTime()
            );

            return;
        }

    case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        pShopOptions[0] = localization->GetString(200); // Sell
        pShopOptions[1] = localization->GetString(113); // Identify
        pShopOptions[2] = localization->GetString(179); // Repair
        all_text_height = 0;
        for (int i = 0; i < 3; ++i)
            all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialog_window, 0);
        v146 = (174 - all_text_height) / 3;
        v23 = (3 * (58 - v146) - all_text_height) / 2 - v146 / 2 + 138;
        pNumString = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
        {
            pButton = pDialogueWindow->GetControl(i);
            pButton->uY = v146 + v23;
            pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0);
            v23 = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0) - 1;
            pButton->uW = v23;
            pTextColor = Color16(0xE1u, 0xCDu, 0x23u);
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
            dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor, pShopOptions[pNumString], 3);
            ++pNumString;
        }
        return;
    }
    break;

    case HOUSE_DIALOGUE_SHOP_SELL:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(199), 0);//"Select the Item to Sell"
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;

        v153 = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
        if (mouse.x <= 13 || mouse.x >= 462)
            return;
        if (!pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v153))
            return;
        selected_item = &pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v153) - 1];
        auto str = BuildDialogueString(pMerchantsSellPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(selected_item, BuildingType_ArmorShop, window_SpeakInHouse->par1C, 3)],
            uActiveCharacter - 1, selected_item, (char *)window_SpeakInHouse->par1C, 3);
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
        return;
    }
    break;
    case HOUSE_DIALOGUE_SHOP_IDENTIFY:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(197), 0); //"Select the Item to Identify"	
        if (HouseUI_CheckIfPlayerCanInteract())
        {

            v153 = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
            if (mouse.x > 13 && mouse.x < 462)
            {
                if (pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v153))
                {
                    selected_item = &pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v153) - 1];

                    String str;
                    if (selected_item->IsIdentified())
                        str = BuildDialogueString("%24", uActiveCharacter - 1, selected_item, (char *)window_SpeakInHouse->ptr_1C, 4);
                    else
                        str = BuildDialogueString(pMerchantsIdentifyPhrases[pPlayers[uActiveCharacter]->SelectPhrasesTransaction(selected_item, BuildingType_ArmorShop,
                            (int)window_SpeakInHouse->ptr_1C, 4)], uActiveCharacter - 1, selected_item, (char *)window_SpeakInHouse->ptr_1C, 4);
                    dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
                    return;
                }
            }
        }
    }
    break;
    case HOUSE_DIALOGUE_SHOP_REPAIR:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(198), 0);
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;

        v153 = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
        if (mouse.x <= 13 || mouse.x >= 462)
            return;
        if (!pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v153)
            || (!(pPlayers[uActiveCharacter]->pOwnItems[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v153) - 1].uAttributes & 2)))
            return;
        selected_item = &pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v153) - 1];
        phrases_id = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(&pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v153) - 1],
            BuildingType_ArmorShop, window_SpeakInHouse->par1C, 5);
        auto str = BuildDialogueString(pMerchantsRepairPhrases[phrases_id], uActiveCharacter - 1, selected_item, (char *)window_SpeakInHouse->ptr_1C, 5);
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
        return;
    }
    break;

    case HOUSE_DIALOGUE_LEARN_SKILLS:
    {
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;
        uint item_num = 0;
        v38 = (signed __int64)(p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].flt_24 * 500.0);
        pPrice = v38 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
        if ((signed int)pPrice < v38 / 3)
            pPrice = v38 / 3;
        all_text_height = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
        {
            if (byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pDialogueWindow->GetControl(i)->msg_param - 36]
                && !pPlayers[uActiveCharacter]->pActiveSkills[pDialogueWindow->GetControl(i)->msg_param - 36])
            {
                all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(pDialogueWindow->GetControl(i)->msg_param - 36), &dialog_window, 0);
                item_num++;
            }
        }
        if (item_num)
        {
            auto str = localization->FormatString(401, pPrice);
            dialog_window.DrawTitleText(pFontArrus, 0, 0x92u, 0, str, 3);
            v146 = (149 - all_text_height) / item_num;
            if ((149 - all_text_height) / item_num > 32)
                v146 = 32;
            v23 = (149 - v146 * item_num - all_text_height) / 2 - v146 / 2 + 162;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
            {
                pButton = pDialogueWindow->GetControl(i);
                if (!byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pButton->msg_param - 36]
                    || pPlayers[uActiveCharacter]->pActiveSkills[pButton->msg_param - 36])
                {
                    pButton->uW = 0;
                    pButton->uHeight = 0;
                    pButton->uY = 0;
                }
                else
                {
                    pButton->uY = v146 + v23;
                    pButton->uHeight = pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialog_window, 0);
                    v23 = pButton->uY + pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialog_window, 0) - 1;
                    pButton->uW = v23;
                    pTextColor = Color16(0xE1u, 0xCDu, 0x23u);
                    if (pDialogueWindow->pCurrentPosActiveItem != i)
                        pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
                    dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor, localization->GetSkillName(pButton->msg_param - 36), 3);
                }
            }
            return;
        }

        auto str =
            localization->FormatString(
                544,
                pPlayers[uActiveCharacter]->pName,
                localization->GetClassName(pPlayers[uActiveCharacter]->classType) // Seek knowledge elsewhere %s the %s
                )
            + "\n \n"
            + localization->GetString(528); // I can offer you nothing further.
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xE1u, 0xCDu, 0x23u), str, 3);
        return;
    }
    break;
    default:
        __debugbreak();
    }
}

//----- (004B9CC6) --------------------------------------------------------
void  AlchemistDialog()
{
    int index; // eax@7
    signed int v5; // esi@9
    int v18; // eax@23
    const char *pText; // edx@29
    unsigned int product_height_1row; // edi@55
    unsigned int product_width_1row; // esi@57
    unsigned int product_height_2row; // edi@67
    unsigned int product_width_2row; // esi@69
    ItemGen *item; // esi@118
    int v71; // eax@123
    int all_text_height; // edi@125
    GUIButton *pButton; // esi@129
    unsigned int pColorText; // ax@129
    GUIWindow dialog_window; // [sp+5Ch] [bp-7Ch]@1
    int v105; // [sp+B0h] [bp-28h]@19
    int pNumString; // [sp+C8h] [bp-10h]@9
    int item_num; // [sp+D4h] [bp-4h]@11
    int pPrice;

    memcpy(&dialog_window, window_SpeakInHouse, sizeof(dialog_window));
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 148;
    dialog_window.uFrameZ = 334;

    Point mouse = pMouse->GetCursorPos();
    switch (dialog_menu_id)
    {
    case HOUSE_DIALOGUE_MAIN:
    {
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            pShopOptions[0] = localization->GetString(134);
            pShopOptions[1] = localization->GetString(152);
            pShopOptions[2] = localization->GetString(159);
            pShopOptions[3] = localization->GetString(160);
            all_text_height = 0;
            for (int i = 0; i < 4; ++i)
                all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialog_window, 0);
            v18 = (174 - all_text_height) / 4;
            v105 = (174 - 4 * (174 - all_text_height) / 4 - all_text_height) / 2 - (174 - all_text_height) / 4 / 2 + 138;
            pNumString = 0;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
            {
                pButton = pDialogueWindow->GetControl(i);
                pButton->uY = v18 + v105;
                pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0);
                v105 = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0) - 1;
                pButton->uW = v105;
                pColorText = Color16(0xE1u, 0xCDu, 0x23u);
                if (pDialogueWindow->pCurrentPosActiveItem != i)
                    pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
                dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
                ++pNumString;
            }
        }
        return;
    }

    case HOUSE_DIALOGUE_SHOP_BUY_STANDARD:
    {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
            {
                product_height_1row = 152 - shop_ui_items_in_store[i]->GetHeight();
                if ((signed int)product_height_1row < 1)
                    product_height_1row = 0;
                product_width_1row = 75 * i - shop_ui_items_in_store[i]->GetWidth() / 2 + 40;
                if (i)
                {
                    if (i == 5)
                    {
                        if ((signed int)product_width_1row > 457 - shop_ui_items_in_store[5]->GetWidth())
                            product_width_1row = 457 - shop_ui_items_in_store[5]->GetWidth();
                    }
                }
                else if ((signed int)product_width_1row < 18)
                    product_width_1row = 18;
                render->DrawTextureAlphaNew(product_width_1row / 640.0f, product_height_1row / 480.0f, shop_ui_items_in_store[i]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[product_width_1row + window->GetWidth() * product_height_1row], shop_ui_items_in_store[i], i + 1);
            }
        }
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i + 6].uItemID)
            {
                product_height_2row = 308 - shop_ui_items_in_store[i + 6]->GetHeight();
                if ((signed int)product_height_2row < 1)
                    product_height_2row = 0;
                product_width_2row = 75 * i - shop_ui_items_in_store[i + 6]->GetWidth() / 2 + 40;
                if (i)
                {
                    if (i == 5)
                    {
                        if ((signed int)product_width_2row > 457 - shop_ui_items_in_store[11]->GetWidth())
                            product_width_2row = 457 - shop_ui_items_in_store[11]->GetWidth();
                    }
                }
                else
                {
                    if ((signed int)product_width_2row < 18)
                        product_width_2row = 18;
                }
                render->DrawTextureAlphaNew(product_width_2row / 640.0f, product_height_2row / 480.0f, shop_ui_items_in_store[i + 6]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[product_width_2row + window->GetWidth() * product_height_2row], shop_ui_items_in_store[i + 6], i + 7);
            }
        }
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            item_num = 0;
            for (uint i = 0; i < 12; ++i)
            {
                if (pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
                    ++item_num;
            }
            if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
                GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0);
            else
                GameUI_StatusBar_DrawImmediate(localization->GetString(195), 0);
            if (!item_num)
            {
                dialog_window.DrawShops_next_generation_time_string(
                    pParty->PartyTimes.Shops_next_generation_time[(unsigned int)window_SpeakInHouse->ptr_1C] - pParty->GetPlayingTime()
                    );
                return;
            }

            if (render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF)
            {
                item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];

                String str;
                if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal())
                {
                    v71 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_AlchemistShop, (int)window_SpeakInHouse->ptr_1C, 2);
                    str = BuildDialogueString(pMerchantsBuyPhrases[v71], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);
                }
                else
                    str = BuildDialogueString(localization->GetString(181), uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);
                dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
                return;
            }
        }
        return;
    }

    case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL:
    {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
            {
                product_height_1row = 152 - shop_ui_items_in_store[i]->GetHeight();
                if ((signed int)product_height_1row < 1)
                    product_height_1row = 0;
                product_width_1row = 75 * i - shop_ui_items_in_store[i]->GetWidth() / 2 + 40;
                if (i)
                {
                    if (i == 5)
                    {
                        if ((signed int)product_width_1row > 457 - shop_ui_items_in_store[5]->GetWidth())
                            product_width_1row = 457 - shop_ui_items_in_store[5]->GetWidth();
                    }
                }
                else
                {
                    if ((signed int)product_width_1row < 18)
                        product_width_1row = 18;
                }
                render->DrawTextureAlphaNew(product_width_1row / 640.0f, product_height_1row / 480.0f, shop_ui_items_in_store[i]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[product_width_1row + window->GetWidth() * product_height_1row], shop_ui_items_in_store[i], i + 1);
            }
        }
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][(signed int)i + 6].uItemID) //not itemid
            {
                product_height_2row = 308 - shop_ui_items_in_store[i + 6]->GetHeight();
                if ((signed int)product_height_2row < 1)
                    product_height_2row = 0;
                product_width_2row = 75 * i - shop_ui_items_in_store[i + 6]->GetWidth() / 2 + 40;
                if (i)
                {
                    if (i == 5)
                    {
                        if ((signed int)product_width_2row > 457 - shop_ui_items_in_store[11]->GetWidth())
                            product_width_2row = 457 - shop_ui_items_in_store[11]->GetWidth();
                    }
                }
                else
                {
                    if ((signed int)product_width_2row < 18)
                        product_width_2row = 18;
                }
                render->DrawTextureAlphaNew(product_width_2row / 640.0f, product_height_2row / 480.0f, shop_ui_items_in_store[i + 6]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[product_width_2row + window->GetWidth() * product_height_2row], shop_ui_items_in_store[i + 6], i + 7);
            }
        }
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            item_num = 0;
            for (uint i = 0; i < 12; ++i)
            {
                if (pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
                    ++item_num;
            }
            if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
                GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0);
            else
                GameUI_StatusBar_DrawImmediate(localization->GetString(196), 0);
            if (!item_num)
            {
                dialog_window.DrawShops_next_generation_time_string(
                    pParty->PartyTimes.Shops_next_generation_time[(unsigned int)window_SpeakInHouse->ptr_1C] - pParty->GetPlayingTime()
                    );
                return;
            }

            if (render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF)
            {
                item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];

                String str;
                if (!OS_IfCtrlPressed() || !pPlayers[uActiveCharacter]->CanSteal())
                {
                    v71 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_AlchemistShop, (int)window_SpeakInHouse->ptr_1C, 2);
                    str = BuildDialogueString(pMerchantsBuyPhrases[v71], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);
                }
                else
                    str = BuildDialogueString(localization->GetString(181), uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);
                dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
                return;
            }
        }
        return;
    }

    case HOUSE_DIALOGUE_SHOP_SELL:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(199), 0);
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;

        index = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
        if (mouse.x <= 13 || mouse.x >= 462)
            return;
        if (!pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&index))
            return;
        v71 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(&pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&index) - 1],
            BuildingType_AlchemistShop, (int)window_SpeakInHouse->ptr_1C, 3);
        auto str = BuildDialogueString(pMerchantsSellPhrases[v71], uActiveCharacter - 1, &pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&index) - 1],
            (char *)window_SpeakInHouse->ptr_1C, 3);
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
        return;
    }

    case HOUSE_DIALOGUE_SHOP_IDENTIFY:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(197), 0);
        if (HouseUI_CheckIfPlayerCanInteract())
            return;

        index = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
        if (mouse.x <= 13 || mouse.x >= 462)
            return;
        if (!pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&index))
            return;
        item = &pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&index) - 1];

        String str;
        if (!item->IsIdentified())
        {
            v71 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_AlchemistShop, (int)window_SpeakInHouse->ptr_1C, 4);
            str = BuildDialogueString(pMerchantsIdentifyPhrases[v71], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 4);
        }
        else
            str = BuildDialogueString("%24", uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 4);
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
        return;
    }

    case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        pShopOptions[0] = localization->GetString(200);
        pShopOptions[1] = localization->GetString(113);
        all_text_height = 0;
        for (int i = 0; i < 2; ++i)
            all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialog_window, 0);
        v18 = (174 - all_text_height) / 2;
        v105 = (2 * (87 - (174 - all_text_height) / 2) - all_text_height) / 2 - (174 - all_text_height) / 2 / 2 + 138;
        pNumString = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
        {
            pButton = pDialogueWindow->GetControl(i);
            pButton->uY = v18 + v105;
            pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0);
            v105 = pButton->uY + pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0) - 1;
            pButton->uW = v105;
            pColorText = Color16(0xE1u, 0xCDu, 0x23u);
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
            dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
            ++pNumString;
        }
        return;
    }

    case HOUSE_DIALOGUE_LEARN_SKILLS:
    {
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;
        all_text_height = 0;
        v5 = (signed __int64)(p2DEvents[(signed int)window_SpeakInHouse->ptr_1C - 1].flt_24 * 500.0);
        pPrice = v5 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
        if (pPrice < v5 / 3)
            pPrice = v5 / 3;
        item_num = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
        {
            pButton = pDialogueWindow->GetControl(i);
            if (byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pButton->msg_param - 36]
                && !pPlayers[uActiveCharacter]->pActiveSkills[pButton->msg_param - 36])
            {
                all_text_height = pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialog_window, 0);
                ++item_num;
            }
        }
        if (!item_num)
        {
            auto str =
                localization->FormatString(
                    544,
                    pPlayers[uActiveCharacter]->pName,
                    localization->GetClassName(pPlayers[uActiveCharacter]->classType)
                    )
                + "\n \n"
                + localization->GetString(528);
            dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(0xE1u, 0xCDu, 0x23u), str, 3);
            return;
        }
        if (item_num)
        {
            auto str = localization->FormatString(401, pPrice);
            dialog_window.DrawTitleText(pFontArrus, 0, 0x92u, 0, str, 3);

            v18 = (149 - all_text_height) / item_num;
            if ((149 - all_text_height) / item_num > 32)
                v18 = 32;
            v105 = (149 - item_num * v18 - all_text_height) / 2 - v18 / 2 + 162;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; i++)
            {
                pButton = pDialogueWindow->GetControl(i);
                if (!byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pButton->msg_param - 36]
                    || pPlayers[uActiveCharacter]->pActiveSkills[pButton->msg_param - 36])
                {
                    pButton->uW = 0;
                    pButton->uHeight = 0;
                    pButton->uY = 0;
                }
                else
                {
                    pButton->uY = v18 + v105;
                    pButton->uHeight = pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialog_window, 0);
                    v105 = pButton->uY + pFontArrus->CalcTextHeight(localization->GetSkillName(pButton->msg_param - 36), &dialog_window, 0) - 1;
                    pButton->uW = v105;
                    pColorText = Color16(0xE1u, 0xCDu, 0x23u);
                    if (pDialogueWindow->pCurrentPosActiveItem != i)
                        pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
                    dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, localization->GetSkillName(pButton->msg_param - 36), 3);
                }
            }
            return;
        }
    }
    default:
        __debugbreak();
    }
}

//----- (004B4FCF) --------------------------------------------------------
void MagicShopDialog()
{
    int all_text_height; // edi@21
    GUIButton *control_button; // esi@25
    int v23; // eax@25
    unsigned __int16 text_color; // ax@25
    signed int v33; // esi@40
    unsigned int product_height_1row; // edi@64
    unsigned int product_width_1row; // esi@66
    unsigned int product_height_2row; // edi@76
    unsigned int product_width_2row; // esi@76
    int v75; // eax@130
    GUIWindow dialog_window; // [sp+7Ch] [bp-7Ch]@1
    int item_num; // [sp+E0h] [bp-18h]@8
    int v117; // [sp+E4h] [bp-14h]@40
    int pPrice; // [sp+E8h] [bp-10h]@24
    int one_string; // [sp+ECh] [bp-Ch]@1
    int pSrtingNum;
    ItemGen *item;

    memcpy(&dialog_window, window_SpeakInHouse, sizeof(dialog_window));
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 148;
    dialog_window.uFrameZ = 334;

    Point mouse = pMouse->GetCursorPos();
    if (dialog_menu_id == HOUSE_DIALOGUE_MAIN)
    {
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;
        pShopOptions[0] = localization->GetString(134); //"Buy Standard"
        pShopOptions[1] = localization->GetString(152); //"Buy Special"
        pShopOptions[2] = localization->GetString(159); //"Display Inventory"
        pShopOptions[3] = localization->GetString(160);
        all_text_height = 0;
        for (int i = 0; i < 4; ++i)
            all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialog_window, 0);
        one_string = (174 - all_text_height) / 4;
        v23 = (174 - 4 * one_string - all_text_height) / 2 - one_string / 2 + 138;
        int pNumString = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem;  ++i)
        {
            control_button = pDialogueWindow->GetControl(i);
            control_button->uY = one_string + v23;
            control_button->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], &dialog_window, 0);
            v23 = control_button->uY + control_button->uHeight - 1;
            control_button->uW = v23;
            text_color = Color16(225, 205, 35);
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                text_color = Color16(255, 255, 255);
            dialog_window.DrawTitleText(pFontArrus, 0, control_button->uY, text_color, pShopOptions[pNumString], 3);
            ++pNumString;
        }
        return;
    }

    if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
    {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
            {
                product_height_1row = 152 - shop_ui_items_in_store[i]->GetHeight();
                if ((signed int)product_height_1row < 1)
                    product_height_1row = 0;
                product_width_1row = 75 * i - shop_ui_items_in_store[i]->GetWidth() / 2 + 40;
                if (i)
                {
                    if (i == 5)
                    {
                        if ((signed int)product_width_1row > 457 - shop_ui_items_in_store[5]->GetWidth())
                            product_width_1row = 457 - shop_ui_items_in_store[5]->GetWidth();
                    }
                }
                else
                {
                    if ((signed int)product_width_1row < 18)
                        product_width_1row = 18;
                }
                render->DrawTextureAlphaNew(product_width_1row / 640.0f, product_height_1row / 480.0f, shop_ui_items_in_store[i]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[product_width_1row + window->GetWidth() * product_height_1row], shop_ui_items_in_store[i], i + 1);
            }
        }
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i + 6].uItemID)
            {
                product_height_2row = 306 - shop_ui_items_in_store[i + 6]->GetHeight();
                product_width_2row = 75 * i - shop_ui_items_in_store[i + 6]->GetWidth() / 2 + 40;
                if (i)
                {
                    if (i == 5)
                    {
                        if ((signed int)product_width_2row > 457 - shop_ui_items_in_store[11]->GetWidth())
                            product_width_2row = 457 - shop_ui_items_in_store[11]->GetWidth();
                    }
                }
                else
                {
                    if ((signed int)product_width_2row < 18)
                        product_width_2row = 18;
                }
                render->DrawTextureAlphaNew(product_width_2row / 640.0f, product_height_2row / 480.0f, shop_ui_items_in_store[i + 6]);
            }
        }
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            item_num = 0;
            for (uint i = 0; i < 12; ++i)
            {
                if (pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
                    ++item_num;
            }
            if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
                GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0);// "Steal item"
            else
                GameUI_StatusBar_DrawImmediate(localization->GetString(195), 0);// "Select the Item to Buy"
            if (!item_num)
            {
                dialog_window.DrawShops_next_generation_time_string(
                    pParty->PartyTimes.Shops_next_generation_time[(unsigned int)window_SpeakInHouse->ptr_1C] - pParty->GetPlayingTime()
                    );
                return;
            }

            if (render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF)
            {
                item = &pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];

                String str;
                if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
                    str = BuildDialogueString(localization->GetString(181), uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);// "Steal %24"
                else
                {
                    v75 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_MagicShop, (int)window_SpeakInHouse->ptr_1C, 2);
                    str = BuildDialogueString(pMerchantsBuyPhrases[v75], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);
                }
                dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(255, 255, 255), str, 3);
                return;
            }
        }
        return;
    }

    if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_SPECIAL)
    {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
            {
                product_height_1row = 152 - shop_ui_items_in_store[i]->GetHeight();
                if ((signed int)product_height_1row < 1)
                    product_height_1row = 0;
                product_width_1row = 75 * i - shop_ui_items_in_store[i]->GetWidth() / 2 + 40;
                if (i)
                {
                    if (i == 5)
                    {
                        if ((signed int)product_width_1row > 457 - shop_ui_items_in_store[5]->GetWidth())
                            product_width_1row = 457 - shop_ui_items_in_store[5]->GetWidth();
                    }
                }
                else
                {
                    if ((signed int)product_width_1row < 18)
                        product_width_1row = 18;
                }
                render->DrawTextureAlphaNew(product_width_1row / 640.0f, product_height_1row / 480.0f, shop_ui_items_in_store[i]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[product_width_1row + window->GetWidth() * product_height_1row], shop_ui_items_in_store[i], i + 1);
            }
        }
        for (uint i = 0; i < 6; ++i)
        {
            if (pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i + 6].uItemID)
            {
                product_height_2row = 306 - shop_ui_items_in_store[i + 6]->GetHeight();
                if ((signed int)product_height_2row < 1)
                    product_height_2row = 0;
                product_width_2row = 75 * i - shop_ui_items_in_store[i + 6]->GetWidth() / 2 + 40;
                if (i)
                {
                    if (i == 5)
                    {
                        if ((signed int)product_width_2row > 457 - shop_ui_items_in_store[11]->GetWidth())
                            product_width_2row = 457 - shop_ui_items_in_store[11]->GetWidth();
                    }
                }
                else
                {
                    if ((signed int)product_width_2row < 18)
                        product_width_2row = 18;
                }
                render->DrawTextureAlphaNew(product_width_2row / 640.0f, product_height_2row / 480.0f, shop_ui_items_in_store[i + 6]);
                //ZBuffer_DoFill2(&render->pActiveZBuffer[product_width_2row + window->GetWidth() * product_height_2row], shop_ui_items_in_store[i + 6], i + 7);
            }
        }
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            item_num = 0;
            for (uint i = 0; i < 12; ++i)
            {
                if (pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][i].uItemID)
                    ++item_num;
            }
            if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
                GameUI_StatusBar_DrawImmediate(localization->GetString(185), 0);// "Steal item"
            else
                GameUI_StatusBar_DrawImmediate(localization->GetString(196), 0);// "Select the Special Item to Buy"
            if (!item_num)
            {
                dialog_window.DrawShops_next_generation_time_string(
                    pParty->PartyTimes.Shops_next_generation_time[(unsigned int)window_SpeakInHouse->ptr_1C] - pParty->GetPlayingTime()
                    );
                return;
            }

            if (render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF)
            {
                item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][(render->pActiveZBuffer[mouse.x + pSRZBufferLineOffsets[mouse.y]] & 0xFFFF) - 1];

                String str;
                if (OS_IfCtrlPressed() && pPlayers[uActiveCharacter]->CanSteal())
                    str = BuildDialogueString(localization->GetString(181), uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);// "Steal %24"
                else
                {
                    v75 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_MagicShop, (int)window_SpeakInHouse->ptr_1C, 2);
                    str = BuildDialogueString(pMerchantsBuyPhrases[v75], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 2);
                }
                dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(255, 255, 255), str, 3);
                return;
            }
        }
        return;
    }

    if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT)
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        pShopOptions[0] = localization->GetString(200);// "Sell"
        pShopOptions[1] = localization->GetString(113);// "Identify"
        pShopOptions[2] = localization->GetString(179);// "Repair"
        all_text_height = 0;
        for (uint i = 0; i < 3; ++i)
            all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], &dialog_window, 0);
        one_string = ((174 - all_text_height) / 3);
        v23 = (3 * (58 - (signed int)one_string) - all_text_height) / 2 - (174 - all_text_height) / 3 / 2 + 138;
        pSrtingNum = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
        {
            control_button = pDialogueWindow->GetControl(i);
            control_button->uY = one_string + v23;
            control_button->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pSrtingNum], &dialog_window, 0);
            v23 = control_button->uHeight + control_button->uY - 1;
            control_button->uW = v23;
            text_color = Color16(225, 205, 35);
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                text_color = Color16(255, 255, 255);
            dialog_window.DrawTitleText(pFontArrus, 0, control_button->uY, text_color, pShopOptions[pSrtingNum], 3);
            ++pSrtingNum;
        }
        return;
    }

    if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_SELL)
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(199), 0);// "Select the Item to Sell"
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;

        v117 = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
        if (mouse.x <= 13 || mouse.x >= 462)
            return;
        if (!pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v117))
            return;
        item = &pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v117) - 1];
        v75 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_MagicShop, (int)window_SpeakInHouse->ptr_1C, 3);
        auto str = BuildDialogueString(pMerchantsSellPhrases[v75], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 3);
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(255, 255, 255), str, 3);
        return;
    }

    if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_IDENTIFY)
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(197), 0);// "Select the Item to Identify"
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            v117 = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
            if (mouse.x > 13 && mouse.x < 462)
            {
                if (pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v117))
                {
                    item = &pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v117) - 1];
                    if (item->uAttributes & ITEM_IDENTIFIED)
                    {
                        auto str = BuildDialogueString("%24", uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 4);
                        dialog_window.DrawTitleText(pFontArrus, 0, (212 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 101,
                            Color16(255, 255, 255), str, 3);
                        return;
                    }
                    v75 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(item, BuildingType_MagicShop, (int)window_SpeakInHouse->ptr_1C, 4);
                    auto str = BuildDialogueString(pMerchantsIdentifyPhrases[v75], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 4);
                    dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(255, 255, 255), str, 3);
                    return;
                }
            }
        }
        return;
    }

    if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_REPAIR)
    {
        draw_leather();
        CharacterUI_InventoryTab_Draw(pPlayers[uActiveCharacter], true);
        GameUI_StatusBar_DrawImmediate(localization->GetString(198), 0);// "Select the Item to Repair"
        if (!HouseUI_CheckIfPlayerCanInteract())
            return;

        v117 = ((mouse.x - 14) >> 5) + 14 * ((mouse.y - 17) >> 5);
        if (mouse.x <= 13 || mouse.x >= 462)
            return;
        if (!pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v117)
            || !(pPlayers[uActiveCharacter]->pOwnItems[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v117) - 1].uAttributes & 2))
            return;
        item = &pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v117) - 1];
        v75 = pPlayers[uActiveCharacter]->SelectPhrasesTransaction(&pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v117) - 1],
            BuildingType_MagicShop, (int)window_SpeakInHouse->ptr_1C, 5);
        auto str = BuildDialogueString(pMerchantsRepairPhrases[v75], uActiveCharacter - 1, item, (char *)window_SpeakInHouse->ptr_1C, 5);
        dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(255, 255, 255), str, 3);
        return;
    }

    if (dialog_menu_id == HOUSE_DIALOGUE_LEARN_SKILLS)
    {
        if (HouseUI_CheckIfPlayerCanInteract())
        {
            all_text_height = 0;
            v33 = (signed __int64)(p2DEvents[(signed int)window_SpeakInHouse->ptr_1C - 1].flt_24 * 500.0);
            pPrice = v33 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
            if (pPrice < v33 / 3)
                pPrice = v33 / 3;
            item_num = 0;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
            {
                if (byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][pDialogueWindow->GetControl(i)->msg_param - 36]
                    && !pPlayers[uActiveCharacter]->pActiveSkills[pDialogueWindow->GetControl(i)->msg_param - 36])
                {
                    all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(pDialogueWindow->GetControl(i)->msg_param - 36), &dialog_window, 0);
                    ++item_num;
                }
            }
            if (!item_num)
            {
                auto str =
                    localization->FormatString(
                        544,
                        pPlayers[uActiveCharacter]->pName,
                        localization->GetClassName(pPlayers[uActiveCharacter]->classType) // Seek knowledge elsewhere %s the %s
                        )
                    + "\n \n"
                    + localization->GetString(528); // I can offer you nothing further.
                dialog_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(str, &dialog_window, 0)) / 2 + 138, Color16(225, 205, 35), str, 3);
                return;
            }

            auto str = localization->FormatString(401, pPrice);// "Skill Cost: %lu"
            dialog_window.DrawTitleText(pFontArrus, 0, 0x92u, 0, str, 3);

            one_string = (149 - all_text_height) / item_num;
            if (one_string > 32)
                one_string = 32;
            v23 = (149 - item_num * one_string - all_text_height) / 2 - one_string / 2 + 162;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton; ++i)
            {
                control_button = pDialogueWindow->GetControl(i);
                if (!byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][control_button->msg_param - 36]
                    || pPlayers[uActiveCharacter]->pActiveSkills[control_button->msg_param - 36])
                {
                    control_button->uW = 0;
                    control_button->uHeight = 0;
                    control_button->uY = 0;
                }
                else
                {
                    control_button->uY = one_string + v23;
                    control_button->uHeight = pFontArrus->CalcTextHeight(localization->GetSkillName(control_button->msg_param - 36), &dialog_window, 0);
                    v23 = control_button->uY + control_button->uHeight - 1;
                    control_button->uW = v23;
                    text_color = Color16(225, 205, 35);
                    if (pDialogueWindow->pCurrentPosActiveItem != i)
                        text_color = Color16(255, 255, 255);
                    dialog_window.DrawTitleText(pFontArrus, 0, control_button->uY, text_color, localization->GetSkillName(control_button->msg_param - 36), 3);
                }
            }
        }
        return;
    }
    return;
}

//----- (004BDB56) --------------------------------------------------------
void  UIShop_Buy_Identify_Repair()
{
    //int v8; // eax@15
    unsigned int pItemID; // esi@20
    ItemGen *item; // esi@21
    unsigned int v15; // eax@33
    int v18; // ecx@37
    float pPriceMultiplier; // ST1C_4@38
    int taken_item; // eax@40
    ItemGen *bought_item; // esi@51
    int party_reputation; // eax@55
    int v39; // eax@63
    int v42; // esi@74
    signed int v43; // ebx@74
    unsigned __int16 *pSkill; // esi@77
    int v55; // [sp+0h] [bp-B4h]@26
    int a6; // [sp+98h] [bp-1Ch]@57
    int a3; // [sp+9Ch] [bp-18h]@53
    unsigned int uNumSeconds; // [sp+A4h] [bp-10h]@53
    unsigned int v79; // [sp+A8h] [bp-Ch]@9
    int uPriceItemService; // [sp+ACh] [bp-8h]@12

    if (current_screen_type == SCREEN_E)
    {
        pPlayers[uActiveCharacter]->OnInventoryLeftClick();
        return;
    }
    if (!HouseUI_CheckIfPlayerCanInteract())
    {
        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
        return;
    }

    Point cursor = pMouse->GetCursorPos();
    switch (dialog_menu_id)
    {
        case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
        {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            pPlayers[uActiveCharacter]->OnInventoryLeftClick();
            break;
        }

        case HOUSE_DIALOGUE_GUILD_BUY_BOOKS:
        {
            v18 = render->pActiveZBuffer[cursor.x + pSRZBufferLineOffsets[cursor.y]] & 0xFFFF;
            if (!v18)
                return;
            bought_item = (ItemGen *)(&pParty->pPlayers[1].uExpressionTimeLength + 18 * (v18 + 12 * (int)window_SpeakInHouse->ptr_1C));
            pPriceMultiplier = p2DEvents[(signed int)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier;
            uPriceItemService = pPlayers[uActiveCharacter]->GetBuyingPrice(bought_item->GetValue(), pPriceMultiplier);

            if (pParty->uNumGold < uPriceItemService)
            {
                PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)2);
                GameUI_StatusBar_OnEvent(localization->GetString(155));
                return;
            }
            taken_item = pPlayers[uActiveCharacter]->AddItem(-1, bought_item->uItemID);
            if (taken_item)
            {
                bought_item->SetIdentified();
                memcpy(&pPlayers[uActiveCharacter]->pInventoryItemList[taken_item - 1], bought_item, 0x24u);
                dword_F8B1E4 = 1;
                Party::TakeGold(uPriceItemService);
                viewparams->bRedrawGameUI = 1;
                bought_item->Reset();
                render->ClearZBuffer(0, 479);
                pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_75, 0);
                return;
            }
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_NoRoom, 0);
            GameUI_StatusBar_OnEvent(localization->GetString(563));  // "Pack is Full!"
            break;
        }

    case HOUSE_DIALOGUE_SHOP_SELL:
    {
        v79 = ((cursor.x - 14) >> 5) + 14 * ((cursor.y - 17) >> 5);
        if (cursor.x <= 13
            || cursor.x >= 462
            || (v15 = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v79), !v15))
            return;
        if (pPlayers[uActiveCharacter]->pInventoryItemList[v15 - 1].MerchandiseTest((int)window_SpeakInHouse->ptr_1C))
        {
            dword_F8B1E4 = 1;
            pPlayers[uActiveCharacter]->SalesProcess(v79, v15 - 1, (int)window_SpeakInHouse->ptr_1C);
            viewparams->bRedrawGameUI = 1;
            render->ClearZBuffer(0, 479);
            pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)77, 0);
            return;
        }
        pPlayers[uActiveCharacter]->PlaySound(SPEECH_79, 0);
        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
        break;
    }
    case HOUSE_DIALOGUE_SHOP_IDENTIFY:
    {
        v79 = ((cursor.x - 14) >> 5) + 14 * ((cursor.y - 17) >> 5);
        if (cursor.x > 13 && cursor.x < 462)
        {
            pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v79);
            if (pItemID)
            {
                uPriceItemService = pPlayers[uActiveCharacter]->GetPriceIdentification(p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier);
                item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
                if (!(item->uAttributes & ITEM_IDENTIFIED))
                {
                    if (item->MerchandiseTest((int)window_SpeakInHouse->ptr_1C))
                    {
                        if (pParty->uNumGold >= uPriceItemService)
                        {
                            dword_F8B1E4 = 1;
                            Party::TakeGold(uPriceItemService);
                            item->uAttributes |= ITEM_IDENTIFIED;
                            pPlayers[uActiveCharacter]->PlaySound(SPEECH_73, 0);
                            GameUI_StatusBar_OnEvent(localization->GetString(569));
                            return;
                        }
                        PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)2);
                        return;
                    }
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
                    pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)79, 0);
                    return;
                }
                pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)76, 0);
                return;
            }
        }
        break;
    }

    case HOUSE_DIALOGUE_SHOP_REPAIR:
    {
        v79 = ((cursor.x - 14) >> 5) + 14 * ((cursor.y - 17) >> 5);
        if (cursor.x > 13)
        {
            if (cursor.x < 462)
            {
                pItemID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex((int *)&v79);
                if (pItemID)
                {
                    item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
                    pPriceMultiplier = p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier;
                    ItemGen* _v = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];
                    uPriceItemService = pPlayers[uActiveCharacter]->GetPriceRepair(_v->GetValue(), pPriceMultiplier);
                    if (item->uAttributes & ITEM_BROKEN)
                    {
                        if (item->MerchandiseTest((int)window_SpeakInHouse->ptr_1C))
                        {
                            if (pParty->uNumGold >= uPriceItemService)
                            {
                                dword_F8B1E4 = 1;
                                Party::TakeGold(uPriceItemService);
                                item->uAttributes = (item->uAttributes & 0xFFFFFFFD) | 1;
                                pPlayers[uActiveCharacter]->PlaySound(SPEECH_74, 0);
                                GameUI_StatusBar_OnEvent(localization->GetString(570));
                                return;
                            }
                            PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)2);
                            return;
                        }
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
                        pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)79, 0);
                        return;
                    }
                    pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)76, 0);
                    return;
                }
            }
        }
        break;
    }
    case HOUSE_DIALOGUE_SHOP_BUY_STANDARD:
    case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL:
    {
		// item picking
		Point mouse = pMouse->GetCursorPos();
		int testx = (mouse.x - 30) / 70;
		

		switch (in_current_building_type){
			case BuildingType_WeaponShop:
				
				
				// testx limits check
				if (testx >= 0 && testx < 6) {

					if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
						bought_item = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
					else
						bought_item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];


					if (bought_item->uItemID) {
						//could check x coords imits here?
						if (mouse.y >= weapons_Ypos[testx] + 30 && mouse.y < (weapons_Ypos[testx] + 30 + shop_ui_items_in_store[testx]->GetHeight())) {
							//good
						}
						else {
							bought_item = nullptr;
							return;
						}
					}
					else
						return;

				}
				break;

			default:
				v18 = render->pActiveZBuffer[cursor.x + pSRZBufferLineOffsets[cursor.y]] & 0xFFFF;
				if (!v18)
					return;
				if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
					bought_item = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][v18 - 1];
				else
					bought_item = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][v18 - 1];
				break;
		}
		
        
        

        uPriceItemService = pPlayers[uActiveCharacter]->GetBuyingPrice(bought_item->GetValue(), p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier);
        uNumSeconds = 0;
        a3 = 0;
        if (pMapStats->GetMapInfo(pCurrentMapName))
            a3 = pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)]._steal_perm;
        party_reputation = pParty->GetPartyReputation();
        if (pPlayers[uActiveCharacter]->CanSteal() && OS_IfCtrlPressed())
        {
            uNumSeconds = pPlayers[uActiveCharacter]->StealFromShop(bought_item, a3, party_reputation, 0, &a6);
            if (!uNumSeconds)
            {
                sub_4B1447_party_fine((int)window_SpeakInHouse->ptr_1C, 0, a6);
                return;
            }
        }
        else if (pParty->uNumGold < uPriceItemService)
        {
            PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)2);
            GameUI_StatusBar_OnEvent(localization->GetString(155));// "You don't have enough gold"
            return;
        }
        v39 = pPlayers[uActiveCharacter]->AddItem(-1, bought_item->uItemID);
        if (v39)
        {
            bought_item->SetIdentified();
            memcpy(&pPlayers[uActiveCharacter]->pInventoryItemList[v39 - 1], bought_item, sizeof(ItemGen));
            if (uNumSeconds != 0)
            {
                pPlayers[uActiveCharacter]->pInventoryItemList[v39 - 1].SetStolen();
                sub_4B1447_party_fine((int)window_SpeakInHouse->ptr_1C, uNumSeconds, a6);
            }
            else
            {
                dword_F8B1E4 = 1;
                Party::TakeGold(uPriceItemService);
            }
            viewparams->bRedrawGameUI = 1;
            bought_item->Reset();
            render->ClearZBuffer(0, 479);
            pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_75, 0);
            return;
        }
        else
        {
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_NoRoom, 0);
            GameUI_StatusBar_OnEvent(localization->GetString(563)); // "Pack is Full!"
            return;
        }
        break;
    }
    default:// if click video screen in shop
    {
        __debugbreak(); // please do record these dialogue ids to the HOUSE_DIALOGUE_MENU  enum
        if (dialog_menu_id >= 36 && dialog_menu_id <= 72)
        {
            v42 = dialog_menu_id - 36;
            //v43 = (signed __int64)(*(float *)&p2DEvents_minus1__24[13 * (unsigned int)ptr_507BC0->ptr_1C] * 500.0);
            v43 = (signed __int64)(p2DEvents[(unsigned int)window_SpeakInHouse->ptr_1C - 1].flt_24 * 500.0);
            uPriceItemService = v43 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
            if (uPriceItemService < v43 / 3)
                uPriceItemService = v43 / 3;
            if (byte_4ED970_skill_learn_ability_by_class_table[pPlayers[uActiveCharacter]->classType][v42])
            {
                pSkill = &pPlayers[uActiveCharacter]->pActiveSkills[v42];
                if (!*pSkill)
                {
                    if (pParty->uNumGold < uPriceItemService)
                    {
                        GameUI_StatusBar_OnEvent(localization->GetString(155));// "You don't have enough gold"
                        if (in_current_building_type == BuildingType_Training)
                            v55 = 4;
                        else
                            v55 = 2;
                        PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)v55);
                        return;
                    }
                    Party::TakeGold(uPriceItemService);
                    dword_F8B1E4 = 1;
                    *pSkill = 1;
                    pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)78, 0);
                    return;
                }
            }
        }
        break;
    }
    }
}


//----- (004B1A2D) --------------------------------------------------------
void  ShowPopupShopItem()
{
    int v3; // ecx@5
  //  unsigned int v6; // eax@13
    ItemGen *v7; // ecx@13


    if (in_current_building_type <= 0)
        return;
    if (dialog_menu_id < HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
        return;

    Point cursor = pMouse->GetCursorPos();
    if (in_current_building_type <= BuildingType_AlchemistShop)
    {
        if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD || dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_SPECIAL)
        {
            //select case weapons armour...

			// item picking
			Point mouse = pMouse->GetCursorPos();
			int testx = (mouse.x - 30) / 70;


			switch (in_current_building_type) {
				case BuildingType_WeaponShop:
					if (testx >= 0 && testx < 6) {
						if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD)
							v7 = (ItemGen *)&pParty->StandartItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];
						else
							v7 = &pParty->SpecialItemsInShops[(int)window_SpeakInHouse->ptr_1C][testx];

						if (v7->uItemID) {
							//could check x coords imits here?
							if (mouse.y >= weapons_Ypos[testx] + 30 && mouse.y < (weapons_Ypos[testx] + 30 + shop_ui_items_in_store[testx]->GetHeight())) {
								GameUI_DrawItemInfo(v7);
							}
						}
						else {
							return;
						}
					}
					else
						return;
					
					break;

			default:
				v3 = render->pActiveZBuffer[cursor.x + pSRZBufferLineOffsets[cursor.y]] & 0xFFFF;
				if (!v3)
					return;
				v7 = &pParty->StandartItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][v3 - 1];
				if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_SPECIAL)
					v7 = &pParty->SpecialItemsInShops[(unsigned int)window_SpeakInHouse->ptr_1C][v3 - 1];
				GameUI_DrawItemInfo(v7);
				return;
				break;
			}


			
        }

        if (dialog_menu_id >= HOUSE_DIALOGUE_SHOP_SELL && dialog_menu_id <= HOUSE_DIALOGUE_SHOP_REPAIR
            || dialog_menu_id == HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT)
        {
            v3 = ((cursor.x - 14) >> 5) + 14 * ((cursor.y - 17) >> 5);
            if (cursor.x <= 13 || cursor.x >= 462
                || !pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&v3))
                return;
            GameUI_DrawItemInfo(&pPlayers[uActiveCharacter]->pInventoryItemList[pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&v3) - 1]);
            return;
        }
    }
    if (in_current_building_type <= BuildingType_16 && dialog_menu_id == HOUSE_DIALOGUE_GUILD_BUY_BOOKS)
    {

        v3 = render->pActiveZBuffer[cursor.x + pSRZBufferLineOffsets[cursor.y]] & 0xFFFF;
        if (v3)
        {
            unsigned int guildId = (unsigned int)window_SpeakInHouse->ptr_1C - 139;
            sub_4B1523_showSpellbookInfo(pParty->SpellBooksInGuilds[guildId][v3].uItemID);
        }
    }
}

//----- (004B1523) --------------------------------------------------------
void sub_4B1523_showSpellbookInfo(int spellItemId)
{
    int spellId; // esi@1
    int spellLevel; // edx@1
    unsigned int v3; // eax@2
    int v4; // eax@4
    long v5; // ecx@4
    int v6; // eax@10
    char *v7; // ST44_4@12
    unsigned __int16 v8; // ax@12
    GUIWindow a1; // [sp+Ch] [bp-68h]@4
    unsigned int spellSchool; // [sp+60h] [bp-14h]@1
    int v13; // [sp+6Ch] [bp-8h]@4
    int v14; // [sp+70h] [bp-4h]@4

    spellId = spellItemId - 399;
    spellLevel = (spellItemId - 400) % 11 + 1;
    spellSchool = 4 * (spellItemId - 400) / 11;

    // sprintf(tmp_str.data(), "%s%03d", spellbook_texture_filename_suffices[v11 / 4], v2); not used

    Point a2 = pMouse->GetCursorPos();
    if (a2.y <= 320)
        v3 = a2.y + 30;
    else
        v3 = 30;
    a1.Hint = nullptr;
    a1.uFrameY = v3;
    a1.uFrameWidth = 328;
    a1.uFrameHeight = 68;
    a1.uFrameX = 90;
    a1.uFrameZ = 417;
    a1.uFrameW = v3 + 67;
    a2.y = pFontSmallnum->GetLineWidth(localization->GetString(431));
    v14 = pFontSmallnum->GetLineWidth(localization->GetString(433));
    v13 = pFontSmallnum->GetLineWidth(localization->GetString(432));
    v4 = pFontSmallnum->GetLineWidth(localization->GetString(96));
    v5 = a2.y;
    if (v14 > a2.y)
        v5 = v14;
    if (v13 > v5)
        v5 = v13;
    if (v4 > v5)
        v5 = v4;

    auto str = StringPrintf(
        "%s\n\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s",
        pSpellStats->pInfos[spellId].pDescription, localization->GetString(431),        // "Normal"
        v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pBasicSkillDesc, localization->GetString(433),        // "Expert"
        v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pExpertSkillDesc, localization->GetString(432),        // "Master"
        v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pMasterSkillDesc, localization->GetString(96),         // "Grand"
        v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pGrandmasterSkillDesc);
    v6 = pFontSmallnum->CalcTextHeight(str, &a1, 0);
    a1.uFrameHeight += v6;
    if ((signed int)a1.uFrameHeight < 150)
        a1.uFrameHeight = 150;
    a1.uFrameWidth = game_viewport_width;
    a1.DrawMessageBox(0);
    a1.uFrameWidth -= 12;
    a1.uFrameHeight -= 12;
    v7 = pSpellStats->pInfos[spellId].pName;
    a1.uFrameZ = a1.uFrameX + a1.uFrameWidth - 1;
    a1.uFrameW = a1.uFrameHeight + a1.uFrameY - 1;
    v8 = Color16(0xFFu, 0xFFu, 0x9Bu);
    a1.DrawTitleText(pFontArrus, 0x78u, 0xCu, v8, v7, 3u);
    a1.DrawText(pFontSmallnum, 120, 44, 0, str, 0, 0, 0);
    a1.uFrameZ = a1.uFrameX + 107;
    a1.uFrameWidth = 108;
    a1.DrawTitleText(pFontComic, 0xCu, 0x4Bu, 0, localization->GetSkillName(spellSchool / 4 + 12), 3u);

    str = StringPrintf("%s\n%d", localization->GetString(522), *(&pSpellDatas[0].uNormalLevelMana + 10 * spellId));
    a1.DrawTitleText(pFontComic, 0xCu, a1.uFrameHeight - pFontComic->GetFontHeight() - 16, 0, str, 3);
}

//----- (004B1D27) --------------------------------------------------------
void  GetHouseGoodbyeSpeech()
{
    signed int v2; // edi@10
    signed int v5; // edi@20
    int v7[4]; // [sp+Ch] [bp-10h]@12

    if (in_current_building_type > 0)
    {
        if (in_current_building_type > BuildingType_MagicShop)
        {
            if (in_current_building_type == BuildingType_Bank)
            {
                if (!dword_F8B1E4)
                    return;
            }
            else
            {
                if (in_current_building_type != BuildingType_Temple)
                    return;
            }
            PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, HouseSound_Greeting_2);
            return;
        }
        if ((signed __int64)pParty->PartyTimes._shop_ban_times[(unsigned int)window_SpeakInHouse->ptr_1C] <= pParty->GetPlayingTime())
        {
            if (pParty->uNumGold <= 10000)
            {
                if (!dword_F8B1E4)
                    return;
                PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, HouseSound_Goodbye);
                return;
            }
            PlayHouseSound((unsigned int)window_SpeakInHouse->ptr_1C, (HouseSoundID)(dword_F8B1E4 + 3));
            if (!dword_F8B1E4 && !_A750D8_player_speech_timer)
            {
                v5 = 0;
                for (uint i = 1; i <= 4; ++i)
                {
                    if (pPlayers[i]->CanAct())
                        v7[v5++] = i;
                }
                if (v5)
                {
                    _A750D8_player_speech_timer = 256i64;
                    PlayerSpeechID = SPEECH_80;
                    uSpeakingCharacter = v7[rand() % v5];
                    return;
                }
            }
        }
        else
        {
            if (!_A750D8_player_speech_timer)
            {
                v2 = 0;
                for (uint i = 1; i <= 4; ++i)
                {
                    if (pPlayers[i]->CanAct())
                        v7[v2++] = i;
                }
                if (v2)
                {
                    _A750D8_player_speech_timer = 256i64;
                    PlayerSpeechID = SPEECH_80;
                    uSpeakingCharacter = v7[rand() % v2];
                    return;
                }
            }
        }
    }
}

//----- (004B1447) --------------------------------------------------------
void sub_4B1447_party_fine(int shopId, int stealingResult, int fineToAdd)
{
	signed int v3; // esi@1
	DDM_DLV_Header *v7; // eax@14

	if (stealingResult == 0 || stealingResult == 1)
	{
		if (pParty->uFine < 4000000)
		{
			if (fineToAdd + pParty->uFine < 0)
				pParty->uFine = 0;
			else if (fineToAdd + pParty->uFine > 4000000)
				pParty->uFine = 4000000;
			else
				pParty->uFine += fineToAdd;
		}
		if (pParty->uFine)
		{
			for (uint i = 1; i <= 4; ++i)
			{
				if (!_449B57_test_bit(pPlayers[i]->_achieved_awards_bits, 1))
					_449B7E_toggle_bit(pPlayers[i]->_achieved_awards_bits, 1, 1);
			}
		}
		if (stealingResult == 1)
			v3 = 2;
		else
			v3 = 1;
	}
	else
		v3 = 2;
	pParty->PartyTimes._shop_ban_times[shopId] = pParty->GetPlayingTime() + GameTime::FromDays(1);
	pParty->InTheShopFlags[shopId] = 1;
	v7 = &pOutdoor->ddm;
	if (uCurrentlyLoadedLevelType != LEVEL_Outdoor)
		v7 = &pIndoor->dlv;
	v7->uReputation += v3;
	if (v7->uReputation > 10000)
		v7->uReputation = 10000;
}