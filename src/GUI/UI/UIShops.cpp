#include <cstdlib>
#include <algorithm>
#include <string>

#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/Events2D.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/Items.h"
#include "Engine/Party.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIPopup.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

Image *shop_ui_background = nullptr;

std::array<Image *, 12> shop_ui_items_in_store;

bool StealingMode(int adventurerId) {
    return keyboardInputHandler->IsStealingToggled() && pPlayers[adventurerId]->CanSteal();
}

void ShopDialogMain(GUIWindow dialogwin) {
    if (HouseUI_CheckIfPlayerCanInteract()) {
        pShopOptions[0] = localization->GetString(LSTR_STANDARD);
        pShopOptions[1] = localization->GetString(LSTR_SPECIAL);
        pShopOptions[2] = localization->GetString(LSTR_DISPLAY);
        pShopOptions[3] = localization->GetString(LSTR_LEARN_SKILLS);

        int all_text_height = 0;
        for (int i = 0; i < 4; ++i)
            all_text_height += pFontArrus->CalcTextHeight(
                pShopOptions[i], dialogwin.uFrameWidth, 0);

        int textspacings = (174 - all_text_height) / 4;
        int textoffset = 138 - (textspacings / 2);

        int pNumString = 0;
        GUIButton *pButton;
        int pColorText;

        for (int i = pDialogueWindow->pStartingPosActiveItem;
             i < pDialogueWindow->pNumPresenceButton +
                     pDialogueWindow->pStartingPosActiveItem;
             ++i) {
            pButton = pDialogueWindow->GetControl(i);
            pButton->uY = textspacings + textoffset;
            pButton->uHeight = pFontArrus->CalcTextHeight(
                pShopOptions[pNumString], dialogwin.uFrameWidth, 0);
            textoffset = pButton->uY +
                         pFontArrus->CalcTextHeight(pShopOptions[pNumString],
                                                    dialogwin.uFrameWidth, 0) -
                         1;
            pButton->uW = textoffset + 6;

            pColorText = colorTable.Jonquil.c16();
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                pColorText = colorTable.White.c16();

            dialogwin.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText,
                                    pShopOptions[pNumString], 3);
            ++pNumString;
        }
    }
}

void ShopDialogDisplayEquip(GUIWindow dialogwin,
                            BuildingType building = BuildingType_WeaponShop) {
    draw_leather();
    CharacterUI_InventoryTab_Draw(pPlayers[pParty->getActiveCharacter()], true);

    pShopOptions[0] = localization->GetString(LSTR_SELL);
    pShopOptions[1] = localization->GetString(LSTR_IDENTIFY);
    pShopOptions[2] = localization->GetString(LSTR_REPAIR);

    int options;
    if (building == BuildingType_AlchemistShop) {
        options = 2;
    } else {
        options = 3;
    }

    int all_text_height = 0;
    for (int i = 0; i < options; ++i)
        all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i],
                                                      dialogwin.uFrameWidth, 0);

    int textspacings = (174 - all_text_height) / options;
    int textoffset = 138 - (textspacings / 2);

    int pNumString = 0;
    GUIButton *pButton;
    int pColorText;

    for (int i = pDialogueWindow->pStartingPosActiveItem;
         i < pDialogueWindow->pNumPresenceButton +
                 pDialogueWindow->pStartingPosActiveItem;
         ++i) {
        pButton = pDialogueWindow->GetControl(i);
        pButton->uY = textspacings + textoffset;
        pButton->uHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString],
                                                      dialogwin.uFrameWidth, 0);
        textoffset = pButton->uY +
                     pFontArrus->CalcTextHeight(pShopOptions[pNumString],
                                                dialogwin.uFrameWidth, 0) -
                     1;
        pButton->uW = textoffset + 6;

        pColorText = colorTable.Jonquil.c16();
        if (pDialogueWindow->pCurrentPosActiveItem != i)
            pColorText = colorTable.White.c16();
        dialogwin.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText,
                                pShopOptions[pNumString], 3);
        ++pNumString;
    }
}

void ShopDialogSellEquip(GUIWindow dialogwin, BuildingType building) {
    draw_leather();
    CharacterUI_InventoryTab_Draw(pPlayers[pParty->getActiveCharacter()], true);

    if (HouseUI_CheckIfPlayerCanInteract()) {
        GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_SELL), 0);

        Pointi pt = dialogwin.mouse->GetCursorPos();

        int invindex = ((pt.x - 14) / 32) + 14 * ((pt.y - 17) / 32);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pPlayers[pParty->getActiveCharacter()]->GetItemListAtInventoryIndex(invindex);
        if (pItemID) {
            ItemGen *item =
                &pPlayers[pParty->getActiveCharacter()]->pInventoryItemList[pItemID - 1];
            MERCHANT_PHRASE phrases_id =
                pPlayers[pParty->getActiveCharacter()]->SelectPhrasesTransaction(
                    item, building, window_SpeakInHouse->wData.val, 3);
            auto str = BuildDialogueString(
                pMerchantsSellPhrases[phrases_id], pParty->getActiveCharacter() - 1, item,
                window_SpeakInHouse->wData.val, 3);
            dialogwin.DrawTitleText(pFontArrus, 0,
                                    (174 - pFontArrus->CalcTextHeight(
                                               str, dialogwin.uFrameWidth, 0)) /
                                            2 +
                                        138, colorTable.White.c16(), str, 3);
        }
    }
}

void ShopDialogIdentify(GUIWindow dialogwin, BuildingType building) {
    draw_leather();
    CharacterUI_InventoryTab_Draw(pPlayers[pParty->getActiveCharacter()], true);

    if (HouseUI_CheckIfPlayerCanInteract()) {
        GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_IDENTIFY), 0);

        Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

        int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
        if (pt.x <= 13 || pt.x >= 462) return;

        int pItemID = pPlayers[pParty->getActiveCharacter()]->GetItemListAtInventoryIndex(invindex);

        if (pItemID) {
            ItemGen *item = &pPlayers[pParty->getActiveCharacter()]->pInventoryItemList[pItemID - 1];

            std::string str;
            if (!item->IsIdentified()) {
                MERCHANT_PHRASE phrases_id = pPlayers[pParty->getActiveCharacter()]->SelectPhrasesTransaction(
                    item, building, window_SpeakInHouse->wData.val, 4);
                str = BuildDialogueString(
                    pMerchantsIdentifyPhrases[phrases_id], pParty->getActiveCharacter() - 1,
                    item, window_SpeakInHouse->wData.val, 4);
            } else {
                str = BuildDialogueString("%24", pParty->getActiveCharacter() - 1, item,
                    window_SpeakInHouse->wData.val, 4);
            }

            dialogwin.DrawTitleText(pFontArrus, 0,
                (174 - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + 138, colorTable.White.c16(), str, 3);
        }
    }
}

