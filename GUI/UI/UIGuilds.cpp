#include "GUI/UI/UIGuilds.h"

#include "Engine/Engine.h"
#include "Engine/Events2D.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"

#include "Engine/Objects/Items.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"

#include "IO/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

using EngineIoc = Engine_::IocContainer;

void GuildDialog() {
    int textoffset;               // ecx@47
    GUIButton *pButton;           // eax@49
    int pTextHeight;              // eax@55
    unsigned __int16 pTextColor;  // ax@55
    int textspacings;             // [sp+2D4h] [bp-18h]@1
    bool pSkillFlag;              // [sp+2DCh] [bp-10h]@35
    int dialogopts;               // [sp+2E0h] [bp-Ch]@35

    int all_text_height;
    int pX;

    GUIWindow working_window = *window_SpeakInHouse;
    working_window.uFrameX = 483;
    working_window.uFrameWidth = 148;
    working_window.uFrameZ = 334;

    int base_teach_price =
        (p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1]
             .fPriceMultiplier *
         500.0);
    int pPrice = base_teach_price *
                 (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
    if (pPrice < base_teach_price / 3) pPrice = base_teach_price / 3;

    if (dialog_menu_id == HOUSE_DIALOGUE_MAIN) {  // change to switch??
        if (!(uint16_t)_449B57_test_bit(
                (uint8_t *)pPlayers[uActiveCharacter]->_achieved_awards_bits,
                guild_mambership_flags[(uint64_t)
                                           window_SpeakInHouse->ptr_1C -
                                       139])) {  // you must me member
            pTextHeight = pFontArrus->CalcTextHeight(
                pNPCTopics[121].pText, working_window.uFrameWidth, 0);
            working_window.DrawTitleText(
                pFontArrus, 0, (212 - pTextHeight) / 2 + 101,
                Color16(0xFFu, 0xFFu, 0x9Bu), pNPCTopics[121].pText, 3);
            pDialogueWindow->pNumPresenceButton = 0;
            return;
        }

        if (!HouseUI_CheckIfPlayerCanInteract()) {
            return;
        }

        dialogopts = 0;
        pSkillFlag = false;
        all_text_height = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
             i < pDialogueWindow->pNumPresenceButton +
                     pDialogueWindow->pStartingPosActiveItem;
             ++i) {
            if (pDialogueWindow->GetControl(i)->msg_param == 18) {
                all_text_height += pFontArrus->CalcTextHeight(
                    localization->GetString(400), working_window.uFrameWidth,
                    0);  // "Buy Spells"
                dialogopts++;
            } else {
                if (byte_4ED970_skill_learn_ability_by_class_table
                        [pPlayers[uActiveCharacter]->classType]
                        [pDialogueWindow->GetControl(i)->msg_param - 36] &&
                    !pPlayers[uActiveCharacter]->pActiveSkills
                         [pDialogueWindow->GetControl(i)->msg_param - 36]) {
                    all_text_height += pFontArrus->CalcTextHeight(
                        localization->GetSkillName(
                            pDialogueWindow->GetControl(i)->msg_param - 36),
                        working_window.uFrameWidth, 0, 0);
                    dialogopts++;
                    pSkillFlag = true;
                }
            }
        }

        if (!dialogopts) {  // code never gets here??
            String str = localization->FormatString(
                             544, pPlayers[uActiveCharacter]->pName,
                             localization->GetClassName(
                                 pPlayers[uActiveCharacter]->classType)) +
                         "\n \n"  // Seek knowledge elsewhere %s the %s
                         + localization->GetString(
                               528);  // "I can offer you nothing further."
            pTextHeight =
                pFontArrus->CalcTextHeight(str, working_window.uFrameWidth, 0);
            working_window.DrawTitleText(pFontArrus, 0,
                                         (174 - pTextHeight) / 2 + 138,
                                         Color16(0xFFu, 0xFFu, 0x9Bu), str, 3);
            return;
        }

        if (pSkillFlag) {
            working_window.DrawTitleText(
                pFontArrus, 0, 0x92u, 0,
                localization->FormatString(401, pPrice), 3);  // Skill Cost: %lu
        }

        textspacings = (149 - all_text_height) / dialogopts;
        if (textspacings > 32) {
            textspacings = 32;
        }

        textoffset = 162 - textspacings / 2;

        for (int i = pDialogueWindow->pStartingPosActiveItem;
             i < pDialogueWindow->pNumPresenceButton +
                     pDialogueWindow->pStartingPosActiveItem;
             ++i) {
            pButton = pDialogueWindow->GetControl(i);

            if (pButton->msg_param == 18) {
                pButton->uY = textspacings + textoffset;
                pTextHeight =
                    pFontArrus->CalcTextHeight(localization->GetString(400),
                                               working_window.uFrameWidth, 0);
                pButton->uHeight = pTextHeight;
                textoffset = pButton->uY + pTextHeight - 1;
                pButton->uW = textoffset + 6;
                pTextColor = Color16(0xFFu, 0xFFu, 0x9Bu);
                if (pDialogueWindow->pCurrentPosActiveItem != i) {
                    pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
                }
                working_window.DrawTitleText(
                    pFontArrus, 0, pButton->uY, pTextColor,
                    localization->GetString(400), 3);  // "Buy Spells"
            } else {
                if (byte_4ED970_skill_learn_ability_by_class_table
                        [pPlayers[uActiveCharacter]->classType]
                        [pButton->msg_param - 36] &&
                    !pPlayers[uActiveCharacter]
                         ->pActiveSkills[pButton->msg_param - 36]) {
                    pButton->uY = textspacings + textoffset;
                    pTextHeight = pFontArrus->CalcTextHeight(
                        localization->GetSkillName(pButton->msg_param - 36),
                        working_window.uFrameWidth, 0);
                    pButton->uHeight = pTextHeight;
                    textoffset = pButton->uY + pTextHeight - 1;
                    pButton->uW = textoffset + 6;
                    pTextColor = Color16(0xFFu, 0xFFu, 0x9Bu);
                    if (pDialogueWindow->pCurrentPosActiveItem != i) {
                        pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
                    }
                    working_window.DrawTitleText(
                        pFontArrus, 0, pButton->uY, pTextColor,
                        localization->GetSkillName(pButton->msg_param - 36), 3);
                } else {
                    pButton->uW = 0;
                    pButton->uHeight = 0;
                    pButton->uY = 0;
                }
            }
        }
        return;
    }

    if (dialog_menu_id == HOUSE_DIALOGUE_GUILD_BUY_BOOKS) {  // buy books
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        int itemxind = 0;

        for (pX = 32; pX < 452; pX += 70) {  //расположение в верхнем ряду
            if (pParty
                    ->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139]
                                        [itemxind]
                    .uItemID) {
                render->DrawTextureAlphaNew(
                    pX / 640.0f, 90 / 480.0f,
                    shop_ui_items_in_store[itemxind]);  // top row
            }
            if (pParty
                    ->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139]
                                        [itemxind + 6]
                    .uItemID) {
                render->DrawTextureAlphaNew(
                    pX / 640.0f, 250 / 480.0f,
                    shop_ui_items_in_store[itemxind + 6]);
            }

            ++itemxind;
        }

        if (HouseUI_CheckIfPlayerCanInteract()) {
            int itemcount = 0;
            for (uint i = 0; i < 12; ++i) {
                if (pParty
                        ->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139]
                                            [i]
                        .uItemID > 0)
                    ++itemcount;
            }

            GameUI_StatusBar_DrawImmediate(localization->GetString(195),
                                           0);  // "Select the Item to Buy"

            if (!itemcount) {  // shop empty
                working_window.DrawShops_next_generation_time_string(
                    pParty->PartyTimes
                        .Shops_next_generation_time[window_SpeakInHouse->par1C -
                                                    139] -
                    pParty->GetPlayingTime());
                return;
            }

            Point pt = EngineIoc::ResolveMouse()->GetCursorPos();
            int testx = (pt.x - 32) / 70;
            if (testx >= 0 && testx < 6) {
                if (pt.y >= 250) {
                    testx += 6;
                }

                ItemGen *item = &pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][testx];

                if (item->uItemID) {
                    int testpos;
                    if (pt.y >= 250) {
                        testpos = 32 + 70 * testx - 420;
                    } else {
                        testpos = 32 + 70 * testx;
                    }

                    if (pt.x >= testpos &&
                        pt.x <=
                            testpos +
                                shop_ui_items_in_store[testx]->GetWidth()) {
                        if ((pt.y >= 90 &&
                            pt.y <= (90 + shop_ui_items_in_store[testx]
                                                  ->GetHeight())) ||
                            (pt.y >= 250 &&
                             pt.y <= (250 + shop_ui_items_in_store[testx]
                                                   ->GetHeight()))) {
                            auto str = BuildDialogueString(
                                pMerchantsBuyPhrases
                                    [pPlayers[uActiveCharacter]
                                         ->SelectPhrasesTransaction(
                                             item, BuildingType_MagicShop,
                                             (int64_t)window_SpeakInHouse->ptr_1C,
                                             2)],
                                uActiveCharacter - 1, item,
                                (char *)window_SpeakInHouse->ptr_1C, 2);
                            pTextHeight = pFontArrus->CalcTextHeight(
                                str, working_window.uFrameWidth, 0);
                            working_window.DrawTitleText(
                                pFontArrus, 0, (174 - pTextHeight) / 2 + 138,
                                Color16(0xFFu, 0xFFu, 0xFFu), str, 3);
                            return;
                        }
                    }
                }
            }
        }
        return;
    }

    if (HouseUI_CheckIfPlayerCanInteract()) {  // buy skills
        if (pPlayers[uActiveCharacter]->pActiveSkills[dialog_menu_id - 36]) {
            GameUI_StatusBar_OnEvent(localization->FormatString(
                403,
                localization->GetSkillName(dialog_menu_id - 36)));  // You already know the %s skill
            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
        } else {
            if (pParty->GetGold() < pPrice) {
                GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
                PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C,
                               HouseSound_NotEnoughMoney_TrainingSuccessful);
            } else {
                Party::TakeGold(pPrice);
                pPlayers[uActiveCharacter]->pActiveSkills[dialog_menu_id - 36] =
                    1;
            }
        }
    }
    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
}

