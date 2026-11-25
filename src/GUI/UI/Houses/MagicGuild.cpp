#include "MagicGuild.h"

#include <array>
#include <string>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/GameType.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/Item.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/AssetsManager.h"
#include "Engine/Data/HouseEnumFunctions.h"

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

// "combined guilds exist only in MM6/MM8" (see `generateSpellBooksForGuild()` below) -
// HOUSE_TYPE_ELEMENTAL_GUILD teaches all 4 elemental schools, HOUSE_TYPE_SELF_GUILD all 5 self
// schools, matching the split MM6/7/8 already use elsewhere for these exact categories (see
// `GuildId::GUILD_OF_ELEMENTS`/`GUILD_OF_SELF` and their own `AWARD_MEMBERSHIP_ELEMENTAL_GUILD`/
// `AWARD_MEMBERSHIP_SELF_GUILD`, both already wired up for the NPC-dialogue guild-membership flow
// in NPCTopics.cpp, just never connected to this house-driven screen before).
static constexpr std::array<MagicSchool, 4> elementalGuildSchools = {
    MAGIC_SCHOOL_FIRE, MAGIC_SCHOOL_AIR, MAGIC_SCHOOL_WATER, MAGIC_SCHOOL_EARTH
};
static constexpr std::array<MagicSchool, 5> selfGuildSchools = {
    MAGIC_SCHOOL_SPIRIT, MAGIC_SCHOOL_MIND, MAGIC_SCHOOL_BODY, MAGIC_SCHOOL_LIGHT, MAGIC_SCHOOL_DARK
};
// MM8 sells Light and Dark only in its Guild of Light and Guild of Dark.
static constexpr std::array<MagicSchool, 3> mm8SelfGuildSchools = {
    MAGIC_SCHOOL_SPIRIT, MAGIC_SCHOOL_MIND, MAGIC_SCHOOL_BODY
};

/**
 * MM8's combined guilds come in three tiers, by the highest spell of a school they sell: 4, 7 or 11.
 *
 * @param house                         A combined guild of MM8.
 * @return                              The highest mastery whose books the guild stocks.
 */
static Mastery mm8CombinedGuildMaxMastery(HouseId house) {
    switch (std::to_underlying(house)) {
    case 139: // Cures and Curses, Dagger Wound Island.
    case 141: // Self Study, Alvar.
        return MASTERY_NOVICE;
    case 140: // Vexation's Hexes, Ravenshore.
    case 145: // Guild of Mind, Balthazar Lair.
        return MASTERY_EXPERT;
    default:
        return MASTERY_GRANDMASTER;
    }
}

static constexpr IndexedArray<Mastery, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildSpellsMastery = {
    {HOUSE_FIRE_GUILD_EMERALD_ISLAND,   MASTERY_NOVICE},
    {HOUSE_FIRE_GUILD_HARMONDALE,       MASTERY_EXPERT},
    {HOUSE_FIRE_GUILD_TULAREAN_FOREST,  MASTERY_MASTER},
    {HOUSE_FIRE_GUILD_MOUNT_NIGHON,     MASTERY_GRANDMASTER},
    {HOUSE_AIR_GUILD_EMERALD_ISLAND,    MASTERY_NOVICE},
    {HOUSE_AIR_GUILD_HARMONDALE,        MASTERY_EXPERT},
    {HOUSE_AIR_GUILD_TULAREAN_FOREST,   MASTERY_MASTER},
    {HOUSE_AIR_GUILD_CELESTE,           MASTERY_GRANDMASTER},
    {HOUSE_WATER_GUILD_HARMONDALE,      MASTERY_NOVICE},
    {HOUSE_WATER_GUILD_TULAREAN_FOREST, MASTERY_EXPERT},
    {HOUSE_WATER_GUILD_BRACADA_DESERT,  MASTERY_MASTER},
    {HOUSE_WATER_GUILD_EVENMORN_ISLAND, MASTERY_GRANDMASTER},
    {HOUSE_EARTH_GUILD_HARMONDALE,      MASTERY_NOVICE},
    {HOUSE_EARTH_GUILD_TULAREAN_FOREST, MASTERY_EXPERT},
    {HOUSE_EARTH_GUILD_STONE_CITY,      MASTERY_MASTER},
    {HOUSE_EARTH_GUILD_PIT,             MASTERY_GRANDMASTER},
    {HOUSE_SPIRIT_GUILD_EMERALD_ISLAND, MASTERY_NOVICE},
    {HOUSE_SPIRIT_GUILD_HARMONDALE,     MASTERY_EXPERT},
    {HOUSE_SPIRIT_GUILD_DEYJA,          MASTERY_MASTER},
    {HOUSE_SPIRIT_GUILD_ERATHIA,        MASTERY_GRANDMASTER},
    {HOUSE_MIND_GUILD_HARMONDALE,       MASTERY_NOVICE},
    {HOUSE_MIND_GUILD_ERATHIA,          MASTERY_EXPERT},
    {HOUSE_MIND_GUILD_TATALIA,          MASTERY_MASTER},
    {HOUSE_MIND_GUILD_AVLEE,            MASTERY_GRANDMASTER},
    {HOUSE_BODY_GUILD_EMERALD_ISLAND,   MASTERY_NOVICE},
    {HOUSE_BODY_GUILD_HARMONDALE,       MASTERY_EXPERT},
    {HOUSE_BODY_GUILD_ERATHIA,          MASTERY_MASTER},
    {HOUSE_BODY_GUILD_AVLEE,            MASTERY_GRANDMASTER},
    {HOUSE_LIGHT_GUILD_BRACADA_DESERT,  MASTERY_EXPERT},
    {HOUSE_LIGHT_GUILD_CELESTE,         MASTERY_GRANDMASTER},
    {HOUSE_DARK_GUILD_DEYJA,            MASTERY_EXPERT},
    {HOUSE_DARK_GUILD_PIT,              MASTERY_GRANDMASTER}
};