void ShopDialogRepair(GUIWindow dialogwin, BuildingType building) {
    draw_leather();
    CharacterUI_InventoryTab_Draw(pPlayers[pParty->getActiveCharacter()], true);

    if (HouseUI_CheckIfPlayerCanInteract()) {
        GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_REPAIR), 0);

        Pointi pt = dialogwin.mouse->GetCursorPos();

        int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pPlayers[pParty->getActiveCharacter()]->GetItemListAtInventoryIndex(invindex);
        if (pItemID == 0)
            return;

        if ((pPlayers[pParty->getActiveCharacter()]->pOwnItems[pItemID - 1].uAttributes &
             ITEM_BROKEN)) {
            ItemGen *item = &pPlayers[pParty->getActiveCharacter()]->pInventoryItemList[pItemID - 1];
            MERCHANT_PHRASE phrases_id = pPlayers[pParty->getActiveCharacter()]->SelectPhrasesTransaction(
                item, building, window_SpeakInHouse->wData.val, 5);
            std::string str = BuildDialogueString(
                pMerchantsRepairPhrases[phrases_id], pParty->getActiveCharacter() - 1, item,
                window_SpeakInHouse->wData.val, 5);
            dialogwin.DrawTitleText(pFontArrus, 0,
                (174 - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + 138, colorTable.White.c16(), str, 3);
        }
    }
}

void ShopDialogLearn(GUIWindow dialogwin) {
    if (!HouseUI_CheckIfPlayerCanInteract()) return;

    uint item_num = 0;
    int all_text_height = 0;

    int baseprice = (int64_t)(p2DEvents[window_SpeakInHouse->wData.val - 1].flt_24 * 500.0);
    int pPrice = baseprice * (100 - pPlayers[pParty->getActiveCharacter()]->GetMerchant()) / 100;
    if (pPrice < baseprice / 3)
        pPrice = baseprice / 3;

    for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton +
        pDialogueWindow->pStartingPosActiveItem;
        ++i) {
        auto skill = GetLearningDialogueSkill(
            (DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param
        );
        if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill] != PLAYER_SKILL_MASTERY_NONE
            && !pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill]) {
            all_text_height += pFontArrus->CalcTextHeight(
                localization->GetSkillName(skill),
                dialogwin.uFrameWidth, 0);
            item_num++;
        }
    }

    SkillTrainingDialogue(&dialogwin, item_num, all_text_height, pPrice);
}

void WeaponShopWares(GUIWindow dialogwin, bool special) {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
    int item_X = 0;

    for (uint i = 0; i < 6; ++i) {
        if ((special == 0 &&
             pParty
                 ->StandartItemsInShops[window_SpeakInHouse->wData.val][i]
                 .uItemID != ITEM_NULL) ||
            (special == 1 &&
             pParty
                 ->SpecialItemsInShops[window_SpeakInHouse->wData.val][i]
                 .uItemID != ITEM_NULL)) {
            render->DrawTextureNew(
                ((60 -
                  ((signed int)shop_ui_items_in_store[i]->GetWidth() / 2)) +
                 item_X) /
                    640.0f,
                (weapons_Ypos[i] + 30) / 480.0f, shop_ui_items_in_store[i]);
        }

        item_X += 70;
    }

    if (HouseUI_CheckIfPlayerCanInteract()) {
        uint item_num = 0;
        for (uint i = 0; i < 6; ++i) {
            if ((special == 0 &&
                 pParty
                     ->StandartItemsInShops[window_SpeakInHouse->wData.val][i]
                     .uItemID != ITEM_NULL) ||
                (special == 1 &&
                 pParty
                     ->SpecialItemsInShops[window_SpeakInHouse->wData.val][i]
                     .uItemID != ITEM_NULL)) {
                ++item_num;
            }
        }

        if (StealingMode(pParty->getActiveCharacter()))
            GameUI_StatusBar_DrawImmediate(
                localization->GetString(LSTR_STEAL_ITEM), 0);
        else
            GameUI_StatusBar_DrawImmediate(
                localization->GetString(LSTR_SELECT_ITEM_TO_BUY), 0);

        if (item_num) {  // this shoudl go into func??
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();
            ItemGen *item;

            int testx = (pt.x - 30) / 70;
            if (testx >= 0 && testx < 6) {  // testx limits check
                if (special == 0) {
                    item = &pParty->StandartItemsInShops[window_SpeakInHouse->wData.val][testx];
                } else {
                    item = &pParty->SpecialItemsInShops[window_SpeakInHouse->wData.val][testx];
                }

                if (item->uItemID != ITEM_NULL) {  // item picking
                    int testpos =
                        ((60 - ((signed int)shop_ui_items_in_store[testx]
                                    ->GetWidth() /
                                2)) +
                         testx * 70);

                    if (pt.x >= testpos &&
                        pt.x < (testpos + (signed int)shop_ui_items_in_store[testx]->GetWidth())) {
                        if (pt.y >= weapons_Ypos[testx] + 30 &&
                            pt.y < (weapons_Ypos[testx] + 30 + shop_ui_items_in_store[testx]->GetHeight())) {
                            std::string str;
                            if (!StealingMode(pParty->getActiveCharacter())) {
                                str = BuildDialogueString(
                                    pMerchantsBuyPhrases[pPlayers[pParty->getActiveCharacter()]->SelectPhrasesTransaction(
                                                 item, BuildingType_WeaponShop,
                                                 window_SpeakInHouse->wData.val, 2)],
                                    pParty->getActiveCharacter() - 1, item,
                                    window_SpeakInHouse->wData.val, 2);
                            } else {
                                str = BuildDialogueString(
                                    localization->GetString(LSTR_STEAL_ITEM_FMT),
                                    pParty->getActiveCharacter() - 1, item,
                                    window_SpeakInHouse->wData.val, 2);
                            }
                            dialogwin.DrawTitleText(
                                pFontArrus, 0,
                                (174 - pFontArrus->CalcTextHeight(
                                           str, dialogwin.uFrameWidth, 0)) /
                                        2 +
                                    138, colorTable.White.c16(), str, 3);
                        }
                    }
                }
            }
        } else {  // shop empty
            dialogwin.DrawShops_next_generation_time_string(
                pParty->PartyTimes.Shops_next_generation_time
                    [window_SpeakInHouse->wData.val] -
                pParty->GetPlayingTime());  //Приходите через 7 дней
        }
    }
}

void WeaponShopDialog() {
    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 143;
    dialog_window.uFrameZ = 334;

    switch (dialog_menu_id) {
        case DIALOGUE_MAIN: {
            ShopDialogMain(dialog_window);
            break;
        }
        case DIALOGUE_SHOP_BUY_STANDARD: {
            WeaponShopWares(dialog_window);
            break;
        }
        case DIALOGUE_SHOP_BUY_SPECIAL: {
            WeaponShopWares(dialog_window, 1);
            break;
        }
        case DIALOGUE_SHOP_SELL: {
            ShopDialogSellEquip(dialog_window, BuildingType_WeaponShop);
            break;
        }
        case DIALOGUE_SHOP_IDENTIFY: {
            ShopDialogIdentify(dialog_window, BuildingType_WeaponShop);
            break;
        }
        case DIALOGUE_SHOP_REPAIR: {
            ShopDialogRepair(dialog_window, BuildingType_WeaponShop);
            break;
        }
        case DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            ShopDialogDisplayEquip(dialog_window);
            break;
        }
        case DIALOGUE_LEARN_SKILLS: {
            ShopDialogLearn(dialog_window);
            break;
        }
        default: {
            __debugbreak();
            break;
        }
    }
}

