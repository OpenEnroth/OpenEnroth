#include "MagicGuild.h"

#include <string>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Item.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/AssetsManager.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/Houses/Shops.h"

#include "Media/Audio/AudioPlayer.h"

#include "Io/Mouse.h"

#include "Engine/Random/Random.h"

static constexpr IndexedArray<MagicSchool, HOUSE_TYPE_FIRE_GUILD, HOUSE_TYPE_DARK_GUILD> guildSpellsSchool = {
    {HOUSE_TYPE_FIRE_GUILD,   MAGIC_SCHOOL_FIRE},
    {HOUSE_TYPE_AIR_GUILD,    MAGIC_SCHOOL_AIR},
    {HOUSE_TYPE_WATER_GUILD,  MAGIC_SCHOOL_WATER},
    {HOUSE_TYPE_EARTH_GUILD,  MAGIC_SCHOOL_EARTH},
    {HOUSE_TYPE_SPIRIT_GUILD, MAGIC_SCHOOL_SPIRIT},
    {HOUSE_TYPE_MIND_GUILD,   MAGIC_SCHOOL_MIND},
    {HOUSE_TYPE_BODY_GUILD,   MAGIC_SCHOOL_BODY},
    {HOUSE_TYPE_LIGHT_GUILD,  MAGIC_SCHOOL_LIGHT},
    {HOUSE_TYPE_DARK_GUILD,   MAGIC_SCHOOL_DARK}
};

static constexpr IndexedArray<DialogueId, HOUSE_TYPE_FIRE_GUILD, HOUSE_TYPE_DARK_GUILD> learnableMagicSkillDialogue = {
    {HOUSE_TYPE_FIRE_GUILD,   DIALOGUE_LEARN_FIRE},
    {HOUSE_TYPE_AIR_GUILD,    DIALOGUE_LEARN_AIR},
    {HOUSE_TYPE_WATER_GUILD,  DIALOGUE_LEARN_WATER},
    {HOUSE_TYPE_EARTH_GUILD,  DIALOGUE_LEARN_EARTH},
    {HOUSE_TYPE_SPIRIT_GUILD, DIALOGUE_LEARN_SPIRIT},
    {HOUSE_TYPE_MIND_GUILD,   DIALOGUE_LEARN_MIND},
    {HOUSE_TYPE_BODY_GUILD,   DIALOGUE_LEARN_BODY},
    {HOUSE_TYPE_LIGHT_GUILD,  DIALOGUE_LEARN_LIGHT},
    {HOUSE_TYPE_DARK_GUILD,   DIALOGUE_LEARN_DARK}
};

static constexpr IndexedArray<DialogueId, HOUSE_TYPE_FIRE_GUILD, HOUSE_TYPE_DARK_GUILD> learnableAdditionalSkillDialogue = {
    {HOUSE_TYPE_FIRE_GUILD,   DIALOGUE_LEARN_LEARNING},
    {HOUSE_TYPE_AIR_GUILD,    DIALOGUE_LEARN_LEARNING},
    {HOUSE_TYPE_WATER_GUILD,  DIALOGUE_LEARN_LEARNING},
    {HOUSE_TYPE_EARTH_GUILD,  DIALOGUE_LEARN_LEARNING},
    {HOUSE_TYPE_SPIRIT_GUILD, DIALOGUE_LEARN_MEDITATION},
    {HOUSE_TYPE_MIND_GUILD,   DIALOGUE_LEARN_MEDITATION},
    {HOUSE_TYPE_BODY_GUILD,   DIALOGUE_LEARN_MEDITATION},
    {HOUSE_TYPE_LIGHT_GUILD,  DIALOGUE_NULL},
    {HOUSE_TYPE_DARK_GUILD,   DIALOGUE_NULL}
};

