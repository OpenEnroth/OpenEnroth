#include "Engine/Objects/Character.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/EngineCallObserver.h"
#include "Engine/AssetsManager.h"
#include "Engine/Data/AwardEnums.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/AttackList.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/PortraitFrameTable.h"
#include "Engine/Tables/HistoryTable.h"
#include "Engine/Tables/AutonoteTable.h"
#include "Engine/Tables/QuestTable.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Conditions.h"
#include "Engine/Evt/EvtEnumFunctions.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIMessageScroll.h"
#include "GUI/UI/UISpell.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/Books/AutonotesBook.h"
#include "GUI/UI/ItemGrid.h"

#include "Library/Logger/Logger.h"

#include "Utility/Memory/MemSet.h"
#include "Utility/IndexedArray.h"

static SpellFxRenderer *spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();

// Race Stat Points Bonus/ Penalty
struct CharacterCreationAttributeProps {
    unsigned char uBaseValue;
    unsigned char uMaxValue;
    unsigned char uDroppedStep;
    unsigned char uBaseStep;
};

static constexpr IndexedArray<IndexedArray<CharacterCreationAttributeProps, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT>, RACE_FIRST, RACE_LAST> StatTable = {
    {RACE_HUMAN, {
        {ATTRIBUTE_MIGHT,         {11, 25, 1, 1}},
        {ATTRIBUTE_INTELLIGENCE,  {11, 25, 1, 1}},
        {ATTRIBUTE_PERSONALITY,   {11, 25, 1, 1}},
        {ATTRIBUTE_ENDURANCE,     {9, 25, 1, 1}},
        {ATTRIBUTE_ACCURACY,      {11, 25, 1, 1}},
        {ATTRIBUTE_SPEED,         {11, 25, 1, 1}},
        {ATTRIBUTE_LUCK,          {9, 25, 1, 1}},
    }},
    {RACE_ELF, {
        {ATTRIBUTE_MIGHT,         {7, 15, 2, 1}},
        {ATTRIBUTE_INTELLIGENCE,  {14, 30, 1, 2}},
        {ATTRIBUTE_PERSONALITY,   {11, 25, 1, 1}},
        {ATTRIBUTE_ENDURANCE,     {7, 15, 2, 1}},
        {ATTRIBUTE_ACCURACY,      {14, 30, 1, 2}},
        {ATTRIBUTE_SPEED,         {11, 25, 1, 1}},
        {ATTRIBUTE_LUCK,          {9, 20, 1, 1}},
    }},
    {RACE_GOBLIN, {
        {ATTRIBUTE_MIGHT,         {14, 30, 1, 2}},
        {ATTRIBUTE_INTELLIGENCE,  {7, 15, 2, 1}},
        {ATTRIBUTE_PERSONALITY,   {7, 15, 2, 1}},
        {ATTRIBUTE_ENDURANCE,     {11, 25, 1, 1}},
        {ATTRIBUTE_ACCURACY,      {11, 25, 1, 1}},
        {ATTRIBUTE_SPEED,         {14, 30, 1, 2}},
        {ATTRIBUTE_LUCK,          {9, 20, 1, 1}},
    }},
    {RACE_DWARF, {
        {ATTRIBUTE_MIGHT,         {14, 30, 1, 2}},
        {ATTRIBUTE_INTELLIGENCE,  {11, 25, 1, 1}},
        {ATTRIBUTE_PERSONALITY,   {11, 25, 1, 1}},
        {ATTRIBUTE_ENDURANCE,     {14, 30, 1, 2}},
        {ATTRIBUTE_ACCURACY,      {7, 15, 2, 1}},
        {ATTRIBUTE_SPEED,         {7, 15, 2, 1}},
        {ATTRIBUTE_LUCK,          {9, 20, 1, 1}}
    }}
};

static constexpr IndexedArray<int, MASTERY_FIRST, MASTERY_LAST> StealingMasteryBonuses = {
    // {CHARACTER_SKILL_MASTERY_NONE, 0},
    {MASTERY_NOVICE, 100},
    {MASTERY_EXPERT, 200},
    {MASTERY_MASTER, 300},
    {MASTERY_GRANDMASTER, 500}
};  // dword_4EDEA0        //the zeroth element isn't accessed, it just
           // helps avoid -1 indexing, originally 4 element array off by one
static constexpr std::array<int, 5> StealingRandomBonuses = { -200, -100, 0, 100, 200 };  // dword_4EDEB4

/**
 * The amount of gold that a character can steal in one go is determined as `[skill_level]d[mastery_die]`, where
 * `skill_level` is the level of stealing skill, and `mastery_die` is picked from the table below.
 */
static constexpr IndexedArray<int, MASTERY_FIRST, MASTERY_LAST> goldStealingDieSidesByMastery = {
    {MASTERY_NOVICE, 2},
    {MASTERY_EXPERT, 4},
    {MASTERY_MASTER, 6},
    {MASTERY_GRANDMASTER, 10}
};

static constexpr unsigned char pBaseHealthByClass[12] = {40, 35, 35, 30, 30, 30,
                                        25, 20, 20, 0,  0,  0};
static constexpr unsigned char pBaseManaByClass[12] = {0, 0, 0, 5, 5, 0, 10, 10, 15, 0, 0, 0};

static constexpr IndexedArray<int, CLASS_FIRST, CLASS_LAST> pBaseHealthPerLevelByClass = {
    {CLASS_KNIGHT,            5},
    {CLASS_CAVALIER,          7},
    {CLASS_CHAMPION,          9},
    {CLASS_BLACK_KNIGHT,      9},
    {CLASS_THIEF,             4},
    {CLASS_ROGUE,             6},
    {CLASS_SPY,               8},
    {CLASS_ASSASSIN,          8},
    {CLASS_MONK,              5},
    {CLASS_INITIATE,          6},
    {CLASS_MASTER,            8},
    {CLASS_NINJA,             8},
    {CLASS_PALADIN,           4},
    {CLASS_CRUSADER,          5},
    {CLASS_HERO,              6},
    {CLASS_VILLIAN,           6},
    {CLASS_ARCHER,            3},
    {CLASS_WARRIOR_MAGE,      4},
    {CLASS_MASTER_ARCHER,     6},
    {CLASS_SNIPER,            6},
    {CLASS_RANGER,            4},
    {CLASS_HUNTER,            5},
    {CLASS_RANGER_LORD,       6},
    {CLASS_BOUNTY_HUNTER,     6},
    {CLASS_CLERIC,            2},
    {CLASS_PRIEST,            3},
    {CLASS_PRIEST_OF_SUN,     4},
    {CLASS_PRIEST_OF_MOON,    4},
    {CLASS_DRUID,             2},
    {CLASS_GREAT_DRUID,       3},
    {CLASS_ARCH_DRUID,        4},
    {CLASS_WARLOCK,           4},
    {CLASS_SORCERER,          2},
    {CLASS_WIZARD,            3},
    {CLASS_ARCHAMGE,          3},
    {CLASS_LICH,              3}
};
static constexpr IndexedArray<int, CLASS_FIRST, CLASS_LAST> pBaseManaPerLevelByClass = {
    {CLASS_KNIGHT,            0},
    {CLASS_CAVALIER,          0},
    {CLASS_CHAMPION,          0},
    {CLASS_BLACK_KNIGHT,      0},
    {CLASS_THIEF,             0},
    {CLASS_ROGUE,             1},
    {CLASS_SPY,               1},
    {CLASS_ASSASSIN,          1},
    {CLASS_MONK,              0},
    {CLASS_INITIATE,          1},
    {CLASS_MASTER,            1},
    {CLASS_NINJA,             1},
    {CLASS_PALADIN,           1},
    {CLASS_CRUSADER,          2},
    {CLASS_HERO,              3},
    {CLASS_VILLIAN,           3},
    {CLASS_ARCHER,            1},
    {CLASS_WARRIOR_MAGE,      2},
    {CLASS_MASTER_ARCHER,     3},
    {CLASS_SNIPER,            3},
    {CLASS_RANGER,            0},
    {CLASS_HUNTER,            2},
    {CLASS_RANGER_LORD,       3},
    {CLASS_BOUNTY_HUNTER,     3},
    {CLASS_CLERIC,            3},
    {CLASS_PRIEST,            4},
    {CLASS_PRIEST_OF_SUN,     5},
    {CLASS_PRIEST_OF_MOON,    5},
    {CLASS_DRUID,             3},
    {CLASS_GREAT_DRUID,       4},
    {CLASS_ARCH_DRUID,        5},
    {CLASS_WARLOCK,           5},
    {CLASS_SORCERER,          3},
    {CLASS_WIZARD,            4},
    {CLASS_ARCHAMGE,          6},
    {CLASS_LICH,              6}
};