void ArmorShopWares(GUIWindow dialogwin, bool special) {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
    int item_x = 0;

    for (int i = 0; i < 8; ++i) {
        if ((special == 0 &&
             pParty->StandartItemsInShops[window_SpeakInHouse->wData.val][i]
                 .uItemID != ITEM_NULL) ||
            (special == 1 &&
             pParty->SpecialItemsInShops[window_SpeakInHouse->wData.val][i]
                 .uItemID != ITEM_NULL)) {
            if (i >= 4) {  // low row
                render->DrawTextureNew(
                    ((90 - (shop_ui_items_in_store[i]->GetWidth() / 2)) +
                     item_x - 420) /
                        640.0f,
                    126 / 480.0f, shop_ui_items_in_store[i]);
            } else {
                render->DrawTextureNew(
                    ((86 - (shop_ui_items_in_store[i]->GetWidth() / 2)) +
                     item_x) /
                        640.0f,
                    (98 - shop_ui_items_in_store[i]->GetHeight()) / 480.0f,
                    shop_ui_items_in_store[i]);
            }
        }
        item_x += 105;
    }

    if (HouseUI_CheckIfPlayerCanInteract()) {
        int pItemCount = 0;
        for (int i = 0; i < 8; ++i) {
            if ((special == 0 &&
                 pParty->StandartItemsInShops[window_SpeakInHouse->wData.val][i]
                     .uItemID != ITEM_NULL) ||
                (special == 1 &&
                 pParty->SpecialItemsInShops[window_SpeakInHouse->wData.val][i]
                     .uItemID != ITEM_NULL))
                ++pItemCount;
        }

        if (!StealingMode(pParty->getActiveCharacter())) {
            GameUI_StatusBar_DrawImmediate(
                localization->GetString(LSTR_SELECT_ITEM_TO_BUY), 0);
        } else {
            GameUI_StatusBar_DrawImmediate(
                localization->GetString(LSTR_STEAL_ITEM), 0);
        }

        if (pItemCount) {  // this should go into func??
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

            int testx = (pt.x - 40) / 105;
            // testx limits check
            if (testx >= 0 && testx < 4) {
                if (pt.y >= 126) {
                    testx += 4;
                }

                ItemGen *item;
                if (special == 0) {
                    item = &pParty->StandartItemsInShops[window_SpeakInHouse->wData.val][testx];
                } else {
                    item = &pParty->SpecialItemsInShops[window_SpeakInHouse->wData.val][testx];
                }

                if (item->uItemID != ITEM_NULL) {
                    int testpos;
                    if (testx >= 4) {
                        testpos =
                            ((90 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) +
                             (testx * 105) - 420);  // low row
                    } else {
                        testpos =
                            ((86 - (shop_ui_items_in_store[testx]->GetWidth() / 2)) +
                             testx * 105);
                    }

                    if (pt.x >= testpos &&
                        pt.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
                        if ((pt.y >= 126 &&
                            pt.y < (126 + shop_ui_items_in_store[testx]->GetHeight())) ||
                            (pt.y <= 98 &&
                                pt.y >= (98 - shop_ui_items_in_store[testx]->GetHeight()))) {
                            // y is 126 to 126 + height low or 98-height to 98

                            std::string str;
                            if (!StealingMode(pParty->getActiveCharacter())) {
                                str = BuildDialogueString(
                                    pMerchantsBuyPhrases
                                        [pPlayers[pParty->getActiveCharacter()]->SelectPhrasesTransaction(
                                                 item, BuildingType_ArmorShop, window_SpeakInHouse->wData.val, 2)],
                                    pParty->getActiveCharacter() - 1, item,
                                    window_SpeakInHouse->wData.val, 2);
                            } else {
                                str = BuildDialogueString(
                                    localization->GetString(LSTR_STEAL_ITEM_FMT),
                                    pParty->getActiveCharacter() - 1, item,
                                    window_SpeakInHouse->wData.val, 2);
                            }
                            dialogwin.DrawTitleText(
                                pFontArrus, 0,
                                (174 - pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + 138, colorTable.White.c16(), str, 3);
                        }
                    }
                }
            }
        } else {
            // empty shop
            dialogwin.DrawShops_next_generation_time_string(
                pParty->PartyTimes.Shops_next_generation_time[window_SpeakInHouse->wData.val] -
                pParty->GetPlayingTime());
        }
    }
}

void ArmorShopDialog() {
    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 143;
    dialog_window.uFrameZ = 334;

    switch (dialog_menu_id) {
        case DIALOGUE_MAIN: {
            ShopDialogMain(dialog_window);
            break;
        }
        case DIALOGUE_SHOP_BUY_STANDARD: {
            ArmorShopWares(dialog_window);
            break;
        }
        case DIALOGUE_SHOP_BUY_SPECIAL: {
            ArmorShopWares(dialog_window, 1);
            break;
        }
        case DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            ShopDialogDisplayEquip(dialog_window);
            break;
        }
        case DIALOGUE_SHOP_SELL: {
            ShopDialogSellEquip(dialog_window, BuildingType_ArmorShop);
            break;
        }
        case DIALOGUE_SHOP_IDENTIFY: {
            ShopDialogIdentify(dialog_window, BuildingType_ArmorShop);
            break;
        }
        case DIALOGUE_SHOP_REPAIR: {
            ShopDialogRepair(dialog_window, BuildingType_ArmorShop);
            break;
        }
        case DIALOGUE_LEARN_SKILLS: {
            ShopDialogLearn(dialog_window);
            break;
        }
        default: {
            __debugbreak();
            break;
        }
    }
}

