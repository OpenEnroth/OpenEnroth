#include "GUI/UI/UIGuilds.h"

#include <string>

#include "Engine/Engine.h"
#include "Engine/Events2D.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

void GuildDialog() {
    int pTextHeight;              // eax@55
    int dialogopts;               // [sp+2E0h] [bp-Ch]@35

    int all_text_height;
    int pX;

    GUIWindow working_window = *window_SpeakInHouse;
    working_window.uFrameX = 483;
    working_window.uFrameWidth = 148;
    working_window.uFrameZ = 334;

    int base_teach_price = (p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier * 500.0);
    int pPrice = base_teach_price * (100 - PriceCalculator::playerMerchant(pPlayers[pParty->getActiveCharacter()])) / 100;
    if (pPrice < base_teach_price / 3)
        pPrice = base_teach_price / 3;

    if (dialog_menu_id == DIALOGUE_MAIN) {  // change to switch??
        if (!_449B57_test_bit((uint8_t *)pPlayers[pParty->getActiveCharacter()]->_achieved_awards_bits, guild_membership_flags[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE])) {
            // you must be a member
            pTextHeight = pFontArrus->CalcTextHeight(
                pNPCTopics[121].pText, working_window.uFrameWidth, 0);
            working_window.DrawTitleText(pFontArrus, 0, (212 - pTextHeight) / 2 + 101, colorTable.PaleCanary.c16(), pNPCTopics[121].pText, 3);
            pDialogueWindow->pNumPresenceButton = 0;
            return;
        }

        if (!HouseUI_CheckIfPlayerCanInteract()) {
            return;
        }

        dialogopts = 0;
        all_text_height = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
            if (pDialogueWindow->GetControl(i)->msg_param == DIALOGUE_GUILD_BUY_BOOKS) {
                all_text_height += pFontArrus->CalcTextHeight(localization->GetString(LSTR_BUY_SPELLS), working_window.uFrameWidth, 0);
                dialogopts++;
            } else {
                PLAYER_SKILL_TYPE skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
                if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill] != PLAYER_SKILL_MASTERY_NONE
                    && !pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill]) {
                    all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(skill), working_window.uFrameWidth, 0, 0);
                    dialogopts++;
                }
            }
        }

        SkillTrainingDialogue(&working_window, dialogopts, all_text_height, pPrice);
        return;
    }

    if (dialog_menu_id == DIALOGUE_GUILD_BUY_BOOKS) {
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
        int itemxind = 0;

        for (pX = 32; pX < 452; pX += 70) {  // top row
            if (pParty->SpellBooksInGuilds
                [window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][itemxind].uItemID != ITEM_NULL) {
                render->DrawTextureNew(pX / 640.0f, 90 / 480.0f, shop_ui_items_in_store[itemxind]);
            }
            if (pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][itemxind + 6].uItemID != ITEM_NULL) {
                render->DrawTextureNew(pX / 640.0f, 250 / 480.0f, shop_ui_items_in_store[itemxind + 6]);
            }

            ++itemxind;
        }

        if (HouseUI_CheckIfPlayerCanInteract()) {
            int itemcount = 0;
            for (uint i = 0; i < 12; ++i) {
                if (pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][i].uItemID != ITEM_NULL)
                    ++itemcount;
            }

            GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), 0);

            if (!itemcount) {  // shop empty
                working_window.DrawShops_next_generation_time_string(
                    pParty->PartyTimes.Shops_next_generation_time[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE] - pParty->GetPlayingTime());
                return;
            }

            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();
            int testx = (pt.x - 32) / 70;
            if (testx >= 0 && testx < 6) {
                if (pt.y >= 250) {
                    testx += 6;
                }

                ItemGen *item = &pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][testx];

                if (item->uItemID != ITEM_NULL) {
                    int testpos;
                    if (pt.y >= 250) {
                        testpos = 32 + 70 * testx - 420;
                    } else {
                        testpos = 32 + 70 * testx;
                    }

                    if (pt.x >= testpos && pt.x <= testpos + shop_ui_items_in_store[testx]->GetWidth()) {
                        if ((pt.y >= 90 && pt.y <= (90 + shop_ui_items_in_store[testx]->GetHeight())) || (pt.y >= 250 && pt.y <= (250 + shop_ui_items_in_store[testx]->GetHeight()))) {
                            std::string str = BuildDialogueString(pMerchantsBuyPhrases[pPlayers[pParty->getActiveCharacter()]->SelectPhrasesTransaction(
                                             item, BuildingType_MagicShop, window_SpeakInHouse->wData.val, 2)],
                                pParty->getActiveCharacter() - 1, item, window_SpeakInHouse->wData.val, 2);
                            pTextHeight = pFontArrus->CalcTextHeight(str, working_window.uFrameWidth, 0);
                            working_window.DrawTitleText(pFontArrus, 0, (174 - pTextHeight) / 2 + 138, colorTable.White.c16(), str, 3);
                            return;
                        }
                    }
                }
            }
        }
        return;
    }

    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
}

//----- (004BC8D5) --------------------------------------------------------
void SpellBookGenerator() {  // for GuildDialogs
    ITEM_TYPE pItemNum;   // esi@1
    int randomnum;  // esi@7

    for (int i = 0; i < 12; ++i) {
        // TODO(captainurist): clean up these ITEM_TYPE casts.
        if (p2DEvents[window_SpeakInHouse->wData.val - 1].uType >= BuildingType_FireGuild) {
            if (p2DEvents[window_SpeakInHouse->wData.val - 1].uType <= BuildingType_DarkGuild) {
                pItemNum = ITEM_TYPE(grng->random(word_4F0F30[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE]) +
                    11 * std::to_underlying(p2DEvents[window_SpeakInHouse->wData.val - 1].uType) +
                                     345);
            } else {
                if (p2DEvents[window_SpeakInHouse->wData.val - 1].uType == BuildingType_ElementalGuild)
                    randomnum = grng->random(4);
                else if (p2DEvents[window_SpeakInHouse->wData.val - 1].uType == BuildingType_SelfGuild)
                    randomnum = grng->random(3) + 4;
                else if (p2DEvents[window_SpeakInHouse->wData.val - 1].uType == BuildingType_MirroredPath)
                    randomnum = grng->random(2) + 7;
                if (p2DEvents[window_SpeakInHouse->wData.val - 1].uType <= BuildingType_MirroredPath)
                    pItemNum = ITEM_TYPE(grng->random(word_4F0F30[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE]) +
                        11 * randomnum +
                                         400);
            }
        }

        if (pItemNum == ITEM_SPELLBOOK_DIVINE_INTERVENTION) {
            if (!_449B57_test_bit(pParty->_quest_bits, QBIT_DIVINE_INTERVENTION_RETRIEVED))
                pItemNum = ITEM_SPELLBOOK_SUNRAY;
        }

        ItemGen *item_spellbook = &pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][i];
        item_spellbook->Reset();
        pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][i].uItemID = pItemNum;
        pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][i].SetIdentified();

        shop_ui_items_in_store[i] = assets->GetImage_ColorKey(pItemTable->pItems[pItemNum].pIconName);
    }
}