//----- (004BC8D5) --------------------------------------------------------
void SpellBookGenerator() {  // for GuildDialogs
    int pItemNum;   // esi@1
    int randomnum;  // esi@7

    for (int i = 0; i < 12; ++i) {
        if (p2DEvents[window_SpeakInHouse->par1C - 1].uType >= 5) {
            if (p2DEvents[window_SpeakInHouse->par1C - 1].uType <= 13) {
                pItemNum =
                    rand() %
                    word_4F0F30[(signed int)window_SpeakInHouse->par1C -
                    139] +
                    11 *
                    p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1]
                    .uType +
                    345;
            } else {
                if (p2DEvents[window_SpeakInHouse->par1C - 1].uType == 14)
                    randomnum = rand() % 4;
                else if (p2DEvents[window_SpeakInHouse->par1C - 1].uType == 15)
                    randomnum = rand() % 3 + 4;
                else if (p2DEvents[window_SpeakInHouse->par1C - 1].uType == 16)
                    randomnum = rand() % 2 + 7;
                if (p2DEvents[window_SpeakInHouse->par1C - 1].uType <= 16)
                    pItemNum =
                        rand() %
                            word_4F0F30[(signed int)window_SpeakInHouse->par1C -
                                        139] +
                        11 * randomnum + 400;
            }
        }

        if (pItemNum == 487) {  // divine intervention check
            if (!(unsigned __int16)_449B57_test_bit(pParty->_quest_bits, 239))
                pItemNum = 486;
        }

        ItemGen *item_spellbook =
            &pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][i];
        item_spellbook->Reset();
        pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][i]
            .uItemID = pItemNum;
        pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][i]
            .IsIdentified();

        shop_ui_items_in_store[i] = assets->GetImage_ColorKey(
            pItemsTable->pItems[pItemNum].pIconName, 0x7FF);
    }
}