Mastery magicGuildMaxMastery(HouseId house, HouseType guildType) {
    // The table above is MM7's, keyed by MM7's house ids. MM8 has one Guild of Light (Murmurwoods) and one Guild of Dark
    // (Shadowspire) and no tiers of them; its Guild of Dark fell on a Novice row and stocked four books, so a Lich had nowhere to
    // buy the spells of his own school. Both sell their whole school, as the one guild of a school does.
    if (gameType() == GAME_TYPE_MM8 && (guildType == HOUSE_TYPE_LIGHT_GUILD || guildType == HOUSE_TYPE_DARK_GUILD))
        return MASTERY_GRANDMASTER;
    return guildSpellsMastery[house];
}

void GUIWindow_MagicGuild::mainDialogue() {
    Recti working_window = this->frameRect;
    working_window.x = SIDE_TEXT_BOX_POS_X;
    working_window.w = SIDE_TEXT_BOX_WIDTH;

    // Combined guilds (see `generateSpellBooksForGuild()`) sit outside the classic
    // HOUSE_FIRST_MAGIC_GUILD..HOUSE_LAST_MAGIC_GUILD house-id range `membershipAwardForGuild
    // (HouseId)` is keyed on, so they need the `GuildId`-based overload instead.
    HouseType guildType = buildingType();
    AwardId membershipAward = isCombinedMagicGuild(guildType)
        ? membershipAwardForGuild(guildType == HOUSE_TYPE_ELEMENTAL_GUILD ? GUILD_OF_ELEMENTS : GUILD_OF_SELF)
        : membershipAwardForGuild(houseId());
    // MM8 awards.txt has no guild membership award, so MM8 guilds are open to any party.
    if (gameType() != GAME_TYPE_MM8 && !pParty->activeCharacter()._achievedAwardsBits[membershipAward]) {
        // you must be a member
        int textHeight = assets->pFontArrus->CalcTextHeight(pNPCTopics[121].pText, working_window.w, 0);
        DrawTitleText(assets->pFontArrus.get(), 0, (212 - textHeight) / 2 + 101, colorTable.PaleCanary, pNPCTopics[121].pText, 3, working_window);
        pDialogueWindow->pNumPresenceButton = 0;
        return;
    }

    if (!checkIfPlayerCanInteract())
        return;

    std::vector<std::string> optionsText;

    bool haveLearnableSkills = false;
    int buttonsLimit = pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton;
    for (int i = pDialogueWindow->pStartingPosActiveItem; i < buttonsLimit; ++i) {
        if (pDialogueWindow->GetControl(i)->msg_param == std::to_underlying(DIALOGUE_GUILD_BUY_BOOKS)) {
            optionsText.push_back(localization->str(LSTR_BUY_SPELLS));
        } else {
            Skill skill = GetLearningDialogueSkill((DialogueId)pDialogueWindow->GetControl(i)->msg_param);
            if (skillMaxMasteryForClass(pParty->activeCharacter().classType, skill) != MASTERY_NONE &&
                !pParty->activeCharacter().pActiveSkills[skill]) {
                optionsText.push_back(localization->skillName(skill));
                haveLearnableSkills = true;
            } else {
                optionsText.push_back("");
            }
        }
    }

    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);

    if (haveLearnableSkills) {
        std::string skill_price_label = localization->format(LSTR_SKILL_COST_LU, pPrice);
        DrawTitleText(assets->pFontArrus.get(), 0, 146, colorTable.White, skill_price_label, 3, working_window);
    }

    drawOptions(optionsText, colorTable.PaleCanary, 24);
}