static constexpr IndexedArray<std::array<int, 19>, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> pConditionAttributeModifier = {
    {ATTRIBUTE_MIGHT,         {100, 100, 100, 120, 50, 200, 75, 60, 50, 30, 25, 10, 100, 100, 100, 100, 100, 100, 100}},
    {ATTRIBUTE_INTELLIGENCE,  {100, 100, 100, 50, 25, 10, 100, 100, 75, 60, 50, 30, 100, 100, 100, 100, 100, 1, 100}},
    {ATTRIBUTE_PERSONALITY,   {100, 100, 100, 50, 25, 10, 100, 100, 75, 60, 50, 30, 100, 100, 100, 100, 100, 1, 100}},
    {ATTRIBUTE_ENDURANCE,     {100, 100, 100, 100, 50, 150, 75, 60, 50, 30, 25, 10, 100, 100, 100, 100, 100, 100, 100}},
    {ATTRIBUTE_ACCURACY,      {100, 100, 100, 50, 10, 100, 75, 60, 50, 30, 25, 10, 100, 100, 100, 100, 100, 50, 100}},
    {ATTRIBUTE_SPEED,         {100, 100, 100, 120, 20, 120, 75, 60, 50, 30, 25, 10, 100, 100, 100, 100, 100, 50, 100}},
    {ATTRIBUTE_LUCK,          {100, 100, 100, 100, 200, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}
};

static constexpr IndexedArray<std::array<int, 4>, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> pAgingAttributeModifier = {
    {ATTRIBUTE_MIGHT,         {100, 75, 40, 10}},
    {ATTRIBUTE_INTELLIGENCE,  {100, 150, 100, 10}},
    {ATTRIBUTE_PERSONALITY,   {100, 150, 100, 10}},
    {ATTRIBUTE_ENDURANCE,     {100, 75, 40, 10}},
    {ATTRIBUTE_ACCURACY,      {100, 100, 40, 10}},
    {ATTRIBUTE_SPEED,         {100, 100, 40, 10}},
    {ATTRIBUTE_LUCK,          {100, 100, 100, 100}}
};

static constexpr unsigned int pAgeingTable[4] = {50, 100, 150, 0xFFFF};

static constexpr short param_to_bonus_table[29] = {
    500, 400, 350, 300, 275, 250, 225, 200, 175, 150, 125, 100, 75, 50, 40,
    35,  30,  25,  21,  19,  17,  15,  13,  11,  9,   7,   5,   3,  0};
static constexpr signed int parameter_to_bonus_value[29] = {
    30, 25, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8,
    7,  6,  5,  4,  3,  2,  1,  0,  -1, -2, -3, -4, -5, -6};

//----- (00490913) --------------------------------------------------------
int CharacterCreation_GetUnspentAttributePointCount() {
    int CurrentStatValue = 50;
    int RemainingStatPoints = 50;
    int StatBaseValue;
    int PenaltyMult;
    int BonusMult;

    for (Character &character : pParty->pCharacters) {
        Race raceId = character.GetRace();

        for (Attribute statNum : allStatAttributes()) {
            CurrentStatValue = character._stats[statNum];
            StatBaseValue = StatTable[raceId][statNum].uBaseValue;

            if (CurrentStatValue >= StatBaseValue) {  // bonus or penalty increase
                PenaltyMult = StatTable[raceId][statNum].uDroppedStep;
                BonusMult = StatTable[raceId][statNum].uBaseStep;
            } else {  // less than base
                PenaltyMult = StatTable[raceId][statNum].uBaseStep;
                BonusMult = StatTable[raceId][statNum].uDroppedStep;
            }

            RemainingStatPoints +=
                PenaltyMult * (StatBaseValue - CurrentStatValue) / BonusMult;
        }
    }

    return RemainingStatPoints;
}

//----- (00427730) --------------------------------------------------------
bool Character::CanCastSpell(unsigned int uRequiredMana) {
    if (engine->config->debug.AllMagic.value()) {
        return true;
    }
    if (mana >= uRequiredMana) {  // enough mana
        return true;
    }

    // not enough mana
    return false;
}

void Character::SpendMana(unsigned int uRequiredMana) {
    if (engine->config->debug.AllMagic.value()) {
        return;
    }
    assert(mana >= uRequiredMana);
    mana -= uRequiredMana; // remove mana required for spell
}

//----- (004BE2DD) --------------------------------------------------------
void Character::SalesProcess(InventoryEntry entry, HouseId houseId) {
    float shop_mult = houseTable[houseId].fPriceMultiplier;
    int sell_price = PriceCalculator::itemSellingPriceForPlayer(this, *entry, shop_mult);

    // remove item and add gold
    inventory.take(entry);
    pParty->AddGold(sell_price);
}

//----- (0043EEF3) --------------------------------------------------------
bool Character::NothingOrJustBlastersEquipped() const {
    for (InventoryConstEntry entry : inventory.equipment())
        if (!isAncientWeapon(entry->itemId))
            return false;
    return true; // nothing or just blaster equipped
}

//----- (004B8040) --------------------------------------------------------
int Character::GetConditionDaysPassed(Condition condition) const {
    // PS - CHECK ?? is this the intedned behavior - RETURN
    // NUMBER OF DAYS CONDITION HAS BEEN ACTIVE FOR

    if (!this->conditions.has(condition))
        return 0;

    Time playtime = pParty->GetPlayingTime();
    Time condtime = this->conditions.get(condition);
    Duration diff = playtime - condtime;

    return diff.days() + 1;
}

//----- (004160CA) --------------------------------------------------------
void Character::ItemsPotionDmgBreak(int count) {
    std::vector<InventoryEntry> entries;
    for (InventoryEntry entry : inventory.entries())
        if (isRegular(entry->itemId))
            entries.push_back(entry);

    if (entries.empty())
        return;

    if (count) {
        for (int i = 0; i < count; ++i) {
            InventoryEntry indexbreak = entries[grng->random(entries.size())];

            if (!(indexbreak->flags & ITEM_HARDENED))
                indexbreak->flags |= ITEM_BROKEN;
        }
    } else {
        for (InventoryEntry entry : entries) // break everything on eradication, item hardening doesn't help in this case.
            entry->flags |= ITEM_BROKEN;
    }
}

//----- (00492C0B) --------------------------------------------------------
bool Character::CanAct() const {
    if (this->IsAsleep() || this->IsParalyzed() || this->IsUnconcious() ||
        this->IsDead() || this->IsPetrified() || this->IsEradicated())

        return false;
    else
        return true;
}

//----- (00492C40) --------------------------------------------------------
bool Character::CanSteal() const {
    return getActualSkillValue(SKILL_STEALING).level() != 0;
}

//----- (00492C4E) --------------------------------------------------------
bool Character::CanEquip_RaceAndAlignmentCheck(ItemId uItemID) const {
    switch (uItemID) {
        case ITEM_RELIC_ETHRICS_STAFF:
        case ITEM_RELIC_OLD_NICK:
        case ITEM_RELIC_TWILIGHT:
            return pParty->isPartyEvil();
            break;

        case ITEM_RELIC_TALEDONS_HELM:
        case ITEM_RELIC_JUSTICE:
            return pParty->isPartyGood();
            break;

        case ITEM_ARTIFACT_ELFBANE:
            return IsRaceGoblin();
            break;

        case ITEM_ARTIFACT_MINDS_EYE:
            return IsRaceHuman();
            break;

        case ITEM_ARTIFACT_ELVEN_CHAINMAIL:
            return IsRaceElf();
            break;

        case ITEM_ARTIFACT_FORGE_GAUNTLETS:
            return IsRaceDwarf();
            break;

        case ITEM_ARTIFACT_HEROS_BELT:
            return IsMale();
            break;

        case ITEM_ARTIFACT_LADYS_ESCORT:
            return IsFemale();
            break;

        case ITEM_QUEST_WETSUIT:
            return NothingOrJustBlastersEquipped();
            break;

        default:
            return 1;
            break;
    }
}

//----- (00492D65) --------------------------------------------------------
void Character::SetCondition(Condition condition, int blockable) {
    if (conditions.has(condition))  // cant get the same condition twice
        return;

    if (blockable && blockCondition(this, condition))
        return;

    switch (condition) {  // conditions noises
        case CONDITION_CURSED:
            playReaction(SPEECH_CURSED);
            break;
        case CONDITION_WEAK:
            playReaction(SPEECH_WEAK);
            break;
        case CONDITION_SLEEP:
            break;  // nosound
        case CONDITION_FEAR:
            playReaction(SPEECH_FEAR);
            break;
        case CONDITION_DRUNK:
            playReaction(SPEECH_DRUNK);
            break;
        case CONDITION_INSANE:
            playReaction(SPEECH_INSANE);
            break;

        case CONDITION_POISON_WEAK:
        case CONDITION_POISON_MEDIUM:
        case CONDITION_POISON_SEVERE:
            playReaction(SPEECH_POISONED);
            break;

        case CONDITION_DISEASE_WEAK:
        case CONDITION_DISEASE_MEDIUM:
        case CONDITION_DISEASE_SEVERE:
            playReaction(SPEECH_DISEASED);
            break;

        case CONDITION_PARALYZED:
            break;  // nosound

        case CONDITION_UNCONSCIOUS:
            playReaction(SPEECH_UNCONSCIOUS);
            if (health > 0) {
                health = 0;
            }
            break;

        case CONDITION_DEAD:
            playReaction(SPEECH_DEAD);
            if (health > 0) {
                health = 0;
            }
            if (mana > 0) {
                mana = 0;
            }
            break;

        case CONDITION_PETRIFIED:
            playReaction(SPEECH_PETRIFIED);
            break;

        case CONDITION_ERADICATED:
            playReaction(SPEECH_ERADICATED);
            if (health > 0) {
                health = 0;
            }
            if (mana > 0) {
                mana = 0;
            }
            break;

        case CONDITION_ZOMBIE:
            if (classType == CLASS_LICH || IsEradicated() || IsZombie() || !IsDead()) { // cant zombified
                return;
            }

            conditions.resetAll();
            health = GetMaxHealth();
            mana = 0;
            uPrevFace = uCurrentFace;
            uPrevVoiceID = uVoiceID;

            if (IsMale()) {
                uCurrentFace = 23;
                uVoiceID = 23;
            } else {
                uCurrentFace = 24;
                uVoiceID = 24;
            }

            playReaction(SPEECH_CHEATED_DEATH);
            break;

        default:
            break;
    }

    int playersBefore = 0;
    for (Character &character : pParty->pCharacters) {  // count active players before activating condition
        playersBefore += character.CanAct() ? 1 : 0;
    }

    conditions.set(condition, pParty->GetPlayingTime());  // set condition

    int playersAfter = 0;
    Character *remainingPlayer = nullptr;
    for (Character &character : pParty->pCharacters) {
        if (character.CanAct()) {
            remainingPlayer = &character;
            playersAfter++;
        }
    }

    if ((playersBefore == 2) && (playersAfter == 1)) { // if was 2 and now down to 1 - "its just you and me now"
        remainingPlayer->playReaction(SPEECH_LAST_MAN_STANDING);
    }

    return;
}

//----- (00492700) --------------------------------------------------------
bool Character::HasSkill(Skill skill) const {
    if (this->pActiveSkills[skill]) {
        return true;
    } else {
        // TODO(captainurist): this doesn't belong to a getter!!!
        engine->_statusBar->setEvent(LSTR_S_DOES_NOT_HAVE_THE_SKILL, this->name);
        return false;
    }
}

//----- (0049107D) --------------------------------------------------------
int Character::GetBodybuilding() const {
    int multiplier =
        GetMultiplierForSkillLevel(SKILL_BODYBUILDING, 1, 2, 3, 5);

    return multiplier * getActualSkillValue(SKILL_BODYBUILDING).level();
}

//----- (004910A8) --------------------------------------------------------
int Character::GetMeditation() const {
    int multiplier =
        GetMultiplierForSkillLevel(SKILL_MEDITATION, 1, 2, 3, 5);

    return multiplier * getActualSkillValue(SKILL_MEDITATION).level();
}

//----- (004910D3) --------------------------------------------------------
bool Character::CanIdentify(const Item &item) const {
    CombinedSkillValue val = getActualSkillValue(SKILL_ITEM_ID);
    int multiplier =
        GetMultiplierForSkillLevel(SKILL_ITEM_ID, 1, 2, 3, 5);

    if (CheckHiredNPCSpeciality(Scholar) || val.mastery() == MASTERY_GRANDMASTER)  // always identify
        return true;

    // check item level against skill
    bool result = (multiplier * val.level()) >=
                  pItemTable->items[item.itemId].identifyAndRepairDifficulty;

    return result;
}

//----- (00491151) --------------------------------------------------------
bool Character::CanRepair(const Item &item) const {
    CombinedSkillValue val = getActualSkillValue(SKILL_REPAIR);
    int multiplier = GetMultiplierForSkillLevel(SKILL_REPAIR, 1, 2, 3, 5);

    // TODO(Nik-RE-dev): is check for boots correct?
    if (CheckHiredNPCSpeciality(Smith) && item.isWeapon() ||
        CheckHiredNPCSpeciality(Armorer) && item.isArmor() ||
        CheckHiredNPCSpeciality(Alchemist) && item.type() >= ITEM_TYPE_BOOTS)
        return true;  // check against hired help

    if (val.mastery() == MASTERY_GRANDMASTER)  // gm repair
        return true;

    // check item level against skill
    bool result = (multiplier * val.level()) >=
                  pItemTable->items[item.itemId].identifyAndRepairDifficulty;

    return result;
}

//----- (0049125A) --------------------------------------------------------
int Character::GetPerception() const {
    CombinedSkillValue val = getActualSkillValue(SKILL_PERCEPTION);
    int multiplier =
        GetMultiplierForSkillLevel(SKILL_PERCEPTION, 1, 2, 3, 5);

    if (val.mastery() == MASTERY_GRANDMASTER)  // gm percept
        return 10000;

    return multiplier * val.level();
}

//----- (004912B0) --------------------------------------------------------
int Character::GetDisarmTrap() const {
    CombinedSkillValue val = getActualSkillValue(SKILL_TRAP_DISARM);
    int multiplier =
        GetMultiplierForSkillLevel(SKILL_TRAP_DISARM, 1, 2, 3, 5);

    if (val.mastery() == MASTERY_GRANDMASTER)  // gm disarm
        return 10000;

    if (wearsEnchantedItem(ITEM_ENCHANTMENT_OF_THIEVERY))  // item has increased disarm
        multiplier++;

    return multiplier * val.level();
}

int Character::getLearningPercent() const {
    int hirelingBonus = 0;
    if (CheckHiredNPCSpeciality(Teacher)) hirelingBonus = 10;
    if (CheckHiredNPCSpeciality(Instructor)) hirelingBonus += 15;
    if (CheckHiredNPCSpeciality(Scholar)) hirelingBonus += 5;

    int skill = getActualSkillValue(SKILL_LEARNING).level();

    if (skill) {
        int multiplier = GetMultiplierForSkillLevel(SKILL_LEARNING, 1, 2, 3, 5);

        return hirelingBonus + multiplier * skill + 9;
    } else {
        return hirelingBonus;
    }
}

//----- (0048C855) --------------------------------------------------------
int Character::GetBaseMight() const {
    return GetBaseStat(ATTRIBUTE_MIGHT);
}

//----- (0048C86C) --------------------------------------------------------
int Character::GetBaseIntelligence() const {
    return GetBaseStat(ATTRIBUTE_INTELLIGENCE);
}

//----- (0048C883) --------------------------------------------------------
int Character::GetBasePersonality() const {
    return GetBaseStat(ATTRIBUTE_PERSONALITY);
}

//----- (0048C89A) --------------------------------------------------------
int Character::GetBaseEndurance() const {
    return GetBaseStat(ATTRIBUTE_ENDURANCE);
}

//----- (0048C8B1) --------------------------------------------------------
int Character::GetBaseAccuracy() const {
    return GetBaseStat(ATTRIBUTE_ACCURACY);
}

//----- (0048C8C8) --------------------------------------------------------
int Character::GetBaseSpeed() const {
    return GetBaseStat(ATTRIBUTE_SPEED);
}

//----- (0048C8DF) --------------------------------------------------------
int Character::GetBaseLuck() const {
    return GetBaseStat(ATTRIBUTE_LUCK);
}

int Character::GetBaseStat(Attribute stat) const {
    return this->_stats[stat] + GetItemsBonus(stat);
}

//----- (0048C8F6) --------------------------------------------------------
int Character::GetBaseLevel() const {
    return this->uLevel + GetItemsBonus(ATTRIBUTE_LEVEL);
}

//----- (0048C90D) --------------------------------------------------------
int Character::GetActualLevel() const {
    return uLevel + sLevelModifier +
           GetMagicalBonus(ATTRIBUTE_LEVEL) +
           GetItemsBonus(ATTRIBUTE_LEVEL);
}

//----- (0048C93C) --------------------------------------------------------
int Character::GetActualMight() const {
    return GetActualStat(ATTRIBUTE_MIGHT);
}

//----- (0048C9C2) --------------------------------------------------------
int Character::GetActualIntelligence() const {
    return GetActualStat(ATTRIBUTE_INTELLIGENCE);
}

//----- (0048CA3F) --------------------------------------------------------
int Character::GetActualPersonality() const {
    return GetActualStat(ATTRIBUTE_PERSONALITY);
}

//----- (0048CABC) --------------------------------------------------------
int Character::GetActualEndurance() const {
    return GetActualStat(ATTRIBUTE_ENDURANCE);
}

//----- (0048CB39) --------------------------------------------------------
int Character::GetActualAccuracy() const {
    return GetActualStat(ATTRIBUTE_ACCURACY);
}

//----- (0048CBB6) --------------------------------------------------------
int Character::GetActualSpeed() const {
    return GetActualStat(ATTRIBUTE_SPEED);
}

//----- (0048CC33) --------------------------------------------------------
int Character::GetActualLuck() const {
    return GetActualStat(ATTRIBUTE_LUCK);
}

//----- (new function) --------------------------------------------------------
int Character::GetActualStat(Attribute stat) const {
    int attrValue = _stats[stat];
    int attrBonus = _statBonuses[stat];

    unsigned uActualAge = this->sAgeModifier + GetBaseAge();
    unsigned uAgeingMultiplier = 100;

    for (unsigned i = 0; i < 4; ++i) {
        if (uActualAge >=
            pAgeingTable[i])  // is the character old enough to need attrib adjust
            uAgeingMultiplier = pAgingAttributeModifier[stat][i];
        else
            break;
    }

    float uConditionMult = 100.0f;
    if (this->conditions.hasNone({ CONDITION_DEAD, CONDITION_ERADICATED, CONDITION_PETRIFIED }))
        for (Condition cond : allConditions())  // accumulate all condition effects
            if (this->conditions.has(cond))
                uConditionMult *= 0.01f * pConditionAttributeModifier[stat][std::to_underlying(cond)];  // weak from disease or poison ect

    int magicBonus = GetMagicalBonus(stat);
    int itemBonus = GetItemsBonus(stat);

    int npcBonus = 0;
    if (stat == ATTRIBUTE_LUCK) {
        if (CheckHiredNPCSpeciality(Fool))
            npcBonus += 5;
        if (CheckHiredNPCSpeciality(ChimneySweep))
            npcBonus += 20;
        if (CheckHiredNPCSpeciality(Psychic))
            npcBonus += 10;
    }

    return uConditionMult * uAgeingMultiplier * attrValue / 100 / 100 +
           magicBonus + itemBonus + attrBonus + npcBonus;
}

//----- (0048CCF5) --------------------------------------------------------
int Character::GetActualAttack(bool onlyMainHandDmg) const {
    int parbonus = GetParameterBonus(
        GetActualAccuracy());  // bonus points for steps of accuracy level
    int atkskillbonus = GetSkillBonus(
        ATTRIBUTE_ATTACK);  // bonus for skill with weapon
    int weapbonus = GetItemsBonus(ATTRIBUTE_ATTACK,
                                  onlyMainHandDmg);  // how good is weapon

    return parbonus + atkskillbonus + weapbonus +
           GetMagicalBonus(ATTRIBUTE_ATTACK) +
           this->_some_attack_bonus;
}

//----- (0048CD45) --------------------------------------------------------
int Character::GetMeleeDamageMinimal() const {
    int parbonus = GetParameterBonus(GetActualMight());
    int weapbonus = GetItemsBonus(ATTRIBUTE_MELEE_DMG_MIN) + parbonus;
    int atkskillbonus =
        GetSkillBonus(ATTRIBUTE_MELEE_DMG_BONUS) + weapbonus;

    int result = _melee_dmg_bonus +
                 GetMagicalBonus(ATTRIBUTE_MELEE_DMG_BONUS) +
                 atkskillbonus;

    if (result < 1) result = 1;

    return result;
}

//----- (0048CD90) --------------------------------------------------------
int Character::GetMeleeDamageMaximal() const {
    int parbonus = GetParameterBonus(GetActualMight());
    int weapbonus = GetItemsBonus(ATTRIBUTE_MELEE_DMG_MAX) + parbonus;
    int atkskillbonus =
        GetSkillBonus(ATTRIBUTE_MELEE_DMG_BONUS) + weapbonus;

    int result = this->_melee_dmg_bonus +
                 GetMagicalBonus(ATTRIBUTE_MELEE_DMG_BONUS) +
                 atkskillbonus;

    if (result < 1) result = 1;

    return result;
}

//----- (0048CDDB) --------------------------------------------------------
int Character::CalculateMeleeDamageTo(bool ignoreSkillBonus, bool ignoreOffhand,
                                      MonsterId uTargetActorID) {
    int mainWpnDmg = 0;
    int offHndWpnDmg = 0;

    if (IsUnarmed()) {  // no weapons
        mainWpnDmg = grng->random(3) + 1;
    } else {
        if (InventoryEntry mainHandItem = inventory.functionalEntry(ITEM_SLOT_MAIN_HAND)) {
            bool addOneDice = false;
            if (mainHandItem->skill() == SKILL_SPEAR &&
                !this->inventory.entry(ITEM_SLOT_OFF_HAND))  // using spear in two hands adds a dice roll
                addOneDice = true;

            mainWpnDmg = CalculateMeleeDmgToEnemyWithWeapon(
                mainHandItem.get(), uTargetActorID, addOneDice);
        }

        if (!ignoreOffhand) {
            if (InventoryEntry offHandItem = inventory.functionalEntry(ITEM_SLOT_OFF_HAND)) {  // has second hand got a weapon that not a shield
                if (!offHandItem->isShield()) {
                    offHndWpnDmg = CalculateMeleeDmgToEnemyWithWeapon(
                        offHandItem.get(), uTargetActorID, false);
                }
            }
        }
    }

    int dmgSum = mainWpnDmg + offHndWpnDmg;

    if (!ignoreSkillBonus) {
        int mightBonus = GetParameterBonus(GetActualMight());
        int mightAndSkillbonus =
            GetSkillBonus(ATTRIBUTE_MELEE_DMG_BONUS) + mightBonus;
        dmgSum += this->_melee_dmg_bonus +
                  GetMagicalBonus(ATTRIBUTE_MELEE_DMG_BONUS) +
                  mightAndSkillbonus;
    }

    if (dmgSum < 1) dmgSum = 1;

    return dmgSum;
}

int Character::CalculateMeleeDmgToEnemyWithWeapon(Item *weapon,
                                                  MonsterId uTargetActorID,
                                                  bool addOneDice) {
    ItemId itemId = weapon->itemId;
    int diceCount = pItemTable->items[itemId].damageDice;

    if (addOneDice) diceCount++;

    int diceSides = pItemTable->items[itemId].damageRoll;
    int diceResult = 0;

    for (int i = 0; i < diceCount; i++) {  // roll dice
        diceResult += grng->random(diceSides) + 1;
    }

    int totalDmg =
            pItemTable->items[itemId].damageMod + diceResult;  // add modifer

    if (uTargetActorID > MONSTER_INVALID) {  // if an actor has been provided
        ItemEnchantment enchType =
            weapon->specialEnchantment;  // check against enchantments

        if (supertypeForMonsterId(uTargetActorID) == MONSTER_SUPERTYPE_UNDEAD &&
            (enchType == ITEM_ENCHANTMENT_UNDEAD_SLAYING || itemId == ITEM_ARTIFACT_GHOULSBANE ||
             itemId == ITEM_ARTIFACT_GIBBET || itemId == ITEM_RELIC_JUSTICE)) {
            totalDmg *= 2;  // double damage vs undead
        } else if (supertypeForMonsterId(uTargetActorID) == MONSTER_SUPERTYPE_KREEGAN &&
                   (enchType == ITEM_ENCHANTMENT_DEMON_SLAYING || itemId == ITEM_ARTIFACT_GIBBET)) {
            totalDmg *= 2;  // double damage vs devils
        } else if (supertypeForMonsterId(uTargetActorID) == MONSTER_SUPERTYPE_DRAGON &&
                   (enchType == ITEM_ENCHANTMENT_DRAGON_SLAYING || itemId == ITEM_ARTIFACT_GIBBET)) {
            totalDmg *= 2;  // double damage vs dragons
        } else if (supertypeForMonsterId(uTargetActorID) == MONSTER_SUPERTYPE_ELF &&
                   (enchType == ITEM_ENCHANTMENT_ELF_SLAYING || itemId == ITEM_RELIC_OLD_NICK)) {
            totalDmg *= 2;  // double damage vs elf
        } else if (supertypeForMonsterId(uTargetActorID) == MONSTER_SUPERTYPE_TITAN &&
                   (enchType == ITEM_ENCHANTMENT_TITAN_SLAYING)) {
            totalDmg *= 2;  // double damage vs titan
        }
    }

    // master dagger triple damage backstab
    if (getActualSkillValue(SKILL_DAGGER).mastery() >= MASTERY_MASTER &&
        weapon->skill() == SKILL_DAGGER && grng->random(100) < 10)
        totalDmg *= 3;

    return totalDmg;
}

//----- (0048D0B9) --------------------------------------------------------
int Character::GetRangedAttack() {
    InventoryConstEntry mainHandItem = inventory.entry(ITEM_SLOT_MAIN_HAND);

    // blasters and charged wands
    if (mainHandItem && (isAncientWeapon(mainHandItem->itemId) || (mainHandItem->isWand() && mainHandItem->numCharges > 0))) {
        return GetActualAttack(true);
    } else { // bows
        int weapbonus = GetItemsBonus(ATTRIBUTE_RANGED_ATTACK) + GetParameterBonus(GetActualAccuracy());
        int skillbonus = GetSkillBonus(ATTRIBUTE_RANGED_ATTACK) + weapbonus;
        return this->_ranged_atk_bonus + GetMagicalBonus(ATTRIBUTE_RANGED_ATTACK) + skillbonus;
    }
}

//----- (0048D124) --------------------------------------------------------
int Character::GetRangedDamageMin() {
    int weapbonus = GetItemsBonus(ATTRIBUTE_RANGED_DMG_MIN);
    int skillbonus =
        GetSkillBonus(ATTRIBUTE_RANGED_DMG_BONUS) + weapbonus;
    int result = this->_ranged_dmg_bonus +
                 GetMagicalBonus(ATTRIBUTE_RANGED_DMG_BONUS) +
                 skillbonus;

    if (result < 0)  // cant be less than 0
        result = 0;

    return result;
}

//----- (0048D191) --------------------------------------------------------
int Character::GetRangedDamageMax() {
    int weapbonus = GetItemsBonus(ATTRIBUTE_RANGED_DMG_MAX);
    int skillbonus =
        GetSkillBonus(ATTRIBUTE_RANGED_DMG_BONUS) + weapbonus;
    int result = this->_ranged_dmg_bonus +
                 GetMagicalBonus(ATTRIBUTE_RANGED_DMG_BONUS) +
                 skillbonus;

    if (result < 0) result = 0;

    return result;
}

//----- (0048D1FE) --------------------------------------------------------
int Character::CalculateRangedDamageTo(MonsterId uMonsterInfoID) {
    InventoryEntry bow = inventory.functionalEntry(ITEM_SLOT_BOW);
    if (!bow)
        return 0;

    ItemEnchantment itemenchant = bow->specialEnchantment;

    signed int dmgperroll = pItemTable->items[bow->itemId].damageRoll;
    int damagefromroll = 0;
    int damage = 0;

    damagefromroll = grng->randomDice(pItemTable->items[bow->itemId].damageDice, dmgperroll);

    damage = pItemTable->items[bow->itemId].damageMod +
             damagefromroll;  // total damage

    if (uMonsterInfoID != MONSTER_INVALID) {  // check against bow enchantments
        if (itemenchant == ITEM_ENCHANTMENT_UNDEAD_SLAYING &&
            supertypeForMonsterId(uMonsterInfoID) == MONSTER_SUPERTYPE_UNDEAD) {  // double damage vs undead
            damage *= 2;
        } else if (itemenchant == ITEM_ENCHANTMENT_DEMON_SLAYING &&
                   supertypeForMonsterId(uMonsterInfoID) == MONSTER_SUPERTYPE_KREEGAN) {  // double vs devils
            damage *= 2;
        } else if (itemenchant == ITEM_ENCHANTMENT_DRAGON_SLAYING &&
                   supertypeForMonsterId(uMonsterInfoID) == MONSTER_SUPERTYPE_DRAGON) {  // double vs dragons
            damage *= 2;
        } else if (itemenchant == ITEM_ENCHANTMENT_ELF_SLAYING &&
                   supertypeForMonsterId(uMonsterInfoID) == MONSTER_SUPERTYPE_ELF) {  // double vs elf
            damage *= 2;
        } else if (itemenchant == ITEM_ENCHANTMENT_TITAN_SLAYING &&
                   supertypeForMonsterId(uMonsterInfoID) == MONSTER_SUPERTYPE_TITAN) { // double vs titans
            damage *= 2;
        }
    }

    return damage + this->GetSkillBonus(ATTRIBUTE_RANGED_DMG_BONUS);
}

//----- (0048D2EA) --------------------------------------------------------
std::string Character::GetMeleeDamageString() {
    int min_damage;
    int max_damage;

    InventoryConstEntry mainHandItem = inventory.entry(ITEM_SLOT_MAIN_HAND);

    if (mainHandItem && mainHandItem->isWand() && mainHandItem->numCharges > 0) {
        return std::string(localization->GetString(LSTR_WAND));
    } else if (mainHandItem && isAncientWeapon(mainHandItem->itemId)) {
        min_damage = GetItemsBonus(ATTRIBUTE_MELEE_DMG_MIN);  // blasters
        max_damage = GetItemsBonus(ATTRIBUTE_MELEE_DMG_MAX);
    } else {
        min_damage = GetMeleeDamageMinimal();  // weapons
        max_damage = GetMeleeDamageMaximal();
    }

    if (min_damage == max_damage) {
        return fmt::format("{}", min_damage);
    } else {
        return fmt::format("{} - {}", min_damage, max_damage);
    }
}

//----- (0048D396) --------------------------------------------------------
std::string Character::GetRangedDamageString() {
    int min_damage;
    int max_damage;

    InventoryConstEntry mainHandItem = inventory.entry(ITEM_SLOT_MAIN_HAND);

    if (mainHandItem && mainHandItem->isWand() && mainHandItem->numCharges > 0) {
        return std::string(localization->GetString(LSTR_WAND));
    } else if (mainHandItem && isAncientWeapon(mainHandItem->itemId)) {
        min_damage = GetItemsBonus(ATTRIBUTE_MELEE_DMG_MIN, true);  // blasters
        max_damage = GetItemsBonus(ATTRIBUTE_MELEE_DMG_MAX, true);
    } else {
        min_damage = GetRangedDamageMin();  // weaposn
        max_damage = GetRangedDamageMax();
    }

    if (max_damage > 0) {
        if (min_damage == max_damage) {
            return fmt::format("{}", min_damage);
        } else {
            return fmt::format("{} - {}", min_damage, max_damage);
        }
    } else {
        return std::string("N/A");  // no bow equipped
    }
}

//----- (0048D45A) --------------------------------------------------------
bool Character::CanTrainToNextLevel() {
    int lvl = this->uLevel + 1;
    int neededExp = ((lvl * (lvl - 1)) / 2 * 1000);
    return this->experience >= neededExp;
}

//----- (0048D498) --------------------------------------------------------
Color Character::GetExperienceDisplayColor() {
    if (CanTrainToNextLevel())
        return ui_character_bonus_text_color;
    else
        return ui_character_default_text_color;
}

//----- (0048D4B3) --------------------------------------------------------
int Character::CalculateIncommingDamage(DamageType dmg_type, int dmg) {
    // TODO(captainurist): these are some weird casts to CharacterAttributeType
    if (classType == CLASS_LICH &&
        ((Attribute)dmg_type == ATTRIBUTE_RESIST_MIND ||
         (Attribute)dmg_type == ATTRIBUTE_RESIST_BODY ||
         (Attribute)dmg_type == ATTRIBUTE_RESIST_SPIRIT))  // TODO(_): determine if spirit
                                                          // resistance should be handled
                                                          // by body res. modifier
        return 0;  // liches are not affected by self magics

    int resist_value = 0;
    switch (dmg_type) {  // get resistance
        case DAMAGE_FIRE:
            resist_value = GetActualResistance(ATTRIBUTE_RESIST_FIRE);
            break;
        case DAMAGE_AIR:
            resist_value = GetActualResistance(ATTRIBUTE_RESIST_AIR);
            break;
        case DAMAGE_WATER:
            resist_value = GetActualResistance(ATTRIBUTE_RESIST_WATER);
            break;
        case DAMAGE_EARTH:
            resist_value = GetActualResistance(ATTRIBUTE_RESIST_EARTH);
            break;
        case DAMAGE_SPIRIT:
            resist_value = GetActualResistance(ATTRIBUTE_RESIST_SPIRIT);
            break;
        case DAMAGE_MIND:
            resist_value = GetActualResistance(ATTRIBUTE_RESIST_MIND);
            break;
        case DAMAGE_BODY:
            resist_value = GetActualResistance(ATTRIBUTE_RESIST_BODY);
            break;
        default:
            break;
    }

    int player_luck = GetActualLuck();
    signed int res_rand_divider =
        GetParameterBonus(player_luck) + resist_value + 30;

    if (GetParameterBonus(player_luck) + resist_value > 0) {
        for (int i = 0; i < 4; i++) {
            if (grng->random(res_rand_divider) >= 30)
                dmg /= 2;  // damage reduction on successful check
            else
                break;
        }
    }

    InventoryConstEntry armor = inventory.functionalEntry(ITEM_SLOT_ARMOUR);
    if (dmg_type == DAMAGE_PHYSICAL && armor) { // physical damage and wearing armour
        Skill armor_skill = armor->skill();

        // master and above half incoming damage
        if (armor_skill == SKILL_PLATE) {
            if (getActualSkillValue(SKILL_PLATE).mastery() >= MASTERY_MASTER)
                return dmg / 2;
        }

        // grandmaster and chain damage reduce
        if (armor_skill == SKILL_CHAIN) {
            if (getActualSkillValue(SKILL_CHAIN).mastery() == MASTERY_GRANDMASTER)
                return dmg * 2 / 3;
        }
    }

    return dmg;
}

//----- (0048D676) --------------------------------------------------------
bool Character::IsUnarmed() const {
    InventoryConstEntry mainHandItem = inventory.functionalEntry(ITEM_SLOT_MAIN_HAND);
    InventoryConstEntry offHandItem = inventory.functionalEntry(ITEM_SLOT_OFF_HAND);
    return !mainHandItem && (!offHandItem || offHandItem->isShield());
}

//----- (0048D6D0) --------------------------------------------------------
bool Character::wearsEnchantedItem(ItemEnchantment enchantment) const {
    assert(enchantment != ITEM_ENCHANTMENT_NULL);
    for (InventoryConstEntry entry : inventory.functionalEquipment())
        if (entry->specialEnchantment == enchantment)
            return true;
    return false;
}

//----- (0048D709) --------------------------------------------------------
bool Character::wearsItem(ItemId itemId) const {
    assert(itemId != ITEM_NULL);
    for (ItemSlot slot : itemSlotsForItemType(pItemTable->items[itemId].type))
        if (InventoryConstEntry entry = inventory.functionalEntry(slot); entry && entry->itemId == itemId)
            return true;
    return false;
}

//----- (0048D76C) --------------------------------------------------------
int Character::StealFromShop(
    Item *itemToSteal, int extraStealDifficulty, int reputation,
    int extraStealFine,
    int *fineIfFailed) {  // returns an int, but is the return value is compared
                          // to zero, so might change to bool

    // fineiffailed is changed!

    if (!itemToSteal || !CanAct()) {
        return 0;  // no item or cant act - no stealing
    } else {
        CombinedSkillValue val = this->getActualSkillValue(SKILL_STEALING);
        int itemValue = itemToSteal->GetValue();

        if (itemToSteal->isWeapon())
            itemValue *= 3;

        int currMaxItemValue = StealingRandomBonuses[grng->random(5)] + val.level() * StealingMasteryBonuses[val.mastery()];
        *fineIfFailed = 100 * (reputation + extraStealDifficulty) + itemValue;

        if (extraStealFine) {
            *fineIfFailed += 500;
        }

        if (grng->random(100) >= 5) {
            if (*fineIfFailed > currMaxItemValue) {
                if (*fineIfFailed - currMaxItemValue < 500) {
                    return 1;  // fail with item
                } else {
                    return 0;  // failed no item
                }
            } else {
                return 2;  // perfect crime
            }
        } else {
            return 0;  // failed no item
        }
    }
}

//----- (0048D88B) --------------------------------------------------------
StealResult Character::StealFromActor(unsigned int uActorID, int _steal_perm, int reputation) {
    // TODO(captainurist): returns not used - should luck attribute affect?

    Actor *actroPtr;
    actroPtr = &pActors[uActorID];

    if (!actroPtr || !CanAct()) {  // no actor to steal from or character cant act
        return STEAL_BUSTED;
    }

    if (!actroPtr->ActorHasItem())  // if actor does not have an item
        actroPtr->SetRandomGoldIfTheresNoItem();  // add some gold

    CombinedSkillValue stealingSkill = this->getActualSkillValue(SKILL_STEALING);
    int currMaxItemValue = StealingRandomBonuses[grng->random(5)] + stealingSkill.level() * StealingMasteryBonuses[stealingSkill.mastery()];
    int fineIfFailed = actroPtr->monsterInfo.level + 100 * (_steal_perm + reputation);

    if (grng->random(100) < 5 || fineIfFailed > currMaxItemValue ||
        actroPtr->ActorEnemy()) {  // busted
        Actor::AggroSurroundingPeasants(uActorID, 1);
        engine->_statusBar->setEvent(LSTR_S_WAS_CAUGHT_STEALING, this->name);
        return STEAL_BUSTED;
    } else {
        int random = grng->random(100);

        if (random >= 70) {  // stealing gold
            if (!actroPtr->items[3].isGold()) {
                // no gold to steal - fail
                engine->_statusBar->setEvent(LSTR_S_FAILED_TO_STEAL_ANYTHING, this->name);
                return STEAL_NOTHING;
            }

            int stolenGold = grng->randomDice(stealingSkill.level(), goldStealingDieSidesByMastery[stealingSkill.mastery()]);

            int *goldPtr = &actroPtr->items[3].goldAmount;  // actor has this amount of gold

            if (stolenGold >= *goldPtr) {  // steal all the gold
                stolenGold = *goldPtr;
                actroPtr->items[3].itemId = ITEM_NULL;
                *goldPtr = 0;
            } else {
                *goldPtr -= stolenGold;  // steal some of the gold
            }

            if (stolenGold) {
                pParty->partyFindsGold(stolenGold, GOLD_RECEIVE_NOSHARE_SILENT);
                engine->_statusBar->setEvent(LSTR_S_STOLE_D_GOLD, this->name, stolenGold);
            } else {
                engine->_statusBar->setEvent(LSTR_S_FAILED_TO_STEAL_ANYTHING, this->name);
            }

            return STEAL_SUCCESS;
        } else if (random >= 40) {  // stealing an item
            Item tempItem;
            tempItem.Reset();

            int randslot = grng->random(4);
            ItemId carriedItemId = actroPtr->carriedItemId;

            // check if we have an item to steal
            if (carriedItemId != ITEM_NULL || actroPtr->items[randslot].itemId != ITEM_NULL && !actroPtr->items[randslot].isGold()) {
                if (carriedItemId != ITEM_NULL) {  // load item into tempitem
                    actroPtr->carriedItemId = ITEM_NULL;
                    tempItem.itemId = carriedItemId;
                    tempItem.postGenerate(ITEM_SOURCE_MONSTER);
                } else {
                    Item *itemToSteal = &actroPtr->items[randslot];
                    tempItem = *itemToSteal;
                    itemToSteal->Reset();
                    carriedItemId = tempItem.itemId;
                }

                if (carriedItemId != ITEM_NULL) {
                    engine->_statusBar->setEvent(LSTR_FMT_S_STOLE_D_ITEM, this->name, pItemTable->items[carriedItemId].unidentifiedName);
                    pParty->setHoldingItem(tempItem);
                    return STEAL_SUCCESS;
                }
            }
        }

        engine->_statusBar->setEvent(LSTR_S_FAILED_TO_STEAL_ANYTHING, this->name);
        return STEAL_NOTHING;
    }
}

//----- (0048DBB9) --------------------------------------------------------
void Character::Heal(int amount) {
    if (!IsEradicated() && !IsDead()) {  // cant heal
        int max_health = GetMaxHealth();

        if (IsZombie())  // zombie health is halved
            max_health /= 2;

        health += amount;         // add health
        if (health > max_health)  // limits check
            health = max_health;

        if (IsUnconcious()) {
            if (health > 0) {  // wake up if health rises above 0
                conditions.reset(CONDITION_UNCONSCIOUS);
            }
        }
    }
}

int Character::receiveDamage(signed int amount, DamageType dmg_type) {
    conditions.reset(CONDITION_SLEEP);  // wake up if asleep
    signed int recieved_dmg = CalculateIncommingDamage(dmg_type, amount);  // get damage
    // for no damage cheat - moved from elsewhere
    if (!engine->config->debug.NoDamage.value()) {
        health -= recieved_dmg;     // reduce health
    }

    if (health < 1) {  // character unconscious or if too hurt - dead
        if ((health + GetBaseEndurance() >= 1) ||
            pCharacterBuffs[CHARACTER_BUFF_PRESERVATION].Active()) {
            SetCondUnconsciousWithBlockCheck(false);
        } else {
            SetCondDeadWithBlockCheck(false);
        }

        // break armor if health has dropped below -10 (if it's not hardened).
        if (health <= -10) {
            if (InventoryEntry armor = inventory.entry(ITEM_SLOT_ARMOUR); armor && !(armor->flags & ITEM_HARDENED))
                armor->SetBroken();
        }
    }

    if (recieved_dmg && CanAct()) {
        playReaction(SPEECH_DAMAGED);  // oww
    }

    return recieved_dmg;
}

//----- (0048DCF6) --------------------------------------------------------
int Character::ReceiveSpecialAttackEffect(MonsterSpecialAttack attType, Actor *pActor) {  // long function - consider breaking into two??
    if (engine->callObserver) {
        engine->callObserver->notify(CALL_SPECIAL_ATTACK, attType);
    }

    int statcheck;
    int statcheckbonus;
    int luckstat = GetActualLuck();
    std::vector<InventoryEntry> itemstobreaklist;
    InventoryEntry itemtobreak;
    InventoryEntry itemtostealinvindex;

    switch (attType) {
        case SPECIAL_ATTACK_CURSE:
            statcheck = GetActualPersonality();
            statcheckbonus = GetParameterBonus(statcheck);
            break;

        case SPECIAL_ATTACK_WEAK:
        case SPECIAL_ATTACK_SLEEP:
        case SPECIAL_ATTACK_DRUNK:
        case SPECIAL_ATTACK_DISEASE_WEAK:
        case SPECIAL_ATTACK_DISEASE_MEDIUM:
        case SPECIAL_ATTACK_DISEASE_SEVERE:
        case SPECIAL_ATTACK_UNCONSCIOUS:
        case SPECIAL_ATTACK_AGING:
            statcheck = GetActualEndurance();
            statcheckbonus = GetParameterBonus(statcheck);
            break;

        case SPECIAL_ATTACK_INSANE:
        case SPECIAL_ATTACK_PARALYZED:
        case SPECIAL_ATTACK_FEAR:
            statcheckbonus =
                GetActualResistance(ATTRIBUTE_RESIST_MIND);
            break;

        case SPECIAL_ATTACK_PETRIFIED:
            statcheckbonus =
                GetActualResistance(ATTRIBUTE_RESIST_EARTH);
            break;

        case SPECIAL_ATTACK_POISON_WEAK:
        case SPECIAL_ATTACK_POISON_MEDIUM:
        case SPECIAL_ATTACK_POISON_SEVERE:
        case SPECIAL_ATTACK_DEAD:
        case SPECIAL_ATTACK_ERADICATED:
            statcheckbonus =
                GetActualResistance(ATTRIBUTE_RESIST_BODY);
            break;

        case SPECIAL_ATTACK_MANA_DRAIN:
            statcheckbonus = (GetParameterBonus(GetActualIntelligence()) +
                              GetParameterBonus(GetActualPersonality())) /
                             2;
            break;

        case SPECIAL_ATTACK_BREAK_ANY:
            // TODO(captainurist): can't break wands b/c they are not regular items. Makes little in-game sense IMO.
            for (InventoryEntry entry : inventory.entries())
                if (isRegular(entry->itemId) && !entry->IsBroken())
                    itemstobreaklist.push_back(entry);

            if (itemstobreaklist.empty()) return 0;

            itemtobreak = grng->randomSample(itemstobreaklist);

            statcheckbonus =
                3 * (std::to_underlying(pItemTable->items[itemtobreak->itemId].rarity) +
                     itemtobreak->GetDamageMod());
            break;

        case SPECIAL_ATTACK_BREAK_ARMOR:
            // TODO(captainurist): This can break a wetsuit, and this looks like vanilla behavior. But the code in
            //                     SPECIAL_ATTACK_BREAK_ANY can't break a wetsuit. Huh.
            for (InventoryEntry entry : inventory.equipment())
                if (!entry->IsBroken() && (entry->type() == ITEM_TYPE_ARMOUR || entry->type() == ITEM_TYPE_SHIELD))
                    itemstobreaklist.push_back(entry);

            if (itemstobreaklist.empty()) return 0;

            itemtobreak = grng->randomSample(itemstobreaklist);
            statcheckbonus =
                3 * (std::to_underlying(pItemTable->items[itemtobreak->itemId].rarity) +
                     itemtobreak->GetDamageMod());
            break;

        case SPECIAL_ATTACK_BREAK_WEAPON:
            // TODO(captainurist): why doesn't this affect wands?
            for (InventoryEntry entry : inventory.equipment())
                if (!entry->IsBroken() && (entry->type() == ITEM_TYPE_BOW || entry->type() == ITEM_TYPE_SINGLE_HANDED || entry->type() == ITEM_TYPE_TWO_HANDED))
                    itemstobreaklist.push_back(entry);

            if (!itemstobreaklist.empty()) return 0;

            itemtobreak = grng->randomSample(itemstobreaklist);
            statcheckbonus =
                3 * (std::to_underlying(pItemTable->items[itemtobreak->itemId].rarity) +
                     itemtobreak->GetDamageMod());
            break;

        case SPECIAL_ATTACK_STEAL:
            for (InventoryEntry entry : inventory.entries())
                if (isRegular(entry->itemId))
                    itemstobreaklist.push_back(entry);

            if (itemstobreaklist.empty()) return 0;

            itemtostealinvindex = grng->randomSample(itemstobreaklist);
            statcheck = GetActualAccuracy();
            statcheckbonus = GetParameterBonus(statcheck);
            break;

        default:
            statcheckbonus = 0;
            break;
    }

    signed int savecheck = GetParameterBonus(luckstat) + statcheckbonus + 30;

    if (grng->random(savecheck) >= 30) {  // saving throw if attacke is avoided
        return 0;
    } else {
        int whichplayer = pParty->getCharacterIdInParty(this);

        // pass this to new fucntion??
        // atttypecast - whichplayer - itemtobreak - itemtostealinvindex

        switch (attType) {
            case SPECIAL_ATTACK_CURSE:
                SetCondition(CONDITION_CURSED, 1);
                pAudioPlayer->playUISound(SOUND_star1);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_WEAK:
                SetCondition(CONDITION_WEAK, 1);
                pAudioPlayer->playUISound(SOUND_star1);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_SLEEP:
                SetCondition(CONDITION_SLEEP, 1);
                pAudioPlayer->playUISound(SOUND_star1);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DRUNK:
                SetCondition(CONDITION_DRUNK, 1);
                pAudioPlayer->playUISound(SOUND_star1);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_INSANE:
                SetCondition(CONDITION_INSANE, 1);
                pAudioPlayer->playUISound(SOUND_star4);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_POISON_WEAK:
                SetCondition(CONDITION_POISON_WEAK, 1);
                pAudioPlayer->playUISound(SOUND_star2);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_POISON_MEDIUM:
                SetCondition(CONDITION_POISON_MEDIUM, 1);
                pAudioPlayer->playUISound(SOUND_star2);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_POISON_SEVERE:
                SetCondition(CONDITION_POISON_SEVERE, 1);
                pAudioPlayer->playUISound(SOUND_star2);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DISEASE_WEAK:
                SetCondition(CONDITION_DISEASE_WEAK, 1);
                pAudioPlayer->playUISound(SOUND_star2);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DISEASE_MEDIUM:
                SetCondition(CONDITION_DISEASE_MEDIUM, 1);
                pAudioPlayer->playUISound(SOUND_star2);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DISEASE_SEVERE:
                SetCondition(CONDITION_DISEASE_SEVERE, 1);
                pAudioPlayer->playUISound(SOUND_star2);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_PARALYZED:
                SetCondition(CONDITION_PARALYZED, 1);
                pAudioPlayer->playUISound(SOUND_star4);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_UNCONSCIOUS:
                SetCondition(CONDITION_UNCONSCIOUS, 1);
                pAudioPlayer->playUISound(SOUND_star4);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_DEAD:
                SetCondition(CONDITION_DEAD, 1);
                pAudioPlayer->playUISound(SOUND_eradicate);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_PETRIFIED:
                SetCondition(CONDITION_PETRIFIED, 1);
                pAudioPlayer->playUISound(SOUND_eradicate);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_ERADICATED:
                SetCondition(CONDITION_ERADICATED, 1);
                pAudioPlayer->playUISound(SOUND_eradicate);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_BREAK_ANY:
            case SPECIAL_ATTACK_BREAK_ARMOR:
            case SPECIAL_ATTACK_BREAK_WEAPON:
                if (!(itemtobreak->flags & ITEM_HARDENED)) {
                    playReaction(SPEECH_ITEM_BROKEN);
                    itemtobreak->SetBroken();
                    pAudioPlayer->playUISound(SOUND_metal_vs_metal03h);
                }
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_STEAL: {
                playReaction(SPEECH_ITEM_BROKEN);
                Item *actoritems = &pActor->items[0];
                if (pActor->items[0].itemId != ITEM_NULL) {
                    actoritems = &pActor->items[1];
                    if (pActor->items[1].itemId != ITEM_NULL) {
                        spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                        return 1;
                    }
                }

                *actoritems = inventory.take(itemtostealinvindex);
                pAudioPlayer->playUISound(SOUND_metal_vs_metal03h);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;
            }

            case SPECIAL_ATTACK_AGING:
                playReaction(SPEECH_AGING);
                ++this->sAgeModifier;
                pAudioPlayer->playUISound(SOUND_eleccircle);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_MANA_DRAIN:
                playReaction(SPEECH_SP_DRAINED);
                this->mana = 0;
                pAudioPlayer->playUISound(SOUND_eleccircle);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            case SPECIAL_ATTACK_FEAR:
                SetCondition(CONDITION_FEAR, 1);
                pAudioPlayer->playUISound(SOUND_star1);
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_DISEASE, whichplayer);
                return 1;
                break;

            default:
                return 0;
        }
    }
}