void AlchemyMagicShopWares(GUIWindow dialogwin, BuildingType building,
                           bool special) {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);

    int itemx;
    int itemy;

    for (uint i = 0; i < 12; ++i) {
        if ((special == 0 &&
             pParty->StandartItemsInShops[window_SpeakInHouse->wData.val][i].uItemID != ITEM_NULL) ||
            (special == 1 &&
             pParty->SpecialItemsInShops[window_SpeakInHouse->wData.val][i].uItemID != ITEM_NULL)) {
            if (i >= 6) {  // low row
                itemy = 308 - shop_ui_items_in_store[i]->GetHeight();
                if (itemy < 0) itemy = 0;

                itemx = 75 * i - shop_ui_items_in_store[i]->GetWidth() / 2 +
                        40 - 450;

            } else {
                itemy = 152 - shop_ui_items_in_store[i]->GetHeight();
                if (itemy < 0) itemy = 0;

                itemx = 75 * i - shop_ui_items_in_store[i]->GetWidth() / 2 + 40;
            }

            if (i == 0 || i == 6) {
                if (itemx < 18) itemx = 18;
            }

            if (i == 5 || i == 11) {
                if (itemx > 457 - shop_ui_items_in_store[i]->GetWidth())
                    itemx = 457 - shop_ui_items_in_store[i]->GetWidth();
            }

            render->DrawTextureNew(itemx / 640.0f, itemy / 480.0f,
                                        shop_ui_items_in_store[i]);
        }
    }

    if (HouseUI_CheckIfPlayerCanInteract()) {
        int item_num = 0;

        for (uint i = 0; i < 12; ++i) {
            if (special == 0 &&
                    pParty
                        ->StandartItemsInShops[window_SpeakInHouse->wData.val][i]
                        .uItemID != ITEM_NULL ||
                special == 1 &&
                    pParty
                        ->SpecialItemsInShops[window_SpeakInHouse->wData.val][i]
                        .uItemID != ITEM_NULL)
                ++item_num;
        }

        if (StealingMode(pParty->getActiveCharacter())) {
            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_STEAL_ITEM), 0);
        } else {
            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), 0);
        }

        if (item_num) {
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

            int testx = (pt.x) / 75;
            // testx limits check
            if (testx >= 0 && testx < 6) {
                if (pt.y > 152) {
                    testx += 6;
                }

                ItemGen *item;
                if (special == 0) {
                    item = &pParty->StandartItemsInShops[window_SpeakInHouse->wData.val][testx];
                } else {
                    item = &pParty->SpecialItemsInShops[window_SpeakInHouse->wData.val][testx];
                }

                if (item->uItemID != ITEM_NULL) {  // item picking
                    int testpos;
                    if (pt.y > 152) {
                        testpos = 75 * testx - shop_ui_items_in_store[testx]->GetWidth() / 2 + 40 - 450;
                    } else {
                        testpos = 75 * testx - shop_ui_items_in_store[testx]->GetWidth() / 2 + 40;
                    }

                    if (pt.x >= testpos &&
                        pt.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
                        if ((pt.y <= 308 &&
                            pt.y >= (308 - shop_ui_items_in_store[testx]->GetHeight())) ||
                            (pt.y <= 152 &&
                            pt.y >= (152 - shop_ui_items_in_store[testx]->GetHeight()))) {
                            // y is 152-h to 152 or 308-height to 308

                            std::string str;
                            if (!StealingMode(pParty->getActiveCharacter())) {
                                str = BuildDialogueString(
                                    pMerchantsBuyPhrases
                                        [pPlayers[pParty->getActiveCharacter()]
                                             ->SelectPhrasesTransaction(item, building, window_SpeakInHouse->wData.val, 2)],
                                    pParty->getActiveCharacter() - 1, item,
                                    window_SpeakInHouse->wData.val, 2);
                            } else {
                                str = BuildDialogueString(
                                    localization->GetString(LSTR_STEAL_ITEM_FMT),
                                    pParty->getActiveCharacter() - 1, item,
                                    window_SpeakInHouse->wData.val, 2);
                            }
                            dialogwin.DrawTitleText(
                                pFontArrus, 0,
                                (174 - pFontArrus->CalcTextHeight(
                                           str, dialogwin.uFrameWidth, 0)) /
                                        2 +
                                    138, colorTable.White.c16(), str, 3);
                        }
                    }
                }
            }
        } else {
            // shop empty
            dialogwin.DrawShops_next_generation_time_string(
                pParty->PartyTimes.Shops_next_generation_time
                    [window_SpeakInHouse->wData.val] -
                pParty->GetPlayingTime());
        }
    }
}

void AlchemistDialog() {
    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 143;
    dialog_window.uFrameZ = 334;

    switch (dialog_menu_id) {
        case DIALOGUE_MAIN: {
            ShopDialogMain(dialog_window);
            break;
        }
        case DIALOGUE_SHOP_BUY_STANDARD: {
            AlchemyMagicShopWares(dialog_window, BuildingType_AlchemistShop);
            break;
        }
        case DIALOGUE_SHOP_BUY_SPECIAL: {
            AlchemyMagicShopWares(dialog_window, BuildingType_AlchemistShop, 1);
            break;
        }
        case DIALOGUE_SHOP_SELL: {
            ShopDialogSellEquip(dialog_window, BuildingType_AlchemistShop);
            break;
        }
        case DIALOGUE_SHOP_IDENTIFY: {
            ShopDialogIdentify(dialog_window, BuildingType_AlchemistShop);
            break;
        }
        case DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            ShopDialogDisplayEquip(dialog_window, BuildingType_AlchemistShop);
            break;
        }
        case DIALOGUE_LEARN_SKILLS: {
            ShopDialogLearn(dialog_window);
            break;
        }
        default: {
            __debugbreak();
            break;
        }
    }
}

void MagicShopDialog() {
    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 143;
    dialog_window.uFrameZ = 334;

    switch (dialog_menu_id) {
        case DIALOGUE_MAIN: {
            ShopDialogMain(dialog_window);
            break;
        }
        case DIALOGUE_SHOP_BUY_STANDARD: {
            AlchemyMagicShopWares(dialog_window, BuildingType_MagicShop);
            break;
        }
        case DIALOGUE_SHOP_BUY_SPECIAL: {
            AlchemyMagicShopWares(dialog_window, BuildingType_MagicShop, 1);
            break;
        }
        case DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            ShopDialogDisplayEquip(dialog_window);
            break;
        }
        case DIALOGUE_SHOP_SELL: {
            ShopDialogSellEquip(dialog_window, BuildingType_MagicShop);
            break;
        }
        case DIALOGUE_SHOP_IDENTIFY: {
            ShopDialogIdentify(dialog_window, BuildingType_MagicShop);
            break;
        }
        case DIALOGUE_SHOP_REPAIR: {
            ShopDialogRepair(dialog_window, BuildingType_MagicShop);
            break;
        }
        case DIALOGUE_LEARN_SKILLS: {
            ShopDialogLearn(dialog_window);
            break;
        }
        default: {
            __debugbreak();
            break;
        }
    }
}

