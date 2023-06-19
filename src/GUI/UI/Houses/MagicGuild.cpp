#include "GUI/UI/Houses/MagicGuild.h"

#include <string>

#include "Engine/Engine.h"
#include "Engine/Tables/BuildingTable.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Items.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/Houses/Shops.h"

#include "Io/Mouse.h"

#include "Library/Random/Random.h"

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

IndexedArray<DIALOGUE_TYPE, BuildingType_FireGuild, BuildingType_DarkGuild> learnableMagicSkillDialogue = {
    {BuildingType_FireGuild,   DIALOGUE_LEARN_FIRE},
    {BuildingType_AirGuild,    DIALOGUE_LEARN_AIR},
    {BuildingType_WaterGuild,  DIALOGUE_LEARN_WATER},
    {BuildingType_EarthGuild,  DIALOGUE_LEARN_EARTH},
    {BuildingType_SpiritGuild, DIALOGUE_LEARN_SPIRIT},
    {BuildingType_MindGuild,   DIALOGUE_LEARN_MIND},
    {BuildingType_BodyGuild,   DIALOGUE_LEARN_BODY},
    {BuildingType_LightGuild,  DIALOGUE_LEARN_LIGHT},
    {BuildingType_DarkGuild,   DIALOGUE_LEARN_DARK}
};

IndexedArray<PLAYER_SKILL_MASTERY, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildSpellsMastery = {
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

// Values are in fact indexes for Player::_achievedAwardsBits
IndexedArray<int, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildMembershipFlags = {
    {HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE,   54},
    {HOUSE_FIRE_GUILD_ADEPT_HARMONDALE,        54},
    {HOUSE_FIRE_GUILD_MASTER_TULAREAN_FOREST,  54},
    {HOUSE_FIRE_GUILD_PARAMOUNT_NIGHON,        54},
    {HOUSE_AIR_GUILD_INITIATE_EMERALD_ISLE,    52},
    {HOUSE_AIR_GUILD_ADEPT_HARMONDALE,         52},
    {HOUSE_AIR_GUILD_MASTER_TULAREAN_FOREST,   52},
    {HOUSE_AIR_GUILD_PARAMOUNT_CELESTE,        52},
    {HOUSE_WATER_GUILD_INITIATE_HARMONDALE,    55},
    {HOUSE_WATER_GUILD_ADEPT_TULAREAN_FOREST,  55},
    {HOUSE_WATER_GUILD_MASTER_BRACADA_DESERT,  55},
    {HOUSE_WATER_GUILD_PARAMOUNT_EVENMORN,     55},
    {HOUSE_EARTH_GUILD_INITIATE_HARMONDALE,    53},
    {HOUSE_EARTH_GUILD_ADEPT_TULAREAN_FOREST,  53},
    {HOUSE_EARTH_GUILD_MASTER_STONE_CITY,      53},
    {HOUSE_EARTH_GUILD_PARAMOUNT_PIT,          53},
    {HOUSE_SPIRIT_GUILD_INITIATE_EMERALD_ISLE, 58},
    {HOUSE_SPIRIT_GUILD_ADEPT_HARMONDALE,      58},
    {HOUSE_SPIRIT_GUILD_MASTER_DEYJA,          58},
    {HOUSE_SPIRIT_GUILD_PARAMOUNT_ERATHIA,     58},
    {HOUSE_MIND_GUILD_INITIATE_HARMONDALE,     57},
    {HOUSE_MIND_GUILD_ADEPT_ERATHIA,           57},
    {HOUSE_MIND_GUILD_MASTER_TATALIA,          57},
    {HOUSE_MIND_GUILD_PARAMOUNT_AVLEE,         57},
    {HOUSE_BODY_GUILD_INITIATE_EMERALD_ISLE,   56},
    {HOUSE_BODY_GUILD_ADEPT_HARMONDALE,        56},
    {HOUSE_BODY_GUILD_MASTER_ERATHIA,          56},
    {HOUSE_BODY_GUILD_PARAMOUNT_AVLEE,         56},
    {HOUSE_LIGHT_GUILD_ADEPT_BRACADA_DESERT,   59},
    {HOUSE_LIGHT_GUILD_PARAMOUNT_CELESTE,      59},
    {HOUSE_DARK_GUILD_ADEPT_DEYJA,             60},
    {HOUSE_DARK_GUILD_PARAMOUNT_PIT,           60}
};

void GUIWindow_MagicGuild::mainDialogue() {
    GUIWindow working_window = *this;
    working_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    working_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    working_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (!pParty->activeCharacter()._achievedAwardsBits[guildMembershipFlags[houseId()]]) {
        // you must be a member
        int textHeight = pFontArrus->CalcTextHeight(pNPCTopics[121].pText, working_window.uFrameWidth, 0);
        working_window.DrawTitleText(pFontArrus, 0, (212 - textHeight) / 2 + 101, colorTable.PaleCanary, pNPCTopics[121].pText, 3);
        pDialogueWindow->pNumPresenceButton = 0;
        return;
    }

    if (!checkIfPlayerCanInteract()) {
        return;
    }

    std::vector<std::string> optionsText;

    bool haveLearnableSkills = false;
    int buttonsLimit = pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton;
    for (int i = pDialogueWindow->pStartingPosActiveItem; i < buttonsLimit; ++i) {
        if (pDialogueWindow->GetControl(i)->msg_param == DIALOGUE_GUILD_BUY_BOOKS) {
            optionsText.push_back(localization->GetString(LSTR_BUY_SPELLS));
        } else {
            PLAYER_SKILL_TYPE skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
            if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE &&
                !pParty->activeCharacter().pActiveSkills[skill]) {
                optionsText.push_back(localization->GetSkillName(skill));
                haveLearnableSkills = true;
            } else {
                optionsText.push_back("");
            }
        }
    }

    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);

    if (haveLearnableSkills) {
        std::string skill_price_label = localization->FormatString(LSTR_FMT_SKILL_COST_D, pPrice);
        working_window.DrawTitleText(pFontArrus, 0, 146, Color(), skill_price_label, 3);
    }

    drawOptions(optionsText, colorTable.Sunflower, 24);
}

