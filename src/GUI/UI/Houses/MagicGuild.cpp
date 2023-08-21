#include "GUI/UI/Houses/MagicGuild.h"

#include <string>

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Tables/BuildingTable.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Items.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/AssetsManager.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/Houses/Shops.h"

#include "Media/Audio/AudioPlayer.h"

#include "Io/Mouse.h"

#include "Library/Random/Random.h"

static constexpr IndexedArray<DAMAGE_TYPE, BUILDING_FIRE_GUILD, BUILDING_DARK_GUILD> guildSpellsSchool = {
    {BUILDING_FIRE_GUILD,   DMGT_FIRE},
    {BUILDING_AIR_GUILD,    DMGT_ELECTR},
    {BUILDING_WATER_GUILD,  DMGT_COLD},
    {BUILDING_EARTH_GUILD,  DMGT_EARTH},
    {BUILDING_SPIRIT_GUILD, DMGT_SPIRIT},
    {BUILDING_MIND_GUILD,   DMGT_MIND},
    {BUILDING_BODY_GUILD,   DMGT_BODY},
    {BUILDING_LIGHT_GUILD,  DMGT_LIGHT},
    {BUILDING_DARK_GUILD,   DMGT_DARK}
};

static constexpr IndexedArray<DIALOGUE_TYPE, BUILDING_FIRE_GUILD, BUILDING_DARK_GUILD> learnableMagicSkillDialogue = {
    {BUILDING_FIRE_GUILD,   DIALOGUE_LEARN_FIRE},
    {BUILDING_AIR_GUILD,    DIALOGUE_LEARN_AIR},
    {BUILDING_WATER_GUILD,  DIALOGUE_LEARN_WATER},
    {BUILDING_EARTH_GUILD,  DIALOGUE_LEARN_EARTH},
    {BUILDING_SPIRIT_GUILD, DIALOGUE_LEARN_SPIRIT},
    {BUILDING_MIND_GUILD,   DIALOGUE_LEARN_MIND},
    {BUILDING_BODY_GUILD,   DIALOGUE_LEARN_BODY},
    {BUILDING_LIGHT_GUILD,  DIALOGUE_LEARN_LIGHT},
    {BUILDING_DARK_GUILD,   DIALOGUE_LEARN_DARK}
};

static constexpr IndexedArray<DIALOGUE_TYPE, BUILDING_FIRE_GUILD, BUILDING_DARK_GUILD> learnableAdditionalSkillDialogue = {
    {BUILDING_FIRE_GUILD,   DIALOGUE_LEARN_LEARNING},
    {BUILDING_AIR_GUILD,    DIALOGUE_LEARN_LEARNING},
    {BUILDING_WATER_GUILD,  DIALOGUE_LEARN_LEARNING},
    {BUILDING_EARTH_GUILD,  DIALOGUE_LEARN_LEARNING},
    {BUILDING_SPIRIT_GUILD, DIALOGUE_LEARN_MEDITATION},
    {BUILDING_MIND_GUILD,   DIALOGUE_LEARN_MEDITATION},
    {BUILDING_BODY_GUILD,   DIALOGUE_LEARN_MEDITATION},
    {BUILDING_LIGHT_GUILD,  DIALOGUE_NULL},
    {BUILDING_DARK_GUILD,   DIALOGUE_NULL}
};

static constexpr IndexedArray<CharacterSkillMastery, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildSpellsMastery = {
    {HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE,   CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_FIRE_GUILD_ADEPT_HARMONDALE,        CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_FIRE_GUILD_MASTER_TULAREAN_FOREST,  CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_FIRE_GUILD_PARAMOUNT_NIGHON,        CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_AIR_GUILD_INITIATE_EMERALD_ISLE,    CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_AIR_GUILD_ADEPT_HARMONDALE,         CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_AIR_GUILD_MASTER_TULAREAN_FOREST,   CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_AIR_GUILD_PARAMOUNT_CELESTE,        CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_WATER_GUILD_INITIATE_HARMONDALE,    CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_WATER_GUILD_ADEPT_TULAREAN_FOREST,  CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_WATER_GUILD_MASTER_BRACADA_DESERT,  CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_WATER_GUILD_PARAMOUNT_EVENMORN,     CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_EARTH_GUILD_INITIATE_HARMONDALE,    CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_EARTH_GUILD_ADEPT_TULAREAN_FOREST,  CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_EARTH_GUILD_MASTER_STONE_CITY,      CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_EARTH_GUILD_PARAMOUNT_PIT,          CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_SPIRIT_GUILD_INITIATE_EMERALD_ISLE, CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_SPIRIT_GUILD_ADEPT_HARMONDALE,      CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_SPIRIT_GUILD_MASTER_DEYJA,          CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_SPIRIT_GUILD_PARAMOUNT_ERATHIA,     CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_MIND_GUILD_INITIATE_HARMONDALE,     CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_MIND_GUILD_ADEPT_ERATHIA,           CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_MIND_GUILD_MASTER_TATALIA,          CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_MIND_GUILD_PARAMOUNT_AVLEE,         CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_BODY_GUILD_INITIATE_EMERALD_ISLE,   CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_BODY_GUILD_ADEPT_HARMONDALE,        CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_BODY_GUILD_MASTER_ERATHIA,          CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_BODY_GUILD_PARAMOUNT_AVLEE,         CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_LIGHT_GUILD_ADEPT_BRACADA_DESERT,   CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_LIGHT_GUILD_PARAMOUNT_CELESTE,      CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_DARK_GUILD_ADEPT_DEYJA,             CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_DARK_GUILD_PARAMOUNT_PIT,           CHARACTER_SKILL_MASTERY_GRANDMASTER}
};