static constexpr IndexedArray<CharacterSkillMastery, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildSpellsMastery = {
    {HOUSE_FIRE_GUILD_EMERALD_ISLAND,   CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_FIRE_GUILD_HARMONDALE,       CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_FIRE_GUILD_TULAREAN_FOREST,  CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_FIRE_GUILD_MOUNT_NIGHON,     CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_AIR_GUILD_EMERALD_ISLAND,    CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_AIR_GUILD_HARMONDALE,        CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_AIR_GUILD_TULAREAN_FOREST,   CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_AIR_GUILD_CELESTE,           CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_WATER_GUILD_HARMONDALE,      CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_WATER_GUILD_TULAREAN_FOREST, CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_WATER_GUILD_BRACADA_DESERT,  CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_WATER_GUILD_EVENMORN_ISLAND, CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_EARTH_GUILD_HARMONDALE,      CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_EARTH_GUILD_TULAREAN_FOREST, CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_EARTH_GUILD_STONE_CITY,      CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_EARTH_GUILD_PIT,             CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_SPIRIT_GUILD_EMERALD_ISLAND, CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_SPIRIT_GUILD_HARMONDALE,     CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_SPIRIT_GUILD_DEYJA,          CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_SPIRIT_GUILD_ERATHIA,        CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_MIND_GUILD_HARMONDALE,       CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_MIND_GUILD_ERATHIA,          CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_MIND_GUILD_TATALIA,          CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_MIND_GUILD_AVLEE,            CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_BODY_GUILD_EMERALD_ISLAND,   CHARACTER_SKILL_MASTERY_NOVICE},
    {HOUSE_BODY_GUILD_HARMONDALE,       CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_BODY_GUILD_ERATHIA,          CHARACTER_SKILL_MASTERY_MASTER},
    {HOUSE_BODY_GUILD_AVLEE,            CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_LIGHT_GUILD_BRACADA_DESERT,  CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_LIGHT_GUILD_CELESTE,         CHARACTER_SKILL_MASTERY_GRANDMASTER},
    {HOUSE_DARK_GUILD_DEYJA,            CHARACTER_SKILL_MASTERY_EXPERT},
    {HOUSE_DARK_GUILD_PIT,              CHARACTER_SKILL_MASTERY_GRANDMASTER}
};

// Values are in fact indexes for Character::_achievedAwardsBits
const IndexedArray<int, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildMembershipFlags = {
    {HOUSE_FIRE_GUILD_EMERALD_ISLAND,   54},
    {HOUSE_FIRE_GUILD_HARMONDALE,       54},
    {HOUSE_FIRE_GUILD_TULAREAN_FOREST,  54},
    {HOUSE_FIRE_GUILD_MOUNT_NIGHON,     54},
    {HOUSE_AIR_GUILD_EMERALD_ISLAND,    52},
    {HOUSE_AIR_GUILD_HARMONDALE,        52},
    {HOUSE_AIR_GUILD_TULAREAN_FOREST,   52},
    {HOUSE_AIR_GUILD_CELESTE,           52},
    {HOUSE_WATER_GUILD_HARMONDALE,      55},
    {HOUSE_WATER_GUILD_TULAREAN_FOREST, 55},
    {HOUSE_WATER_GUILD_BRACADA_DESERT,  55},
    {HOUSE_WATER_GUILD_EVENMORN_ISLAND, 55},
    {HOUSE_EARTH_GUILD_HARMONDALE,      53},
    {HOUSE_EARTH_GUILD_TULAREAN_FOREST, 53},
    {HOUSE_EARTH_GUILD_STONE_CITY,      53},
    {HOUSE_EARTH_GUILD_PIT,             53},
    {HOUSE_SPIRIT_GUILD_EMERALD_ISLAND, 58},
    {HOUSE_SPIRIT_GUILD_HARMONDALE,     58},
    {HOUSE_SPIRIT_GUILD_DEYJA,          58},
    {HOUSE_SPIRIT_GUILD_ERATHIA,        58},
    {HOUSE_MIND_GUILD_HARMONDALE,       57},
    {HOUSE_MIND_GUILD_ERATHIA,          57},
    {HOUSE_MIND_GUILD_TATALIA,          57},
    {HOUSE_MIND_GUILD_AVLEE,            57},
    {HOUSE_BODY_GUILD_EMERALD_ISLAND,   56},
    {HOUSE_BODY_GUILD_HARMONDALE,       56},
    {HOUSE_BODY_GUILD_ERATHIA,          56},
    {HOUSE_BODY_GUILD_AVLEE,            56},
    {HOUSE_LIGHT_GUILD_BRACADA_DESERT,  59},
    {HOUSE_LIGHT_GUILD_CELESTE,         59},
    {HOUSE_DARK_GUILD_DEYJA,            60},
    {HOUSE_DARK_GUILD_PIT,              60}
};