//----- (004BDB56) --------------------------------------------------------
void UIShop_Buy_Identify_Repair() {
    unsigned int pItemID;  // esi@20
    ItemGen *item;         // esi@21

    // int v18;                   // ecx@37
    float fPriceMultiplier;    // ST1C_4@38
    int taken_item;            // eax@40
    ItemGen *bought_item = nullptr;      // esi@51
    int party_reputation;      // eax@55
    int v39;                   // eax@63
    signed int v43;            // ebx@74
    uint16_t *pSkill;  // esi@77
    int v55;                   // [sp+0h] [bp-B4h]@26
    int a6;                    // [sp+98h] [bp-1Ch]@57
    int a3;                    // [sp+9Ch] [bp-18h]@53
    unsigned int uNumSeconds;  // [sp+A4h] [bp-10h]@53
    unsigned int invindex;     // [sp+A8h] [bp-Ch]@9
    int uPriceItemService;     // [sp+ACh] [bp-8h]@12

    if (current_screen_type == CURRENT_SCREEN::SCREEN_SHOP_INVENTORY) {
        pPlayers[pParty->getActiveCharacter()]->OnInventoryLeftClick();
        return;
    }

    if (!HouseUI_CheckIfPlayerCanInteract()) {
        pAudioPlayer->playUISound(SOUND_error);
        return;
    }

    Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

    switch (dialog_menu_id) {
        case DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            pPlayers[pParty->getActiveCharacter()]->OnInventoryLeftClick();
            break;
        }

        case DIALOGUE_GUILD_BUY_BOOKS: {
            int testx = (pt.x - 32) / 70;
            if (testx >= 0 && testx < 6) {
                if (pt.y >= 250) {
                    testx += 6;
                }

                bought_item = &pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - 139][testx];
                if (bought_item->uItemID != ITEM_NULL) {
                    int testpos;
                    if (pt.y >= 250) {
                        testpos = 32 + 70 * testx - 420;
                    } else {
                        testpos = 32 + 70 * testx;
                    }

                    if (pt.x >= testpos && pt.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
                        if ((pt.y >= 90 && pt.y <= (90 + shop_ui_items_in_store[testx]->GetHeight())) ||
                            (pt.y >= 250 && pt.y <= (250 + shop_ui_items_in_store[testx]->GetHeight()))) {
                            fPriceMultiplier = p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier;
                            uPriceItemService =
                                pParty
                                    ->getItemTreatmentOptionallyStrongestEffect(
                                        ITEM_TREATMENT_BUY,
                                        bought_item->GetValue(),
                                        fPriceMultiplier);

                            if (pParty->GetGold() < uPriceItemService) {
                                PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)2);
                                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                                return;
                            }

                            taken_item = pPlayers[pParty->getActiveCharacter()]->AddItem(-1, bought_item->uItemID);
                            if (taken_item) {
                                bought_item->SetIdentified();
                                memcpy(
                                    &pPlayers[pParty->getActiveCharacter()]->pInventoryItemList[taken_item - 1],
                                    bought_item, 0x24u);
                                dword_F8B1E4 = 1;
                                pParty->TakeGold(uPriceItemService);
                                bought_item->Reset();
                                render->ClearZBuffer();
                                pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_ItemBuy);
                                return;
                            }

                            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_NoRoom);
                            GameUI_SetStatusBar(LSTR_INVENTORY_IS_FULL);
                            break;
                        }
                    }
                }
            }

            return;  // no item
            break;
        }

        case DIALOGUE_SHOP_SELL: {
            invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462 ||
                (pItemID = pPlayers[pParty->getActiveCharacter()]->GetItemListAtInventoryIndex(invindex),
                 !pItemID))
                return;

            if (pPlayers[pParty->getActiveCharacter()]
                    ->pInventoryItemList[pItemID - 1]
                    .MerchandiseTest(window_SpeakInHouse->wData.val)) {
                dword_F8B1E4 = 1;
                pPlayers[pParty->getActiveCharacter()]->SalesProcess(
                    invindex, pItemID - 1, window_SpeakInHouse->wData.val);
                render->ClearZBuffer();
                pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_ItemSold);
                return;
            }

            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_WrongShop);
            pAudioPlayer->playUISound(SOUND_error);
            break;
        }

        case DIALOGUE_SHOP_IDENTIFY: {
            invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462 ||
                (pItemID = pPlayers[pParty->getActiveCharacter()]->GetItemListAtInventoryIndex(invindex),
                 !pItemID))
                return;

            uPriceItemService =
                pParty->getItemTreatmentOptionallyStrongestEffect(
                    ITEM_TREATMENT_IDENTIFY,
                    p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
            item = &pPlayers[pParty->getActiveCharacter()]->pInventoryItemList[pItemID - 1];

            if (!(item->uAttributes & ITEM_IDENTIFIED)) {
                if (item->MerchandiseTest(window_SpeakInHouse->wData.val)) {
                    if (pParty->GetGold() >= uPriceItemService) {
                        dword_F8B1E4 = 1;
                        pParty->TakeGold(uPriceItemService);
                        item->uAttributes |= ITEM_IDENTIFIED;
                        pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_ShopIdentify);
                        GameUI_SetStatusBar(LSTR_DONE);
                        return;
                    }

                    PlayHouseSound(window_SpeakInHouse->wData.val,
                                   (HouseSoundID)2);
                    return;
                }

                pAudioPlayer->playUISound(SOUND_error);
                pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_WrongShop);
                return;
            }

            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_AlreadyIdentified);
            break;
        }

        case DIALOGUE_SHOP_REPAIR: {
            invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462 ||
                (pItemID = pPlayers[pParty->getActiveCharacter()]->GetItemListAtInventoryIndex(
                         invindex),
                 !pItemID))
                return;

            item = &pPlayers[pParty->getActiveCharacter()]->pInventoryItemList[pItemID - 1];
            fPriceMultiplier =
                p2DEvents[window_SpeakInHouse->wData.val - 1]
                    .fPriceMultiplier;
            uPriceItemService =
                pParty->getItemTreatmentOptionallyStrongestEffect(
                    ITEM_TREATMENT_REPAIR, item->GetValue());
            if (item->uAttributes & ITEM_BROKEN) {
                if (item->MerchandiseTest(window_SpeakInHouse->wData.val)) {
                    if (pParty->GetGold() >= uPriceItemService) {
                        dword_F8B1E4 = 1;
                        pParty->TakeGold(uPriceItemService);
                        item->uAttributes =
                            (item->uAttributes & ~ITEM_BROKEN) | ITEM_IDENTIFIED;
                        pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_ShopRepair);
                        GameUI_SetStatusBar(LSTR_GOOD_AS_NEW);
                        return;
                    }

                    PlayHouseSound(window_SpeakInHouse->wData.val,
                                   (HouseSoundID)2);
                    return;
                }

                pAudioPlayer->playUISound(SOUND_error);
                pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_WrongShop);
                return;
            }

            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_AlreadyIdentified);
            break;
        }

        case DIALOGUE_SHOP_BUY_STANDARD:
        case DIALOGUE_SHOP_BUY_SPECIAL: {
            int testx;
            int testpos;

            switch (in_current_building_type) {
                case BuildingType_WeaponShop:

                    testx = (pt.x - 30) / 70;
                    if (testx >= 0 && testx < 6) {
                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            bought_item =
                                (ItemGen *)&pParty->StandartItemsInShops
                                    [window_SpeakInHouse->wData.val][testx];
                        else
                            bought_item =
                                &pParty->SpecialItemsInShops
                                     [window_SpeakInHouse->wData.val][testx];

                        if (bought_item->uItemID != ITEM_NULL) {
                            testpos =
                                ((60 -
                                  ((signed int)shop_ui_items_in_store[testx]
                                       ->GetWidth() /
                                   2)) +
                                 testx * 70);
                            if (pt.x >= testpos &&
                                pt.x <
                                    (testpos +
                                     (signed int)shop_ui_items_in_store[testx]
                                         ->GetWidth())) {
                                if (pt.y >= weapons_Ypos[testx] + 30 &&
                                    pt.y < (weapons_Ypos[testx] + 30 +
                                               shop_ui_items_in_store[testx]
                                                   ->GetHeight())) {
                                    break;  // good
                                }
                            } else {
                                bought_item = nullptr;
                                return;
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                case BuildingType_ArmorShop:

                    testx = (pt.x - 40) / 105;
                    if (testx >= 0 && testx < 4) {
                        if (pt.y >= 126) {
                            testx += 4;
                        }

                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            bought_item =
                                (ItemGen *)&pParty->StandartItemsInShops
                                    [window_SpeakInHouse->wData.val][testx];
                        else
                            bought_item =
                                &pParty->SpecialItemsInShops
                                     [window_SpeakInHouse->wData.val][testx];

                        if (bought_item->uItemID != ITEM_NULL) {
                            if (testx >= 4) {
                                testpos = ((90 - (shop_ui_items_in_store[testx]
                                                      ->GetWidth() /
                                                  2)) +
                                           (testx * 105) - 420);  // low row
                            } else {
                                testpos = ((86 - (shop_ui_items_in_store[testx]
                                                      ->GetWidth() /
                                                  2)) +
                                           testx * 105);
                            }

                            if (pt.x >= testpos &&
                                pt.x <=
                                    testpos + shop_ui_items_in_store[testx]
                                                  ->GetWidth()) {
                                if ((pt.y >= 126 &&
                                    pt.y <
                                         (126 + shop_ui_items_in_store[testx]
                                                    ->GetHeight())) ||
                                    (pt.y <= 98 &&
                                        pt.y >=
                                         (98 - shop_ui_items_in_store[testx]
                                                   ->GetHeight()))) {
                                    break;  // good
                                }
                            } else {
                                bought_item = nullptr;
                                return;
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                case BuildingType_AlchemistShop:
                case BuildingType_MagicShop:

                    testx = (pt.x) / 75;
                    if (testx >= 0 && testx < 6) {
                        if (pt.y > 152) {
                            testx += 6;
                        }

                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            bought_item =
                                (ItemGen *)&pParty->StandartItemsInShops
                                    [window_SpeakInHouse->wData.val][testx];
                        else
                            bought_item =
                                &pParty->SpecialItemsInShops
                                     [window_SpeakInHouse->wData.val][testx];

                        if (bought_item->uItemID != ITEM_NULL) {
                            if (pt.y > 152) {
                                testpos =
                                    75 * testx -
                                    shop_ui_items_in_store[testx]->GetWidth() /
                                        2 +
                                    40 - 450;
                            } else {
                                testpos =
                                    75 * testx -
                                    shop_ui_items_in_store[testx]->GetWidth() /
                                        2 +
                                    40;
                            }

                            if (pt.x >= testpos &&
                                pt.x <=
                                    testpos + shop_ui_items_in_store[testx]
                                                  ->GetWidth()) {
                                if ((pt.y <= 308 &&
                                    pt.y >=
                                         (308 - shop_ui_items_in_store[testx]
                                                    ->GetHeight())) ||
                                    (pt.y <= 152 &&
                                        pt.y >=
                                         (152 - shop_ui_items_in_store[testx]
                                                    ->GetHeight()))) {
                                    // y is 152-h to 152 or 308-height to 308
                                    break;  // good
                                }
                            } else {
                                bought_item = nullptr;
                                return;
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                default:
                    return;
            }
            uPriceItemService =
                pParty->getItemTreatmentOptionallyStrongestEffect(
                    ITEM_TREATMENT_BUY, bought_item->GetValue(),
                    p2DEvents[window_SpeakInHouse->wData.val - 1]
                        .fPriceMultiplier);
            uNumSeconds = 0;
            a3 = 0;
            if (pMapStats->GetMapInfo(pCurrentMapName))
                a3 = pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)]
                         ._steal_perm;
            party_reputation = pParty->GetPartyReputation();
            if (StealingMode(pParty->getActiveCharacter())) {
                uNumSeconds = pPlayers[pParty->getActiveCharacter()]->StealFromShop(
                    bought_item, a3, party_reputation, 0, &a6);
                if (!uNumSeconds) {
                    // caught stealing no item
                    sub_4B1447_party_fine(window_SpeakInHouse->wData.val, 0, a6);
                    return;
                }
            } else if (pParty->GetGold() < uPriceItemService) {
                PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)2);
                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                return;
            }

            v39 = pPlayers[pParty->getActiveCharacter()]->AddItem(-1, bought_item->uItemID);
            if (v39) {
                bought_item->SetIdentified();
                memcpy(&pPlayers[pParty->getActiveCharacter()]->pInventoryItemList[v39 - 1],
                       bought_item, sizeof(ItemGen));
                if (uNumSeconds != 0) {  // stolen
                    pPlayers[pParty->getActiveCharacter()]
                        ->pInventoryItemList[v39 - 1]
                        .SetStolen();
                    sub_4B1447_party_fine(window_SpeakInHouse->wData.val,
                                          uNumSeconds, a6);
                } else {
                    dword_F8B1E4 = 1;
                    pParty->TakeGold(uPriceItemService);
                }
                bought_item->Reset();
                render->ClearZBuffer();
                pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_ItemBuy);
                return;
            } else {
                pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_NoRoom);
                GameUI_SetStatusBar(LSTR_INVENTORY_IS_FULL);
                return;
            }
            break;
        }
        default:  // if click video screen in shop
        {
            if (IsSkillLearningDialogue(dialog_menu_id)) {
                PLAYER_SKILL_TYPE skill = GetLearningDialogueSkill(dialog_menu_id);
                v43 = (int64_t)(p2DEvents[
                    window_SpeakInHouse->wData.val - 1].flt_24 * 500.0);
                uPriceItemService = v43 *
                    (100 - pPlayers[pParty->getActiveCharacter()]->GetMerchant()) / 100;
                if (uPriceItemService < v43 / 3) uPriceItemService = v43 / 3;
                if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill] != PLAYER_SKILL_MASTERY_NONE) {
                    pSkill = &pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill];
                    if (!*pSkill) {
                        if (pParty->GetGold() < uPriceItemService) {
                            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                            if (in_current_building_type == BuildingType_Training)
                                v55 = HouseSound_Goodbye;
                            else
                                v55 = HouseSound_NotEnoughMoney;
                            PlayHouseSound(
                                window_SpeakInHouse->wData.val,
                                (HouseSoundID)v55);
                            return;
                        }
                        pParty->TakeGold(uPriceItemService);
                        dword_F8B1E4 = 1;
                        *pSkill = 1;
                        pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_SkillLearned);
                        return;
                    }
                }
            }
            break;
        }
    }
}