// Values are in fact indexes for Character::_achievedAwardsBits
const IndexedArray<int, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildMembershipFlags = {
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
            CharacterSkillType skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
            if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != CHARACTER_SKILL_MASTERY_NONE &&
                !pParty->activeCharacter().pActiveSkills[skill]) {
                optionsText.push_back(localization->GetSkillName(skill));
                haveLearnableSkills = true;
            } else {
                optionsText.push_back("");
            }
        }
    }

    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[houseId()]);

    if (haveLearnableSkills) {
        std::string skill_price_label = localization->FormatString(LSTR_FMT_SKILL_COST_D, pPrice);
        working_window.DrawTitleText(pFontArrus, 0, 146, colorTable.White, skill_price_label, 3);
    }

    drawOptions(optionsText, colorTable.PaleCanary, 24);
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

        engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), colorTable.White);

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
                        MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, BUILDING_MAGIC_SHOP, houseId(), 2);
                        std::string str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, item, houseId(), 2);
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
    _currentDialogue = option;
    if (option == DIALOGUE_GUILD_BUY_BOOKS) {
        if (pParty->PartyTimes.guildNextRefreshTime[houseId()] >= pParty->GetPlayingTime()) {
            for (int i = 0; i < itemAmountInShop[buildingType()]; ++i) {
                if (pParty->spellBooksInGuilds[houseId()][i].uItemID != ITEM_NULL)
                    shop_ui_items_in_store[i] = assets->getImage_ColorKey(pParty->spellBooksInGuilds[houseId()][i].GetIconName());
            }
        } else {
            GameTime nextGenTime = pParty->GetPlayingTime() + GameTime::FromDays(buildingTable[houseId()].generation_interval_days);
            generateSpellBooksForGuild();
            pParty->PartyTimes.guildNextRefreshTime[houseId()] = nextGenTime;
        }
    } else if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

void GUIWindow_MagicGuild::houseSpecificDialogue() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_GUILD_BUY_BOOKS:
        buyBooksDialogue();
        break;
      default:
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        break;
    }
}

std::vector<DIALOGUE_TYPE> GUIWindow_MagicGuild::listDialogueOptions() {
    BuildingType guildType = buildingType();

    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        if (learnableAdditionalSkillDialogue[guildType] != DIALOGUE_NULL) {
            return {DIALOGUE_GUILD_BUY_BOOKS, learnableMagicSkillDialogue[guildType], learnableAdditionalSkillDialogue[guildType]};
        } else {
            return {DIALOGUE_GUILD_BUY_BOOKS, learnableMagicSkillDialogue[guildType]};
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
                    float fPriceMultiplier = buildingTable[houseId()].fPriceMultiplier;
                    int uPriceItemService = PriceCalculator::itemBuyingPriceForPlayer(&pParty->activeCharacter(), boughtItem.GetValue(), fPriceMultiplier);

                    if (pParty->GetGold() < uPriceItemService) {
                        playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                        engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
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
                        pParty->activeCharacter().playReaction(SPEECH_ITEM_BUY);
                        return;
                    }

                    pParty->activeCharacter().playReaction(SPEECH_NO_ROOM);
                    engine->_statusBar->setEvent(LSTR_INVENTORY_IS_FULL);
                }
            }
        }
    }
}

void GUIWindow_MagicGuild::generateSpellBooksForGuild() {
    BuildingType guildType = buildingType();

    // Combined guilds exist only in MM6/MM8 and need to be processed separately
    assert(guildType >= BUILDING_FIRE_GUILD && guildType <= BUILDING_DARK_GUILD);

    DAMAGE_TYPE schoolType = guildSpellsSchool[guildType];
    CharacterSkillMastery maxMastery = guildSpellsMastery[houseId()];
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