void GUIWindow_MagicGuild::buyBooksDialogue() {
    GUIWindow working_window = *this;
    working_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    working_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    working_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
    int itemxind = 0;

    for (int pX = 32; pX < 452; pX += 70) {  // top row
        if (pParty->spellBooksInGuilds[houseId()][itemxind].uItemID != ITEM_NULL) {
            render->DrawTextureNew(pX / 640.0f, 90 / 480.0f, shop_ui_items_in_store[itemxind]);
        }
        if (pParty->spellBooksInGuilds[houseId()][itemxind + 6].uItemID != ITEM_NULL) {
            render->DrawTextureNew(pX / 640.0f, 250 / 480.0f, shop_ui_items_in_store[itemxind + 6]);
        }

        ++itemxind;
    }

    if (checkIfPlayerCanInteract()) {
        int itemcount = 0;
        for (int i = 0; i < itemAmountInShop[buildingType()]; ++i) {
            if (pParty->spellBooksInGuilds[houseId()][i].uItemID != ITEM_NULL)
                ++itemcount;
        }

        GameUI_StatusBar_DrawImmediate(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), Color());

        if (!itemcount) {  // shop empty
            GameTime nextGenTime = pParty->PartyTimes.guildNextRefreshTime[houseId()];
            working_window.DrawShops_next_generation_time_string(nextGenTime - pParty->GetPlayingTime());
            return;
        }

        Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();
        int testx = (pt.x - 32) / 70;
        if (testx >= 0 && testx < 6) {
            if (pt.y >= 250) {
                testx += 6;
            }

            ItemGen *item = &pParty->spellBooksInGuilds[houseId()][testx];

            if (item->uItemID != ITEM_NULL) {
                int testpos;
                if (pt.y >= 250) {
                    testpos = 32 + 70 * testx - 420;
                } else {
                    testpos = 32 + 70 * testx;
                }

                if (pt.x >= testpos && pt.x <= testpos + shop_ui_items_in_store[testx]->width()) {
                    if ((pt.y >= 90 && pt.y <= (90 + shop_ui_items_in_store[testx]->height())) || (pt.y >= 250 && pt.y <= (250 + shop_ui_items_in_store[testx]->height()))) {
                        MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, BuildingType_MagicShop, std::to_underlying(houseId()), 2);
                        std::string str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, item, std::to_underlying(houseId()), 2);
                        int textHeight = pFontArrus->CalcTextHeight(str, working_window.uFrameWidth, 0);
                        working_window.DrawTitleText(pFontArrus, 0, (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - textHeight) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET, colorTable.White, str, 3);
                        return;
                    }
                }
            }
        }
    }
}

void GUIWindow_MagicGuild::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    if (option == DIALOGUE_GUILD_BUY_BOOKS) {
        if (pParty->PartyTimes.guildNextRefreshTime[houseId()] >= pParty->GetPlayingTime()) {
            for (int i = 0; i < itemAmountInShop[buildingType()]; ++i) {
                if (pParty->spellBooksInGuilds[houseId()][i].uItemID != ITEM_NULL)
                    shop_ui_items_in_store[i] = assets->getImage_ColorKey(pParty->spellBooksInGuilds[houseId()][i].GetIconName());
            }
        } else {
            GameTime nextGenTime = pParty->GetPlayingTime() + GameTime::FromDays(buildingTable[wData.val - 1].generation_interval_days);
            generateSpellBooksForGuild();
            pParty->PartyTimes.guildNextRefreshTime[houseId()] = nextGenTime;
        }
    } else if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