//----- new function
void ShowPopupShopSkills() {
    int pX = 0;
    int pY = 0;
    mouse->GetClickPos(&pX, &pY);

    if (pDialogueWindow) {
        for (GUIButton *pButton : pDialogueWindow->vButtons) {
            if (pX >= pButton->uX && pX < pButton->uZ && pY >= pButton->uY && pY < pButton->uW) {
                if (IsSkillLearningDialogue((DIALOGUE_TYPE)pButton->msg_param)) {
                    auto skill_id = GetLearningDialogueSkill((DIALOGUE_TYPE)pButton->msg_param);
                    if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill_id] != PLAYER_SKILL_MASTERY_NONE
                        && !pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill_id]) {
                        // is this skill visible
                        std::string pSkillDescText = CharacterUI_GetSkillDescText(pParty->getActiveCharacter() - 1, skill_id);
                        CharacterUI_DrawTooltip(localization->GetSkillName(skill_id), pSkillDescText);
                    }
                }
            }
        }
    }
}


//----- (004B1A2D) --------------------------------------------------------
void ShowPopupShopItem() {
    ItemGen *item;  // ecx@13
    int invindex;
    int testpos;

    if (in_current_building_type == BuildingType_Invalid) return;
    if (dialog_menu_id < DIALOGUE_SHOP_BUY_STANDARD) return;

    Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();
    int testx;

    if (in_current_building_type <= BuildingType_AlchemistShop) {
        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD ||
            dialog_menu_id == DIALOGUE_SHOP_BUY_SPECIAL) {
            switch (in_current_building_type) {
                case BuildingType_WeaponShop: {
                    testx = (pt.x - 30) / 70;
                    if (testx >= 0 && testx < 6) {
                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            item =
                                (ItemGen *)&pParty->StandartItemsInShops
                                    [window_SpeakInHouse->wData.val][testx];
                        else
                            item =
                                &pParty->SpecialItemsInShops
                                     [window_SpeakInHouse->wData.val][testx];

                        if (item->uItemID != ITEM_NULL) {
                            testpos =
                                ((60 -
                                  (shop_ui_items_in_store[testx]->GetWidth() /
                                   2)) +
                                 testx * 70);
                            if (pt.x >= testpos &&
                                pt.x <
                                    (testpos + shop_ui_items_in_store[testx]
                                                   ->GetWidth())) {
                                if (pt.y >= weapons_Ypos[testx] + 30 &&
                                    pt.y < (weapons_Ypos[testx] + 30 +
                                               shop_ui_items_in_store[testx]
                                                   ->GetHeight())) {
                                    GameUI_DrawItemInfo(item);
                                }
                            } else {
                                return;
                            }
                        }
                    } else {
                        return;
                    }

                    break;
                }

                case BuildingType_ArmorShop:
                    testx = (pt.x - 40) / 105;
                    if (testx >= 0 && testx < 4) {
                        if (pt.y >= 126) {
                            testx += 4;
                        }

                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            item =
                                (ItemGen *)&pParty->StandartItemsInShops
                                    [window_SpeakInHouse->wData.val][testx];
                        else
                            item =
                                &pParty->SpecialItemsInShops
                                     [window_SpeakInHouse->wData.val][testx];

                        if (item->uItemID != ITEM_NULL) {
                            if (testx >= 4) {
                                testpos = ((90 - (shop_ui_items_in_store[testx]
                                                      ->GetWidth() /
                                                  2)) +
                                           (testx * 105) - 420);  // low row
                            } else {
                                testpos = ((86 - (shop_ui_items_in_store[testx]
                                                      ->GetWidth() /
                                                  2)) +
                                           testx * 105);
                            }

                            if (pt.x >= testpos &&
                                pt.x <=
                                    testpos + shop_ui_items_in_store[testx]
                                                  ->GetWidth()) {
                                if ((pt.y >= 126 &&
                                    pt.y <
                                         (126 + shop_ui_items_in_store[testx]
                                                    ->GetHeight())) ||
                                    (pt.y <= 98 &&
                                        pt.y >=
                                         (98 - shop_ui_items_in_store[testx]
                                                   ->GetHeight()))) {
                                    GameUI_DrawItemInfo(item);
                                } else {
                                    return;
                                }
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                case BuildingType_AlchemistShop:
                case BuildingType_MagicShop:
                    testx = (pt.x) / 75;
                    // testx limits check
                    if (testx >= 0 && testx < 6) {
                        if (pt.y >= 152) {
                            testx += 6;
                        }

                        if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD)
                            item =
                                (ItemGen *)&pParty->StandartItemsInShops
                                    [window_SpeakInHouse->wData.val][testx];
                        else
                            item =
                                &pParty->SpecialItemsInShops
                                     [window_SpeakInHouse->wData.val][testx];

                        if (item->uItemID != ITEM_NULL) {
                            if (pt.y > 152) {
                                testpos =
                                    75 * testx -
                                    shop_ui_items_in_store[testx]->GetWidth() /
                                        2 +
                                    40 - 450;
                            } else {
                                testpos =
                                    75 * testx -
                                    shop_ui_items_in_store[testx]->GetWidth() /
                                        2 +
                                    40;
                            }

                            if (pt.x >= testpos &&
                                pt.x <=
                                    testpos + shop_ui_items_in_store[testx]
                                                  ->GetWidth()) {
                                if ((pt.y <= 308 &&
                                    pt.y >=
                                         (308 - shop_ui_items_in_store[testx]
                                                    ->GetHeight())) ||
                                    (pt.y <= 152 &&
                                        pt.y >=
                                         (152 - shop_ui_items_in_store[testx]
                                                    ->GetHeight()))) {
                                    GameUI_DrawItemInfo(item);
                                } else {
                                    return;
                                }
                            }
                        } else {
                            return;
                        }
                    }
                    break;

                default:
                    // v3 = render->pActiveZBuffer[mouse.x +
                    // pSRZBufferLineOffsets[mouse.y]] & 0xFFFF; if (!v3)
                    // return;
                    // v7 = &pParty->StandartItemsInShops[(unsigned
                    // int)window_SpeakInHouse->ptr_1C][v3 - 1]; if
                    // (dialog_menu_id == DIALOGUE_SHOP_BUY_SPECIAL) v7 =
                    // &pParty->SpecialItemsInShops[(unsigned
                    // int)window_SpeakInHouse->ptr_1C][v3 - 1];
                    // GameUI_DrawItemInfo(v7);
                    return;
                    break;
            }
        }

        if (dialog_menu_id >= DIALOGUE_SHOP_SELL &&
                dialog_menu_id <= DIALOGUE_SHOP_REPAIR ||
            dialog_menu_id == DIALOGUE_SHOP_DISPLAY_EQUIPMENT) {
            invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462 ||
                !pPlayers[pParty->getActiveCharacter()]->GetItemListAtInventoryIndex(
                    invindex))
                return;

            GameUI_DrawItemInfo(
                pPlayers[pParty->getActiveCharacter()]->GetItemAtInventoryIndex(invindex));
            return;
        }
    }

    if (in_current_building_type <= BuildingType_MirroredPath && dialog_menu_id == DIALOGUE_GUILD_BUY_BOOKS) {
        int testx = (pt.x - 32) / 70;
        if (testx >= 0 && testx < 6) {
            if (pt.y >= 250) {
                testx += 6;
            }

            item = &pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][testx];

            if (item->uItemID != ITEM_NULL) {
                int testpos;
                if (pt.y >= 250) {
                    testpos = 32 + 70 * testx - 420;
                } else {
                    testpos = 32 + 70 * testx;
                }

                if (pt.x >= testpos && pt.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
                    if ((pt.y >= 90 && pt.y <= (90 + shop_ui_items_in_store[testx]->GetHeight())) || (pt.y >= 250 && pt.y <= (250 + shop_ui_items_in_store[testx]->GetHeight()))) {
                        unsigned int guildId = window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE;
                        sub_4B1523_showSpellbookInfo(pParty->SpellBooksInGuilds[guildId][testx].uItemID);
                    }
                }
            }
        }
    }
}

