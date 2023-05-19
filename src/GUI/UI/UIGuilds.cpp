#include "GUI/UI/UIGuilds.h"

#include <string>

#include "Engine/Engine.h"
#include "Engine/Tables/BuildingTable.h"
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

#include "Utility/IndexedArray.h"

IndexedArray<DAMAGE_TYPE, BuildingType_FireGuild, BuildingType_DarkGuild> guildSpellsSchool = {
    {BuildingType_FireGuild,   DMGT_FIRE},
    {BuildingType_AirGuild,    DMGT_ELECTR},
    {BuildingType_WaterGuild,  DMGT_COLD},
    {BuildingType_EarthGuild,  DMGT_EARTH},
    {BuildingType_SpiritGuild, DMGT_SPIRIT},
    {BuildingType_MindGuild,   DMGT_MIND},
    {BuildingType_BodyGuild,   DMGT_BODY},
    {BuildingType_LightGuild,  DMGT_LIGHT},
    {BuildingType_DarkGuild,   DMGT_DARK}
};

IndexedArray<PLAYER_SKILL_MASTERY, HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE, HOUSE_DARK_GUILD_PARAMOUNT_PIT> guildSpellsMastery = {
    {HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE,   PLAYER_SKILL_MASTERY_NOVICE},
    {HOUSE_FIRE_GUILD_ADEPT_HARMONDALE,        PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_FIRE_GUILD_MASTER_TULAREAN_FOREST,  PLAYER_SKILL_MASTERY_MASTER},
    {HOUSE_FIRE_GUILD_PARAMOUNT_NIGHON,        PLAYER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_AIR_GUILD_INITIATE_EMERALD_ISLE,    PLAYER_SKILL_MASTERY_NOVICE},
    {HOUSE_AIR_GUILD_ADEPT_HARMONDALE,         PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_AIR_GUILD_MASTER_TULAREAN_FOREST,   PLAYER_SKILL_MASTERY_MASTER},
    {HOUSE_AIR_GUILD_PARAMOUNT_CELESTE,        PLAYER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_WATER_GUILD_INITIATE_HARMONDALE,    PLAYER_SKILL_MASTERY_NOVICE},
    {HOUSE_WATER_GUILD_ADEPT_TULAREAN_FOREST,  PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_WATER_GUILD_MASTER_BRACADA_DESERT,  PLAYER_SKILL_MASTERY_MASTER},
    {HOUSE_WATER_GUILD_PARAMOUNT_EVENMORN,     PLAYER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_EARTH_GUILD_INITIATE_HARMONDALE,    PLAYER_SKILL_MASTERY_NOVICE},
    {HOUSE_EARTH_GUILD_ADEPT_TULAREAN_FOREST,  PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_EARTH_GUILD_MASTER_STONE_CITY,      PLAYER_SKILL_MASTERY_MASTER},
    {HOUSE_EARTH_GUILD_PARAMOUNT_PIT,          PLAYER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_SPIRIT_GUILD_INITIATE_EMERALD_ISLE, PLAYER_SKILL_MASTERY_NOVICE},
    {HOUSE_SPIRIT_GUILD_ADEPT_HARMONDALE,      PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_SPIRIT_GUILD_MASTER_DEYJA,          PLAYER_SKILL_MASTERY_MASTER},
    {HOUSE_SPIRIT_GUILD_PARAMOUNT_ERATHIA,     PLAYER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_MIND_GUILD_INITIATE_HARMONDALE,     PLAYER_SKILL_MASTERY_NOVICE},
    {HOUSE_MIND_GUILD_ADEPT_ERATHIA,           PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_MIND_GUILD_MASTER_TATALIA,          PLAYER_SKILL_MASTERY_MASTER},
    {HOUSE_MIND_GUILD_PARAMOUNT_AVLEE,         PLAYER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_BODY_GUILD_INITIATE_EMERALD_ISLE,   PLAYER_SKILL_MASTERY_NOVICE},
    {HOUSE_BODY_GUILD_ADEPT_HARMONDALE,        PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_BODY_GUILD_MASTER_ERATHIA,          PLAYER_SKILL_MASTERY_MASTER},
    {HOUSE_BODY_GUILD_PARAMOUNT_AVLEE,         PLAYER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_LIGHT_GUILD_ADEPT_BRACADA_DESERT,   PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_LIGHT_GUILD_PARAMOUNT_CELESTE,      PLAYER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_DARK_GUILD_ADEPT_DEYJA,             PLAYER_SKILL_MASTERY_EXPERT},
    {HOUSE_DARK_GUILD_PARAMOUNT_PIT,           PLAYER_SKILL_MASTERY_GRANDMASTER}
};

void GuildDialog() {
    int pTextHeight;              // eax@55
    int dialogopts;               // [sp+2E0h] [bp-Ch]@35

    int all_text_height;
    int pX;

    GUIWindow working_window = *window_SpeakInHouse;
    working_window.uFrameX = 483;
    working_window.uFrameWidth = 148;
    working_window.uFrameZ = 334;

    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(),
                                                             buildingTable[window_SpeakInHouse->wData.val - 1]);

    if (dialog_menu_id == DIALOGUE_MAIN) {  // change to switch??
        if (!pParty->activeCharacter()._achievedAwardsBits[guild_membership_flags[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE]]) {
            // you must be a member
            pTextHeight = pFontArrus->CalcTextHeight(pNPCTopics[121].pText, working_window.uFrameWidth, 0);
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
                if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE
                    && !pParty->activeCharacter().pActiveSkills[skill]) {
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
                            std::string str = BuildDialogueString(pMerchantsBuyPhrases[pParty->activeCharacter().SelectPhrasesTransaction(
                                             item, BuildingType_MagicShop, window_SpeakInHouse->wData.val, 2)],
                                pParty->activeCharacterIndex() - 1, item, window_SpeakInHouse->wData.val, 2);
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

void generateSpellBooksForGuild() {
    BuildingType guildType = buildingTable[window_SpeakInHouse->wData.val - 1].uType;

    // Combined guilds exist only in MM6/MM8 and need to be processed separately
    assert(guildType >= BuildingType_FireGuild && guildType <= BuildingType_DarkGuild);

    DAMAGE_TYPE schoolType = guildSpellsSchool[guildType];
    PLAYER_SKILL_MASTERY maxMastery = guildSpellsMastery[HOUSE_ID(window_SpeakInHouse->wData.val)];
    Segment<ITEM_TYPE> spellsForGuild = spellsOfSchool(schoolType, maxMastery);

    for (int i = 0; i < 12; ++i) {
        ITEM_TYPE pItemNum = grng->randomSample(spellsForGuild);

        if (pItemNum == ITEM_SPELLBOOK_DIVINE_INTERVENTION) {
            if (!pParty->_questBits[QBIT_DIVINE_INTERVENTION_RETRIEVED]) {
                pItemNum = ITEM_SPELLBOOK_SUNRAY;
            }
        }

        ItemGen *itemSpellbook = &pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][i];
        itemSpellbook->Reset();
        itemSpellbook->uItemID = pItemNum;
        itemSpellbook->SetIdentified();

        shop_ui_items_in_store[i] = assets->getImage_ColorKey(pItemTable->pItems[pItemNum].iconName);
    }
}