void GUIWindow_MagicGuild::mainDialogue() {
    GUIWindow working_window = *this;
    working_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    working_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    working_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (!pParty->activeCharacter()._achievedAwardsBits[guildMembershipFlags[houseId()]]) {
        // you must be a member
        int textHeight = assets->pFontArrus->CalcTextHeight(pNPCTopics[121].pText, working_window.uFrameWidth, 0);
        working_window.DrawTitleText(assets->pFontArrus.get(), 0, (212 - textHeight) / 2 + 101, colorTable.PaleCanary, pNPCTopics[121].pText, 3);
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
        if (pDialogueWindow->GetControl(i)->msg_param == std::to_underlying(DIALOGUE_GUILD_BUY_BOOKS)) {
            optionsText.push_back(localization->GetString(LSTR_BUY_SPELLS));
        } else {
            CharacterSkillType skill = GetLearningDialogueSkill((DialogueId)pDialogueWindow->GetControl(i)->msg_param);
            if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != CHARACTER_SKILL_MASTERY_NONE &&
                !pParty->activeCharacter().pActiveSkills[skill]) {
                optionsText.push_back(localization->GetSkillName(skill));
                haveLearnableSkills = true;
            } else {
                optionsText.push_back("");
            }
        }
    }

    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);

    if (haveLearnableSkills) {
        std::string skill_price_label = localization->FormatString(LSTR_SKILL_COST_LU, pPrice);
        working_window.DrawTitleText(assets->pFontArrus.get(), 0, 146, colorTable.White, skill_price_label, 3);
    }

    drawOptions(optionsText, colorTable.PaleCanary, 24);
}

void GUIWindow_MagicGuild::buyBooksDialogue() {
    // TODO(pskelton): Extract common item picking code
    GUIWindow working_window = *this;
    working_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    working_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    working_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
    int itemxind = 0;

    for (int pX = 32; pX < 452; pX += 70) {  // top row
        if (pParty->spellBooksInGuilds[houseId()][itemxind].itemId != ITEM_NULL) {
            render->DrawTextureNew(pX / 640.0f, 90 / 480.0f, shop_ui_items_in_store[itemxind]);
        }
        if (pParty->spellBooksInGuilds[houseId()][itemxind + 6].itemId != ITEM_NULL) {
            render->DrawTextureNew(pX / 640.0f, 250 / 480.0f, shop_ui_items_in_store[itemxind + 6]);
        }

        ++itemxind;
    }

    if (checkIfPlayerCanInteract()) {
        int itemcount = 0;
        for (int i = 0; i < itemAmountInShop[buildingType()]; ++i) {
            if (pParty->spellBooksInGuilds[houseId()][i].itemId != ITEM_NULL)
                ++itemcount;
        }

        engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_THE_ITEM_TO_BUY), colorTable.White);

        if (!itemcount) {  // shop empty
            Time nextGenTime = pParty->PartyTimes.guildNextRefreshTime[houseId()];
            working_window.DrawShops_next_generation_time_string(nextGenTime - pParty->GetPlayingTime());
            return;
        }

        Pointi pt = EngineIocContainer::ResolveMouse()->position();
        int testx = (pt.x - 32) / 70;
        if (testx >= 0 && testx < 6) {
            if (pt.y >= 250) {
                testx += 6;
            }

            Item *item = &pParty->spellBooksInGuilds[houseId()][testx];

            if (item->itemId != ITEM_NULL) {
                int testpos;
                if (pt.y >= 250) {
                    testpos = 32 + 70 * testx - 420;
                } else {
                    testpos = 32 + 70 * testx;
                }

                if (pt.x >= testpos && pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                    if ((pt.y >= 90 && pt.y <= (90 + (shop_ui_items_in_store[testx]->height()))) || (pt.y >= 250 && pt.y <= (250 + (shop_ui_items_in_store[testx]->height())))) {
                        MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, HOUSE_TYPE_MAGIC_SHOP, houseId(), SHOP_SCREEN_BUY);
                        std::string str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_BUY);
                        int textHeight = assets->pFontArrus->CalcTextHeight(str, working_window.uFrameWidth, 0);
                        working_window.DrawTitleText(assets->pFontArrus.get(), 0, (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - textHeight) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET, colorTable.White, str, 3);
                        return;
                    }
                }
            }
        }
    }
}