void sub_4B1523_showSpellbookInfo(ITEM_TYPE spellItemId) {
    int v4;               // eax@4
    int v6;               // eax@10
    char *v7;             // ST44_4@12
    uint16_t v8;  // ax@12
    int v13;              // [sp+6Ch] [bp-8h]@4
    int v14;              // [sp+70h] [bp-4h]@4

    // TODO(captainurist): deal away with casts.
    SPELL_TYPE spellId = static_cast<SPELL_TYPE>(std::to_underlying(spellItemId) - 399);
    int spellLevel = (std::to_underlying(spellItemId) - 400) % 11 + 1;
    unsigned int spellSchool = 4 * (std::to_underlying(spellItemId) - 400) / 11;

    // sprintf(tmp_str.data(), "%s%03d", spellbook_texture_filename_suffices[v11
    // / 4], v2); not used

    Pointi a2 = EngineIocContainer::ResolveMouse()->GetCursorPos();
    unsigned int v3 = 30;
    if (a2.y <= 320) {
        v3 = a2.y + 30;
    }

    GUIWindow a1;
    a1.Init();
    a1.uFrameY = v3;
    a1.uFrameWidth = 328;
    a1.uFrameHeight = 68;
    a1.uFrameX = 90;
    a1.uFrameZ = 417;
    a1.uFrameW = v3 + 67;

    int v5 = std::max({
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_NORMAL)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_EXPERT)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_MASTER)),
        pFontSmallnum->GetLineWidth(localization->GetString(LSTR_GRAND))
    });

    auto str = fmt::format(
        "{}\n\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}\t000\n{}\t{:03}:\t{:03}{}",
        pSpellStats->pInfos[spellId].pDescription,
        localization->GetString(LSTR_NORMAL), v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pBasicSkillDesc,
        localization->GetString(LSTR_EXPERT), v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pExpertSkillDesc,
        localization->GetString(LSTR_MASTER), v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pMasterSkillDesc,
        localization->GetString(LSTR_GRAND), v5 + 3, v5 + 10, pSpellStats->pInfos[spellId].pGrandmasterSkillDesc
    );
    v6 = pFontSmallnum->CalcTextHeight(str, a1.uFrameWidth, 0);
    a1.uFrameHeight += v6;
    if (a1.uFrameHeight < 150) {
        a1.uFrameHeight = 150;
    }
    a1.uFrameWidth = game_viewport_width;
    a1.DrawMessageBox(0);
    a1.uFrameWidth -= 12;
    a1.uFrameHeight -= 12;
    v7 = pSpellStats->pInfos[spellId].pName;
    a1.uFrameZ = a1.uFrameX + a1.uFrameWidth - 1;
    a1.uFrameW = a1.uFrameHeight + a1.uFrameY - 1;
    v8 = colorTable.PaleCanary.c16();
    a1.DrawTitleText(pFontArrus, 0x78u, 0xCu, v8, v7, 3u);
    a1.DrawText(pFontSmallnum, {120, 44}, colorTable.Black.c16(), str, 0, 0, 0);
    a1.uFrameZ = a1.uFrameX + 107;
    a1.uFrameWidth = 108;
    a1.DrawTitleText(pFontComic, 0xCu, 0x4Bu, 0,
                     localization->GetSkillName(static_cast<PLAYER_SKILL_TYPE>(spellSchool / 4 + 12)), 3u);

    str = fmt::format("{}\n{}", localization->GetString(LSTR_SP_COST), pSpellDatas[spellId].uNormalLevelMana);
    a1.DrawTitleText(pFontComic, 0xCu, a1.uFrameHeight - pFontComic->GetHeight() - 16, 0, str, 3);
}