// 48DCF6: using guessed type char var_94[140];

//----- (0048E1B5) --------------------------------------------------------
Duration Character::GetAttackRecoveryTime(bool attackUsesBow) const {
    InventoryConstEntry weapon;
    Duration weapon_recovery = base_recovery_times_per_weapon_type[SKILL_STAFF];
    if (attackUsesBow) {
        weapon = inventory.functionalEntry(ITEM_SLOT_BOW);
        assert(weapon);
        weapon_recovery = base_recovery_times_per_weapon_type[weapon->skill()];
    } else if (IsUnarmed() && getActualSkillValue(SKILL_UNARMED).level() > 0) {
        weapon_recovery = base_recovery_times_per_weapon_type[SKILL_UNARMED];
    } else if (weapon = inventory.functionalEntry(ITEM_SLOT_MAIN_HAND)) {
        if (weapon->isWand()) {
            weapon_recovery = pSpellDatas[spellForWand(weapon->itemId)].recovery_per_skill[MASTERY_EXPERT];
        } else {
            weapon_recovery = base_recovery_times_per_weapon_type[weapon->skill()];
        }
    }

    // TODO(captainurist): I don't like this logic. We first take the weapon with larger recovery time, then apply
    //                     recovery bonuses. Should be the other way around.

    Duration shield_recovery;
    if (InventoryConstEntry offHandItem = inventory.functionalEntry(ITEM_SLOT_OFF_HAND)) {
        if (offHandItem->isShield()) {
            Skill skill_type = offHandItem->skill();
            Duration shield_base_recovery = base_recovery_times_per_weapon_type[skill_type];
            float multiplier = GetArmorRecoveryMultiplierFromSkillLevel(skill_type, 1.0f, 0, 0, 0);
            shield_recovery = shield_base_recovery * multiplier;
        } else {
            if (base_recovery_times_per_weapon_type[offHandItem->skill()] > weapon_recovery) {
                weapon = offHandItem;
                weapon_recovery = base_recovery_times_per_weapon_type[weapon->skill()];
            }
        }
    }

    Duration armour_recovery;
    if (InventoryConstEntry armor = inventory.functionalEntry(ITEM_SLOT_ARMOUR)) {
        Skill armour_skill_type = armor->skill();
        Duration base_armour_recovery = base_recovery_times_per_weapon_type[armour_skill_type];
        float multiplier;

        if (armour_skill_type == SKILL_LEATHER) {
            multiplier = GetArmorRecoveryMultiplierFromSkillLevel(armour_skill_type, 1.0f, 0, 0, 0);
        } else if (armour_skill_type == SKILL_CHAIN) {
            multiplier = GetArmorRecoveryMultiplierFromSkillLevel(armour_skill_type, 1.0f, 0.5f, 0, 0);
        } else if (armour_skill_type == SKILL_PLATE) {
            multiplier = GetArmorRecoveryMultiplierFromSkillLevel(armour_skill_type, 1.0f, 0.5f, 0.5f, 0);
        } else {
            assert(armour_skill_type == SKILL_MISC && armor->itemId == ITEM_QUEST_WETSUIT);
            multiplier = GetArmorRecoveryMultiplierFromSkillLevel(armour_skill_type, 1.0f, 1.0f, 1.0f, 1.0f);
        }

        armour_recovery = base_armour_recovery * multiplier;
    }

    Duration player_speed_recovery_reduction = Duration::fromTicks(GetParameterBonus(GetActualSpeed()));

    Duration sword_axe_bow_recovery_reduction;
    if (weapon) {
        CombinedSkillValue weaponSkill = getActualSkillValue(weapon->skill());
        if (weaponSkill.level() > 0 &&
            (weapon->skill() == SKILL_SWORD ||
             weapon->skill() == SKILL_AXE ||
             weapon->skill() == SKILL_BOW)) {
            // Expert Sword, Axe & Bow reduce recovery
            if (weaponSkill.mastery() >= MASTERY_EXPERT)
                sword_axe_bow_recovery_reduction = Duration::fromTicks(weaponSkill.level());
        }
    }

    bool shooting_laser = weapon && weapon->skill() == SKILL_BLASTER;
    assert(!shooting_laser || !attackUsesBow); // For blasters we expect attackUsesBow == false.

    Duration armsmaster_recovery_reduction;
    if (!attackUsesBow && !shooting_laser) {
        CombinedSkillValue armsmasterSkill = getActualSkillValue(SKILL_ARMSMASTER);
        if (armsmasterSkill.level() > 0) {
            armsmaster_recovery_reduction = Duration::fromTicks(armsmasterSkill.level());
            if (armsmasterSkill.mastery() >= MASTERY_GRANDMASTER)
                armsmaster_recovery_reduction *= 2;
        }
    }

    Duration hasteRecoveryReduction;
    if (pCharacterBuffs[CHARACTER_BUFF_HASTE].Active())
        hasteRecoveryReduction = 25_ticks;
    if (pParty->pPartyBuffs[PARTY_BUFF_HASTE].Active())
        hasteRecoveryReduction = 25_ticks;

    Duration weapon_enchantment_recovery_reduction;
    if (weapon) {
        if (weapon->specialEnchantment == ITEM_ENCHANTMENT_SWIFT ||
            weapon->specialEnchantment == ITEM_ENCHANTMENT_OF_DARKNESS ||
            weapon->itemId == ITEM_ARTIFACT_PUCK)
            weapon_enchantment_recovery_reduction = 20_ticks;
    }

    Duration recovery = weapon_recovery + armour_recovery + shield_recovery -
                   armsmaster_recovery_reduction -
                   weapon_enchantment_recovery_reduction -
                   hasteRecoveryReduction - sword_axe_bow_recovery_reduction -
                   player_speed_recovery_reduction;

    Duration minRecovery;
    if (shooting_laser) {
        minRecovery = Duration::fromTicks(engine->config->gameplay.MinRecoveryBlasters.value());
    } else if (attackUsesBow) {
        minRecovery = Duration::fromTicks(engine->config->gameplay.MinRecoveryRanged.value());
    } else {
        minRecovery = Duration::fromTicks(engine->config->gameplay.MinRecoveryMelee.value());
    }

    if (recovery < minRecovery)
        recovery = minRecovery;

    return recovery;
}

//----- new --------------------------------------------------------
float Character::GetArmorRecoveryMultiplierFromSkillLevel(Skill armour_skill_type, float mult1, float mult2, float mult3, float mult4) const {
    Mastery skillMastery = getSkillValue(armour_skill_type).mastery();

    switch (skillMastery) {
        case MASTERY_NOVICE:
            return mult1;
        case MASTERY_EXPERT:
            return mult2;
        case MASTERY_MASTER:
            return mult3;
        case MASTERY_GRANDMASTER:
            return mult4;
        default:
            assert(false);
            return 0;
    }
}

//----- (0048E4F8) --------------------------------------------------------
int Character::GetMaxHealth() const {
    int endbonus = GetParameterBonus(GetActualEndurance());
    int healthbylevel =
        pBaseHealthPerLevelByClass[classType] * (GetActualLevel() + endbonus);
    int itembonus = GetItemsBonus(ATTRIBUTE_HEALTH) + healthbylevel;
    int maxhealth = uFullHealthBonus + pBaseHealthByClass[std::to_underlying(classType) / 4] +
                    GetSkillBonus(ATTRIBUTE_HEALTH) + itembonus;

    if (maxhealth < 0)  // min zero
        maxhealth = 0;

    return maxhealth;
}

//----- (0048E565) --------------------------------------------------------
int Character::GetMaxMana() const {
    int mainmanastat;
    int statbonus;
    int addmanastat;

    switch (classType) {
        case CLASS_ROGUE:
        case CLASS_SPY:
        case CLASS_ASSASSIN:
        case CLASS_ARCHER:
        case CLASS_WARRIOR_MAGE:
        case CLASS_MASTER_ARCHER:
        case CLASS_SNIPER:
        case CLASS_SORCERER:
        case CLASS_WIZARD:
        case CLASS_ARCHAMGE:
        case CLASS_LICH:
            // intelligence based mana
            mainmanastat = GetActualIntelligence();
            statbonus = GetParameterBonus(mainmanastat);
            break;

        case CLASS_INITIATE:
        case CLASS_MASTER:
        case CLASS_NINJA:
        case CLASS_PALADIN:
        case CLASS_CRUSADER:
        case CLASS_HERO:
        case CLASS_VILLIAN:
        case CLASS_CLERIC:
        case CLASS_PRIEST:
        case CLASS_PRIEST_OF_SUN:
        case CLASS_PRIEST_OF_MOON:
            // personality based mana
            mainmanastat = GetActualPersonality();
            statbonus = GetParameterBonus(mainmanastat);
            break;

        case CLASS_HUNTER:
        case CLASS_RANGER_LORD:
        case CLASS_BOUNTY_HUNTER:
        case CLASS_DRUID:
        case CLASS_GREAT_DRUID:
        case CLASS_ARCH_DRUID:
        case CLASS_WARLOCK:
            // mixed base mana
            mainmanastat = GetActualPersonality();
            statbonus = GetParameterBonus(mainmanastat);
            addmanastat = GetActualIntelligence();
            statbonus += GetParameterBonus(addmanastat);
            break;

        default:  // no magic
            return 0;
            break;
    }

    int manabylevel =
        pBaseManaPerLevelByClass[classType] * (GetActualLevel() + statbonus);
    int itembonus = GetItemsBonus(ATTRIBUTE_MANA) + manabylevel;
    int maxmana = uFullManaBonus + pBaseManaByClass[std::to_underlying(classType) / 4] +
                  GetSkillBonus(ATTRIBUTE_MANA) + itembonus;

    if (maxmana < 0)  // min of 0
        maxmana = 0;

    return maxmana;
}

//----- (0048E656) --------------------------------------------------------
int Character::GetBaseAC() const {
    int spd = GetActualSpeed();
    int spdbonus = GetParameterBonus(spd);
    int itembonus = GetItemsBonus(ATTRIBUTE_AC_BONUS) + spdbonus;
    int skillbonus = GetSkillBonus(ATTRIBUTE_AC_BONUS) + itembonus;

    if (skillbonus < 0)  // min zero
        skillbonus = 0;

    return skillbonus;
}

//----- (0048E68F) --------------------------------------------------------
int Character::GetActualAC() const {
    int spd = GetActualSpeed();
    int spdbonus = GetParameterBonus(spd);
    int itembonus = GetItemsBonus(ATTRIBUTE_AC_BONUS) + spdbonus;
    int skillbonus = GetSkillBonus(ATTRIBUTE_AC_BONUS) + itembonus;

    int result = this->sACModifier + GetMagicalBonus(ATTRIBUTE_AC_BONUS) + skillbonus;

    if (result < 0)  // min zero
        result = 0;

    return result;
}

//----- (0048E6DC) --------------------------------------------------------
unsigned int Character::GetBaseAge() const {
    return pParty->GetPlayingTime().toYears() - this->uBirthYear + game_starting_year;
}

//----- (0048E72C) --------------------------------------------------------
unsigned int Character::GetActualAge() const {
    return this->sAgeModifier + GetBaseAge();
}

//----- (0048E73F) --------------------------------------------------------
int Character::GetBaseResistance(Attribute a2) const {
    int v7;  // esi@20
    int racialBonus = 0;
    const int16_t *resStat;
    int result;

    switch (a2) {
        case ATTRIBUTE_RESIST_FIRE:
            resStat = &sResFireBase;
            if (IsRaceGoblin()) racialBonus = 5;
            break;
        case ATTRIBUTE_RESIST_AIR:
            resStat = &sResAirBase;
            if (IsRaceGoblin()) racialBonus = 5;
            break;
        case ATTRIBUTE_RESIST_WATER:
            resStat = &sResWaterBase;
            if (IsRaceDwarf()) racialBonus = 5;
            break;
        case ATTRIBUTE_RESIST_EARTH:
            resStat = &sResEarthBase;
            if (IsRaceDwarf()) racialBonus = 5;
            break;
        case ATTRIBUTE_RESIST_MIND:
            resStat = &sResMindBase;
            if (IsRaceElf()) racialBonus = 10;
            break;
        case ATTRIBUTE_RESIST_BODY:
        case ATTRIBUTE_RESIST_SPIRIT:
            resStat = &sResBodyBase;
            if (IsRaceHuman()) racialBonus = 5;
            break;
        default:
            assert(false);
            return 0;
    }
    v7 = GetItemsBonus(a2) + racialBonus;
    result = v7 + *resStat;
    if (classType == CLASS_LICH) {
        if (result > 200) result = 200;
    }
    return result;
}

//----- (0048E7D0) --------------------------------------------------------
int Character::GetActualResistance(Attribute resistance) const {
    signed int v10 = 0;  // [sp+14h] [bp-4h]@1
    const int16_t *resStat;
    int result;
    int baseRes;

    CombinedSkillValue leatherSkill = getActualSkillValue(SKILL_LEATHER);

    if (CheckHiredNPCSpeciality(Enchanter)) v10 = 20;
    if ((resistance == ATTRIBUTE_RESIST_FIRE ||
         resistance == ATTRIBUTE_RESIST_AIR ||
         resistance == ATTRIBUTE_RESIST_WATER ||
         resistance == ATTRIBUTE_RESIST_EARTH) &&
        leatherSkill.mastery() == MASTERY_GRANDMASTER &&
        inventory.functionalEntry(ITEM_SLOT_ARMOUR) &&
        inventory.functionalEntry(ITEM_SLOT_ARMOUR)->skill() == SKILL_LEATHER)
        v10 += leatherSkill.level();

    switch (resistance) {
        case ATTRIBUTE_RESIST_FIRE:
            resStat = &sResFireBonus;
            break;
        case ATTRIBUTE_RESIST_AIR:
            resStat = &sResAirBonus;
            break;
        case ATTRIBUTE_RESIST_WATER:
            resStat = &sResWaterBonus;
            break;
        case ATTRIBUTE_RESIST_EARTH:
            resStat = &sResEarthBonus;
            break;
        case ATTRIBUTE_RESIST_MIND:
            resStat = &sResMindBonus;
            break;
        case ATTRIBUTE_RESIST_BODY:
        case ATTRIBUTE_RESIST_SPIRIT:
            resStat = &sResBodyBonus;
            break;
        default:
            assert(false);
            return 0;
    }
    baseRes = GetBaseResistance(resistance);
    result = v10 + GetMagicalBonus(resistance) + baseRes + *(resStat);
    if (classType == CLASS_LICH) {
        if (result > 200) result = 200;
    }
    return result;
}

//----- (0048E8F5) --------------------------------------------------------
bool Character::Recover(Duration dt) {
    Duration timepassed =
        dt * (100 + GetSpecialItemBonus(ITEM_ENCHANTMENT_OF_RECOVERY)) / 100;

    if (timeToRecovery > timepassed) {  // need more time till recovery
        timeToRecovery -= timepassed;
        return true;
    } else {
        timeToRecovery = 0_ticks;  // recovered

        if (!pParty->hasActiveCharacter())  // set recoverd char as active
            pParty->switchToNextActiveCharacter();

        return false;
    }
}

//----- (0048E96A) --------------------------------------------------------
void Character::SetRecoveryTime(Duration rec) {
    // to avoid switching characters if endurance eliminates hit recovery
    if (rec <= 0_ticks) return;

    if (rec > timeToRecovery) timeToRecovery = rec;

    if (pParty->hasActiveCharacter() && &pParty->activeCharacter() == this &&
        !enchantingActiveCharacter)
        pParty->switchToNextActiveCharacter();
}

//----- (0048E9B7) --------------------------------------------------------
void Character::RandomizeName() {
    if (!portraitTimePassed)
        name = pNPCStats->pNPCNames[grng->random(pNPCStats->uNumNPCNames[uSex])][uSex];
}

//----- (0048E9F4) --------------------------------------------------------
Condition Character::GetMajorConditionIdx() const {
    for (Condition condition : conditionImportancyTable()) {
        if (conditions.has(condition))
            return condition;  // return worst condition
    }
    return CONDITION_GOOD;  // condition good
}

//----- (0048EA1B) --------------------------------------------------------
int Character::GetParameterBonus(int player_parameter) const {
    int i;  // eax@1
    i = 0;
    while (param_to_bonus_table[i]) {
        if (player_parameter >= param_to_bonus_table[i]) break;
        ++i;
    }
    return parameter_to_bonus_value[i];
}

//----- (0048EA46) --------------------------------------------------------
int Character::GetSpecialItemBonus(ItemEnchantment enchantment) const {
    for (InventoryConstEntry entry : inventory.functionalEquipment()) {
        if (enchantment == ITEM_ENCHANTMENT_OF_RECOVERY) {
            if (entry->specialEnchantment == ITEM_ENCHANTMENT_OF_RECOVERY ||
                entry->itemId == ITEM_ARTIFACT_ELVEN_CHAINMAIL)
                return 50;
        }

        if (enchantment == ITEM_ENCHANTMENT_OF_FORCE) {
            if (entry->specialEnchantment == ITEM_ENCHANTMENT_OF_FORCE)
                return 5;
        }
    }
    return 0;
}

//----- (0048EAAE) --------------------------------------------------------
int Character::GetItemsBonus(Attribute attr, bool getOnlyMainHandDmg /*= false*/) const {
    int v5;                     // edi@1
    int v14;                    // ecx@58
    int v15;                    // eax@58
    int v25;                    // ecx@80
    int v26;                    // edi@80
    int v56;                    // eax@365
    Skill v58;             // [sp-4h] [bp-20h]@10
    int v61;                    // [sp+10h] [bp-Ch]@1
    int v62;                    // [sp+14h] [bp-8h]@1
    bool no_skills;

    v5 = 0;
    v62 = 0;
    v61 = 0;

    no_skills = false;
    switch (attr) {
        case ATTRIBUTE_SKILL_ALCHEMY:
            v58 = SKILL_ALCHEMY;
            break;
        case ATTRIBUTE_SKILL_STEALING:
            v58 = SKILL_STEALING;
            break;
        case ATTRIBUTE_SKILL_TRAP_DISARM:
            v58 = SKILL_TRAP_DISARM;
            break;
        case ATTRIBUTE_SKILL_ITEM_ID:
            v58 = SKILL_ITEM_ID;
            break;
        case ATTRIBUTE_SKILL_MONSTER_ID:
            v58 = SKILL_MONSTER_ID;
            break;
        case ATTRIBUTE_SKILL_ARMSMASTER:
            v58 = SKILL_ARMSMASTER;
            break;
        case ATTRIBUTE_SKILL_DODGE:
            v58 = SKILL_DODGE;
            break;
        case ATTRIBUTE_SKILL_UNARMED:
            v58 = SKILL_UNARMED;
            break;
        case ATTRIBUTE_SKILL_FIRE:
            v58 = SKILL_FIRE;
            break;
        case ATTRIBUTE_SKILL_AIR:
            v58 = SKILL_AIR;
            break;
        case ATTRIBUTE_SKILL_WATER:
            v58 = SKILL_WATER;
            break;
        case ATTRIBUTE_SKILL_EARTH:
            v58 = SKILL_EARTH;
            break;
        case ATTRIBUTE_SKILL_SPIRIT:
            v58 = SKILL_SPIRIT;
            break;
        case ATTRIBUTE_SKILL_MIND:
            v58 = SKILL_MIND;
            break;
        case ATTRIBUTE_SKILL_BODY:
            v58 = SKILL_BODY;
            break;
        case ATTRIBUTE_SKILL_LIGHT:
            v58 = SKILL_LIGHT;
            break;
        case ATTRIBUTE_SKILL_DARK:
            v58 = SKILL_DARK;
            break;
        case ATTRIBUTE_SKILL_MEDITATION:
            v58 = SKILL_MEDITATION;
            break;
        case ATTRIBUTE_SKILL_BOW:
            v58 = SKILL_BOW;
            break;
        case ATTRIBUTE_SKILL_SHIELD:
            v58 = SKILL_SHIELD;
            break;
        case ATTRIBUTE_SKILL_LEARNING:
            v58 = SKILL_LEARNING;
            break;
        default:
            no_skills = true;
    }
    if (!no_skills) {
        if (!this->pActiveSkills[v58]) return 0;
    }

    switch (attr) {  // TODO(_) would be nice to move these into separate functions
        case ATTRIBUTE_RANGED_DMG_BONUS:
        case ATTRIBUTE_RANGED_ATTACK:
            if (InventoryConstEntry bow = inventory.functionalEntry(ITEM_SLOT_BOW))
                v5 = bow->GetDamageMod();
            return v5;
            break;

        case ATTRIBUTE_RANGED_DMG_MIN:
            if (InventoryConstEntry bow = inventory.functionalEntry(ITEM_SLOT_BOW)) {
                v5 = bow->GetDamageMod();
                v56 = bow->GetDamageDice();
                return v5 + v56;
            } else {
                return 0;
            }
            break;

        case ATTRIBUTE_RANGED_DMG_MAX:
            if (InventoryConstEntry bow = inventory.functionalEntry(ITEM_SLOT_BOW)) {
                v5 = bow->GetDamageDice() * bow->GetDamageRoll();
                v56 = bow->GetDamageMod();
                return v5 + v56;
            } else {
                return 0;
            }

        case ATTRIBUTE_LEVEL:
            if (!Character::wearsEnchantedItem(ITEM_ENCHANTMENT_OF_POWER)) return 0;
            return 5;
            break;

        case ATTRIBUTE_MELEE_DMG_MAX:
            if (IsUnarmed()) {
                return 3;
            } else {
                if (InventoryConstEntry mainHandItem = inventory.functionalEntry(ITEM_SLOT_MAIN_HAND); mainHandItem && mainHandItem->isWeapon()) { // Not a wand.
                    v26 = mainHandItem->GetDamageRoll();
                    if (inventory.entry(ITEM_SLOT_OFF_HAND) ||
                        mainHandItem->skill() != SKILL_SPEAR) {
                        v25 = mainHandItem->GetDamageDice();
                    } else {
                        v25 = mainHandItem->GetDamageDice() + 1;
                    }
                    v5 = mainHandItem->GetDamageMod() + v25 * v26;
                }
                if (getOnlyMainHandDmg ||
                    !inventory.functionalEntry(ITEM_SLOT_OFF_HAND) ||
                    !inventory.functionalEntry(ITEM_SLOT_OFF_HAND)->isWeapon()) {
                    return v5;
                } else {
                    InventoryConstEntry offHandItem = inventory.entry(ITEM_SLOT_OFF_HAND);
                    v15 = offHandItem->GetDamageMod();
                    v14 = offHandItem->GetDamageDice() *
                          offHandItem->GetDamageRoll();
                    return v5 + v15 + v14;
                }
            }
            break;

        case ATTRIBUTE_MELEE_DMG_BONUS:
        case ATTRIBUTE_ATTACK: {
            if (IsUnarmed()) {
                return 0;
            }
            if (InventoryConstEntry mainHandItem = inventory.functionalEntry(ITEM_SLOT_MAIN_HAND); mainHandItem && mainHandItem->isWeapon()) { // Not a wand.
                v5 = mainHandItem->GetDamageMod();
            }
            InventoryConstEntry offHandItem = inventory.functionalEntry(ITEM_SLOT_OFF_HAND);
            if (getOnlyMainHandDmg || !offHandItem || !offHandItem->isWeapon()) {
                return v5;
            } else {
                v56 = offHandItem->GetDamageMod();
                return v5 + v56;
            }
            break;
        }

        case ATTRIBUTE_MELEE_DMG_MIN: {
            if (IsUnarmed()) {
                return 1;
            }
            if (InventoryConstEntry mainHandItem = inventory.functionalEntry(ITEM_SLOT_MAIN_HAND); mainHandItem && mainHandItem->isWeapon()) { // Not a wand.
                v5 = mainHandItem->GetDamageDice() +
                     mainHandItem->GetDamageMod();
                if (!inventory.entry(ITEM_SLOT_OFF_HAND) && mainHandItem->skill() == SKILL_SPEAR) {
                    ++v5;
                }
            }

            InventoryConstEntry offHandItem = inventory.functionalEntry(ITEM_SLOT_OFF_HAND);
            if (getOnlyMainHandDmg || !offHandItem || !offHandItem->isWeapon()) {
                return v5;
            } else {
                v14 = offHandItem->GetDamageMod();
                v15 = offHandItem->GetDamageDice();
                return v5 + v15 + v14;
            }
            break;
        }

        case ATTRIBUTE_MIGHT:
        case ATTRIBUTE_INTELLIGENCE:
        case ATTRIBUTE_PERSONALITY:
        case ATTRIBUTE_ENDURANCE:
        case ATTRIBUTE_ACCURACY:
        case ATTRIBUTE_SPEED:
        case ATTRIBUTE_LUCK:
        case ATTRIBUTE_HEALTH:
        case ATTRIBUTE_MANA:
        case ATTRIBUTE_AC_BONUS:

        case ATTRIBUTE_RESIST_FIRE:
        case ATTRIBUTE_RESIST_AIR:
        case ATTRIBUTE_RESIST_WATER:
        case ATTRIBUTE_RESIST_EARTH:
        case ATTRIBUTE_RESIST_MIND:
        case ATTRIBUTE_RESIST_BODY:
        case ATTRIBUTE_RESIST_SPIRIT:

        case ATTRIBUTE_SKILL_ALCHEMY:
        case ATTRIBUTE_SKILL_STEALING:
        case ATTRIBUTE_SKILL_TRAP_DISARM:
        case ATTRIBUTE_SKILL_ITEM_ID:
        case ATTRIBUTE_SKILL_MONSTER_ID:
        case ATTRIBUTE_SKILL_ARMSMASTER:
        case ATTRIBUTE_SKILL_DODGE:
        case ATTRIBUTE_SKILL_UNARMED:

        case ATTRIBUTE_SKILL_FIRE:
        case ATTRIBUTE_SKILL_AIR:
        case ATTRIBUTE_SKILL_WATER:
        case ATTRIBUTE_SKILL_EARTH:
        case ATTRIBUTE_SKILL_SPIRIT:
        case ATTRIBUTE_SKILL_MIND:
        case ATTRIBUTE_SKILL_BODY:
        case ATTRIBUTE_SKILL_LIGHT:
        case ATTRIBUTE_SKILL_DARK:
        case ATTRIBUTE_SKILL_MEDITATION:
        case ATTRIBUTE_SKILL_BOW:
        case ATTRIBUTE_SKILL_SHIELD:
        case ATTRIBUTE_SKILL_LEARNING:
            for (InventoryConstEntry entry : inventory.functionalEquipment()) {
                if (attr == ATTRIBUTE_AC_BONUS) {
                    if (isPassiveEquipment(entry->type())) {
                        v5 += entry->GetDamageDice() + entry->GetDamageMod();
                    }
                }

                if (entry->rarity() == RARITY_ARTIFACT || entry->rarity() == RARITY_RELIC) {
                    entry->GetItemBonusArtifact(this, attr, &v62);
                } else if (entry->standardEnchantment == attr) {
                    v5 += entry->standardEnchantmentStrength;
                } else {
                    entry->GetItemBonusSpecialEnchantment(this, attr, &v5, &v61);
                }
            }
            return v5 + v62 + v61;
            break;
        default:
            return 0;
    }
}

//----- (0048F73C) --------------------------------------------------------
int Character::GetMagicalBonus(Attribute a2) const {
    int v3 = 0;  // eax@4
    int v4 = 0;  // ecx@5

    switch (a2) {
        case ATTRIBUTE_RESIST_FIRE:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_RESIST_FIRE].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_FIRE].power;
            break;
        case ATTRIBUTE_RESIST_AIR:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_RESIST_AIR].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_AIR].power;
            break;
        case ATTRIBUTE_RESIST_BODY:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_RESIST_BODY].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_BODY].power;
            break;
        case ATTRIBUTE_RESIST_WATER:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_RESIST_WATER].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_WATER].power;
            break;
        case ATTRIBUTE_RESIST_EARTH:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_RESIST_EARTH].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_EARTH].power;
            break;
        case ATTRIBUTE_RESIST_MIND:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_RESIST_MIND].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_RESIST_MIND].power;
            break;
        case ATTRIBUTE_ATTACK:
        case ATTRIBUTE_RANGED_ATTACK:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_BLESS]
                     .power;  // only character effect spell in both VI and VII
            break;
        case ATTRIBUTE_MELEE_DMG_BONUS:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_HEROISM].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_HEROISM].power;
            break;
        case ATTRIBUTE_MIGHT:
            v3 = pCharacterBuffs[CHARACTER_BUFF_STRENGTH].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].power;
            break;
        case ATTRIBUTE_INTELLIGENCE:
            v3 = pCharacterBuffs[CHARACTER_BUFF_INTELLIGENCE].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].power;
            break;
        case ATTRIBUTE_PERSONALITY:
            v3 = pCharacterBuffs[CHARACTER_BUFF_PERSONALITY].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].power;
            break;
        case ATTRIBUTE_ENDURANCE:
            v3 = pCharacterBuffs[CHARACTER_BUFF_ENDURANCE].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].power;
            break;
        case ATTRIBUTE_ACCURACY:
            v3 = pCharacterBuffs[CHARACTER_BUFF_ACCURACY].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].power;
            break;
        case ATTRIBUTE_SPEED:
            v3 = pCharacterBuffs[CHARACTER_BUFF_SPEED].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].power;
            break;
        case ATTRIBUTE_LUCK:
            v3 = pCharacterBuffs[CHARACTER_BUFF_LUCK].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].power;
            break;
        case ATTRIBUTE_AC_BONUS:
            v3 = this->pCharacterBuffs[CHARACTER_BUFF_STONESKIN].power;
            v4 = pParty->pPartyBuffs[PARTY_BUFF_STONE_SKIN].power;
            break;
        default:
            break;
    }
    return v3 + v4;
}