void GUIWindow_MagicGuild::buyBooksDialogue() {
    // TODO(pskelton): Extract common item picking code
    Recti working_window = this->frameRect;
    working_window.x = SIDE_TEXT_BOX_POS_X;
    working_window.w = SIDE_TEXT_BOX_WIDTH;

    render->DrawQuad2D(_shopBackground, {8, 8});
    int itemxind = 0;

    for (int pX = 32; pX < 452; pX += 70) {  // top row
        if (pParty->spellBooksInGuilds[houseId()][itemxind].itemId != ITEM_NULL)
            render->DrawQuad2D(shop_ui_items_in_store[itemxind], {pX, 90});
        if (pParty->spellBooksInGuilds[houseId()][itemxind + 6].itemId != ITEM_NULL)
            render->DrawQuad2D(shop_ui_items_in_store[itemxind + 6], {pX, 250});

        ++itemxind;
    }

    if (checkIfPlayerCanInteract()) {
        int itemcount = 0;
        for (int i = 0; i < itemAmountInShop[buildingType()]; ++i)
            if (pParty->spellBooksInGuilds[houseId()][i].itemId != ITEM_NULL)
                ++itemcount;

        engine->_statusBar->drawForced(localization->str(LSTR_SELECT_THE_ITEM_TO_BUY), colorTable.White);

        if (!itemcount) {  // shop empty
            Time nextGenTime = pParty->PartyTimes.guildNextRefreshTime[houseId()];
            DrawShops_next_generation_time_string(nextGenTime - pParty->GetPlayingTime(), working_window);
            return;
        }

        Pointi pt = EngineIocContainer::ResolveMouse()->position();
        int testx = (pt.x - 32) / 70;
        if (testx >= 0 && testx < 6) {
            if (pt.y >= 250)
                testx += 6;

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
                        std::string str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex(), houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_BUY);
                        int textHeight = assets->pFontArrus->CalcTextHeight(str, working_window.w, 0);
                        DrawTitleText(assets->pFontArrus.get(), 0, (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - textHeight) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET, colorTable.White, str, 3, working_window);
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
            for (int i = 0; i < itemAmountInShop[buildingType()]; ++i)
                if (pParty->spellBooksInGuilds[houseId()][i].itemId != ITEM_NULL)
                    shop_ui_items_in_store[i] = assets->getIcon(pParty->spellBooksInGuilds[houseId()][i].GetIconName());
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
        if (guildType == HOUSE_TYPE_ELEMENTAL_GUILD) {
            return {DIALOGUE_GUILD_BUY_BOOKS, DIALOGUE_LEARN_FIRE, DIALOGUE_LEARN_AIR, DIALOGUE_LEARN_WATER, DIALOGUE_LEARN_EARTH, DIALOGUE_LEARN_LEARNING};
        } else if (guildType == HOUSE_TYPE_SELF_GUILD) {
            return {DIALOGUE_GUILD_BUY_BOOKS, DIALOGUE_LEARN_SPIRIT, DIALOGUE_LEARN_MIND, DIALOGUE_LEARN_BODY, DIALOGUE_LEARN_LIGHT, DIALOGUE_LEARN_DARK, DIALOGUE_LEARN_MEDITATION};
        } else if (learnableAdditionalSkillDialogue[guildType] != DIALOGUE_NULL) {
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
        if (pt.y >= 250)
            testx += 6;

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

                    std::optional<Pointi> pos = pParty->activeCharacter().inventory.findSpace(boughtItem);
                    if (pos) {
                        boughtItem.SetIdentified();
                        pParty->activeCharacter().inventory.add(*pos, boughtItem);
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

    for (int i = 0; i < itemAmountInShop[guildType]; ++i) {
        ItemId pItemNum;
        if (isCombinedMagicGuild(guildType)) {
            // A combined guild teaches every school in its category (see the
            // `elementalGuildSchools`/`selfGuildSchools` comment above) - each shelf slot
            // independently rolls a school, then a spellbook within that school. MM6 and MM7 roll
            // the full (grandmaster) range, MM8 the guild's own tier.
            const bool mm8 = gameType() == GAME_TYPE_MM8;
            MagicSchool schoolType = guildType == HOUSE_TYPE_ELEMENTAL_GUILD ? grng->randomSample(elementalGuildSchools)
                                     : mm8                                    ? grng->randomSample(mm8SelfGuildSchools)
                                                                              : grng->randomSample(selfGuildSchools);
            pItemNum = grng->randomSample(spellbooksForSchool(schoolType, mm8 ? mm8CombinedGuildMaxMastery(houseId()) : MASTERY_GRANDMASTER));
        } else {
            assert(guildType >= HOUSE_TYPE_FIRE_GUILD && guildType <= HOUSE_TYPE_DARK_GUILD);
            MagicSchool schoolType = guildSpellsSchool[guildType];
            Mastery maxMastery = magicGuildMaxMastery(houseId(), guildType);
            pItemNum = grng->randomSample(spellbooksForSchool(schoolType, maxMastery));
        }

        if (pItemNum == ITEM_SPELLBOOK_DIVINE_INTERVENTION && !pParty->_questBits[QBIT_DIVINE_INTERVENTION_RETRIEVED])
            pItemNum = ITEM_SPELLBOOK_SUNRAY;

        Item *itemSpellbook = &pParty->spellBooksInGuilds[houseId()][i];
        itemSpellbook->Reset();
        itemSpellbook->itemId = pItemNum;
        itemSpellbook->SetIdentified();

        shop_ui_items_in_store[i] = assets->getIcon(pItemTable->items[pItemNum].iconName);
    }
}
