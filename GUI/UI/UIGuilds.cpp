#include "GUI/UI/UIGuilds.h"

#include "Engine/Engine.h"
#include "Engine/Events2D.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Party.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

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

    if (dialog_menu_id == DIALOGUE_MAIN) {  // change to switch??
        if (!_449B57_test_bit(
                (uint8_t *)pPlayers[uActiveCharacter]->_achieved_awards_bits,
                guild_mambership_flags[(uint64_t)window_SpeakInHouse->ptr_1C - 139])) {
            // you must be a member
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
            if (pDialogueWindow->GetControl(i)->msg_param == DIALOGUE_GUILD_BUY_BOOKS) {
                all_text_height += pFontArrus->CalcTextHeight(
                    localization->GetString(LSTR_BUY_SPELLS), working_window.uFrameWidth,
                    0);
                dialogopts++;
            } else {
                auto skill = GetLearningDialogueSkill(
                    (DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param
                );
                if (byte_4ED970_skill_learn_ability_by_class_table
                        [pPlayers[uActiveCharacter]->classType][skill] &&
                    !pPlayers[uActiveCharacter]->pActiveSkills[skill]) {
                    all_text_height += pFontArrus->CalcTextHeight(
                        localization->GetSkillName(skill),
                        working_window.uFrameWidth, 0, 0);
                    dialogopts++;
                    pSkillFlag = true;
                }
            }
        }

        SkillTrainingDialogue(&working_window, dialogopts, all_text_height, pPrice);
        return;
    }

    if (dialog_menu_id == DIALOGUE_GUILD_BUY_BOOKS) {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        int itemxind = 0;

        for (pX = 32; pX < 452; pX += 70) {  // top row
            if (pParty->SpellBooksInGuilds
                [window_SpeakInHouse->par1C - 139][itemxind].uItemID) {
                render->DrawTextureAlphaNew(
                    pX / 640.0f, 90 / 480.0f,
                    shop_ui_items_in_store[itemxind]);
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
                if (pParty->SpellBooksInGuilds
                    [window_SpeakInHouse->par1C - 139][i].uItemID > 0)
                    ++itemcount;
            }

            GameUI_StatusBar_DrawImmediate(
                localization->GetString(LSTR_SELECT_ITEM_TO_BUY), 0);

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
        auto skill = GetLearningDialogueSkill(dialog_menu_id);
        if (pPlayers[uActiveCharacter]->pActiveSkills[skill]) {
            GameUI_SetStatusBar(
                LSTR_FMT_ALREADY_KNOW_THE_S_SKILL,
                localization->GetSkillName(skill)
            );
            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
        } else {
            if (pParty->GetGold() < pPrice) {
                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C,
                               HouseSound_NotEnoughMoney_TrainingSuccessful);
            } else {
                Party::TakeGold(pPrice);
                pPlayers[uActiveCharacter]->pActiveSkills[skill] = 1;
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
                pItemNum = rand() %
                    word_4F0F30[(signed int)window_SpeakInHouse->par1C - 139]
                    + 11 * p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1].uType
                    + 345;
            } else {
                if (p2DEvents[window_SpeakInHouse->par1C - 1].uType == 14)
                    randomnum = rand() % 4;
                else if (p2DEvents[window_SpeakInHouse->par1C - 1].uType == 15)
                    randomnum = rand() % 3 + 4;
                else if (p2DEvents[window_SpeakInHouse->par1C - 1].uType == 16)
                    randomnum = rand() % 2 + 7;
                if (p2DEvents[window_SpeakInHouse->par1C - 1].uType <= 16)
                    pItemNum = rand() %
                        word_4F0F30[(signed int)window_SpeakInHouse->par1C - 139]
                        + 11 * randomnum + 400;
            }
        }

        if (pItemNum == ITEM_SPELLBOOK_LIGHT_DIVINE_INTERVENTION) {
            if (!_449B57_test_bit(pParty->_quest_bits, QBIT_DIVINE_INTERVENTION_RETRIEVED))
                pItemNum = ITEM_SPELLBOOK_LIGHT_SUN_BURST;
        }

        ItemGen *item_spellbook = &pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][i];
        item_spellbook->Reset();
        pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][i].uItemID = pItemNum;
        pParty->SpellBooksInGuilds[window_SpeakInHouse->par1C - 139][i].IsIdentified();

        shop_ui_items_in_store[i] = assets->GetImage_ColorKey(
            pItemsTable->pItems[pItemNum].pIconName, render->teal_mask_16);
    }
}