//----- (004B1D27) --------------------------------------------------------
void GetHouseGoodbyeSpeech() {
    int v7[4];      // [sp+Ch] [bp-10h]@12

    if (in_current_building_type != BuildingType_Invalid) {
        if (in_current_building_type > BuildingType_MagicShop) {
            if (in_current_building_type == BuildingType_Bank) {
                if (!dword_F8B1E4) return;
            } else {
                if (in_current_building_type != BuildingType_Temple) return;
            }
            PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Greeting_2);
            return;
        }
        if (pParty->PartyTimes._shop_ban_times[window_SpeakInHouse->wData.val] <= pParty->GetPlayingTime()) {
            if (pParty->GetGold() <= 10000) {
                if (!dword_F8B1E4) return;
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Goodbye);
                return;
            }
            PlayHouseSound(window_SpeakInHouse->wData.val,
                           (HouseSoundID)(dword_F8B1E4 + 3));
            if (!dword_F8B1E4 && !_A750D8_player_speech_timer) {
                int id = pParty->getRandomActiveCharacterId(vrng.get());

                if (id != -1) {
                    _A750D8_player_speech_timer = 256;
                    PlayerSpeechID = SPEECH_ShopRude;
                    uSpeakingCharacter = id;
                    return;
                }
            }
        } else {  // caught stealing
            if (!_A750D8_player_speech_timer) {
                int id = pParty->getRandomActiveCharacterId(vrng.get());

                if (id != -1) {
                    _A750D8_player_speech_timer = 256;
                    PlayerSpeechID = SPEECH_ShopRude;
                    uSpeakingCharacter = id;
                    return;
                }
            }
        }
    }
}

//----- (004B1447) --------------------------------------------------------
void sub_4B1447_party_fine(int shopId, int stealingResult,
                           int fineToAdd) {  // not working properly??
    int v3;       // esi@1
    DDM_DLV_Header *v7;  // eax@14

    if (stealingResult == 0 || stealingResult == 1) {  // got caught
        if (pParty->uFine < 4000000) {
            if (fineToAdd + pParty->uFine < 0)
                pParty->uFine = 0;
            else if (fineToAdd + pParty->uFine > 4000000)
                pParty->uFine = 4000000;
            else
                pParty->uFine += fineToAdd;
        }
        if (pParty->uFine) {
            for (Player &player : pParty->pPlayers) {
                if (!_449B57_test_bit(player._achieved_awards_bits, Award_Fine))
                    _449B7E_toggle_bit(player._achieved_awards_bits, Award_Fine, 1);
            }
        }
        if (stealingResult == 1)
            v3 = 2;
        else
            v3 = 1;

        pParty->PartyTimes._shop_ban_times[shopId] = GameTime(pParty->GetPlayingTime()
            + GameTime::FromDays(1));  // only ban when caught
    } else {
        v3 = 2;
    }

    pParty->InTheShopFlags[shopId] = 1;
    v7 = &pOutdoor->ddm;
    if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) v7 = &pIndoor->dlv;
    v7->uReputation += v3;
    if (v7->uReputation > 10000) v7->uReputation = 10000;
}