//----- (0048F882) --------------------------------------------------------
int Character::actualSkillLevel(Skill skill) const {
    if (skill == SKILL_CLUB && engine->config->gameplay.TreatClubAsMace.value()) {
        // some items loaded in as clubs
        skill = SKILL_MACE;
    }

    // Vanilla returned 0 for CHARACTER_SKILL_MISC here, we return 1.
    int base = getSkillValue(skill).level();

    int bonus = 0;
    switch (skill) {
        case SKILL_MONSTER_ID: {
            if (CheckHiredNPCSpeciality(Hunter)) bonus = 6;
            if (CheckHiredNPCSpeciality(Sage)) bonus += 6;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_MONSTER_ID);
        } break;

        case SKILL_ARMSMASTER: {
            if (CheckHiredNPCSpeciality(Armsmaster)) bonus = 2;
            if (CheckHiredNPCSpeciality(Weaponsmaster)) bonus += 3;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_ARMSMASTER);
        } break;

        case SKILL_STEALING: {
            if (CheckHiredNPCSpeciality(Burglar)) bonus = 8;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_STEALING);
        } break;

        case SKILL_ALCHEMY: {
            if (CheckHiredNPCSpeciality(Herbalist)) bonus = 4;
            if (CheckHiredNPCSpeciality(Apothecary)) bonus += 8;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_ALCHEMY);
        } break;

        case SKILL_LEARNING: {
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_LEARNING);
        } break;

        case SKILL_UNARMED: {
            if (CheckHiredNPCSpeciality(Monk)) bonus = 2;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_UNARMED);
        } break;

        case SKILL_DODGE: {
            if (CheckHiredNPCSpeciality(Monk)) bonus = 2;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_DODGE);
        } break;

        case SKILL_BOW:
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_BOW);
            break;
        case SKILL_SHIELD:
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_SHIELD);
            break;

        case SKILL_EARTH:
            if (CheckHiredNPCSpeciality(Apprentice)) bonus = 2;
            if (CheckHiredNPCSpeciality(Mystic)) bonus += 3;
            if (CheckHiredNPCSpeciality(Spellmaster)) bonus += 4;
            if (classType == CLASS_WARLOCK && PartyHasDragon())
                bonus += 3;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_EARTH);
            break;
        case SKILL_FIRE:
            if (CheckHiredNPCSpeciality(Apprentice)) bonus = 2;
            if (CheckHiredNPCSpeciality(Mystic)) bonus += 3;
            if (CheckHiredNPCSpeciality(Spellmaster)) bonus += 4;
            if (classType == CLASS_WARLOCK && PartyHasDragon())
                bonus += 3;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_FIRE);
            break;
        case SKILL_AIR:
            if (CheckHiredNPCSpeciality(Apprentice)) bonus = 2;
            if (CheckHiredNPCSpeciality(Mystic)) bonus += 3;
            if (CheckHiredNPCSpeciality(Spellmaster)) bonus += 4;
            if (classType == CLASS_WARLOCK && PartyHasDragon())
                bonus += 3;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_AIR);
            break;
        case SKILL_WATER:
            if (CheckHiredNPCSpeciality(Apprentice)) bonus = 2;
            if (CheckHiredNPCSpeciality(Mystic)) bonus += 3;
            if (CheckHiredNPCSpeciality(Spellmaster)) bonus += 4;
            if (classType == CLASS_WARLOCK && PartyHasDragon())
                bonus += 3;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_WATER);
            break;
        case SKILL_SPIRIT:
            if (CheckHiredNPCSpeciality(Acolyte2)) bonus = 2;
            if (CheckHiredNPCSpeciality(Initiate)) bonus += 3;
            if (CheckHiredNPCSpeciality(Prelate)) bonus += 4;
            if (classType == CLASS_WARLOCK && PartyHasDragon())
                bonus += 3;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_SPIRIT);
            break;
        case SKILL_MIND:
            if (CheckHiredNPCSpeciality(Acolyte2)) bonus = 2;
            if (CheckHiredNPCSpeciality(Initiate)) bonus += 3;
            if (CheckHiredNPCSpeciality(Prelate)) bonus += 4;
            if (classType == CLASS_WARLOCK && PartyHasDragon())
                bonus += 3;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_MIND);
            break;
        case SKILL_BODY:
            if (CheckHiredNPCSpeciality(Acolyte2)) bonus = 2;
            if (CheckHiredNPCSpeciality(Initiate)) bonus += 3;
            if (CheckHiredNPCSpeciality(Prelate)) bonus += 4;
            if (classType == CLASS_WARLOCK && PartyHasDragon())
                bonus += 3;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_BODY);
            break;
        case SKILL_LIGHT:
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_LIGHT);
            break;
        case SKILL_DARK: {
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_DARK);
        } break;

        case SKILL_MERCHANT: {
            if (CheckHiredNPCSpeciality(Trader)) bonus = 4;
            if (CheckHiredNPCSpeciality(Merchant)) bonus += 6;
            if (CheckHiredNPCSpeciality(Gypsy)) bonus += 3;
            if (CheckHiredNPCSpeciality(Duper)) bonus += 8;
        } break;

        case SKILL_PERCEPTION: {
            if (CheckHiredNPCSpeciality(Scout)) bonus = 6;
            if (CheckHiredNPCSpeciality(Psychic)) bonus += 5;
        } break;

        case SKILL_ITEM_ID:
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_ITEM_ID);
            break;
        case SKILL_MEDITATION:
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_MEDITATION);
            break;
        case SKILL_TRAP_DISARM: {
            if (CheckHiredNPCSpeciality(Tinker)) bonus = 4;
            if (CheckHiredNPCSpeciality(Locksmith)) bonus += 6;
            if (CheckHiredNPCSpeciality(Burglar)) bonus += 8;
            bonus += GetItemsBonus(ATTRIBUTE_SKILL_TRAP_DISARM);
        } break;
        default:
            break;
    }

    // Cap skill and bonus at 60.
    return std::min(skills_max_level[skill], bonus + base);
}

CombinedSkillValue Character::getActualSkillValue(Skill skill) const {
    int level = actualSkillLevel(skill);
    Mastery mastery = pActiveSkills[skill].mastery();

    if (level > 0)
        mastery = std::max(mastery, MASTERY_NOVICE);

    return CombinedSkillValue(level, mastery);
}

//----- (0048FC00) --------------------------------------------------------
int Character::GetSkillBonus(Attribute inSkill) const {
                    // TODO(_): move the individual implementations to attribute
                    // classes once possible ?? check
    int armsMasterBonus = 0;
    int armmaster_skill = getActualSkillValue(SKILL_ARMSMASTER).level();
    if (armmaster_skill > 0) {
        int multiplier = 0;
        if (inSkill == ATTRIBUTE_MELEE_DMG_BONUS) {
            multiplier = GetMultiplierForSkillLevel(SKILL_ARMSMASTER, 0, 0, 1, 2);
        } else if (inSkill == ATTRIBUTE_ATTACK) {
            multiplier = GetMultiplierForSkillLevel(SKILL_ARMSMASTER, 0, 1, 1, 2);
        }
        armsMasterBonus = multiplier * armmaster_skill;
    }

    switch (inSkill) {
        case ATTRIBUTE_RANGED_DMG_BONUS:
            if (inventory.functionalEntry(ITEM_SLOT_BOW)) {
                int bowSkillLevel = getActualSkillValue(SKILL_BOW).level();
                int multiplier = GetMultiplierForSkillLevel(SKILL_BOW, 0, 0, 0, 1);
                return multiplier * bowSkillLevel;
            }
            return 0;

        case ATTRIBUTE_HEALTH: {
            int base_value = pBaseHealthPerLevelByClass[classType];
            int attrib_modif = GetBodybuilding();
            return base_value * attrib_modif;
        }

        case ATTRIBUTE_MANA: {
            int base_value = pBaseManaPerLevelByClass[classType];
            int attrib_modif = GetMeditation();
            return base_value * attrib_modif;
        }

        case ATTRIBUTE_AC_BONUS: {
            bool wearingArmor = false;
            bool wearingLeather = false;
            unsigned int ACSum = 0;

            for (InventoryConstEntry item : inventory.functionalEquipment()) {
                Skill itemSkillType = item->skill();
                int currArmorSkillLevel = 0;
                int multiplier = 0;
                switch (itemSkillType) {
                    case SKILL_STAFF:
                        currArmorSkillLevel = getActualSkillValue(itemSkillType).level();
                        multiplier = GetMultiplierForSkillLevel(itemSkillType, 0, 1, 1, 1);
                        break;
                    case SKILL_SWORD:
                    case SKILL_SPEAR:
                        currArmorSkillLevel = getActualSkillValue(itemSkillType).level();
                        multiplier = GetMultiplierForSkillLevel(itemSkillType, 0, 0, 0, 1);
                        break;
                    case SKILL_SHIELD:
                        currArmorSkillLevel = getActualSkillValue(itemSkillType).level();
                        wearingArmor = true;
                        multiplier = GetMultiplierForSkillLevel(itemSkillType, 1, 1, 2, 2);
                        break;
                    case SKILL_LEATHER:
                        currArmorSkillLevel = getActualSkillValue(itemSkillType).level();
                        wearingLeather = true;
                        multiplier = GetMultiplierForSkillLevel(itemSkillType, 1, 1, 2, 2);
                        break;
                    case SKILL_CHAIN:
                        currArmorSkillLevel = getActualSkillValue(itemSkillType).level();
                        wearingArmor = true;
                        multiplier = GetMultiplierForSkillLevel(itemSkillType, 1, 1, 1, 1);
                        break;
                    case SKILL_PLATE:
                        currArmorSkillLevel = getActualSkillValue(itemSkillType).level();
                        wearingArmor = true;
                        multiplier = GetMultiplierForSkillLevel(itemSkillType, 1, 1, 1, 1);
                        break;
                    default:
                        break;
                }
                ACSum += multiplier * currArmorSkillLevel;
            }

            CombinedSkillValue dodgeValue = getActualSkillValue(SKILL_DODGE);
            int multiplier =
                GetMultiplierForSkillLevel(SKILL_DODGE, 1, 2, 3, 3);
            if (!wearingArmor && (!wearingLeather || dodgeValue.mastery() == MASTERY_GRANDMASTER)) {
                ACSum += multiplier * dodgeValue.level();
            }
            return ACSum;
        }

        case ATTRIBUTE_ATTACK:
            if (this->IsUnarmed()) {
                int unarmedSkill = this->getActualSkillValue(SKILL_UNARMED).level();
                if (!unarmedSkill) {
                    return 0;
                }
                int multiplier = GetMultiplierForSkillLevel(SKILL_UNARMED, 1, 1, 2, 2);
                return armsMasterBonus + multiplier * unarmedSkill;
            }
            for (InventoryConstEntry item : inventory.functionalEquipment()) {
                if (item->isMeleeWeapon()) {
                    Skill currItemSkillType = item->skill();
                    int currentItemSkillLevel = this->getActualSkillValue(currItemSkillType).level();
                    if (currItemSkillType == SKILL_BLASTER) {
                        int multiplier = GetMultiplierForSkillLevel(currItemSkillType, 1, 2, 3, 5);
                        return multiplier * currentItemSkillLevel;
                    } else if (currItemSkillType == SKILL_STAFF && this->getActualSkillValue(SKILL_STAFF).mastery() == MASTERY_GRANDMASTER) {
                        int unarmedSkillLevel = this->getActualSkillValue(SKILL_UNARMED).level();
                        int multiplier = GetMultiplierForSkillLevel(SKILL_UNARMED, 1, 1, 2, 2);
                        return multiplier * unarmedSkillLevel + armsMasterBonus + currentItemSkillLevel;
                    } else {
                        return armsMasterBonus + currentItemSkillLevel;
                    }
                }
            }
            return 0;

        case ATTRIBUTE_RANGED_ATTACK:
            for (InventoryConstEntry item : inventory.functionalEquipment()) {
                if (item->isWeapon()) {
                    Skill currentItemSkillType = item->skill();
                    int currentItemSkillLevel = this->getActualSkillValue(currentItemSkillType).level();
                    if (currentItemSkillType == SKILL_BOW) {
                        int multiplier = GetMultiplierForSkillLevel(SKILL_BOW, 1, 1, 1, 1);
                        return multiplier * currentItemSkillLevel;
                    } else if (currentItemSkillType == SKILL_BLASTER) {
                        int multiplier = GetMultiplierForSkillLevel(SKILL_BLASTER, 1, 2, 3, 5);
                        return multiplier * currentItemSkillLevel;
                    }
                }
            }
            return 0;

        case ATTRIBUTE_MELEE_DMG_BONUS:
            if (this->IsUnarmed()) {
                int unarmedSkillLevel = this->getActualSkillValue(SKILL_UNARMED).level();
                if (!unarmedSkillLevel) {
                    return 0;
                }
                int multiplier = GetMultiplierForSkillLevel(SKILL_UNARMED, 0, 1, 2, 2);
                return multiplier * unarmedSkillLevel;
            }
            for (InventoryConstEntry item : inventory.functionalEquipment()) {
                if (item->isMeleeWeapon()) {
                    Skill currItemSkillType = item->skill();
                    int currItemSkillLevel = this->getActualSkillValue(currItemSkillType).level();
                    int baseSkillBonus;
                    int multiplier;
                    switch (currItemSkillType) {
                    case SKILL_STAFF:
                        if (this->getActualSkillValue(SKILL_STAFF).mastery() >= MASTERY_GRANDMASTER &&
                            this->getActualSkillValue(SKILL_UNARMED).level() > 0) {
                            int unarmedSkillLevel = this->getActualSkillValue(SKILL_UNARMED).level();
                            int multiplier = GetMultiplierForSkillLevel(SKILL_UNARMED, 0, 1, 2, 2);
                            return multiplier * unarmedSkillLevel;
                        } else {
                            return armsMasterBonus;
                        }
                    case SKILL_DAGGER:
                        multiplier = GetMultiplierForSkillLevel(SKILL_DAGGER, 0, 0, 0, 1);
                        baseSkillBonus = multiplier * currItemSkillLevel;
                        return armsMasterBonus + baseSkillBonus;
                    case SKILL_SWORD:
                        multiplier = GetMultiplierForSkillLevel(SKILL_SWORD, 0, 0, 0, 0);
                        baseSkillBonus = multiplier * currItemSkillLevel;
                        return armsMasterBonus + baseSkillBonus;
                    case SKILL_MACE:
                    case SKILL_SPEAR:
                        multiplier = GetMultiplierForSkillLevel(currItemSkillType, 0, 1, 1, 1);
                        baseSkillBonus = multiplier * currItemSkillLevel;
                        return armsMasterBonus + baseSkillBonus;
                    case SKILL_AXE:
                        multiplier = GetMultiplierForSkillLevel(SKILL_AXE, 0, 0, 1, 1);
                        baseSkillBonus = multiplier * currItemSkillLevel;
                        return armsMasterBonus + baseSkillBonus;
                    default:
                        break;
                    }
                }
            }
            return 0;

        default:
            assert(false);
            return 0;
    }
}

unsigned int Character::GetMultiplierForSkillLevel(
    Skill uSkillType, int mult1, int mult2, int mult3,
    int mult4) const {  // TODO(pskelton): ?? needs changing - check behavious
    Mastery masteryLvl = getActualSkillValue(uSkillType).mastery();
    switch (masteryLvl) {
        case MASTERY_NOVICE:
            return mult1;
        case MASTERY_EXPERT:
            return mult2;
        case MASTERY_MASTER:
            return mult3;
        case MASTERY_GRANDMASTER:
            return mult4;
        default:
            return 0;
    }
}

//----- (00490109) --------------------------------------------------------
// faces are:  0  1  2  3   human males
//             4  5  6  7   human females
//                   8  9   elf males
//                  10 11   elf females
//                  12 13   dwarf males
//                  14 15   dwarf females
//                  16 17   goblin males
//                  18 19   goblin females
//                     20   lich male
//                     21   lich female
//                     22   underwater suits (unused)
//                     23   zombie male
//                     24   zombie female
enum Race Character::GetRace() const {
    if (uCurrentFace <= 7) {
        return RACE_HUMAN;
    } else if (uCurrentFace <= 11) {
        return RACE_ELF;
    } else if (uCurrentFace <= 15) {
        return RACE_DWARF;
    } else if (uCurrentFace <= 19) {
        return RACE_GOBLIN;
    } else {
        return RACE_HUMAN;
    }
}

std::string Character::GetRaceName() const {
    switch (GetRace()) {
        case RACE_HUMAN: return localization->GetString(LSTR_RACE_HUMAN);
        case RACE_ELF: return localization->GetString(LSTR_RACE_ELF);
        case RACE_GOBLIN: return localization->GetString(LSTR_RACE_GOBLIN);
        case RACE_DWARF: return localization->GetString(LSTR_RACE_DWARF);
        default:
            assert(false);
            return std::string();  // Make the compiler happy.
    }
}

//----- (00490141) --------------------------------------------------------
Sex Character::GetSexByVoice() const {
    switch (this->uVoiceID) {
        case 0u:
        case 1u:
        case 2u:
        case 3u:
        case 8u:
        case 9u:
        case 0xCu:
        case 0xDu:
        case 0x10u:
        case 0x11u:
        case 0x14u:
        case 0x17u:
            return SEX_MALE;

        case 4u:
        case 5u:
        case 6u:
        case 7u:
        case 0xAu:
        case 0xBu:
        case 0xEu:
        case 0xFu:
        case 0x12u:
        case 0x13u:
        case 0x15u:
        case 0x18u:
            return SEX_FEMALE;

        default:
            assert(false);
            return SEX_MALE;
    }
}

//----- (00490188) --------------------------------------------------------
void Character::SetInitialStats() {
    Race race = GetRace();
    for (Attribute stat : allStatAttributes())
        _stats[stat] = StatTable[race][stat].uBaseValue;
}

//----- (004901FC) --------------------------------------------------------
void Character::SetSexByVoice() {
    switch (this->uVoiceID) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 0xC:
        case 0xD:
        case 0x10:
        case 0x11:
        case 0x14:
        case 0x17:
            this->uSex = SEX_MALE;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 0xA:
        case 0xB:
        case 0xE:
        case 0xF:
        case 0x12:
        case 0x13:
        case 0x15:
        case 0x18:
            this->uSex = SEX_FEMALE;
            break;
        default:
            assert(false);
            break;
    }
}

//----- (0049024A) --------------------------------------------------------
void Character::ChangeClass(Class cls) {
    classType = cls;
    uLevel = 1;
    experience = 251ll + grng->random(100);
    uBirthYear = 1147 - grng->random(6);

    for (Skill i : allVisibleSkills()) {
        if (pSkillAvailabilityPerClass[std::to_underlying(classType) / 4][i] != SKILL_AFFINITY_PRIMARY) {
            setSkillValue(i, CombinedSkillValue());
        } else {
            setSkillValue(i, CombinedSkillValue::novice());
        }
    }

    health = GetMaxHealth();
    mana = GetMaxMana();
}

//----- (004903C9) --------------------------------------------------------
Skill Character::GetSkillIdxByOrder(signed int order) {
    int counter;  // edx@5
    bool canBeInactive;
    SkillAffinity requiredValue;
    signed int offset;

    if (order <= 1) {
        canBeInactive = false;
        requiredValue = SKILL_AFFINITY_PRIMARY;  // 2 - primary skill
        offset = 0;
    } else if (order <= 3) {
        canBeInactive = false;
        requiredValue = SKILL_AFFINITY_AVAILABLE;  // 1 - available
        offset = 2;
    } else if (order <= 12) {
        canBeInactive = true;
        requiredValue = SKILL_AFFINITY_AVAILABLE;  // 1 - available
        offset = 4;
    } else {
        return SKILL_INVALID;
    }
    counter = 0;
    for (Skill i : allVisibleSkills()) {
        if ((this->pActiveSkills[i] || canBeInactive) &&
            pSkillAvailabilityPerClass[std::to_underlying(classType) / 4][i] == requiredValue) {
            if (counter == order - offset) return i;
            ++counter;
        }
    }

    return SKILL_INVALID;
}

//----- (0049048D) --------------------------------------------------------
// uint16_t PartyCreation_BtnMinusClick(Character *_this, int eAttribute)
void Character::DecreaseAttribute(Attribute eAttribute) {
    int pBaseValue;    // ecx@1
    int pDroppedStep;  // ebx@1
    int pStep;         // esi@1
    int uMinValue;     // [sp+Ch] [bp-4h]@1

    Race raceId = GetRace();
    pBaseValue = StatTable[raceId][eAttribute].uBaseValue;
    pDroppedStep = StatTable[raceId][eAttribute].uDroppedStep;
    uMinValue = pBaseValue - 2;
    pStep = StatTable[raceId][eAttribute].uBaseStep;
    int *AttrToChange = &this->_stats[eAttribute];
    if (*AttrToChange <= pBaseValue) pStep = pDroppedStep;
    if (*AttrToChange - pStep >= uMinValue) *AttrToChange -= pStep;
}

//----- (004905F5) --------------------------------------------------------
// signed int  PartyCreation_BtnPlusClick(Character *this, int eAttribute)
void Character::IncreaseAttribute(Attribute eAttribute) {
    int maxValue;            // ebx@1
    signed int baseStep;     // edi@1
    signed int tmp;          // eax@17
    signed int result;       // eax@18
    int baseValue;           // [sp+Ch] [bp-8h]@1
    signed int droppedStep;  // [sp+10h] [bp-4h]@1

    Race raceId = GetRace();
    maxValue = StatTable[raceId][eAttribute].uMaxValue;
    baseStep = StatTable[raceId][eAttribute].uBaseStep;
    baseValue = StatTable[raceId][eAttribute].uBaseValue;
    droppedStep = StatTable[raceId][eAttribute].uDroppedStep;
    CharacterCreation_GetUnspentAttributePointCount();
    int *statToChange = &this->_stats[eAttribute];
    if (*statToChange < baseValue) {
        tmp = baseStep;
        baseStep = droppedStep;
        droppedStep = tmp;
    }
    result = CharacterCreation_GetUnspentAttributePointCount();
    if (result >= droppedStep) {
        if (baseStep + *statToChange <= maxValue) *statToChange += baseStep;
    }
}

//----- (0049070F) --------------------------------------------------------
void Character::resetTempBonuses() {
    // this is also used during party rest and heal so only buffs and bonuses are reset
    this->sLevelModifier = 0;
    this->sACModifier = 0;
    this->_statBonuses.fill(0);
    this->sResFireBonus = 0;
    this->sResAirBonus = 0;
    this->sResWaterBonus = 0;
    this->sResEarthBonus = 0;
    this->sResPhysicalBonus = 0;
    this->sResMagicBonus = 0;
    this->sResSpiritBonus = 0;
    this->sResMindBonus = 0;
    this->sResBodyBonus = 0;
    this->sResLightBonus = 0;
    this->sResDarkBonus = 0;

    this->_ranged_dmg_bonus = 0;
    this->_ranged_atk_bonus = 0;
    this->_melee_dmg_bonus = 0;
    this->_some_attack_bonus = 0;
    this->_mana_related = 0;
    this->uFullManaBonus = 0;
    this->_health_related = 0;
    this->uFullHealthBonus = 0;
}

//----- (004907E7) --------------------------------------------------------
Color Character::GetStatColor(Attribute uStat) const {
    int base_attribute_value = StatTable[GetRace()][uStat].uBaseValue;

    int attribute_value = _stats[uStat];
    if (attribute_value == base_attribute_value)
        return ui_character_stat_default_color;
    else if (attribute_value > base_attribute_value)
        return ui_character_stat_buffed_color;
    else
        return ui_character_stat_debuffed_color;
}

//----- (004908A8) --------------------------------------------------------
bool Character::DiscardConditionIfLastsLongerThan(Condition uCondition,
                                                  Time time) {
    if (conditions.has(uCondition) && time < conditions.get(uCondition)) {
        conditions.reset(uCondition);
        return true;
    } else {
        return false;
    }
}