void GUIWindow_MagicGuild::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
    if (option == DIALOGUE_GUILD_BUY_BOOKS) {
        if (pParty->PartyTimes.guildNextRefreshTime[houseId()] >= pParty->GetPlayingTime()) {
            for (int i = 0; i < itemAmountInShop[buildingType()]; ++i) {
                if (pParty->spellBooksInGuilds[houseId()][i].itemId != ITEM_NULL)
                    shop_ui_items_in_store[i] = assets->getImage_ColorKey(pParty->spellBooksInGuilds[houseId()][i].GetIconName());
            }
        } else {
            Time nextGenTime = pParty->GetPlayingTime() + Duration::fromDays(houseTable[houseId()].generation_interval_days);
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

std::vector<DialogueId> GUIWindow_MagicGuild::listDialogueOptions() {
    HouseType guildType = buildingType();

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

    Pointi pt = EngineIocContainer::ResolveMouse()->position();

    int testx = (pt.x - 32) / 70;
    if (testx >= 0 && testx < 6) {
        if (pt.y >= 250) {
            testx += 6;
        }

        Item &boughtItem = pParty->spellBooksInGuilds[houseId()][testx];
        if (boughtItem.itemId != ITEM_NULL) {
            int testpos;
            if (pt.y >= 250) {
                testpos = 32 + 70 * testx - 420;
            } else {
                testpos = 32 + 70 * testx;
            }

            if (pt.x >= testpos && pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                if ((pt.y >= 90 && pt.y <= (90 + (shop_ui_items_in_store[testx]->height()))) ||
                    (pt.y >= 250 && pt.y <= (250 + (shop_ui_items_in_store[testx]->height())))) {
                    float fPriceMultiplier = houseTable[houseId()].fPriceMultiplier;
                    int uPriceItemService = PriceCalculator::itemBuyingPriceForPlayer(&pParty->activeCharacter(), boughtItem.GetValue(), fPriceMultiplier);

                    if (pParty->GetGold() < uPriceItemService) {
                        playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                        engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
                        return;
                    }

                    int itemSlot = pParty->activeCharacter().AddItem(-1, boughtItem. itemId);
                    if (itemSlot) {
                        boughtItem.SetIdentified();
                        pParty->activeCharacter().pInventoryItemList[itemSlot - 1] = boughtItem;
                        _transactionPerformed = true;
                        pParty->TakeGold(uPriceItemService);
                        boughtItem.Reset();
                        pParty->activeCharacter().playReaction(SPEECH_ITEM_BUY);
                        return;
                    }

                    pParty->activeCharacter().playReaction(SPEECH_NO_ROOM);
                    engine->_statusBar->setEvent(LSTR_PACK_IS_FULL);
                }
            }
        }
    }
}

void GUIWindow_MagicGuild::generateSpellBooksForGuild() {
    HouseType guildType = buildingType();

    // Combined guilds exist only in MM6/MM8 and need to be processed separately
    assert(guildType >= HOUSE_TYPE_FIRE_GUILD && guildType <= HOUSE_TYPE_DARK_GUILD);

    MagicSchool schoolType = guildSpellsSchool[guildType];
    CharacterSkillMastery maxMastery = guildSpellsMastery[houseId()];
    Segment<ItemId> spellbooksForGuild = spellbooksForSchool(schoolType, maxMastery);

    for (int i = 0; i < itemAmountInShop[guildType]; ++i) {
        ItemId pItemNum = grng->randomSample(spellbooksForGuild);

        if (pItemNum == ITEM_SPELLBOOK_DIVINE_INTERVENTION) {
            if (!pParty->_questBits[QBIT_DIVINE_INTERVENTION_RETRIEVED]) {
                pItemNum = ITEM_SPELLBOOK_SUNRAY;
            }
        }

        Item *itemSpellbook = &pParty->spellBooksInGuilds[houseId()][i];
        itemSpellbook->Reset();
        itemSpellbook->itemId = pItemNum;
        itemSpellbook->SetIdentified();

        shop_ui_items_in_store[i] = assets->getImage_ColorKey(pItemTable->items[pItemNum].iconName);
    }
}