void GUIWindow_MagicGuild::houseSpecificDialogue() {
    switch (dialog_menu_id) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_GUILD_BUY_BOOKS:
        buyBooksDialogue();
        break;
      default:
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
}

std::vector<DIALOGUE_TYPE> GUIWindow_MagicGuild::listDialogueOptions(DIALOGUE_TYPE option) {
    BuildingType guildType = buildingType();

    switch (option) {
      case DIALOGUE_MAIN:
        if (guildType == BuildingType_LightGuild || guildType == BuildingType_DarkGuild) {
            return {DIALOGUE_GUILD_BUY_BOOKS, learnableMagicSkillDialogue[guildType]};
        } else {
            return {DIALOGUE_GUILD_BUY_BOOKS, learnableMagicSkillDialogue[guildType], DIALOGUE_LEARN_MEDITATION};
        }
      default:
        return {};
    }
}

void GUIWindow_MagicGuild::houseScreenClick() {
    if (!checkIfPlayerCanInteract()) {
        pAudioPlayer->playUISound(SOUND_error);
        return;
    }

    Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

    int testx = (pt.x - 32) / 70;
    if (testx >= 0 && testx < 6) {
        if (pt.y >= 250) {
            testx += 6;
        }

        ItemGen &boughtItem = pParty->spellBooksInGuilds[houseId()][testx];
        if (boughtItem.uItemID != ITEM_NULL) {
            int testpos;
            if (pt.y >= 250) {
                testpos = 32 + 70 * testx - 420;
            } else {
                testpos = 32 + 70 * testx;
            }

            if (pt.x >= testpos && pt.x <= testpos + shop_ui_items_in_store[testx]->width()) {
                if ((pt.y >= 90 && pt.y <= (90 + shop_ui_items_in_store[testx]->height())) ||
                    (pt.y >= 250 && pt.y <= (250 + shop_ui_items_in_store[testx]->height()))) {
                    float fPriceMultiplier = buildingTable[wData.val - 1].fPriceMultiplier;
                    int uPriceItemService = PriceCalculator::itemBuyingPriceForPlayer(&pParty->activeCharacter(), boughtItem.GetValue(), fPriceMultiplier);

                    if (pParty->GetGold() < uPriceItemService) {
                        playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                        return;
                    }

                    int itemSlot = pParty->activeCharacter().AddItem(-1, boughtItem. uItemID);
                    if (itemSlot) {
                        boughtItem.SetIdentified();
                        pParty->activeCharacter().pInventoryItemList[itemSlot - 1] = boughtItem;
                        _transactionPerformed = true;
                        pParty->TakeGold(uPriceItemService);
                        boughtItem.Reset();
                        render->ClearZBuffer();
                        pParty->activeCharacter().playReaction(SPEECH_ItemBuy);
                        return;
                    }

                    pParty->activeCharacter().playReaction(SPEECH_NoRoom);
                    GameUI_SetStatusBar(LSTR_INVENTORY_IS_FULL);
                }
            }
        }
    }
}

void GUIWindow_MagicGuild::generateSpellBooksForGuild() {
    BuildingType guildType = buildingType();

    // Combined guilds exist only in MM6/MM8 and need to be processed separately
    assert(guildType >= BuildingType_FireGuild && guildType <= BuildingType_DarkGuild);

    DAMAGE_TYPE schoolType = guildSpellsSchool[guildType];
    PLAYER_SKILL_MASTERY maxMastery = guildSpellsMastery[houseId()];
    Segment<ITEM_TYPE> spellbooksForGuild = spellbooksOfSchool(schoolType, maxMastery);

    for (int i = 0; i < itemAmountInShop[guildType]; ++i) {
        ITEM_TYPE pItemNum = grng->randomSample(spellbooksForGuild);

        if (pItemNum == ITEM_SPELLBOOK_DIVINE_INTERVENTION) {
            if (!pParty->_questBits[QBIT_DIVINE_INTERVENTION_RETRIEVED]) {
                pItemNum = ITEM_SPELLBOOK_SUNRAY;
            }
        }

        ItemGen *itemSpellbook = &pParty->spellBooksInGuilds[houseId()][i];
        itemSpellbook->Reset();
        itemSpellbook->uItemID = pItemNum;
        itemSpellbook->SetIdentified();

        shop_ui_items_in_store[i] = assets->getImage_ColorKey(pItemTable->pItems[pItemNum].iconName);
    }
}