void Character::useItem(int targetCharacter, bool isPortraitClick) {
    Character *playerAffected = &pParty->pCharacters[targetCharacter];
    if (pParty->bTurnBasedModeOn && (pTurnEngine->turn_stage == TE_WAIT || pTurnEngine->turn_stage == TE_MOVEMENT)) {
        return;
    }
    if (pParty->pPickedItem.isReagent()) {
        // TODO(Nik-RE-dev): this looks like some artifact from MM6 (where you can eat reagents)
        // In MM7 these item IDs are invalid (plus index 161 used twice which is wrong)
        if (pParty->pPickedItem.itemId == ITEM_161) {
            playerAffected->SetCondition(CONDITION_POISON_WEAK, 1);
        } else if (pParty->pPickedItem.itemId == ITEM_161) {
            playerAffected->mana += 2;
            if (playerAffected->mana > playerAffected->GetMaxMana()) {
                playerAffected->mana = playerAffected->GetMaxMana();
            }
            playerAffected->playReaction(SPEECH_DRINK_POTION);
        } else if (pParty->pPickedItem.itemId == ITEM_162) {
            playerAffected->Heal(2);
            playerAffected->playReaction(SPEECH_DRINK_POTION);
        } else {
            engine->_statusBar->setEvent(LSTR_S_CAN_NOT_BE_USED_THAT_WAY, pParty->pPickedItem.GetDisplayName());
            pAudioPlayer->playUISound(SOUND_error);
            return;
        }
        pAudioPlayer->playUISound(SOUND_eat);

        if (pGUIWindow_CurrentMenu &&
            pGUIWindow_CurrentMenu->eWindowType != WINDOW_null) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
        }
        //if (v73) {
            if (pParty->bTurnBasedModeOn) {
                pParty->pTurnBasedCharacterRecoveryTimes[targetCharacter] = 100_ticks;
                this->SetRecoveryTime(100_ticks);
                pTurnEngine->ApplyPlayerAction();
            } else {
                this->SetRecoveryTime(debug_non_combat_recovery_mul * flt_debugrecmod3 * 100_ticks);
            }
        //}
        pParty->takeHoldingItem();
        return;
    }

    if (pParty->pPickedItem.isPotion()) {
        // TODO(Nik-RE-dev): no CanAct check?
        int potionStrength = pParty->pPickedItem.potionPower;
        Duration buffDuration = Duration::fromMinutes(30 * potionStrength); // all buffs have same duration based on potion strength
        switch (pParty->pPickedItem.itemId) {
            case ITEM_POTION_CATALYST:
                playerAffected->SetCondition(CONDITION_POISON_WEAK, 1);
                break;

            case ITEM_POTION_CURE_WOUNDS:
                playerAffected->Heal(potionStrength + 10);
                break;

            case ITEM_POTION_MAGIC:
                playerAffected->mana += potionStrength + 10;
                if (playerAffected->mana > playerAffected->GetMaxMana()) {
                    playerAffected->mana = playerAffected->GetMaxMana();
                }
                break;

            case ITEM_POTION_CURE_WEAKNESS:
                playerAffected->conditions.reset(CONDITION_WEAK);
                break;

            case ITEM_POTION_CURE_DISEASE:
                playerAffected->conditions.reset(CONDITION_DISEASE_SEVERE);
                playerAffected->conditions.reset(CONDITION_DISEASE_MEDIUM);
                playerAffected->conditions.reset(CONDITION_DISEASE_WEAK);
                break;

            case ITEM_POTION_CURE_POISON:
                playerAffected->conditions.reset(CONDITION_POISON_SEVERE);
                playerAffected->conditions.reset(CONDITION_POISON_MEDIUM);
                playerAffected->conditions.reset(CONDITION_POISON_WEAK);
                break;

            case ITEM_POTION_AWAKEN:
                playerAffected->conditions.reset(CONDITION_SLEEP);
                break;

            case ITEM_POTION_HASTE:
                if (!playerAffected->conditions.has(CONDITION_WEAK)) {
                    playerAffected->pCharacterBuffs[CHARACTER_BUFF_HASTE].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER, 5, 0, 0);
                }
                break;

            case ITEM_POTION_HEROISM:
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_HEROISM].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER, 5, 0, 0);
                break;

            case ITEM_POTION_BLESS:
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_BLESS].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER, 5, 0, 0);
                break;

            case ITEM_POTION_PRESERVATION:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_PRESERVATION].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_SHIELD:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_SHIELD].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_STONESKIN:
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_STONESKIN].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER, 5, 0, 0);
                break;

            case ITEM_POTION_WATER_BREATHING:
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_WATER_WALK].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER, 5, 0, 0);
                // Drink potion reaction was missing
                break;

            case ITEM_POTION_REMOVE_FEAR:
                playerAffected->conditions.reset(CONDITION_FEAR);
                break;

            case ITEM_POTION_REMOVE_CURSE:
                playerAffected->conditions.reset(CONDITION_CURSED);
                break;

            case ITEM_POTION_CURE_INSANITY:
                playerAffected->conditions.reset(CONDITION_INSANE);
                break;

            case ITEM_POTION_MIGHT_BOOST:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_STRENGTH].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_INTELLECT_BOOST:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_INTELLIGENCE].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_PERSONALITY_BOOST:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_PERSONALITY].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_ENDURANCE_BOOST:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_ENDURANCE].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_SPEED_BOOST:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_SPEED].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_ACCURACY_BOOST:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_ACCURACY].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_CURE_PARALYSIS:
                playerAffected->conditions.reset(CONDITION_PARALYZED);
                break;

            case ITEM_POTION_DIVINE_RESTORATION:
            {
                Time deadTime = playerAffected->conditions.get(CONDITION_DEAD);
                Time petrifedTime = playerAffected->conditions.get(CONDITION_PETRIFIED);
                Time eradicatedTime = playerAffected->conditions.get(CONDITION_ERADICATED);
                // TODO(Nik-RE-dev): why not playerAffected?
                conditions.resetAll();
                playerAffected->conditions.set(CONDITION_DEAD, deadTime);
                playerAffected->conditions.set(CONDITION_PETRIFIED, petrifedTime);
                playerAffected->conditions.set(CONDITION_ERADICATED, eradicatedTime);
                break;
            }

            case ITEM_POTION_DIVINE_CURE:
                playerAffected->Heal(5 * potionStrength);
                break;

            case ITEM_POTION_DIVINE_POWER:
                playerAffected->mana += 5 * potionStrength;
                if (playerAffected->mana > playerAffected->GetMaxMana())
                    playerAffected->mana = playerAffected->GetMaxMana();
                break;

            case ITEM_POTION_LUCK_BOOST:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_LUCK].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_FIRE_RESISTANCE:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_RESIST_FIRE].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_AIR_RESISTANCE:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_RESIST_AIR].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_WATER_RESISTANCE:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_RESIST_WATER].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_EARTH_RESISTANCE:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_RESIST_EARTH].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_MIND_RESISTANCE:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_RESIST_MIND].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_BODY_RESISTANCE:
                // mastery was NONE
                playerAffected->pCharacterBuffs[CHARACTER_BUFF_RESIST_BODY].Apply(pParty->GetPlayingTime() + buffDuration, MASTERY_MASTER,
                        potionStrength * 3, 0, 0);
                break;

            case ITEM_POTION_STONE_TO_FLESH:
                playerAffected->conditions.reset(CONDITION_PETRIFIED);
                break;

            case ITEM_POTION_PURE_LUCK:
            case ITEM_POTION_PURE_SPEED:
            case ITEM_POTION_PURE_INTELLECT:
            case ITEM_POTION_PURE_ENDURANCE:
            case ITEM_POTION_PURE_PERSONALITY:
            case ITEM_POTION_PURE_ACCURACY:
            case ITEM_POTION_PURE_MIGHT: {
                Attribute stat = statForPureStatPotion(pParty->pPickedItem.itemId);
                if (!playerAffected->_pureStatPotionUsed[stat]) {
                    playerAffected->_stats[stat] += 50;
                    playerAffected->_pureStatPotionUsed[stat] = true;
                }
                break;
            }

            case ITEM_POTION_REJUVENATION:
                playerAffected->sAgeModifier = 0;
                break;

            default:
                engine->_statusBar->setEvent(LSTR_S_CAN_NOT_BE_USED_THAT_WAY, pParty->pPickedItem.GetDisplayName());
                pAudioPlayer->playUISound(SOUND_error);
                return;
        }
        if (pParty->pPickedItem.itemId != ITEM_POTION_CATALYST) {
            playerAffected->playReaction(SPEECH_DRINK_POTION);
        }
        pAudioPlayer->playUISound(SOUND_drink);
        if (pGUIWindow_CurrentMenu && pGUIWindow_CurrentMenu->eWindowType != WINDOW_null) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
        }
        if (pParty->bTurnBasedModeOn) {
            pParty->pTurnBasedCharacterRecoveryTimes[targetCharacter] = 100_ticks;
            this->SetRecoveryTime(100_ticks);
            pTurnEngine->ApplyPlayerAction();
        } else {
            this->SetRecoveryTime(debug_non_combat_recovery_mul * flt_debugrecmod3 * 100_ticks);
        }
        pParty->takeHoldingItem();
        return;
    }

    if (pParty->pPickedItem.isSpellScroll()) {
        if (current_screen_type == SCREEN_CASTING) {
            return;
        }
        if (!playerAffected->CanAct()) {
            engine->_statusBar->setEvent(LSTR_THAT_PLAYER_IS_S, localization->GetCharacterConditionName(playerAffected->GetMajorConditionIdx()));
            pAudioPlayer->playUISound(SOUND_error);
            return;
        }
        if (engine->IsUnderwater()) {
            engine->_statusBar->setEvent(LSTR_YOU_CAN_NOT_DO_THAT_WHILE_YOU_ARE);
            pAudioPlayer->playUISound(SOUND_error);
            return;
        }
        if (playerAffected->timeToRecovery) {
            engine->_statusBar->setEvent(LSTR_PLAYER_IS_NOT_ACTIVE);
            pAudioPlayer->playUISound(SOUND_error);
            return;
        }

        // TODO(Nik-RE-dev): spell scroll is removed before actual casting and will be consumed even if casting is canceled.
        SpellId scrollSpellId = spellForScroll(pParty->pPickedItem.itemId);
        if (isSpellTargetsItem(scrollSpellId)) {
            pParty->takeHoldingItem();
            pGUIWindow_CurrentMenu->Release();
            current_screen_type = SCREEN_GAME;
            pushScrollSpell(scrollSpellId, targetCharacter);
        } else {
            pParty->takeHoldingItem();
            // Process spell on next frame after game exits inventory window.
            engine->_messageQueue->addMessageNextFrame(UIMSG_SpellScrollUse, std::to_underlying(scrollSpellId), targetCharacter);
            if (current_screen_type != SCREEN_GAME && pGUIWindow_CurrentMenu && (pGUIWindow_CurrentMenu->eWindowType != WINDOW_null)) {
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
            }
        }
        return;
    }

    if (pParty->pPickedItem.isBook()) {
        SpellId bookSpellId = spellForSpellbook(pParty->pPickedItem.itemId);
        if (playerAffected->bHaveSpell[bookSpellId]) {
            engine->_statusBar->setEvent(LSTR_YOU_ALREADY_KNOW_THE_S_SPELL, pParty->pPickedItem.GetDisplayName());
            pAudioPlayer->playUISound(SOUND_error);
            return;
        }
        if (!playerAffected->CanAct()) {
            engine->_statusBar->setEvent(LSTR_THAT_PLAYER_IS_S, localization->GetCharacterConditionName(playerAffected->GetMajorConditionIdx()));
            pAudioPlayer->playUISound(SOUND_error);
            return;
        }

        Mastery requiredMastery = pSpellDatas[bookSpellId].skillMastery;
        Skill skill = skillForSpell(bookSpellId);
        CombinedSkillValue val = playerAffected->getSkillValue(skill);

        if (requiredMastery > val.mastery() || val.level() == 0) {
            engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_THE_SKILL_TO_LEARN_S, pParty->pPickedItem.GetDisplayName());
            playerAffected->playReaction(SPEECH_CANT_LEARN_SPELL);
            return;
        }
        playerAffected->bHaveSpell[bookSpellId] = true;
        playerAffected->playReaction(SPEECH_LEARN_SPELL);

        // if (pGUIWindow_CurrentMenu && pGUIWindow_CurrentMenu->eWindowType != WINDOW_null) {
        //     if (!v73) { // v73 is always 0 at this point
        //         mouse->RemoveHoldingItem();
        //         return;
        //     }
        //     engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
        // }
        //       if ( v73 ) // v73 is always 0 at this point
        //       {
        //         if ( pParty->bTurnBasedModeOn )
        //         {
        //           pParty->pTurnBasedCharacterRecoveryTimes[player_num-1] = 100;
        //           thisb->SetRecoveryTime(100);
        //           pTurnEngine->ApplyPlayerAction();
        //         }
        //         else
        //         {
        //           thisb->SetRecoveryTime(flt_6BE3A4_debug_recmod1 * 213.3333333333333);
        //         }
        //       }
        pParty->takeHoldingItem();
        return;
    }

    if (pParty->pPickedItem.isMessageScroll()) {
        if (playerAffected->CanAct()) {
            CreateMsgScrollWindow(pParty->pPickedItem.itemId);
            playerAffected->playReaction(SPEECH_READ_SCROLL);
            return;
        }

        engine->_statusBar->setEvent(LSTR_THAT_PLAYER_IS_S, localization->GetCharacterConditionName(playerAffected->GetMajorConditionIdx()));
        pAudioPlayer->playUISound(SOUND_error);
        return;
    }

    // Everything else
    {
        if (pParty->pPickedItem.itemId == ITEM_GENIE_LAMP) {
            int value = pParty->uCurrentMonthWeek + 1;

            std::string status;
            switch (pParty->uCurrentMonth) {
                case 0: // Jan
                    playerAffected->_stats[ATTRIBUTE_MIGHT] += value;
                    status = fmt::format("+{} {} {}", value, localization->GetAttirubteName(ATTRIBUTE_MIGHT), localization->GetString(LSTR_PERMANENT));
                    break;
                case 1: // Feb
                    playerAffected->_stats[ATTRIBUTE_INTELLIGENCE] += value;
                    status = fmt::format("+{} {} {}", value, localization->GetAttirubteName(ATTRIBUTE_INTELLIGENCE), localization->GetString(LSTR_PERMANENT));
                    break;
                case 2: // Mar
                    playerAffected->_stats[ATTRIBUTE_PERSONALITY] += value;
                    status = fmt::format("+{} {} {}", value, localization->GetAttirubteName(ATTRIBUTE_PERSONALITY), localization->GetString(LSTR_PERMANENT));
                    break;
                case 3: // Apr
                    playerAffected->_stats[ATTRIBUTE_ENDURANCE] += value;
                    status = fmt::format("+{} {} {}", value, localization->GetAttirubteName(ATTRIBUTE_ENDURANCE), localization->GetString(LSTR_PERMANENT));
                    break;
                case 4: // May
                    playerAffected->_stats[ATTRIBUTE_ACCURACY] += value;
                    status = fmt::format("+{} {} {}", value, localization->GetAttirubteName(ATTRIBUTE_ACCURACY), localization->GetString(LSTR_PERMANENT));
                    break;
                case 5: // Jun
                    playerAffected->_stats[ATTRIBUTE_SPEED] += value;
                    status = fmt::format("+{} {} {}", value, localization->GetAttirubteName(ATTRIBUTE_SPEED), localization->GetString(LSTR_PERMANENT));
                    break;
                case 6: // Jul
                    playerAffected->_stats[ATTRIBUTE_LUCK] += value;
                    status = fmt::format("+{} {} {}", value, localization->GetAttirubteName(ATTRIBUTE_LUCK), localization->GetString(LSTR_PERMANENT));
                    break;
                case 7: // Aug
                    pParty->partyFindsGold(1000 * value, GOLD_RECEIVE_SHARE);
                    status = fmt::format("+{} {}", 1000 * value, localization->GetString(LSTR_GOLD));
                    break;
                case 8: // Sep
                    pParty->GiveFood(5 * value);
                    status = fmt::format("+{} {}", 5 * value, localization->GetString(LSTR_FOOD));
                    break;
                case 9: // Oct
                    playerAffected->uSkillPoints += 2 * value;
                    status = fmt::format("+{} {}", 2 * value, localization->GetString(LSTR_SKILL_POINTS));
                    break;
                case 10: // Nov
                    playerAffected->experience += 2500ll * value;
                    status = fmt::format("+{} {}", 2500ll * value, localization->GetString(LSTR_EXPERIENCE));
                    break;
                case 11: { // Dec
                    static constexpr std::initializer_list<MagicSchool> possibleResistances = {
                        MAGIC_SCHOOL_FIRE,
                        MAGIC_SCHOOL_AIR,
                        MAGIC_SCHOOL_WATER,
                        MAGIC_SCHOOL_EARTH,
                        MAGIC_SCHOOL_MIND,
                        MAGIC_SCHOOL_BODY
                        // Note: no spirit resistance.
                    };
                    static_assert(possibleResistances.size() == 6);
                    MagicSchool res = grng->randomSample(possibleResistances);

                    std::string spell_school_name = localization->GetSpellSchoolName(res);

                    switch (res) {
                        case MAGIC_SCHOOL_FIRE:
                            playerAffected->sResFireBase += value;
                            break;
                        case MAGIC_SCHOOL_AIR:
                            playerAffected->sResAirBase += value;
                            break;
                        case MAGIC_SCHOOL_WATER:
                            playerAffected->sResWaterBase += value;
                            break;
                        case MAGIC_SCHOOL_EARTH:
                            playerAffected->sResEarthBase += value;
                            break;
                        case MAGIC_SCHOOL_MIND:
                            playerAffected->sResMindBase += value;
                            break;
                        case MAGIC_SCHOOL_BODY:
                            playerAffected->sResBodyBase += value;
                            break;
                        default:
                            assert(false);
                            return;
                    }
                    status = fmt::format("+{} {} {}", value, spell_school_name, localization->GetString(LSTR_PERMANENT));
                    break;
                }
            }
            engine->_statusBar->setEvent(status);

            spell_fx_renderer->SetPlayerBuffAnim(SPELL_QUEST_COMPLETED, targetCharacter);
            playerAffected->playReaction(SPEECH_QUEST_GOT);
            pAudioPlayer->playUISound(SOUND_chimes);
            if (pParty->uCurrentDayOfMonth == 6 || pParty->uCurrentDayOfMonth == 20) {
                playerAffected->SetCondition(CONDITION_ERADICATED, 0);
                pAudioPlayer->playUISound(SOUND_gong);
            } else if (pParty->uCurrentDayOfMonth == 12 || pParty->uCurrentDayOfMonth == 26) {
                playerAffected->SetCondition(CONDITION_DEAD, 0);
                pAudioPlayer->playUISound(SOUND_gong);
            } else if (pParty->uCurrentDayOfMonth == 4 || pParty->uCurrentDayOfMonth == 25) {
                playerAffected->SetCondition(CONDITION_PETRIFIED, 0);
                pAudioPlayer->playUISound(SOUND_gong);
            }
        } else if (pParty->pPickedItem.itemId == ITEM_RED_APPLE) {
            pParty->GiveFood(1);
            pAudioPlayer->playUISound(SOUND_eat);
        } else if (pParty->pPickedItem.itemId == ITEM_QUEST_LUTE) {
            pAudioPlayer->playUISound(SOUND_luteguitar);
            return;
        } else if (pParty->pPickedItem.itemId == ITEM_QUEST_FAERIE_PIPES) {
            pAudioPlayer->playUISound(SOUND_panflute);
            return;
        } else if (pParty->pPickedItem.itemId == ITEM_QUEST_GRYPHONHEARTS_TRUMPET) {
            pAudioPlayer->playUISound(SOUND_trumpet);
            return;
        } else if (pParty->pPickedItem.itemId == ITEM_HORSESHOE) {
            spell_fx_renderer->SetPlayerBuffAnim(SPELL_QUEST_COMPLETED, targetCharacter);
            //v5 = Pid(OBJECT_Character, player_num + 49);
            //pAudioPlayer->playSound(SOUND_quest, v5);
            pAudioPlayer->playUISound(SOUND_quest);
            engine->_statusBar->setEvent(LSTR_2_SKILL_POINTS);
            playerAffected->uSkillPoints += 2;
        } else if (pParty->pPickedItem.itemId == ITEM_TEMPLE_IN_A_BOTTLE) {
            TeleportToNWCDungeon();
            return;
        } else {
            engine->_statusBar->setEvent(LSTR_S_CAN_NOT_BE_USED_THAT_WAY, pParty->pPickedItem.GetDisplayName());
            pAudioPlayer->playUISound(SOUND_error);
            return;
        }

        pParty->takeHoldingItem();
        return;
    }
}

bool CmpSkillValue(int valToCompare, CombinedSkillValue skillValue) {
    int val;
    if (valToCompare <= 63)
        val = skillValue.level();
    else
        val = skillValue.joined();
    return val >= valToCompare;
}

//----- (00449BB4) --------------------------------------------------------
bool Character::CompareVariable(EvtVariable VarNum, int pValue) {
    // in some cases this calls only calls v4 >= pValue, which i've
    // changed to return false, since these values are supposed to
    // be positive and v4 was -1 by default
    assert(pValue >= 0 && "Compare variable shouldn't have negative arguments");

    signed int v4;                         // edi@1
    uint8_t test_bit_value;        // eax@25
    uint8_t byteWithRequestedBit;  // cl@25
    LocationInfo *v19;                   // eax@122
    LocationInfo *v21;                   // eax@126
    int actStat;                           // ebx@161
    int baseStat;                          // eax@161

    if (VarNum >= VAR_MapPersistentVariable_0 && VarNum <= VAR_MapPersistentVariable_74)
        return (uint8_t)engine->_persistentVariables.mapVars[std::to_underlying(VarNum) - std::to_underlying(VAR_MapPersistentVariable_0)] >= pValue;

    // not really sure whether the number gets up to 99, but can't ignore the possibility
    if (VarNum >= VAR_MapPersistentDecorVariable_0 && VarNum <= VAR_MapPersistentDecorVariable_24)
        return (uint8_t)engine->_persistentVariables.decorVars[std::to_underlying(VarNum) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] >= pValue;

    switch (VarNum) {
        case VAR_Sex:
            return pValue == std::to_underlying(this->uSex);
        case VAR_Class:
            return pValue == std::to_underlying(this->classType);
        case VAR_Race:
            return pValue == std::to_underlying(GetRace());
        case VAR_CurrentHP:
            return this->health >= pValue;
        case VAR_MaxHP:
            return (this->health >= GetMaxHealth());
        case VAR_CurrentSP:
            return this->mana >= pValue;
        case VAR_MaxSP:
            return (this->mana >= GetMaxMana());
        case VAR_ActualAC:
            return GetActualAC() >= pValue;
        case VAR_ACModifier:
            return this->sACModifier >= pValue;
        case VAR_BaseLevel:
            return this->uLevel >= pValue;
        case VAR_LevelModifier:
            return this->sLevelModifier >= pValue;
        case VAR_Age:
            return GetActualAge() >= (unsigned int)pValue;
        case VAR_Award:
            return _achievedAwardsBits[pValue];
        case VAR_Experience:
            return this->experience >= pValue;  // TODO(_) change pValue to long long
        case VAR_QBits_QuestsDone:
            return pParty->_questBits[static_cast<QuestBit>(pValue)]; // TODO(captainurist): values coming from scripts should be bound-checked.
        case VAR_PlayerItemInHands:
            // for (int i = 0; i < 138; i++)
            if (inventory.find(static_cast<ItemId>(pValue)))
                return true;
            return pParty->pPickedItem.itemId == static_cast<ItemId>(pValue);

        case VAR_Hour:
            return pParty->GetPlayingTime().toCivilTime().hour == pValue;

        case VAR_DayOfYear:
            return pParty->GetPlayingTime().toDays() % 336 + 1 == pValue;

        case VAR_DayOfWeek:
            return pParty->GetPlayingTime().toDays() % 7 == pValue;

        case VAR_FixedGold:
            return pParty->GetGold() >= pValue;
        case VAR_FixedFood:
            return pParty->GetFood() >= pValue;
        case VAR_MightBonus:
            return this->_statBonuses[ATTRIBUTE_MIGHT] >= pValue;
        case VAR_IntellectBonus:
            return this->_statBonuses[ATTRIBUTE_INTELLIGENCE] >= pValue;
        case VAR_PersonalityBonus:
            return this->_statBonuses[ATTRIBUTE_PERSONALITY] >= pValue;
        case VAR_EnduranceBonus:
            return this->_statBonuses[ATTRIBUTE_ENDURANCE] >= pValue;
        case VAR_SpeedBonus:
            return this->_statBonuses[ATTRIBUTE_SPEED] >= pValue;
        case VAR_AccuracyBonus:
            return this->_statBonuses[ATTRIBUTE_ACCURACY] >= pValue;
        case VAR_LuckBonus:
            return this->_statBonuses[ATTRIBUTE_LUCK] >= pValue;
        case VAR_BaseMight:
            return this->_stats[ATTRIBUTE_MIGHT] >= pValue;
        case VAR_BaseIntellect:
            return this->_stats[ATTRIBUTE_INTELLIGENCE] >= pValue;
        case VAR_BasePersonality:
            return this->_stats[ATTRIBUTE_PERSONALITY] >= pValue;
        case VAR_BaseEndurance:
            return this->_stats[ATTRIBUTE_ENDURANCE] >= pValue;
        case VAR_BaseSpeed:
            return this->_stats[ATTRIBUTE_SPEED] >= pValue;
        case VAR_BaseAccuracy:
            return this->_stats[ATTRIBUTE_ACCURACY] >= pValue;
        case VAR_BaseLuck:
            return this->_stats[ATTRIBUTE_LUCK] >= pValue;
        case VAR_ActualMight:
            return GetActualMight() >= pValue;
        case VAR_ActualIntellect:
            return GetActualIntelligence() >= pValue;
        case VAR_ActualPersonality:
            return GetActualPersonality() >= pValue;
        case VAR_ActualEndurance:
            return GetActualEndurance() >= pValue;
        case VAR_ActualSpeed:
            return GetActualSpeed() >= pValue;
        case VAR_ActualAccuracy:
            return GetActualAccuracy() >= pValue;
        case VAR_ActualLuck:
            return GetActualLuck() >= pValue;
        case VAR_FireResistance:
            return this->sResFireBase >= pValue;
        case VAR_AirResistance:
            return this->sResAirBase >= pValue;
        case VAR_WaterResistance:
            return this->sResWaterBase >= pValue;
        case VAR_EarthResistance:
            return this->sResEarthBase >= pValue;
        case VAR_SpiritResistance:
            return this->sResSpiritBase >= pValue;
        case VAR_MindResistance:
            return this->sResMindBase >= pValue;
        case VAR_BodyResistance:
            return this->sResBodyBase >= pValue;
        case VAR_LightResistance:
            return this->sResLightBase >= pValue;
        case VAR_DarkResistance:
            return this->sResDarkBase >= pValue;
        case VAR_PhysicalResistance:
            logger->error("Physical resistance isn't used in events");
            return false;
        case VAR_MagicResistance:
            return this->sResMagicBase >= pValue;
        case VAR_FireResistanceBonus:
            return this->sResFireBonus >= pValue;
        case VAR_AirResistanceBonus:
            return this->sResAirBonus >= pValue;
        case VAR_WaterResistanceBonus:
            return this->sResWaterBonus >= pValue;
        case VAR_EarthResistanceBonus:
            return this->sResEarthBonus >= pValue;
        case VAR_SpiritResistanceBonus:
            return this->sResSpiritBonus >= pValue;
        case VAR_MindResistanceBonus:
            return this->sResMindBonus >= pValue;
        case VAR_BodyResistanceBonus:
            return this->sResBodyBonus >= pValue;
        case VAR_LightResistanceBonus:
            return this->sResLightBonus >= pValue;
        case VAR_DarkResistanceBonus:
            return this->sResDarkBonus >= pValue;
        case VAR_MagicResistanceBonus:
            return this->sResMagicBonus >= pValue;
        case VAR_StaffSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_STAFF]);
        case VAR_SwordSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_SWORD]);
        case VAR_DaggerSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_DAGGER]);
        case VAR_AxeSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_AXE]);
        case VAR_SpearSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_SPEAR]);
        case VAR_BowSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_BOW]);
        case VAR_MaceSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_MACE]);
        case VAR_BlasterSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_BLASTER]);
        case VAR_ShieldSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_SHIELD]);
        case VAR_LeatherSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_LEATHER]);
        case VAR_SkillChain:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_CHAIN]);
        case VAR_PlateSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_PLATE]);
        case VAR_FireSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_FIRE]);
        case VAR_AirSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_AIR]);
        case VAR_WaterSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_WATER]);
        case VAR_EarthSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_EARTH]);
        case VAR_SpiritSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_SPIRIT]);
        case VAR_MindSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_MIND]);
        case VAR_BodySkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_BODY]);
        case VAR_LightSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_LIGHT]);
        case VAR_DarkSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_DARK]);
        case VAR_IdentifyItemSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_ITEM_ID]);
        case VAR_MerchantSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_MERCHANT]);
        case VAR_RepairSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_REPAIR]);
        case VAR_BodybuildingSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_BODYBUILDING]);
        case VAR_MeditationSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_MEDITATION]);
        case VAR_PerceptionSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_PERCEPTION]);
        case VAR_DiplomacySkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_DIPLOMACY]);
        case VAR_ThieverySkill:
            // Original binary had this:
            // Error("Thievery isn't used in events");
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_THIEVERY]);
        case VAR_DisarmTrapSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_TRAP_DISARM]);
        case VAR_DodgeSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_DODGE]);
        case VAR_UnarmedSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_UNARMED]);
        case VAR_IdentifyMonsterSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_MONSTER_ID]);
        case VAR_ArmsmasterSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_ARMSMASTER]);
        case VAR_StealingSkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_STEALING]);
        case VAR_AlchemySkill:  // wasn't in the original
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_ALCHEMY]);
        case VAR_LearningSkill:
            return CmpSkillValue(pValue, this->pActiveSkills[SKILL_LEARNING]);
        case VAR_Cursed:
            return conditions.has(CONDITION_CURSED);
        case VAR_Weak:
            return conditions.has(CONDITION_WEAK);
        case VAR_Asleep:
            return conditions.has(CONDITION_SLEEP);
        case VAR_Afraid:
            return conditions.has(CONDITION_FEAR);
        case VAR_Drunk:
            return conditions.has(CONDITION_DRUNK);
        case VAR_Insane:
            return conditions.has(CONDITION_INSANE);
        case VAR_PoisonedGreen:
            return conditions.has(CONDITION_POISON_WEAK);
        case VAR_DiseasedGreen:
            return conditions.has(CONDITION_DISEASE_WEAK);
        case VAR_PoisonedYellow:
            return conditions.has(CONDITION_POISON_MEDIUM);
        case VAR_DiseasedYellow:
            return conditions.has(CONDITION_DISEASE_MEDIUM);
        case VAR_PoisonedRed:
            return conditions.has(CONDITION_POISON_SEVERE);
        case VAR_DiseasedRed:
            return conditions.has(CONDITION_DISEASE_SEVERE);
        case VAR_Paralyzed:
            return conditions.has(CONDITION_PARALYZED);
        case VAR_Unconsious:
            return conditions.has(CONDITION_UNCONSCIOUS);
        case VAR_Dead:
            return conditions.has(CONDITION_DEAD);
        case VAR_Stoned:
            return conditions.has(CONDITION_PETRIFIED);
        case VAR_Eradicated:
            return conditions.has(CONDITION_ERADICATED);
        case VAR_MajorCondition: {
            Condition condition = GetMajorConditionIdx();
            if (condition != CONDITION_GOOD) {
                return std::to_underlying(condition) >= pValue;
            }
            return true;
        }
        case VAR_AutoNotes:
            return pParty->_autonoteBits[pValue];
        case VAR_IsMightMoreThanBase:
            actStat = GetActualMight();
            baseStat = GetBaseMight();
            return (actStat >= baseStat);
        case VAR_IsIntellectMoreThanBase:
            actStat = GetActualIntelligence();
            baseStat = GetBaseIntelligence();
            return (actStat >= baseStat);
        case VAR_IsPersonalityMoreThanBase:
            actStat = GetActualPersonality();
            baseStat = GetBasePersonality();
            return (actStat >= baseStat);
        case VAR_IsEnduranceMoreThanBase:
            actStat = GetActualEndurance();
            baseStat = GetBaseEndurance();
            return (actStat >= baseStat);
        case VAR_IsSpeedMoreThanBase:
            actStat = GetActualSpeed();
            baseStat = GetBaseSpeed();
            return (actStat >= baseStat);
        case VAR_IsAccuracyMoreThanBase:
            actStat = GetActualAccuracy();
            baseStat = GetBaseAccuracy();
            return (actStat >= baseStat);
        case VAR_IsLuckMoreThanBase:
            actStat = GetActualLuck();
            baseStat = GetBaseLuck();
            return (actStat >= baseStat);
        case VAR_PlayerBits:
            return this->_characterEventBits[pValue];
        case VAR_NPCs2:
            return pNPCStats->pNPCData[pValue].Hired();
        case VAR_IsFlying:
            if (pParty->bFlying && pParty->pPartyBuffs[PARTY_BUFF_FLY].Active())
                return true;
            return false;
        case VAR_HiredNPCHasSpeciality:
            return CheckHiredNPCSpeciality((NpcProfession)pValue);
        case VAR_CircusPrises:  // isn't used in MM6 since 0x1D6u is a book of
                                // regeneration
            v4 = 0;
            for (Character &character : pParty->pCharacters) {
                for (InventoryEntry entry : character.inventory.entries()) {
                    ItemId itemId = entry->itemId;

                    switch (itemId) {
                        case ITEM_SPELLBOOK_REGENERATION:
                            ++v4;
                            break;
                        case ITEM_SPELLBOOK_CURE_POISON:
                            v4 += 3;
                            break;
                        case ITEM_SPELLBOOK_LIGHT_BOLT:
                            v4 += 5;
                            break;
                        default:
                            break;
                    }
                }
            }
            return v4 >= pValue;
        case VAR_NumSkillPoints:
            return this->uSkillPoints >= (unsigned int)pValue;
        case VAR_MonthIs:
            return (pParty->uCurrentMonth == (unsigned int)pValue);
        case VAR_Counter1:
        case VAR_Counter2:
        case VAR_Counter3:
        case VAR_Counter4:
        case VAR_Counter5:
        case VAR_Counter6:
        case VAR_Counter7:
        case VAR_Counter8:
        case VAR_Counter9:
        case VAR_Counter10:
        {
            int idx = std::to_underlying(VarNum) - std::to_underlying(VAR_Counter1);
            if (pParty->PartyTimes.CounterEventValues[idx].isValid()) {
                return (pParty->PartyTimes.CounterEventValues[idx] + Duration::fromHours(pValue)) <= pParty->GetPlayingTime();
            }
            return false;
        }

        case VAR_ReputationInCurrentLocation:
            v19 = &currentLocationInfo();
            return (v19->reputation >= pValue);

        case VAR_Unknown1:
            v21 = &currentLocationInfo();
            return (v21->alertStatus == pValue);  // yes, equality, not >=

        case VAR_GoldInBank:
            return pParty->uNumGoldInBank >= (unsigned int)pValue;

        case VAR_NumDeaths:
            return pParty->uNumDeaths >= (unsigned int)pValue;

        case VAR_NumBounties:
            return pParty->uNumBountiesCollected >= (unsigned int)pValue;

        case VAR_PrisonTerms:
            return pParty->uNumPrisonTerms >= pValue;
        case VAR_ArenaWinsPage:
            return pParty->uNumArenaWins[ARENA_LEVEL_PAGE] >= pValue;
        case VAR_ArenaWinsSquire:
            return pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE] >= pValue;
        case VAR_ArenaWinsKnight:
            return pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT] >= pValue;
        case VAR_ArenaWinsLord:
            return pParty->uNumArenaWins[ARENA_LEVEL_LORD] >= pValue;
        case VAR_Invisible:
            return pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active();
        case VAR_ItemEquipped:
            return wearsItem(static_cast<ItemId>(pValue));
        default:
            return false;
    }
}

//----- (0044A5CB) --------------------------------------------------------
void Character::SetVariable(EvtVariable var_type, signed int var_value) {
    int gold{}, food{};
    LocationInfo *ddm;
    Item item;

    if (var_type >= VAR_History_0 && var_type <= VAR_History_28) {
        if (!pParty->PartyTimes.HistoryEventTimes[historyIndex(var_type)]) {
            pParty->PartyTimes.HistoryEventTimes[historyIndex(var_type)] = pParty->GetPlayingTime();
            if (!pHistoryTable->historyLines[1 + historyIndex(var_type)].pText.empty()) {
                bFlashHistoryBook = true;
                PlayAwardSound();
            }
        }
        return;
    }

    if (var_type >= VAR_MapPersistentVariable_0 && var_type <= VAR_MapPersistentVariable_74) {
        engine->_persistentVariables.mapVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentVariable_0)] = (char)var_value;
        return;
    }

    // not really sure whether the number gets up to 99, but can't ignore the possibility
    if (var_type >= VAR_MapPersistentDecorVariable_0 && var_type <= VAR_MapPersistentDecorVariable_24) {
        engine->_persistentVariables.decorVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] = (unsigned char)var_value;
        return;
    }

    if (var_type >= VAR_UnknownTimeEvent0 && var_type <= VAR_UnknownTimeEvent19) {
        pParty->PartyTimes._s_times[std::to_underlying(var_type) - std::to_underlying(VAR_UnknownTimeEvent0)] = pParty->GetPlayingTime();
        PlayAwardSound();
        return;
    }

    switch (var_type) {
        case VAR_Sex:
            this->uSex = (Sex)var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_Class:
            this->classType = (Class)var_value;
            if ((Class)var_value == CLASS_LICH) {
                for (InventoryEntry entry : inventory.entries()) {
                    if (entry->itemId == ITEM_QUEST_LICH_JAR_EMPTY) {
                        entry->itemId = ITEM_QUEST_LICH_JAR_FULL;
                        entry->lichJarCharacterIndex = getCharacterIndex();
                    }
                }
                if (this->sResFireBase < 20) this->sResFireBase = 20;
                if (this->sResAirBase < 20) this->sResAirBase = 20;
                if (this->sResWaterBase < 20) this->sResWaterBase = 20;
                if (this->sResEarthBase < 20) this->sResEarthBase = 20;
                this->sResMindBase = 200;
                this->sResBodyBase = 200;
                Sex sex = this->GetSexByVoice();
                this->uPrevVoiceID = this->uVoiceID;
                this->uPrevFace = this->uCurrentFace;
                if (sex == SEX_FEMALE) {
                    this->uCurrentFace = 21;
                    this->uVoiceID = 21;
                } else {
                    this->uCurrentFace = 20;
                    this->uVoiceID = 20;
                }
                GameUI_ReloadPlayerPortraits(getCharacterIndex(),
                                             this->uCurrentFace);
            }
            PlayAwardSound_Anim();
            return;
        case VAR_CurrentHP:
            this->health = var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_MaxHP:
            this->health = GetMaxHealth();
            return;
        case VAR_CurrentSP:
            this->mana = var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_MaxSP:
            this->mana = GetMaxMana();
            return;
        case VAR_ACModifier:
            this->sACModifier = (uint8_t)var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_BaseLevel:
            this->uLevel = (uint8_t)var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_LevelModifier:
            this->sLevelModifier = (uint8_t)var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_Age:
            this->sAgeModifier = var_value;
            return;
        case VAR_Award:
            if (!this->_achievedAwardsBits[var_value] && !pAwards[var_value].pText.empty()) {
                PlayAwardSound_Anim();
                this->playReaction(SPEECH_AWARD_GOT);
            }
            this->_achievedAwardsBits.set(var_value);
            return;
        case VAR_Experience:
            this->experience = var_value;
            PlayAwardSound_Anim();
            return;
        case VAR_QBits_QuestsDone:
            // TODO(captainurist): qbits value is coming from a script, need to bound-check.
            if (!pParty->_questBits[static_cast<QuestBit>(var_value)] && !pQuestTable[static_cast<QuestBit>(var_value)].empty()) {
                bFlashQuestBook = true;
                spell_fx_renderer->SetPlayerBuffAnim(BECOME_MAGIC_GUILD_MEMBER, getCharacterIndex());
                PlayAwardSound();
                this->playReaction(SPEECH_QUEST_GOT);
            }
            pParty->_questBits.set(static_cast<QuestBit>(var_value));
            return;
        case VAR_PlayerItemInHands:
            item.Reset();
            item.itemId = ItemId(var_value);
            item.flags = ITEM_IDENTIFIED;
            pParty->setHoldingItem(item);
            if (isSpawnableArtifact(ItemId(var_value)))
                pParty->pIsArtifactFound[ItemId(var_value)] = true;
            return;
        case VAR_FixedGold:
            pParty->SetGold(var_value);
            return;
        case VAR_RandomGold:
            gold = grng->random(var_value) + 1;
            pParty->SetGold(gold);
            engine->_statusBar->setEvent(LSTR_YOU_HAVE_LU_GOLD, gold);
            GameUI_DrawFoodAndGold();
            return;
        case VAR_FixedFood:
            pParty->SetFood(var_value);
            PlayAwardSound_Anim();
            return;
        case VAR_RandomFood:
            food = grng->random(var_value) + 1;
            pParty->SetFood(food);
            engine->_statusBar->setEvent(LSTR_YOU_HAVE_LU_FOOD, food);
            GameUI_DrawFoodAndGold();
            PlayAwardSound_Anim();
            return;
        case VAR_BaseMight:
            this->_stats[ATTRIBUTE_MIGHT] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseIntellect:
            this->_stats[ATTRIBUTE_INTELLIGENCE] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BasePersonality:
            this->_stats[ATTRIBUTE_PERSONALITY] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseEndurance:
            this->_stats[ATTRIBUTE_ENDURANCE] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseSpeed:
            this->_stats[ATTRIBUTE_SPEED] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseAccuracy:
            this->_stats[ATTRIBUTE_ACCURACY] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseLuck:
            this->_stats[ATTRIBUTE_LUCK] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MightBonus:
        case VAR_ActualMight:
            this->_statBonuses[ATTRIBUTE_MIGHT] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            this->_statBonuses[ATTRIBUTE_INTELLIGENCE] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            this->_statBonuses[ATTRIBUTE_PERSONALITY] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            this->_statBonuses[ATTRIBUTE_ENDURANCE] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            this->_statBonuses[ATTRIBUTE_SPEED] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            this->_statBonuses[ATTRIBUTE_ACCURACY] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            this->_statBonuses[ATTRIBUTE_LUCK] = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FireResistance:
            this->sResFireBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_AirResistance:
            this->sResAirBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_WaterResistance:
            this->sResWaterBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_EarthResistance:
            this->sResEarthBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_SpiritResistance:
            this->sResSpiritBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MindResistance:
            this->sResMindBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BodyResistance:
            this->sResBodyBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_LightResistance:
            this->sResLightBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_DarkResistance:
            this->sResDarkBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MagicResistance:
            this->sResMagicBase = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FireResistanceBonus:
            this->sResFireBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AirResistanceBonus:
            this->sResAirBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_WaterResistanceBonus:
            this->sResWaterBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EarthResistanceBonus:
            this->sResEarthBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpiritResistanceBonus:
            this->sResSpiritBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_MindResistanceBonus:
            this->sResMindBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_BodyResistanceBonus:
            this->sResBodyBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_LightResistanceBonus:
            this->sResLightBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_DarkResistanceBonus:
            this->sResDarkBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_PhysicalResistanceBonus:
            logger->error("Physical res. bonus not used");
            return;
        case VAR_MagicResistanceBonus:
            this->sResMagicBonus = (uint8_t)var_value;
            PlayAwardSound_Anim_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_Cursed:
            this->SetCondition(CONDITION_CURSED, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Weak:
            this->SetCondition(CONDITION_WEAK, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Asleep:
            this->SetCondition(CONDITION_SLEEP, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Afraid:
            this->SetCondition(CONDITION_FEAR, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Drunk:
            this->SetCondition(CONDITION_DRUNK, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Insane:
            this->SetCondition(CONDITION_INSANE, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_PoisonedGreen:
            this->SetCondition(CONDITION_POISON_WEAK, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_DiseasedGreen:
            this->SetCondition(CONDITION_DISEASE_WEAK, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_PoisonedYellow:
            this->SetCondition(CONDITION_POISON_MEDIUM, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_DiseasedYellow:
            this->SetCondition(CONDITION_DISEASE_MEDIUM, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_PoisonedRed:
            this->SetCondition(CONDITION_POISON_SEVERE, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_DiseasedRed:
            this->SetCondition(CONDITION_DISEASE_SEVERE, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Paralyzed:
            this->SetCondition(CONDITION_PARALYZED, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Unconsious:
            this->SetCondition(CONDITION_UNCONSCIOUS, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Dead:
            this->SetCondition(CONDITION_DEAD, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Stoned:
            this->SetCondition(CONDITION_PETRIFIED, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_Eradicated:
            this->SetCondition(CONDITION_ERADICATED, 1);
            PlayAwardSound_Anim();
            return;
        case VAR_MajorCondition:
            conditions.resetAll();
            PlayAwardSound_Anim();
            return;
        case VAR_AutoNotes:
            assert(var_value > 0);
            if (!pParty->_autonoteBits[var_value] && !pAutonoteTxt[var_value].pText.empty()) {
                spell_fx_renderer->SetPlayerBuffAnim(BECOME_MAGIC_GUILD_MEMBER, getCharacterIndex());
                this->playReaction(SPEECH_AWARD_GOT);
                bFlashAutonotesBook = true;
                autonoteBookDisplayType = pAutonoteTxt[var_value].eType;  // dword_72371C[2 * a3];
            }
            pParty->_autonoteBits.set(var_value);
            PlayAwardSound();
            return;
        case VAR_PlayerBits:
            _characterEventBits.set(var_value);
            return;
        case VAR_NPCs2:
            pParty->hirelingScrollPosition = 0;
            pNPCStats->pNPCData[var_value].uFlags |= NPC_HIRED;
            pParty->CountHirelings();
            return;
        case VAR_NumSkillPoints:
            this->uSkillPoints = var_value;
            return;

        case VAR_Counter1:
        case VAR_Counter2:
        case VAR_Counter3:
        case VAR_Counter4:
        case VAR_Counter5:
        case VAR_Counter6:
        case VAR_Counter7:
        case VAR_Counter8:
        case VAR_Counter9:
        case VAR_Counter10:
            pParty->PartyTimes.CounterEventValues[std::to_underlying(var_type) - std::to_underlying(VAR_Counter1)] = pParty->GetPlayingTime();
            return;

        case VAR_ReputationInCurrentLocation:
            ddm = &currentLocationInfo();
            ddm->reputation = var_value;
            if (var_value > 10000)
                ddm->reputation = 10000;
            return;
        case VAR_GoldInBank:
            pParty->uNumGoldInBank = var_value;
            return;
        case VAR_NumDeaths:
            pParty->uNumDeaths = var_value;
            return;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected = var_value;
            return;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms = var_value;
            return;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaWins[ARENA_LEVEL_PAGE] = var_value;
            return;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE] = var_value;
            return;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT] = var_value;
            return;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaWins[ARENA_LEVEL_LORD] = var_value;
            return;
        case VAR_StaffSkill:
            pActiveSkills[SKILL_STAFF] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_SwordSkill:
            pActiveSkills[SKILL_SWORD] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_DaggerSkill:
            pActiveSkills[SKILL_DAGGER] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_AxeSkill:
            pActiveSkills[SKILL_AXE] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_SpearSkill:
            pActiveSkills[SKILL_SPEAR] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_BowSkill:
            pActiveSkills[SKILL_BOW] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_MaceSkill:
            pActiveSkills[SKILL_MACE] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_BlasterSkill:
            pActiveSkills[SKILL_BLASTER] = CombinedSkillValue::fromJoined(var_value);;
            SetSkillReaction();
            return;
        case VAR_ShieldSkill:
            pActiveSkills[SKILL_SHIELD] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_LeatherSkill:
            pActiveSkills[SKILL_LEATHER] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_SkillChain:
            pActiveSkills[SKILL_CHAIN] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_PlateSkill:
            pActiveSkills[SKILL_PLATE] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_FireSkill:
            pActiveSkills[SKILL_FIRE] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_AirSkill:
            pActiveSkills[SKILL_AIR] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_WaterSkill:
            pActiveSkills[SKILL_WATER] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_EarthSkill:
            pActiveSkills[SKILL_EARTH] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_SpiritSkill:
            pActiveSkills[SKILL_SPIRIT] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_MindSkill:
            pActiveSkills[SKILL_MIND] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_BodySkill:
            pActiveSkills[SKILL_BODY] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_LightSkill:
            pActiveSkills[SKILL_LIGHT] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_DarkSkill:
            pActiveSkills[SKILL_DARK] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_IdentifyItemSkill:
            pActiveSkills[SKILL_ITEM_ID] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_MerchantSkill:
            pActiveSkills[SKILL_MERCHANT] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_RepairSkill:
            pActiveSkills[SKILL_REPAIR] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_BodybuildingSkill:
            pActiveSkills[SKILL_BODYBUILDING] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_MeditationSkill:
            pActiveSkills[SKILL_MEDITATION] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_PerceptionSkill:
            pActiveSkills[SKILL_PERCEPTION] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_DiplomacySkill:
            pActiveSkills[SKILL_DIPLOMACY] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_ThieverySkill:
            logger->error("Thieving unsupported");
            return;
        case VAR_DisarmTrapSkill:
            pActiveSkills[SKILL_TRAP_DISARM] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_DodgeSkill:
            pActiveSkills[SKILL_DODGE] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_UnarmedSkill:
            pActiveSkills[SKILL_UNARMED] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_IdentifyMonsterSkill:
            pActiveSkills[SKILL_MONSTER_ID] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_ArmsmasterSkill:
            pActiveSkills[SKILL_ARMSMASTER] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_StealingSkill:
            pActiveSkills[SKILL_STEALING] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_AlchemySkill:
            pActiveSkills[SKILL_ALCHEMY] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        case VAR_LearningSkill:
            pActiveSkills[SKILL_LEARNING] = CombinedSkillValue::fromJoined(var_value);
            SetSkillReaction();
            return;
        default:
            return;
    }
}

//----- (new function) --------------------------------------------------------
void Character::PlayAwardSound() {
    //int playerIndex = getCharacterIndex();
    //int v25 = Pid(OBJECT_Character, playerIndex + 48);
    //pAudioPlayer->playSound(SOUND_quest, v25);
    pAudioPlayer->playUISound(SOUND_quest);
}

//----- (new function) --------------------------------------------------------
void Character::PlayAwardSound_Anim() {
    int playerIndex = getCharacterIndex();
    spell_fx_renderer->SetPlayerBuffAnim(BECOME_MAGIC_GUILD_MEMBER, playerIndex);
    PlayAwardSound();
}

//----- (new function) --------------------------------------------------------
void Character::PlayAwardSound_Anim_Face(SpeechId speech) {
    this->playReaction(speech);
    PlayAwardSound_Anim();
}

//----- (new function) --------------------------------------------------------
void Character::SetSkillReaction() {
    int playerIndex = getCharacterIndex();
    spell_fx_renderer->SetPlayerBuffAnim(BECOME_MAGIC_GUILD_MEMBER, playerIndex);
    PlayAwardSound();
}

//----- (0044AFFB) --------------------------------------------------------
void Character::AddVariable(EvtVariable var_type, signed int val) {
    int food{};
    LocationInfo *ddm;
    Item item;

    if (var_type >= VAR_Counter1 && var_type <= VAR_Counter10) {
        pParty->PartyTimes.CounterEventValues[std::to_underlying(var_type) - std::to_underlying(VAR_Counter1)] = pParty->GetPlayingTime();
        return;
    }

    if (var_type >= VAR_UnknownTimeEvent0 && var_type <= VAR_UnknownTimeEvent19) {
        pParty->PartyTimes._s_times[std::to_underlying(var_type) - std::to_underlying(VAR_UnknownTimeEvent0)] = pParty->GetPlayingTime();
        PlayAwardSound();
        return;
    }

    if (var_type >= VAR_MapPersistentVariable_0 && var_type <= VAR_MapPersistentVariable_74) {
        if (255 - val > engine->_persistentVariables.mapVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentVariable_0)]) {
            engine->_persistentVariables.mapVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentVariable_0)] += val;
        } else {
            engine->_persistentVariables.mapVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentVariable_0)] = 255;
        }
        return;
    }
    if (var_type >= VAR_MapPersistentDecorVariable_0 && var_type <= VAR_MapPersistentDecorVariable_24) {
        if (255 - val > engine->_persistentVariables.decorVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentDecorVariable_0)]) {
            engine->_persistentVariables.decorVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] += val;
        } else {
            engine->_persistentVariables.decorVars[std::to_underlying(var_type) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] = 255;
        }
        return;
    }

    if (var_type >= VAR_History_0 && var_type <= VAR_History_28) {
        if (!pParty->PartyTimes.HistoryEventTimes[historyIndex(var_type)]) {
            pParty->PartyTimes.HistoryEventTimes[historyIndex(var_type)] = pParty->GetPlayingTime();
            if (!pHistoryTable->historyLines[1 + historyIndex(var_type)].pText.empty()) {
                bFlashHistoryBook = true;
                PlayAwardSound();
            }
        }
        return;
    }

    switch (var_type) {
        case VAR_RandomGold:
            if (val == 0) val = 1;
            pParty->partyFindsGold(grng->random(val) + 1, GOLD_RECEIVE_NOSHARE_MSG);
            GameUI_DrawFoodAndGold();
            return;
        case VAR_RandomFood:
            if (val == 0) val = 1;
            food = grng->random(val) + 1;
            pParty->GiveFood(food);
            engine->_statusBar->setEvent(LSTR_YOU_FIND_LU_FOOD, food);
            GameUI_DrawFoodAndGold();
            PlayAwardSound();
            return;
        case VAR_Sex:
            this->uSex = (Sex)val;
            PlayAwardSound_Anim97();
            return;
        case VAR_Class:
            this->classType = (Class)val;
            PlayAwardSound_Anim97();
            return;
        case VAR_CurrentHP:
            this->health = std::min(this->health + val, this->GetMaxHealth());
            PlayAwardSound_Anim97();
            return;
        case VAR_MaxHP:
            this->_health_related = 0;
            this->uFullHealthBonus = 0;
            this->health = this->GetMaxHealth();
            return;
        case VAR_CurrentSP:
            this->mana = std::min(this->mana + val, this->GetMaxMana());
            PlayAwardSound_Anim97();
            return;
        case VAR_MaxSP:
            this->_mana_related = 0;
            this->uFullManaBonus = 0;
            this->mana = GetMaxMana();
            return;
        case VAR_ACModifier:
            this->sACModifier = std::min(this->sACModifier + val, 255);
            PlayAwardSound_Anim97();
            return;
        case VAR_BaseLevel:
            this->uLevel = std::min(this->uLevel + val, 255);
            PlayAwardSound_Anim97();
            return;
        case VAR_LevelModifier:
            this->sLevelModifier = std::min(this->sLevelModifier + val, 255);
            PlayAwardSound_Anim97();
            return;
        case VAR_Age:
            this->sAgeModifier += val;
            return;
        case VAR_Award:
            if (this->_achievedAwardsBits[val] && !pAwards[val].pText.empty()) {
                PlayAwardSound_Anim97_Face(SPEECH_AWARD_GOT);
            }
            this->_achievedAwardsBits.set(val);
            return;
        case VAR_Experience:
            this->experience = std::min((uint64_t)(this->experience + val), UINT64_C(4000000000));
            PlayAwardSound_Anim97();
            return;
        case VAR_QBits_QuestsDone:
            // TODO(captainurist): quest bit is coming from a script, do range checking here.
            if (!pParty->_questBits[static_cast<QuestBit>(val)] && !pQuestTable[static_cast<QuestBit>(val)].empty()) {
                bFlashQuestBook = true;
                PlayAwardSound_Anim97_Face(SPEECH_QUEST_GOT);
            }
            pParty->_questBits.set(static_cast<QuestBit>(val));
            return;
        case VAR_PlayerItemInHands:
            item.Reset();
            item.flags = ITEM_IDENTIFIED;
            item.itemId = ItemId(val);
            item.postGenerate(ITEM_SOURCE_SCRIPT);

            if (isSpawnableArtifact(ItemId(val)))
                pParty->pIsArtifactFound[ItemId(val)] = true;
            pParty->setHoldingItem(item);
            return;
        case VAR_FixedGold:
            pParty->partyFindsGold(val, GOLD_RECEIVE_NOSHARE_MSG);
            return;
        case VAR_BaseMight:
            this->_stats[ATTRIBUTE_MIGHT] = std::min(this->_stats[ATTRIBUTE_MIGHT] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseIntellect:
            this->_stats[ATTRIBUTE_INTELLIGENCE] = std::min(this->_stats[ATTRIBUTE_INTELLIGENCE] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BasePersonality:
            this->_stats[ATTRIBUTE_PERSONALITY] = std::min(this->_stats[ATTRIBUTE_PERSONALITY] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseEndurance:
            this->_stats[ATTRIBUTE_ENDURANCE] = std::min(this->_stats[ATTRIBUTE_ENDURANCE] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseSpeed:
            this->_stats[ATTRIBUTE_SPEED] = std::min(this->_stats[ATTRIBUTE_SPEED] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseAccuracy:
            this->_stats[ATTRIBUTE_ACCURACY] = std::min(this->_stats[ATTRIBUTE_ACCURACY] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseLuck:
            this->_stats[ATTRIBUTE_LUCK] = std::min(this->_stats[ATTRIBUTE_LUCK] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FixedFood:
            pParty->GiveFood(val);
            engine->_statusBar->setEvent(LSTR_YOU_FIND_LU_FOOD, val);
            PlayAwardSound();
            return;
        case VAR_MightBonus:
        case VAR_ActualMight:
            this->_statBonuses[ATTRIBUTE_MIGHT] = std::min(this->_statBonuses[ATTRIBUTE_MIGHT] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            this->_statBonuses[ATTRIBUTE_INTELLIGENCE] = std::min(this->_statBonuses[ATTRIBUTE_INTELLIGENCE] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            this->_statBonuses[ATTRIBUTE_PERSONALITY] = std::min(this->_statBonuses[ATTRIBUTE_PERSONALITY] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            this->_statBonuses[ATTRIBUTE_ENDURANCE] = std::min(this->_statBonuses[ATTRIBUTE_ENDURANCE] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            this->_statBonuses[ATTRIBUTE_SPEED] = std::min(this->_statBonuses[ATTRIBUTE_SPEED] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            this->_statBonuses[ATTRIBUTE_ACCURACY] = std::min(this->_statBonuses[ATTRIBUTE_ACCURACY] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            this->_statBonuses[ATTRIBUTE_LUCK] = std::min(this->_statBonuses[ATTRIBUTE_LUCK] + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_FireResistance:
            this->sResFireBase = std::min(this->sResFireBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_AirResistance:
            this->sResAirBase = std::min(this->sResAirBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_WaterResistance:
            this->sResWaterBase = std::min(this->sResWaterBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_EarthResistance:
            this->sResEarthBase = std::min(this->sResEarthBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_SpiritResistance:
            this->sResSpiritBase = std::min(this->sResSpiritBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MindResistance:
            this->sResMindBase = std::min(this->sResMindBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BodyResistance:
            this->sResBodyBase = std::min(this->sResBodyBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_LightResistance:
            this->sResLightBase = std::min(this->sResLightBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_DarkResistance:
            this->sResDarkBase = std::min(this->sResDarkBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MagicResistance:
            this->sResMagicBase = std::min(this->sResMagicBase + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FireResistanceBonus:
            this->sResFireBonus = std::min(this->sResFireBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AirResistanceBonus:
            this->sResAirBonus = std::min(this->sResAirBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_WaterResistanceBonus:
            this->sResWaterBonus = std::min(this->sResWaterBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EarthResistanceBonus:
            this->sResEarthBonus = std::min(this->sResEarthBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpiritResistanceBonus:
            this->sResSpiritBonus = std::min(this->sResSpiritBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_MindResistanceBonus:
            this->sResMindBonus = std::min(this->sResMindBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_BodyResistanceBonus:
            this->sResBodyBonus = std::min(this->sResBodyBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_LightResistanceBonus:
            this->sResLightBonus = std::min(this->sResLightBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_DarkResistanceBonus:
            this->sResDarkBonus = std::min(this->sResDarkBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_MagicResistanceBonus:
            this->sResMagicBonus = std::min(this->sResMagicBonus + val, 255);
            PlayAwardSound_Anim97_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_Cursed:
            this->SetCondition(CONDITION_CURSED, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Weak:
            this->SetCondition(CONDITION_WEAK, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Asleep:
            this->SetCondition(CONDITION_SLEEP, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Afraid:
            this->SetCondition(CONDITION_FEAR, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Drunk:
            this->SetCondition(CONDITION_DRUNK, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Insane:
            this->SetCondition(CONDITION_INSANE, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedGreen:
            this->SetCondition(CONDITION_POISON_WEAK, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedGreen:
            this->SetCondition(CONDITION_DISEASE_WEAK, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedYellow:
            this->SetCondition(CONDITION_POISON_MEDIUM, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedYellow:
            this->SetCondition(CONDITION_DISEASE_MEDIUM, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_PoisonedRed:
            this->SetCondition(CONDITION_POISON_SEVERE, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_DiseasedRed:
            this->SetCondition(CONDITION_DISEASE_SEVERE, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Paralyzed:
            this->SetCondition(CONDITION_PARALYZED, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Unconsious:
            this->SetCondition(CONDITION_UNCONSCIOUS, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Dead:
            this->SetCondition(CONDITION_DEAD, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Stoned:
            this->SetCondition(CONDITION_PETRIFIED, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_Eradicated:
            this->SetCondition(CONDITION_ERADICATED, 1);
            PlayAwardSound_Anim97();
            return;
        case VAR_MajorCondition:
            conditions.resetAll();
            PlayAwardSound_Anim97();
            return;
        case VAR_AutoNotes:
            assert(val > 0);
            if (!pParty->_autonoteBits[val] && !pAutonoteTxt[val].pText.empty()) {
                this->playReaction(SPEECH_AWARD_GOT);
                bFlashAutonotesBook = true;
                autonoteBookDisplayType = pAutonoteTxt[val].eType;
                spell_fx_renderer->SetPlayerBuffAnim(SPELL_QUEST_COMPLETED, getCharacterIndex());
            }
            pParty->_autonoteBits.set(val);
            PlayAwardSound();
            return;
        case VAR_PlayerBits:
            _characterEventBits.set(val);
            return;
        case VAR_NPCs2:
            pParty->hirelingScrollPosition = 0;
            pNPCStats->pNPCData[val].uFlags |= NPC_HIRED;
            pParty->CountHirelings();
            return;
        case VAR_NumSkillPoints:
            this->uSkillPoints += val;
            return;
        case VAR_ReputationInCurrentLocation:
            ddm = &currentLocationInfo();
            ddm->reputation += val;
            if (ddm->reputation > 10000)
                ddm->reputation = 10000;
            return;
        case VAR_GoldInBank:
            pParty->uNumGoldInBank += val;
            return;
        case VAR_NumDeaths:
            pParty->uNumDeaths += val;
            return;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected += val;
            return;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms += val;
            return;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaWins[ARENA_LEVEL_PAGE] += val;
            return;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE] += val;
            return;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT] += val;
            return;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaWins[ARENA_LEVEL_LORD] += val;
            return;
        case VAR_StaffSkill:
            AddSkillByEvent(SKILL_STAFF, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_SwordSkill:
            AddSkillByEvent(SKILL_SWORD, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_DaggerSkill:
            AddSkillByEvent(SKILL_DAGGER, val);;
            PlayAwardSound_Anim97();
            return;
        case VAR_AxeSkill:
            AddSkillByEvent(SKILL_AXE, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_SpearSkill:
            AddSkillByEvent(SKILL_SPEAR, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_BowSkill:
            AddSkillByEvent(SKILL_BOW, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_MaceSkill:
            AddSkillByEvent(SKILL_MACE, val);;
            PlayAwardSound_Anim97();
            return;
        case VAR_BlasterSkill:
            AddSkillByEvent(SKILL_BLASTER, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_ShieldSkill:
            AddSkillByEvent(SKILL_SHIELD, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_LeatherSkill:
            AddSkillByEvent(SKILL_LEATHER, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_SkillChain:
            AddSkillByEvent(SKILL_CHAIN, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_PlateSkill:
            AddSkillByEvent(SKILL_PLATE, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_FireSkill:
            AddSkillByEvent(SKILL_FIRE, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_AirSkill:
            AddSkillByEvent(SKILL_AIR, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_WaterSkill:
            AddSkillByEvent(SKILL_WATER, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_EarthSkill:
            AddSkillByEvent(SKILL_EARTH, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_SpiritSkill:
            AddSkillByEvent(SKILL_SPIRIT, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_MindSkill:
            AddSkillByEvent(SKILL_MIND, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_BodySkill:
            AddSkillByEvent(SKILL_BODY, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_LightSkill:
            AddSkillByEvent(SKILL_LIGHT, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_DarkSkill:
            AddSkillByEvent(SKILL_DARK, val);;
            PlayAwardSound_Anim97();
            return;
        case VAR_IdentifyItemSkill:
            AddSkillByEvent(SKILL_ITEM_ID, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_MerchantSkill:
            AddSkillByEvent(SKILL_MERCHANT, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_RepairSkill:
            AddSkillByEvent(SKILL_REPAIR, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_BodybuildingSkill:
            AddSkillByEvent(SKILL_BODYBUILDING, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_MeditationSkill:
            AddSkillByEvent(SKILL_MEDITATION, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_PerceptionSkill:
            AddSkillByEvent(SKILL_PERCEPTION, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_DiplomacySkill:
            AddSkillByEvent(SKILL_DIPLOMACY, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_ThieverySkill:
            logger->error("Thieving unsupported");
            return;
        case VAR_DisarmTrapSkill:
            AddSkillByEvent(SKILL_TRAP_DISARM, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_DodgeSkill:
            AddSkillByEvent(SKILL_DODGE, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_UnarmedSkill:
            AddSkillByEvent(SKILL_UNARMED, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_IdentifyMonsterSkill:
            AddSkillByEvent(SKILL_MONSTER_ID, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_ArmsmasterSkill:
            AddSkillByEvent(SKILL_ARMSMASTER, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_StealingSkill:
            AddSkillByEvent(SKILL_STEALING, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_AlchemySkill:
            AddSkillByEvent(SKILL_ALCHEMY, val);
            PlayAwardSound_Anim97();
            return;
        case VAR_LearningSkill:
            AddSkillByEvent(SKILL_LEARNING, val);
            PlayAwardSound_Anim97();
            return;
        default:
            return;
    }
}

//----- (new function) --------------------------------------------------------
void Character::PlayAwardSound_Anim97() {
    int playerIndex = getCharacterIndex();
    spell_fx_renderer->SetPlayerBuffAnim(SPELL_QUEST_COMPLETED, playerIndex);
    PlayAwardSound();
}

//----- (new function) --------------------------------------------------------
void Character::PlayAwardSound_Anim97_Face(SpeechId speech) {
    this->playReaction(speech);
    PlayAwardSound_Anim97();
}

//----- (new function) --------------------------------------------------------
void Character::AddSkillByEvent(Skill skill, uint16_t addSkillValue) {
    auto [addLevel, addMastery] = CombinedSkillValue::fromJoinedUnchecked(addSkillValue);

    int newLevel = pActiveSkills[skill].level() + addLevel;
    Mastery newMastery = std::max(pActiveSkills[skill].mastery(), addMastery);

    pActiveSkills[skill] = CombinedSkillValue(newLevel, newMastery);
}

//----- (0044B9C4) --------------------------------------------------------
void Character::SubtractVariable(EvtVariable VarNum, signed int pValue) {
    LocationInfo *locationHeader;  // eax@90
    int randGold;
    int randFood;

    if (VarNum >= VAR_MapPersistentVariable_0 && VarNum <= VAR_MapPersistentVariable_74) {
        engine->_persistentVariables.mapVars[std::to_underlying(VarNum) - std::to_underlying(VAR_MapPersistentVariable_0)] -= pValue;
        return;
    }
    if (VarNum >= VAR_MapPersistentDecorVariable_0 && VarNum <= VAR_MapPersistentDecorVariable_24) {
        engine->_persistentVariables.decorVars[std::to_underlying(VarNum) - std::to_underlying(VAR_MapPersistentDecorVariable_0)] -= pValue;
        return;
    }

    switch (VarNum) {
        case VAR_CurrentHP:
            receiveDamage((signed int)pValue, DAMAGE_PHYSICAL);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_CurrentSP:
            this->mana = std::max(this->mana - pValue, 0);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_ACModifier:
            this->sACModifier -= (uint8_t)pValue;
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_BaseLevel:
            this->uLevel -= (uint8_t)pValue;
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_LevelModifier:
            this->sLevelModifier -= (uint8_t)pValue;
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Age:
            this->sAgeModifier -= (int16_t)pValue;
            return;
        case VAR_Award:
            this->_achievedAwardsBits.reset(pValue);
            return;
        case VAR_Experience:
            this->experience -= pValue;
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_QBits_QuestsDone:
            // TODO(captainurist): quest bit is coming from a script, do range checking here.
            pParty->_questBits.reset(static_cast<QuestBit>(pValue));
            this->playReaction(SPEECH_AWARD_GOT);
            return;
        case VAR_PlayerItemInHands:
            for (InventoryEntry entry : inventory.entries()) {
                if (entry->itemId == static_cast<ItemId>(pValue)) {
                    inventory.take(entry);
                    return;
                }
            }
            if (pParty->pPickedItem.itemId == static_cast<ItemId>(pValue)) {
                pParty->takeHoldingItem();
                return;
            }
            return;
        case VAR_FixedGold:
            if (pValue > pParty->GetGold()) {
                dword_5B65C4_cancelEventProcessing = 1;
                return;
            }
            pParty->TakeGold(pValue);
            return;
        case VAR_RandomGold:
            randGold = grng->random(pValue) + 1;
            if (randGold > pParty->GetGold())
                randGold = pParty->GetGold();
            pParty->TakeGold(randGold);
            engine->_statusBar->setEvent(LSTR_YOU_LOSE_LU_GOLD, randGold);
            GameUI_DrawFoodAndGold();
            return;
        case VAR_FixedFood:
            pParty->TakeFood(pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_RandomFood:
            randFood = grng->random(pValue) + 1;
            if (randFood > pParty->GetFood())
                randFood = pParty->GetFood();
            pParty->TakeFood(randFood);
            engine->_statusBar->setEvent(LSTR_YOU_LOSE_LU_FOOD, randFood);
            GameUI_DrawFoodAndGold();
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_MightBonus:
        case VAR_ActualMight:
            this->_statBonuses[ATTRIBUTE_MIGHT] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_IntellectBonus:
        case VAR_ActualIntellect:
            this->_statBonuses[ATTRIBUTE_INTELLIGENCE] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_PersonalityBonus:
        case VAR_ActualPersonality:
            this->_statBonuses[ATTRIBUTE_PERSONALITY] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EnduranceBonus:
        case VAR_ActualEndurance:
            this->_statBonuses[ATTRIBUTE_ENDURANCE] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpeedBonus:
        case VAR_ActualSpeed:
            this->_statBonuses[ATTRIBUTE_SPEED] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_AccuracyBonus:
        case VAR_ActualAccuracy:
            this->_statBonuses[ATTRIBUTE_ACCURACY] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_LuckBonus:
        case VAR_ActualLuck:
            this->_statBonuses[ATTRIBUTE_LUCK] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_BaseMight:
            this->_stats[ATTRIBUTE_MIGHT] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseIntellect:
            this->_stats[ATTRIBUTE_INTELLIGENCE] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BasePersonality:
            this->_stats[ATTRIBUTE_PERSONALITY] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseEndurance:
            this->_stats[ATTRIBUTE_ENDURANCE] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseSpeed:
            this->_stats[ATTRIBUTE_SPEED] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseAccuracy:
            this->_stats[ATTRIBUTE_ACCURACY] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BaseLuck:
            this->_stats[ATTRIBUTE_LUCK] -= (uint16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FireResistance:
            this->sResFireBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_AirResistance:
            this->sResAirBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_WaterResistance:
            this->sResWaterBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_EarthResistance:
            this->sResEarthBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_SpiritResistance:
            this->sResSpiritBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MindResistance:
            this->sResMindBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_BodyResistance:
            this->sResBodyBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_LightResistance:
            this->sResLightBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_DarkResistance:
            this->sResDarkBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_MagicResistance:
            this->sResMagicBase -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_FireResistanceBonus:
            this->sResFireBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_AirResistanceBonus:
            this->sResAirBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BASE_INC);
            return;
        case VAR_WaterResistanceBonus:
            this->sResWaterBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_EarthResistanceBonus:
            this->sResEarthBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_SpiritResistanceBonus:
            this->sResSpiritBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_MindResistanceBonus:
            this->sResMindBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_BodyResistanceBonus:
            this->sResBodyBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_LightResistanceBonus:
            this->sResLightBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_DarkResistanceBonus:
            this->sResDarkBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_MagicResistanceBonus:
            this->sResMagicBonus -= (int16_t)pValue;
            this->PlayAwardSound_AnimSubtract_Face(SPEECH_STAT_BONUS_INC);
            return;
        case VAR_StaffSkill:
            SubtractSkillByEvent(SKILL_STAFF, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_SwordSkill:
            SubtractSkillByEvent(SKILL_SWORD, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_DaggerSkill:
            SubtractSkillByEvent(SKILL_DAGGER, pValue);;
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_AxeSkill:
            SubtractSkillByEvent(SKILL_AXE, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_SpearSkill:
            SubtractSkillByEvent(SKILL_BOW, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_BowSkill:
            SubtractSkillByEvent(SKILL_BOW, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_MaceSkill:
            SubtractSkillByEvent(SKILL_MACE, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_BlasterSkill:
            SubtractSkillByEvent(SKILL_BLASTER, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_ShieldSkill:
            SubtractSkillByEvent(SKILL_SHIELD, pValue);;
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_LeatherSkill:
            SubtractSkillByEvent(SKILL_LEATHER, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_SkillChain:
            SubtractSkillByEvent(SKILL_CHAIN, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_PlateSkill:
            SubtractSkillByEvent(SKILL_PLATE, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_FireSkill:
            SubtractSkillByEvent(SKILL_FIRE, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_AirSkill:
            SubtractSkillByEvent(SKILL_AIR, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_WaterSkill:
            SubtractSkillByEvent(SKILL_WATER, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_EarthSkill:
            SubtractSkillByEvent(SKILL_EARTH, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_SpiritSkill:
            SubtractSkillByEvent(SKILL_SPIRIT, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_MindSkill:
            SubtractSkillByEvent(SKILL_MIND, pValue);;
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_BodySkill:
            SubtractSkillByEvent(SKILL_BODY, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_LightSkill:
            SubtractSkillByEvent(SKILL_LIGHT, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_DarkSkill:
            SubtractSkillByEvent(SKILL_DARK, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_IdentifyItemSkill:
            SubtractSkillByEvent(SKILL_ITEM_ID, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_MerchantSkill:
            SubtractSkillByEvent(SKILL_MERCHANT, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_RepairSkill:
            SubtractSkillByEvent(SKILL_REPAIR, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_BodybuildingSkill:
            SubtractSkillByEvent(SKILL_BODYBUILDING, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_MeditationSkill:
            SubtractSkillByEvent(SKILL_MEDITATION, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_PerceptionSkill:
            SubtractSkillByEvent(SKILL_PERCEPTION, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_DiplomacySkill:
            SubtractSkillByEvent(SKILL_DIPLOMACY, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_ThieverySkill:
            logger->error("Thieving unsupported");
            return;
        case VAR_DisarmTrapSkill:
            SubtractSkillByEvent(SKILL_TRAP_DISARM, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_DodgeSkill:
            SubtractSkillByEvent(SKILL_DODGE, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_UnarmedSkill:
            SubtractSkillByEvent(SKILL_UNARMED, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_IdentifyMonsterSkill:
            SubtractSkillByEvent(SKILL_MONSTER_ID, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_ArmsmasterSkill:
            SubtractSkillByEvent(SKILL_ARMSMASTER, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_StealingSkill:
            SubtractSkillByEvent(SKILL_STEALING, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_AlchemySkill:
            SubtractSkillByEvent(SKILL_ALCHEMY, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_LearningSkill:
            SubtractSkillByEvent(SKILL_LEARNING, pValue);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Cursed:
            this->conditions.reset(CONDITION_CURSED);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Weak:
            this->conditions.reset(CONDITION_WEAK);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Asleep:
            this->conditions.reset(CONDITION_SLEEP);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Afraid:
            this->conditions.reset(CONDITION_FEAR);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Drunk:
            this->conditions.reset(CONDITION_DRUNK);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Insane:
            this->conditions.reset(CONDITION_INSANE);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_PoisonedGreen:
            this->conditions.reset(CONDITION_POISON_WEAK);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_DiseasedGreen:
            this->conditions.reset(CONDITION_DISEASE_WEAK);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_PoisonedYellow:
            this->conditions.reset(CONDITION_POISON_MEDIUM);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_DiseasedYellow:
            this->conditions.reset(CONDITION_DISEASE_MEDIUM);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_PoisonedRed:
            this->conditions.reset(CONDITION_POISON_SEVERE);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_DiseasedRed:
            this->conditions.reset(CONDITION_DISEASE_SEVERE);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Paralyzed:
            this->conditions.reset(CONDITION_PARALYZED);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Unconsious:
            this->conditions.reset(CONDITION_UNCONSCIOUS);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Dead:
            this->conditions.reset(CONDITION_DEAD);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Stoned:
            this->conditions.reset(CONDITION_PETRIFIED);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_Eradicated:
            this->conditions.reset(CONDITION_ERADICATED);
            PlayAwardSound_AnimSubtract();
            return;
        case VAR_AutoNotes:
            // TODO(Nik-RE-dev): decreasing 1 seems wrong, also bits indexing was changed
            assert(false);
            //pParty->_autonoteBits.reset(pValue - 1);
            return;
        case VAR_PlayerBits:
            _characterEventBits.reset(pValue);
            return;
        case VAR_NPCs2:
            if (getNPCType(speakingNpcId) == NPC_TYPE_QUEST && speakingNpcId == pValue) {
                npcIdToDismissAfterDialogue = pValue;
            } else {
                npcIdToDismissAfterDialogue = 0;
                pParty->hirelingScrollPosition = 0;
                pNPCStats->pNPCData[(int)pValue].uFlags &= ~NPC_HIRED;
                pParty->CountHirelings();
            }
            return;
        case VAR_HiredNPCHasSpeciality:
            for (unsigned int i = 0; i < pNPCStats->uNumNewNPCs; i++) {
                if (pNPCStats->pNPCData[i].profession == (NpcProfession)pValue) {
                    pNPCStats->pNPCData[(int)pValue].uFlags &= ~NPC_HIRED;
                }
            }
            if (pParty->pHirelings[0].profession == (NpcProfession)pValue) {
                pParty->pHirelings[0] = NPCData();
            }
            if (pParty->pHirelings[1].profession == (NpcProfession)pValue) {
                pParty->pHirelings[1] = NPCData();
            }
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            return;
        case VAR_NumSkillPoints:
            if ((unsigned int)pValue <= this->uSkillPoints) {
                this->uSkillPoints -= pValue;
            } else {
                this->uSkillPoints = 0;
            }
            return;
        case VAR_ReputationInCurrentLocation:
            locationHeader = &currentLocationInfo();
            locationHeader->reputation -= pValue;
            if (locationHeader->reputation < -10000)
                locationHeader->reputation = -10000;
            return;
        case VAR_GoldInBank:
            if ((unsigned int)pValue <= pParty->uNumGoldInBank) {
                pParty->uNumGoldInBank -= (unsigned int)pValue;
            } else {
                dword_5B65C4_cancelEventProcessing = 1;
            }
            return;
        case VAR_NumDeaths:
            pParty->uNumDeaths -= (unsigned int)pValue;
            return;
        case VAR_NumBounties:
            pParty->uNumBountiesCollected -= (unsigned int)pValue;
            return;
        case VAR_PrisonTerms:
            pParty->uNumPrisonTerms -= (int)pValue;
            return;
        case VAR_ArenaWinsPage:
            pParty->uNumArenaWins[ARENA_LEVEL_PAGE] -= (char)pValue;
            return;
        case VAR_ArenaWinsSquire:
            pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE] -= (char)pValue;
            return;
        case VAR_ArenaWinsKnight:
            pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT] -= (char)pValue;
            return;
        case VAR_ArenaWinsLord:
            pParty->uNumArenaWins[ARENA_LEVEL_LORD] -= (char)pValue;
            return;
        default:
            return;
    }
}

//----- (new function) --------------------------------------------------------
void Character::PlayAwardSound_AnimSubtract() {
    int playerIndex = getCharacterIndex();
    spell_fx_renderer->SetPlayerBuffAnim(SPELL_STAT_DECREASE, playerIndex);
    PlayAwardSound();
}

//----- (new function) --------------------------------------------------------
void Character::PlayAwardSound_AnimSubtract_Face(SpeechId speech) {
    this->playReaction(speech);
    PlayAwardSound_AnimSubtract();
}

//----- (new function) --------------------------------------------------------
void Character::SubtractSkillByEvent(Skill skill, uint16_t subSkillValue) {
    auto [subLevel, subMastery] = CombinedSkillValue::fromJoinedUnchecked(subSkillValue);

    if (pActiveSkills[skill] == CombinedSkillValue::none())
        return; // Already at zero!

    int newLevel = std::max(1, pActiveSkills[skill].level() - subLevel);
    pActiveSkills[skill] = CombinedSkillValue(newLevel, pActiveSkills[skill].mastery());
    // TODO(pskelton): check - should this be able to forget a skill '0' or min of '1'
    // TODO(pskelton): check - should this modify mastery as well
}

int cycleCharacter(bool backwards) {
    int currentId = pParty->activeCharacterIndex() - 1;

    for (int i = 0; i < pParty->pCharacters.size(); i++) {
        currentId += (backwards ? -1 : 1);

        if (currentId < 0) {
            currentId = pParty->pCharacters.size() - 1;
        } else if (currentId >= pParty->pCharacters.size()) {
            currentId = 0;
        }
        if (!pParty->pCharacters[currentId].timeToRecovery) {
            return currentId + 1;
        }
    }

    return pParty->activeCharacterIndex();
}

bool Character::hasUnderwaterSuitEquipped() const {
    InventoryConstEntry armor = inventory.entry(ITEM_SLOT_ARMOUR);
    return armor && armor->itemId == ITEM_QUEST_WETSUIT;
}

//----- (0043EDB9) --------------------------------------------------------
bool ShouldLoadTexturesForRaceAndGender(unsigned int _this) {
    Race race;  // edi@2
    Sex sex;       // eax@2

    for (Character &character : pParty->pCharacters) {
        race = character.GetRace();
        sex = character.GetSexByVoice();
        switch (_this) {
            case 0:
                if ((race == RACE_HUMAN ||
                     race == RACE_ELF ||
                     race == RACE_GOBLIN) &&
                    sex == SEX_MALE)
                    return true;
                break;
            case 1:
                if ((race == RACE_HUMAN ||
                     race == RACE_ELF ||
                     race == RACE_GOBLIN) &&
                    sex == SEX_FEMALE)
                    return true;
                break;
            case 2:
                if (race == RACE_DWARF && sex == SEX_MALE)
                    return true;
                break;
            case 3:
                if (race == RACE_DWARF && sex == SEX_FEMALE)
                    return true;
                break;
        }
    }
    return false;
}

//----- (0043ED6F) --------------------------------------------------------
bool IsDwarfPresentInParty(bool a1) {
    for (Character &character : pParty->pCharacters) {
        Race race = character.GetRace();

        if (race == RACE_DWARF && a1)
            return true;
        else if (race != RACE_DWARF && !a1)
            return true;
    }
    return false;
}

//----- (00439FCB) --------------------------------------------------------
void DamageCharacterFromMonster(Pid uObjID, ActorAbility dmgSource, signed int targetchar) {
    // target character? if any

    SpellId spellId;
    signed int recvdMagicDmg;     // eax@139
    int healthBeforeRecvdDamage;  // [sp+48h] [bp-Ch]@3

    ObjectType pidtype = uObjID.type();

    /*    OBJECT_None = 0x0,
    OBJECT_Door = 0x1,
    OBJECT_Sprite = 0x2,
    OBJECT_Actor = 0x3,
    OBJECT_Character = 0x4,
    OBJECT_Decoration = 0x5,
    OBJECT_Face = 0x6,*/

    if (pidtype != OBJECT_Sprite) {  // not an item
        // hit by monster
        if (pidtype != OBJECT_Actor) assert(false);

        if (targetchar == -1) assert(false);

        unsigned int uActorID = uObjID.id();

        Character *playerPtr = &pParty->pCharacters[targetchar];
        Actor *actorPtr = &pActors[uActorID];
        healthBeforeRecvdDamage = playerPtr->health;
        if (uObjID.type() != OBJECT_Actor || !actorPtr->ActorHitOrMiss(playerPtr))
            return;

        // GM unarmed 1% chance to evade attacks per skill point
        if (playerPtr->getActualSkillValue(SKILL_UNARMED).mastery() >= MASTERY_GRANDMASTER &&
            grng->random(100) < playerPtr->getActualSkillValue(SKILL_UNARMED).level()) {
            engine->_statusBar->setEvent(LSTR_S_EVADES_DAMAGE, playerPtr->name);
            playerPtr->playReaction(SPEECH_AVOID_DAMAGE);
            return;
        }

        // play hit sound
        InventoryConstEntry armor = playerPtr->inventory.functionalEntry(ITEM_SLOT_ARMOUR);
        SoundId soundToPlay;
        if (!armor || (armor->skill() != SKILL_CHAIN && armor->skill() != SKILL_PLATE)) {
            soundToPlay = vrng->randomSample({SOUND_dull_armor_strike1, SOUND_dull_armor_strike2, SOUND_dull_armor_strike3, SOUND_dull_strike});
        } else {
            soundToPlay = vrng->randomSample({SOUND_metal_armor_strike1, SOUND_metal_armor_strike2, SOUND_metal_armor_strike3, SOUND_metal_vs_metal01h});
        }
        pAudioPlayer->playNonResetableSound(soundToPlay);

        // calc damage
        int dmgToReceive = actorPtr->_43B3E0_CalcDamage(dmgSource);
        if (actorPtr->buffs[ACTOR_BUFF_SHRINK].Active()) {
            int16_t spellPower = actorPtr->buffs[ACTOR_BUFF_SHRINK].power;
            if (spellPower > 0)
                dmgToReceive /= spellPower;
        }

        DamageType damageType;
        switch (dmgSource) {
            case ABILITY_ATTACK1:
                damageType = actorPtr->monsterInfo.attack1Type;
                break;
            case ABILITY_ATTACK2:
                damageType = actorPtr->monsterInfo.attack2Type;
                break;
            case ABILITY_SPELL1:
                spellId = actorPtr->monsterInfo.spell1Id;
                damageType = pSpellStats->pInfos[spellId].damageType;
                break;
            case ABILITY_SPELL2:
                spellId = actorPtr->monsterInfo.spell2Id;
                damageType = pSpellStats->pInfos[spellId].damageType;
                break;
            case ABILITY_SPECIAL:
                damageType = static_cast<DamageType>(actorPtr->monsterInfo.field_3C_some_special_attack);
                break;
            default:
                damageType = DAMAGE_PHYSICAL;
                break;
        }

        // calc damage
        dmgToReceive = playerPtr->receiveDamage(dmgToReceive, damageType);

        // pain reflection back on attacker
        if (playerPtr->pCharacterBuffs[CHARACTER_BUFF_PAIN_REFLECTION].Active()) {
            AIState actorState = actorPtr->aiState;
            if (actorState != Dying && actorState != Dead) {
                int reflectedDamage = actorPtr->CalcMagicalDamageToActor(damageType, dmgToReceive);
                actorPtr->currentHP -= reflectedDamage;
                if (reflectedDamage >= 0) {
                    if (actorPtr->currentHP >= 1) {
                        Actor::AI_Stun(uActorID, Pid(OBJECT_Character, targetchar), 0);  // todo extract this branch to a function
                                    // once Actor::functions are changed to
                                    // nonstatic actor functions
                        Actor::AggroSurroundingPeasants(uActorID, 1);
                    } else {
                        // actor has died from retaliation
                        Actor::Die(uActorID);
                        Actor::ApplyFineForKillingPeasant(uActorID);
                        Actor::AggroSurroundingPeasants(uActorID, 1);
                        if (actorPtr->monsterInfo.exp)
                            pParty->GivePartyExp(pMonsterStats->infos[actorPtr->monsterInfo.id].exp);

                        // kill speech
                        SpeechId speechToPlay = SPEECH_ATTACK_HIT;
                        if (vrng->random(100) < 20) {
                            speechToPlay = actorPtr->monsterInfo.hp >= 100 ? SPEECH_KILL_STRONG_ENEMY : SPEECH_KILL_WEAK_ENEMY;
                        }
                        playerPtr->playReaction(speechToPlay);
                    }
                }
            }
        }

        // special attack trigger
        if (!engine->config->debug.NoDamage.value() && actorPtr->monsterInfo.specialAttackType != SPECIAL_ATTACK_NONE &&
            grng->random(100) < actorPtr->monsterInfo.level * actorPtr->monsterInfo.specialAttackLevel) {
            playerPtr->ReceiveSpecialAttackEffect(actorPtr->monsterInfo.specialAttackType, actorPtr);
        }

        // add recovery after being hit
        if (!pParty->bTurnBasedModeOn) {
            int actEndurance = playerPtr->GetActualEndurance();
            Duration recoveryTime = Duration::fromTicks((20 - playerPtr->GetParameterBonus(actEndurance)) *
                      debug_non_combat_recovery_mul * flt_debugrecmod3);
            playerPtr->SetRecoveryTime(recoveryTime);
        }

        // badly hurt speech
        int yellThreshold = playerPtr->GetMaxHealth() / 4;
        if (yellThreshold > playerPtr->health &&
            yellThreshold <= healthBeforeRecvdDamage &&
            playerPtr->health > 0) {
            playerPtr->playReaction(SPEECH_BADLY_HURT);
        }
        return;
    } else {  // is an item
        int spriteId = uObjID.id();
        SpriteObject *spritefrom = &pSpriteObjects[spriteId];
        ObjectType uActorType = spritefrom->spell_caster_pid.type();
        int uActorID = spritefrom->spell_caster_pid.id();

        if (uActorType == OBJECT_Sprite) {
            Character *playerPtr;  // eax@81

            // select char target or pick random
            if (targetchar != -1) {
                playerPtr = &pParty->pCharacters[targetchar];
            } else {
                int id = pParty->getRandomActiveCharacterId(grng);

                if (id != -1) {
                    playerPtr = &pParty->pCharacters[id];
                } else {
                    // for rare instances where party is "dead" at this point but still being damaged
                    playerPtr = &pParty->pCharacters[grng->random(3)];
                }
            }

            int damage;
            DamageType damagetype;
            if (uActorType != OBJECT_Character ||spritefrom->uSpellID != SPELL_BOW_ARROW) {
                int playerMaxHp = playerPtr->GetMaxHealth();
                damage = CalcSpellDamage(spritefrom->uSpellID,
                                         spritefrom->spell_level,
                                         spritefrom->spell_skill, playerMaxHp);
                damagetype = pSpellStats->pInfos[spritefrom->uSpellID].damageType;
            } else {
                damage = pParty->pCharacters[uActorID].CalculateRangedDamageTo(MONSTER_INVALID);
                damagetype = DAMAGE_FIRE; // TODO(captainurist): doesn't look like a proper default.
            }
            playerPtr->receiveDamage(damage, damagetype);
            if (uActorType == OBJECT_Character) {
                pParty->setDelayedReaction(SPEECH_DAMAGED_PARTY, uActorID);
            }
            return;
        } else if (uActorType == OBJECT_Actor) {  // missile fired by actor
            Actor *actorPtr = &pActors[uActorID];
            if (targetchar == -1) targetchar = stru_50C198.which_player_to_attack(actorPtr);
            Character *playerPtr = &pParty->pCharacters[targetchar];
            int dmgToReceive = actorPtr->_43B3E0_CalcDamage(dmgSource);
            SpriteId spriteType = spritefrom->uType;

            if (spriteType == SPRITE_PROJECTILE_ARROW) {  // arrows
                // GM unarmed 1% chance to evade attack per skill point
                if (playerPtr->getActualSkillValue(SKILL_UNARMED).mastery() >= MASTERY_GRANDMASTER &&
                    grng->random(100) < playerPtr->getActualSkillValue(SKILL_UNARMED).level()) {
                    engine->_statusBar->setEvent(LSTR_S_EVADES_DAMAGE, playerPtr->name);
                    playerPtr->playReaction(SPEECH_AVOID_DAMAGE);
                    return;
                }
            }

            // TODO(captainurist): I don't think magic projectiles should be in this list.
            if (spriteType == SPRITE_PROJECTILE_ARROW ||
                       spriteType == SPRITE_PROJECTILE_BLASTER ||
                       spriteType == SPRITE_PROJECTILE_AIR_BOLT ||  // dragonflies firebolt
                       spriteType == SPRITE_PROJECTILE_EARTH_BOLT ||
                       spriteType == SPRITE_PROJECTILE_FIRE_BOLT ||
                       spriteType == SPRITE_PROJECTILE_WATER_BOLT ||
                       spriteType == SPRITE_PROJECTILE_BODY_BOLT ||
                       spriteType == SPRITE_PROJECTILE_MIND_BOLT ||
                       spriteType == SPRITE_PROJECTILE_SPIRIT_BOLT ||
                       spriteType == SPRITE_PROJECTILE_LIGHT_BOLT ||
                       spriteType == SPRITE_PROJECTILE_DARK_BOLT) {
                if (!actorPtr->ActorHitOrMiss(playerPtr))
                    return;

                bool shielded = false;
                if (playerPtr->pCharacterBuffs[CHARACTER_BUFF_SHIELD].Active())
                    shielded = true;
                if (pParty->pPartyBuffs[PARTY_BUFF_SHIELD].Active())
                    shielded = true;
                if (playerPtr->wearsEnchantedItem(ITEM_ENCHANTMENT_OF_SHIELDING))
                    shielded = true;
                if (playerPtr->wearsEnchantedItem(ITEM_ENCHANTMENT_OF_STORM))
                    shielded = true;
                if (playerPtr->wearsItem(ITEM_ARTIFACT_GOVERNORS_ARMOR))
                    shielded = true;
                if (playerPtr->wearsItem(ITEM_RELIC_KELEBRIM))
                    shielded = true;
                if (playerPtr->wearsItem(ITEM_ARTIFACT_ELFBANE))
                    shielded = true;
                if (InventoryConstEntry offHandItem = playerPtr->inventory.functionalEntry(ITEM_SLOT_OFF_HAND);
                    offHandItem && offHandItem->isShield() && playerPtr->getActualSkillValue(SKILL_SHIELD).mastery() == MASTERY_GRANDMASTER)
                    shielded = true;
                if (shielded)
                    dmgToReceive >>= 1;
            }

            if (actorPtr->buffs[ACTOR_BUFF_SHRINK].Active()) {
                int spellPower = actorPtr->buffs[ACTOR_BUFF_SHRINK].power;
                if (spellPower > 0) dmgToReceive /= spellPower;
            }

            DamageType damageType;
            switch (dmgSource) {
                case ABILITY_ATTACK1:
                    damageType = actorPtr->monsterInfo.attack1Type;
                    break;
                case ABILITY_ATTACK2:
                    damageType = actorPtr->monsterInfo.attack2Type;
                    break;
                case ABILITY_SPELL1:
                    spellId = actorPtr->monsterInfo.spell1Id;
                    damageType = pSpellStats->pInfos[spellId].damageType;
                    break;
                case ABILITY_SPELL2:
                    spellId = actorPtr->monsterInfo.spell2Id;
                    damageType = pSpellStats->pInfos[spellId].damageType;
                    break;
                case ABILITY_SPECIAL:
                    damageType = static_cast<DamageType>(actorPtr->monsterInfo.field_3C_some_special_attack);
                    break;
                default:
                    damageType = DAMAGE_PHYSICAL;
                    break;
            }

            int reflectedDmg = playerPtr->receiveDamage(dmgToReceive, damageType);
            if (playerPtr->pCharacterBuffs[CHARACTER_BUFF_PAIN_REFLECTION].Active()) {
                AIState actorState = actorPtr->aiState;
                if (actorState != Dying && actorState != Dead) {
                    recvdMagicDmg = actorPtr->CalcMagicalDamageToActor(damageType, reflectedDmg);
                    actorPtr->currentHP -= recvdMagicDmg;

                    if (recvdMagicDmg >= 0) {
                        if (actorPtr->currentHP >= 1) {
                            Actor::AI_Stun(uActorID, Pid(OBJECT_Character, targetchar), 0);
                            Actor::AggroSurroundingPeasants(uActorID, 1);
                        } else {
                            // actor killed by retaliation
                            Actor::Die(uActorID);
                            Actor::ApplyFineForKillingPeasant(uActorID);
                            Actor::AggroSurroundingPeasants(uActorID, 1);
                            if (actorPtr->monsterInfo.exp)
                                pParty->GivePartyExp(pMonsterStats->infos[actorPtr->monsterInfo.id].exp);

                            SpeechId speechToPlay = SPEECH_ATTACK_HIT;
                            if (vrng->random(100) < 20) {
                                speechToPlay = actorPtr->monsterInfo.hp >= 100 ? SPEECH_KILL_STRONG_ENEMY : SPEECH_KILL_WEAK_ENEMY;
                            }
                            playerPtr->playReaction(speechToPlay);
                        }
                    }
                }
            }

            // special attack trigger
            if (dmgSource == ABILITY_ATTACK1 && !engine->config->debug.NoDamage.value() &&
                actorPtr->monsterInfo.specialAttackType != SPECIAL_ATTACK_NONE &&
                grng->random(100) < actorPtr->monsterInfo.level * actorPtr->monsterInfo.specialAttackLevel) {
                playerPtr->ReceiveSpecialAttackEffect(actorPtr->monsterInfo.specialAttackType, actorPtr);
            }

            // set recovery after hit
            if (!pParty->bTurnBasedModeOn) {
                int actEnd = playerPtr->GetActualEndurance();
                Duration recTime =
                    Duration::fromTicks((20 - playerPtr->GetParameterBonus(actEnd)) *
                          debug_non_combat_recovery_mul * flt_debugrecmod3);
                playerPtr->SetRecoveryTime(recTime);
            }
            return;
        } else {
            // party hits self
            Character *playerPtr = &pParty->pCharacters[targetchar];
            int damage;
            DamageType damagetype;
            if (uActorType != OBJECT_Character ||
                spritefrom->uSpellID != SPELL_BOW_ARROW) {
                int playerMaxHp = playerPtr->GetMaxHealth();
                damage = CalcSpellDamage(spritefrom->uSpellID,
                                         spritefrom->spell_level,
                                         spritefrom->spell_skill, playerMaxHp);
                damagetype = pSpellStats->pInfos[spritefrom->uSpellID].damageType;
            } else {
                damage = pParty->pCharacters[uActorID].CalculateRangedDamageTo(MONSTER_INVALID);
                damagetype = DAMAGE_FIRE; // TODO(captainurist): another weird default.
            }

            playerPtr->receiveDamage(damage, damagetype);
            if (uActorType == OBJECT_Character) {
                pParty->setDelayedReaction(SPEECH_DAMAGED_PARTY, uActorID);
            }

            return;
        }
    }
}

void Character::OnInventoryLeftClick() {
    if (current_character_screen_window != WINDOW_CharacterWindow_Inventory) {
        return;
    }

    Pointi mousePos = mouse->position();
    Pointi mouseOffset = mouse->pickedItemOffset;

    Pointi inventoryPos = mapToInventoryGrid(mousePos + mouseOffset, Pointi(14, 17), &(pParty->pPickedItem));
    // If a held item is overlapping outside the grid
    if (pParty->pPickedItem.itemId != ITEM_NULL && !inventory.gridRect().contains(Recti(inventoryPos, pParty->pPickedItem.inventorySize()))) {
        pAudioPlayer->playUISound(SOUND_error);
        return;
    }

    if (inventory.gridRect().contains(inventoryPos)) {
        if (IsEnchantingInProgress) {
            InventoryEntry enchantedItemPos = inventory.entry(inventoryPos);

            if (enchantedItemPos) {
                /* *((char *)pGUIWindow_CastTargetedSpell->ptr_1C + 8) &=
                    *0x7Fu;
                    *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 2) =
                    *pParty->activeCharacterIndex() - 1;
                    *((int *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) =
                    *enchantedItemPos - 1;
                    *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) =
                    *invMatrixIndex;*/
                CastSpellInfo* pSpellInfo;
                pSpellInfo = pGUIWindow_CastTargetedSpell->spellInfo();
                pSpellInfo->flags &= ~ON_CAST_TargetedEnchantment;
                pSpellInfo->targetCharacterIndex = pParty->activeCharacterIndex() - 1;
                pSpellInfo->targetInventoryIndex = enchantedItemPos.index();
                ptr_50C9A4_ItemToEnchant = enchantedItemPos.get();
                IsEnchantingInProgress = false;

                engine->_messageQueue->clear();

                mouse->SetCursorImage("MICON1");
                AfterEnchClickEventId = UIMSG_Escape;
                AfterEnchClickEventSecondParam = 0;
                AfterEnchClickEventTimeout = Duration::fromRealtimeSeconds(2);
            }

            return;
        }

        if (ptr_50C9A4_ItemToEnchant)
            return;

        InventoryEntry entry = inventory.entry(inventoryPos);
        if (!entry && pParty->pPickedItem.itemId == ITEM_NULL) {
            return; // nothing to do
        }

        // calc offsets of where on the item was clicked
        // first need index of top left corner of the item
        Pointi corner = inventory.entry(inventoryPos).geometry().topLeft();
        int itemXOffset = mousePos.x + mouseOffset.x - 14 - (corner.x * 32);
        int itemYOffset = mousePos.y + mouseOffset.y - 17 - (corner.y * 32);

        if (entry) {
            auto tex = assets->getImage_Alpha(entry->GetIconName());
            itemXOffset -= itemOffset(tex->width());
            itemYOffset -= itemOffset(tex->height());
        }

        if (pParty->pPickedItem.itemId == ITEM_NULL) {
            // pick up the item
            pParty->setHoldingItem(inventory.take(entry), {-itemXOffset, -itemYOffset});
            return;
        } else {
            if (entry) {
                // take out
                Pointi pos = entry.geometry().topLeft();
                Item tmp = inventory.take(entry);

                // try to add where we clicked
                if (!inventory.tryAdd(pos, pParty->pPickedItem)) {
                    // try to add anywhere
                    if (!inventory.tryAdd(pParty->pPickedItem)) {
                        // failed to add, put back the old item
                        pAudioPlayer->playUISound(SOUND_error);
                        inventory.add(pos, tmp);
                        return;
                    }
                }

                pParty->takeHoldingItem();
                pParty->setHoldingItem(tmp);
            } else {
                // place picked item
                if (inventory.tryAdd(inventoryPos, pParty->pPickedItem)) {
                    pParty->takeHoldingItem();
                } else {
                    pAudioPlayer->playUISound(SOUND_error); // Overlapping items or out of inventory space.
                }
            }
        }
    }
}

bool Character::IsWeak() const {
    return this->conditions.has(CONDITION_WEAK);
}

bool Character::IsDead() const {
    return this->conditions.has(CONDITION_DEAD);
}

bool Character::IsEradicated() const {
    return this->conditions.has(CONDITION_ERADICATED);
}

bool Character::IsZombie() const {
    return this->conditions.has(CONDITION_ZOMBIE);
}

bool Character::IsCursed() const {
    return this->conditions.has(CONDITION_CURSED);
}

bool Character::IsPetrified() const {
    return this->conditions.has(CONDITION_PETRIFIED);
}

bool Character::IsUnconcious() const {
    return this->conditions.has(CONDITION_UNCONSCIOUS);
}

bool Character::IsAsleep() const {
    return this->conditions.has(CONDITION_SLEEP);
}

bool Character::IsParalyzed() const {
    return this->conditions.has(CONDITION_PARALYZED);
}

bool Character::IsDrunk() const {
    return this->conditions.has(CONDITION_DRUNK);
}

void Character::SetCondWeakWithBlockCheck(int blockable) {
    SetCondition(CONDITION_WEAK, blockable);
}

void Character::SetCondInsaneWithBlockCheck(int blockable) {
    SetCondition(CONDITION_INSANE, blockable);
}

void Character::SetCondDeadWithBlockCheck(int blockable) {
    SetCondition(CONDITION_DEAD, blockable);
}

void Character::SetCondUnconsciousWithBlockCheck(int blockable) {
    SetCondition(CONDITION_UNCONSCIOUS, blockable);
}

int Character::getCharacterIndex() {
    return pParty->getCharacterIdInParty(this);
}

//----- (004272F5) --------------------------------------------------------
bool Character::characterHitOrMiss(Actor *pActor, int distancemod, int skillmod) {  // PS - RETURN IF ATTACK WILL HIT
    int naturalArmor = pActor->monsterInfo.ac;  // actor usual armour
    int armorBuff = 0;

    if (pActor->buffs[ACTOR_BUFF_SOMETHING_THAT_HALVES_AC]
            .Active())  // gm axe effect??
        naturalArmor /= 2;

    if (pActor->buffs[ACTOR_BUFF_HOUR_OF_POWER].Active())
        armorBuff = pActor->buffs[ACTOR_BUFF_SHIELD].power;

    if (pActor->buffs[ACTOR_BUFF_STONESKIN].Active() &&
        pActor->buffs[ACTOR_BUFF_STONESKIN].power > armorBuff)
        armorBuff = pActor->buffs[ACTOR_BUFF_STONESKIN].power;

    int effectiveActorArmor = armorBuff + naturalArmor;

    int attBonus;  // character attack bonus
    if (distancemod)
        attBonus = this->GetRangedAttack();  // range
    else
        attBonus = this->GetActualAttack(false);  // melee

    int attPositiveMod =
        skillmod + grng->random(effectiveActorArmor + 2 * attBonus + 30);  // positive effects to hit on attack

    int attNegativeMod;  // negative effects to hit on attack

    if (distancemod == 2) {  // medium range
        attNegativeMod =
            ((effectiveActorArmor + 15) / 2) + effectiveActorArmor + 15;
    } else if (distancemod == 3) {  // far range
        attNegativeMod = 2 * effectiveActorArmor + 30;
    } else {  // close range
        attNegativeMod = effectiveActorArmor + 15;
    }

    return (attPositiveMod > attNegativeMod);  // do the pos outweight the negs
}

//----- (0042ECB5) --------------------------------------------------------
void Character::_42ECB5_CharacterAttacksActor() {
    Character *character = &pParty->activeCharacter();
    if (!character->CanAct()) return;

    CastSpellInfoHelpers::cancelSpellCastInProgress();
    // v3 = 0;
    if (pParty->Invisible())
        pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();

    // v31 = character->pEquipment[ITEM_SLOT_BOW];
    InventoryEntry bow = character->inventory.entry(ITEM_SLOT_BOW);
    if (bow && bow->IsBroken())
        bow = nullptr;

    ItemId wand_item_id = ITEM_NULL;
    ItemId laser_weapon_item_id = ITEM_NULL;

    InventoryEntry main_hand = character->inventory.entry(ITEM_SLOT_MAIN_HAND);
    if (main_hand) {
        if (!main_hand->IsBroken()) {
            if (main_hand->isWand()) {
                if (main_hand->numCharges <= 0) {
                    if (engine->config->gameplay.DestroyDischargedWands.value()) {
                        character->inventory.take(main_hand);
                    }
                } else {
                    wand_item_id = main_hand->itemId;
                }
            } else if (isAncientWeapon(main_hand->itemId)) {
                laser_weapon_item_id = main_hand->itemId;
            }
        }
    }

    Pid target_pid = mouse->uPointingObjectID;
    ObjectType target_type = target_pid.type();
    int target_id = target_pid.id();
    if (target_type != OBJECT_Actor || !pActors[target_id].CanAct()) {
        target_pid = stru_50C198.FindClosestActor(5120, 0, 0);
        target_type = target_pid.type();
        target_id = target_pid.id();
    }

    Actor *actor = nullptr;
    int actor_distance = 0;

    if (target_type == OBJECT_Actor) {
        actor = &pActors[target_id];

        int distance_x = actor->pos.x - pParty->pos.x,
            distance_y = actor->pos.y - pParty->pos.y,
            distance_z = actor->pos.z - pParty->pos.z;
        actor_distance =
            integer_sqrt(distance_x * distance_x + distance_y * distance_y +
                         distance_z * distance_z) -
            actor->radius;
        if (actor_distance < 0) actor_distance = 0;
    }

    bool shooting_bow = false, shotting_laser = false, shooting_wand = false,
         melee_attack = false;
    if (laser_weapon_item_id != ITEM_NULL) {
        shotting_laser = true;
        pushSpellOrRangedAttack(SPELL_LASER_PROJECTILE,
                                pParty->activeCharacterIndex() - 1, CombinedSkillValue::none(), 0,
                                pParty->activeCharacterIndex() + 8); // TODO(captainurist): +8???
    } else if (wand_item_id != ITEM_NULL) {
        shooting_wand = true;

        pushSpellOrRangedAttack(spellForWand(wand_item_id),
                                pParty->activeCharacterIndex() - 1, WANDS_SKILL_VALUE, 0, pParty->activeCharacterIndex() + 8);

        // reduce wand charges
        if (!--main_hand->numCharges && engine->config->gameplay.DestroyDischargedWands.value()) {
            character->inventory.take(main_hand);
        }
    } else if (target_type == OBJECT_Actor && actor_distance <= 407.2) {
        melee_attack = true;

        Vec3f a3 = actor->pos - pParty->pos;
        a3.normalize();

        Actor::DamageMonsterFromParty(Pid(OBJECT_Character, pParty->activeCharacterIndex() - 1),
                                      target_id, a3);
        if (character->wearsItem(ITEM_ARTIFACT_SPLITTER))
            _42FA66_do_explosive_impact(actor->pos + Vec3f(0, 0, actor->height / 2), 0, 512, pParty->activeCharacterIndex());
    } else if (bow) {
        shooting_bow = true;
        pushSpellOrRangedAttack(SPELL_BOW_ARROW, pParty->activeCharacterIndex() - 1, CombinedSkillValue::none(), 0, 0);
    } else {
        melee_attack = true;
        // actor out of range or no actor; no ranged weapon so melee attacking air
    }

    if (!pParty->bTurnBasedModeOn && melee_attack) {
        // wands, bows & lasers will add recovery while shooting spell effect
        Duration recovery = character->GetAttackRecoveryTime(false);
        character->SetRecoveryTime(debug_non_combat_recovery_mul * flt_debugrecmod3 * recovery);
    }

    Skill skill = SKILL_STAFF;
    if (shooting_wand) {
        return;
    } else if (shooting_bow) {
        skill = SKILL_BOW;
        character->playReaction(SPEECH_SHOOT);
    } else if (shotting_laser) {
        skill = SKILL_BLASTER;
    } else {
        if (character->inventory.functionalEntry(ITEM_SLOT_MAIN_HAND) && main_hand)
            skill = main_hand->skill();

        pTurnEngine->ApplyPlayerAction();
    }

    switch (skill) {
        case SKILL_STAFF:
            pAudioPlayer->playUISound(SOUND_swing_with_blunt_weapon01);
            break;
        case SKILL_SWORD:
            pAudioPlayer->playUISound(SOUND_swing_with_sword01);
            break;
        case SKILL_DAGGER:
            pAudioPlayer->playUISound(SOUND_swing_with_sword02);
            break;
        case SKILL_AXE:
            pAudioPlayer->playUISound(SOUND_swing_with_axe01);
            break;
        case SKILL_SPEAR:
            pAudioPlayer->playUISound(SOUND_swing_with_axe03);
            break;
        case SKILL_BOW:
            pAudioPlayer->playUISound(SOUND_shoot_bow01);
            break;
        case SKILL_MACE:
            pAudioPlayer->playUISound(SOUND_swing_with_blunt_weapon03);
            break;
        case SKILL_BLASTER:
            pAudioPlayer->playUISound(SOUND_shoot_blaster01);
            break;
        default:
            break;
    }
}

//----- (0042FA66) --------------------------------------------------------
void Character::_42FA66_do_explosive_impact(Vec3f pos, int a4, int16_t a5, int actchar) {
        // EXPLOSIVE IMPACT OF ARTIFACT SPLITTER

    // a5 is range?

    SpriteObject a1a;
    a1a.uType = SPRITE_OBJECT_EXPLODE;
    a1a.containing_item.Reset();
    a1a.uSpellID = SPELL_FIRE_FIREBALL;
    a1a.spell_level = 8;
    a1a.spell_skill = MASTERY_MASTER;
    a1a.uObjectDescID = pObjectList->ObjectIDByItemID(a1a.uType);
    a1a.vPosition = pos;
    a1a.uAttributes = 0;
    a1a.uSectorID = pIndoor->GetSector(pos);
    a1a.timeSinceCreated = 0_ticks;
    a1a.spell_target_pid = Pid();
    a1a.field_60_distance_related_prolly_lod = 0;
    a1a.uFacing = 0;
    a1a.uSoundID = 0;

    if (actchar >= 1 || actchar <= 4) {
        a1a.spell_caster_pid = Pid(OBJECT_Character, actchar - 1);
    } else {
        a1a.spell_caster_pid = Pid();
    }

    int id = a1a.Create(0, 0, 0, 0);
    if (id != -1) {
        pushAoeAttack(Pid(OBJECT_Sprite, id), a5, a1a.vPosition, ABILITY_ATTACK1);
    }
}

CombinedSkillValue Character::getSkillValue(Skill skill) const {
    return pActiveSkills[skill];
}

void Character::setSkillValue(Skill skill, const CombinedSkillValue &value) {
    pActiveSkills[skill] = value;
}

void Character::setXP(int xp) {
    if (xp > 4000000000 || xp < 0) {
        xp = 0;
    }
    experience = xp;
}

void Character::tickRegeneration(int tick5, const RegenData &rData, bool stacking) {
    if (stacking) {
        if (rData.hpSpellRegen || rData.hpRegen)
            health = std::min(GetMaxHealth(), health + tick5 * (rData.hpRegen + rData.hpSpellRegen));

        if (rData.spRegen)
            mana = std::min(GetMaxMana(), mana + tick5 * rData.spRegen);
    } else {
        if (rData.hpSpellRegen)
            health = std::min(GetMaxHealth(), health + tick5 * rData.hpSpellRegen);
        else if (rData.hpRegen)
            health = std::min(GetMaxHealth(), health + tick5);

        if (rData.spRegen)
            mana = std::min(GetMaxMana(), mana + tick5);
    }
}

void Character::playReaction(SpeechId speech, int a3) {
    int speechCount = 0;
    int portraitCount = 0;
    int pickedSoundID = 0;

    if (engine->config->settings.VoiceLevel.value() > 0) {
        for (int i = 0; i < speechVariants[speech].size(); i++) {
            if (speechVariants[speech][i]) {
                speechCount++;
            }
        }
        if (speechCount) {
            // TODO(captainurist): encapsulate the logic here.
            int pickedVariant = speechVariants[speech][vrng->random(speechCount)];
            int numberOfSubvariants = byte_4ECF08[pickedVariant - 1][uVoiceID];
            if (numberOfSubvariants > 0) {
                pickedSoundID = vrng->random(numberOfSubvariants) + 2 * (pickedVariant + 50 * uVoiceID) + 4998;
                pAudioPlayer->playSound((SoundId)pickedSoundID, SOUND_MODE_PID, Pid(OBJECT_Character, getCharacterIndex()));
            }
        }
    }

    for (int i = 0; i < portraitVariants[speech].size(); i++) {
        if (portraitVariants[speech][i] != PORTRAIT_INVALID) {
            portraitCount++;
        }
    }
    if (portraitCount) {
        PortraitId portrait = portraitVariants[speech][vrng->random(portraitCount)];
        Duration expressionDuration;
        if (portrait == PORTRAIT_TALK && pickedSoundID) {
            if (pickedSoundID >= 0) {
                expressionDuration = Duration::fromRealtimeMilliseconds(1000 * pAudioPlayer->getSoundLength(static_cast<SoundId>(pickedSoundID))); // Was (sLastTrackLengthMS << 7) / 1000;
            }
        }
        playEmotion(portrait, expressionDuration);
    }
}

void Character::playEmotion(PortraitId newPortrait, Duration duration) {
    // 38 - sparkles 1 character?

    if (portrait == PORTRAIT_DEAD ||
        portrait == PORTRAIT_ERADICATED) {
        return;  // no react
    } else if (portrait == PORTRAIT_PETRIFIED && newPortrait != PORTRAIT_WAKE_UP) {
        return;  // no react
    } else {
        if (!(portrait == PORTRAIT_SLEEP && newPortrait == PORTRAIT_WAKE_UP)) {
            if (portrait >= PORTRAIT_CURSED && portrait <= PORTRAIT_UNCONSCIOUS && portrait != PORTRAIT_POISONED &&
                !(newPortrait == PORTRAIT_DMGRECVD_MINOR ||
                  newPortrait == PORTRAIT_DMGRECVD_MODERATE ||
                  newPortrait == PORTRAIT_DMGRECVD_MAJOR)) {
                return;  // no react
            }
        }
    }

    this->portraitTimePassed = 0_ticks;

    if (!duration) {
        this->portraitTimeLength = pPortraitFrameTable->animationDuration(newPortrait);
        assert(this->portraitTimeLength); // GetDurationByExpression should have found the expression.
    } else {
        this->portraitTimeLength = duration;
    }

    if (newPortrait == PORTRAIT_TALK)
        talkAnimation.init();

    portrait = newPortrait;
}

bool Character::isClass(Class class_type, bool check_honorary) const {
    if (classType == class_type) {
        return true;
    }

    if (!check_honorary) {
        return false;
    }

    switch (class_type) {
    case CLASS_PRIEST_OF_SUN:
        return _achievedAwardsBits[Award_Promotion_PriestOfLight_Honorary];
    case CLASS_PRIEST_OF_MOON:
        return _achievedAwardsBits[Award_Promotion_PriestOfDark_Honorary];
    case CLASS_ARCHAMGE:
        return _achievedAwardsBits[Award_Promotion_Archmage_Honorary];
    case CLASS_LICH:
        return _achievedAwardsBits[Award_Promotion_Lich_Honorary];
    default:
        assert(false); // TODO(captainurist): just implement properly?
        return false;
    }
}

//----- (00490EEE) --------------------------------------------------------
MerchantPhrase Character::SelectPhrasesTransaction(Item *pItem, HouseType building_type, HouseId houseId, ShopScreen ShopMenuType) {
    // TODO(_): probably move this somewhere else, not really Character:: stuff
    ItemId idemId;   // edx@1
    ItemType equipType;  // esi@1
    float multiplier;      // ST04_4@26
    int price;             // edi@26
    int merchantLevel;     // [sp+10h] [bp-8h]@1
    int itemValue;

    merchantLevel = getActualSkillValue(SKILL_MERCHANT).level();
    idemId = pItem->itemId;
    equipType = pItem->type();
    itemValue = pItem->GetValue();

    switch (building_type) {
        case HOUSE_TYPE_WEAPON_SHOP:
            if (idemId >= ITEM_ARTIFACT_HERMES_SANDALS)
                return MERCHANT_PHRASE_INVALID_ACTION;
            if (!isWeapon(equipType))
                return MERCHANT_PHRASE_INCOMPATIBLE_ITEM;
            break;
        case HOUSE_TYPE_ARMOR_SHOP:
            if (idemId >= ITEM_ARTIFACT_HERMES_SANDALS)
                return MERCHANT_PHRASE_INVALID_ACTION;
            if (!isArmor(equipType))
                return MERCHANT_PHRASE_INCOMPATIBLE_ITEM;
            break;
        case HOUSE_TYPE_MAGIC_SHOP:
            if (idemId >= ITEM_ARTIFACT_HERMES_SANDALS)
                return MERCHANT_PHRASE_INVALID_ACTION;
            if (pItem->skill() != SKILL_MISC)
                return MERCHANT_PHRASE_INCOMPATIBLE_ITEM;
            break;
        case HOUSE_TYPE_ALCHEMY_SHOP:
            if (idemId >= ITEM_ARTIFACT_HERMES_SANDALS && !isRecipe(idemId))
                return MERCHANT_PHRASE_INVALID_ACTION;
            if (equipType != ITEM_TYPE_REAGENT && equipType != ITEM_TYPE_POTION && equipType != ITEM_TYPE_MESSAGE_SCROLL)
                return MERCHANT_PHRASE_INCOMPATIBLE_ITEM;
            break;
        default:
            assert(false);
            break;
    }
    if (pItem->IsStolen())
        return MERCAHNT_PHRASE_STOLEN_ITEM;

    multiplier = houseTable[houseId].fPriceMultiplier;
    switch (ShopMenuType) {
        case SHOP_SCREEN_BUY:
            price = PriceCalculator::itemBuyingPriceForPlayer(this, itemValue, multiplier);
            break;
        case SHOP_SCREEN_SELL:
            // if (pItem->IsBroken())
            // price = 1;
            // else
            price = PriceCalculator::itemSellingPriceForPlayer(this, *pItem, multiplier);
            break;
        case SHOP_SCREEN_IDENTIFY:
            price = PriceCalculator::itemIdentificationPriceForPlayer(this, multiplier);
            break;
        case SHOP_SCREEN_REPAIR:
            price = PriceCalculator::itemRepairPriceForPlayer(this, itemValue, multiplier);
            break;
        default:
            assert(false);
            break;
    }
    if (merchantLevel) {
        if (price == itemValue) {
            return MERCHANT_PHRASE_PRICE_HAGGLE_TO_ACTUAL_PRICE;
        } else {
            return MERCHANT_PHRASE_PRICE_HAGGLE;
        }
    } else {
        return MERCHANT_PHRASE_PRICE;
    }
}

//----- (0048C6AF) --------------------------------------------------------
Character::Character() {
    Zero();
}

void Character::Zero() {
    name = std::string();
    uSex = SEX_MALE;
    classType = CLASS_KNIGHT;
    uCurrentFace = uPrevFace = 0;
    uVoiceID = uPrevVoiceID = 0;
    uSkillPoints = 0;
    // Stats
    _stats.fill(0);
    _statBonuses.fill(0);
    // HP MP AC
    health = uFullHealthBonus = _health_related = 0;
    mana = uFullManaBonus = _mana_related = 0;
    sACModifier = 0;

    conditions.resetAll();

    uBirthYear = sAgeModifier = 0;
    uLevel = sLevelModifier = 0;
    experience = 0;

    _some_attack_bonus = 0;
    _melee_dmg_bonus = 0;
    _ranged_atk_bonus = 0;
    _ranged_dmg_bonus = 0;
    timeToRecovery = 0_ticks;
    // Resistances
    sResFireBase = sResFireBonus = 0;
    sResAirBase = sResAirBonus = 0;
    sResWaterBase = sResWaterBonus = 0;
    sResEarthBase = sResEarthBonus = 0;
    sResPhysicalBase = sResPhysicalBonus = 0;
    sResMagicBase = sResMagicBonus = 0;
    sResSpiritBase = sResSpiritBonus = 0;
    sResMindBase = sResMindBonus = 0;
    sResBodyBase = sResBodyBonus = 0;
    sResLightBase = sResLightBonus = 0;
    sResDarkBase = sResDarkBonus = 0;
    // Skills
    pActiveSkills.fill(CombinedSkillValue());
    pActiveSkills[SKILL_CLUB] = CombinedSkillValue::novice(); // Hidden skills, always known.
    pActiveSkills[SKILL_MISC] = CombinedSkillValue::novice();
    // Inventory
    inventory = CharacterInventory();
    // Buffs
    for (auto& buf : pCharacterBuffs) {
        buf.Reset();
    }
    // Spells
    bHaveSpell.fill(false);
    lastOpenedSpellbookPage = MAGIC_SCHOOL_FIRE;
    uQuickSpell = SPELL_NONE;
    uNumDivineInterventionCastsThisDay = 0;
    uNumArmageddonCasts = 0;
    uNumFireSpikeCasts = 0; // TODO(pskelton): firespike meant to remain permanantly??
    for (int z = 0; z < 5; z++) {
        if (vBeacons[z])
            vBeacons[z]->image->Release();
        vBeacons[z].reset();
    }
    // Character bits
    _characterEventBits.reset();
    _achievedAwardsBits.reset();
    // Expression
    portrait = PORTRAIT_INVALID;
    portraitTimePassed = 0_ticks;
    portraitTimeLength = 0_ticks;
    portraitImageIndex = 0;
    talkAnimation = TalkAnimation();
    // Black potions
    _pureStatPotionUsed.fill(false);
}

bool Character::matchesAttackPreference(MonsterAttackPreference preference) const {
    switch (preference) {
    // TODO(captainurist): isn't it weird that promotions aren't included in comparisons here?
    case ATTACK_PREFERENCE_KNIGHT:      return classType == CLASS_KNIGHT;
    case ATTACK_PREFERENCE_PALADIN:     return classType == CLASS_PALADIN;
    case ATTACK_PREFERENCE_ARCHER:      return classType == CLASS_ARCHER;
    case ATTACK_PREFERENCE_DRUID:       return classType == CLASS_DRUID;
    case ATTACK_PREFERENCE_CLERIC:      return classType == CLASS_CLERIC;
    case ATTACK_PREFERENCE_SORCERER:    return classType == CLASS_SORCERER;
    case ATTACK_PREFERENCE_RANGER:      return classType == CLASS_RANGER;
    case ATTACK_PREFERENCE_THIEF:       return classType == CLASS_THIEF;
    case ATTACK_PREFERENCE_MONK:        return classType == CLASS_MONK;
    case ATTACK_PREFERENCE_MALE:        return uSex == SEX_MALE;
    case ATTACK_PREFERENCE_FEMALE:      return uSex == SEX_FEMALE;
    case ATTACK_PREFERENCE_HUMAN:       return GetRace() == RACE_HUMAN;
    case ATTACK_PREFERENCE_ELF:         return GetRace() == RACE_ELF;
    case ATTACK_PREFERENCE_DWARF:       return GetRace() == RACE_DWARF;
    case ATTACK_PREFERENCE_GOBLIN:      return GetRace() == RACE_GOBLIN;
    default:
        assert(false);
        return false;
    }
}

void Character::cleanupBeacons() {
    for (int i = 0; i < 5; i++) {
        if (!vBeacons[i] || vBeacons[i]->uBeaconTime >= pParty->GetPlayingTime())
            continue;
        vBeacons[i]->image->Release();
        vBeacons[i].reset();
    }
}

bool Character::setBeacon(int index, Duration duration) {
    if (engine->_currentLoadedMapId == MAP_INVALID) {
        return false;
    }

    LloydBeacon beacon;

    beacon.image = GraphicsImage::Create(render->MakeViewportScreenshot(92, 68));
    beacon.uBeaconTime = pParty->GetPlayingTime() + duration;
    beacon._partyPos = pParty->pos;
    beacon._partyViewYaw = pParty->_viewYaw;
    beacon._partyViewPitch = pParty->_viewPitch;
    beacon.mapId = engine->_currentLoadedMapId;

    if (vBeacons[index]) {
        // overwrite so clear image
        vBeacons[index]->image->Release();
    }
    vBeacons[index] = beacon;

    return true;
}
