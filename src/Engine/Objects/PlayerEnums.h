#pragma once

#include <cassert>

#include "Utility/Segment.h"

enum PLAYER_BUFFS {
    PLAYER_BUFF_RESIST_AIR = 0,
    PLAYER_BUFF_BLESS = 1,
    PLAYER_BUFF_RESIST_BODY = 2,
    PLAYER_BUFF_RESIST_EARTH = 3,
    PLAYER_BUFF_FATE = 4,
    PLAYER_BUFF_RESIST_FIRE = 5,
    PLAYER_BUFF_HAMMERHANDS = 6,
    PLAYER_BUFF_HASTE = 7,
    PLAYER_BUFF_HEROISM = 8,
    PLAYER_BUFF_RESIST_MIND = 9,
    PLAYER_BUFF_PAIN_REFLECTION = 10,
    PLAYER_BUFF_PRESERVATION = 11,
    PLAYER_BUFF_REGENERATION = 12,
    PLAYER_BUFF_SHIELD = 13,
    PLAYER_BUFF_STONESKIN = 14,
    PLAYER_BUFF_ACCURACY = 15,
    PLAYER_BUFF_ENDURANCE = 16,
    PLAYER_BUFF_INTELLIGENCE = 17,
    PLAYER_BUFF_LUCK = 18,
    PLAYER_BUFF_STRENGTH = 19,
    PLAYER_BUFF_WILLPOWER = 20,
    PLAYER_BUFF_SPEED = 21,
    PLAYER_BUFF_RESIST_WATER = 22,
    PLAYER_BUFF_WATER_WALK = 23
};


/*  301 */
enum PlayerSpeech {
    SPEECH_None = 0,
    SPEECH_KillWeakEnemy = 1,
    SPEECH_KillStrongEnemy = 2,
    SPEECH_StoreClosed = 3,
    SPEECH_TrapDisarmed = 4,
    SPEECH_TrapExploded = 5,
    SPEECH_AvoidDamage = 6,
    SPEECH_IndentifyItemWeak = 7,
    SPEECH_IndentifyItemStrong = 8,
    SPEECH_IndentifyItemFail = 9,
    SPEECH_RepairSuccess = 10,
    SPEECH_RepairFail = 11,
    SPEECH_SetQuickSpell = 12,
    SPEECH_CantRestHere = 13,
    SPEECH_SkillIncrease = 14,
    SPEECH_NoRoom = 15,
    SPEECH_PotionSuccess = 16,
    SPEECH_PotionExplode = 17,
    SPEECH_DoorLocked = 18,
    SPEECH_WontBudge = 19,
    SPEECH_CantLearnSpell = 20,
    SPEECH_LearnSpell = 21,
    SPEECH_GoodDay = 22,
    SPEECH_GoodEvening = 23,
    SPEECH_Damaged = 24,
    SPEECH_Weak = 25,
    SPEECH_Fear = 26,
    SPEECH_Poisoned = 27,
    SPEECH_Diseased = 28,
    SPEECH_Insane = 29,
    SPEECH_Cursed = 30,
    SPEECH_Drunk = 31,
    SPEECH_Unconscious = 32,
    SPEECH_Dead = 33,
    SPEECH_Petrified = 34,
    SPEECH_Eradicated = 35,
    SPEECH_DrinkPotion = 36,
    SPEECH_ReadScroll = 37,
    SPEECH_NotEnoughGold = 38,
    SPEECH_CantEquip = 39,
    SPEECH_ItemBroken = 40,
    SPEECH_SPDrained = 41,
    SPEECH_Aging = 42,
    SPEECH_SpellFailed = 43,
    SPEECH_DamagedParty = 44,
    SPEECH_Tired = 45,
    SPEECH_EnterDungeon = 46,
    SPEECH_LeaveDungeon = 47,
    SPEECH_BadlyHurt = 48,
    SPEECH_CastSpell = 49,
    SPEECH_Shoot = 50,
    SPEECH_AttackHit = 51,
    SPEECH_AttackMiss = 52,
    SPEECH_Beg = 53,
    SPEECH_BegFail = 54,
    SPEECH_Threat = 55,
    SPEECH_ThreatFail = 56,
    SPEECH_Bribe = 57,
    SPEECH_BribeFail = 58,
    SPEECH_NPCDontTalk = 59,
    SPEECH_FoundItem = 60,
    SPEECH_HireNPC = 61,
    SPEECH_62 = 62,   // unknown
    SPEECH_LookUp = 63,
    SPEECH_LookDown = 64,
    SPEECH_Yell = 65,
    SPEECH_Falling = 66,
    SPEECH_PacksFull = 67,
    SPEECH_TavernDrink = 68,
    SPEECH_TavernGotDrunk = 69,
    SPEECH_TavernTip = 70,
    SPEECH_TravelHorse = 71,
    SPEECH_TravelBoat = 72,
    SPEECH_ShopIdentify = 73,
    SPEECH_ShopRepair = 74,
    SPEECH_ItemBuy = 75,
    SPEECH_AlreadyIdentified = 76,
    SPEECH_ItemSold = 77,
    SPEECH_SkillLearned = 78,
    SPEECH_WrongShop = 79,
    SPEECH_ShopRude = 80,
    SPEECH_BankDeposit = 81,
    SPEECH_TempleHeal = 82,
    SPEECH_TempleDonate = 83,
    SPEECH_HelloHouse = 84,
    SPEECH_SkillMasteryInc = 85,
    SPEECH_JoinedGuild = 86,
    SPEECH_LevelUp = 87,
    SPEECH_88 = 88,  // unknown
    SPEECH_89 = 89,  // unknown
    SPEECH_90 = 90,  // unknown
    SPEECH_StatBonusInc = 91,
    SPEECH_StatBaseInc = 92,
    SPEECH_QuestGot = 93,
    SPEECH_94 = 94,  // unknown
    SPEECH_95 = 95,  // unknown
    SPEECH_AwardGot = 96,  // award
    SPEECH_97 = 97,  // unknown
    SPEECH_AfraidSilent = 98,
    SPEECH_CheatedDeath = 99,  // zombie/ death groan
    SPEECH_InPrison = 100,
    SPEECH_101 = 101,  // unknown
    SPEECH_PickMe = 102,
    SPEECH_Awaken = 103,
    SPEECH_IDMonsterWeak = 104,
    SPEECH_IDMonsterStrong = 105,
    SPEECH_IDMonsterFail = 106,
    SPEECH_LastManStanding = 107,
    SPEECH_NotEnoughFood = 108,
    SPEECH_DeathBlow = 109,
    SPEECH_110 = 110,  // unknown

    SPEECH_FIRST = SPEECH_None,
    SPEECH_LAST = SPEECH_110
};

/*  339 */
enum CHARACTER_RACE {
    CHARACTER_RACE_HUMAN = 0,
    CHARACTER_RACE_ELF = 1,
    CHARACTER_RACE_GOBLIN = 2,
    CHARACTER_RACE_DWARF = 3,
};

/* Skill encodes level and mastery where first 0x3F are for skill level and 0x1C0 bits are for skill mastery.
 * So max possible stored skill level is 63.
 */
typedef uint16_t PLAYER_SKILL;
typedef uint8_t PLAYER_SKILL_LEVEL;

enum class PLAYER_SKILL_MASTERY: int32_t { // TODO: type could be changed to something else when SpriteObject_MM7 implemented in LegacyImages
    PLAYER_SKILL_MASTERY_NONE = 0,
    PLAYER_SKILL_MASTERY_NOVICE = 1,
    PLAYER_SKILL_MASTERY_EXPERT = 2,
    PLAYER_SKILL_MASTERY_MASTER = 3,
    PLAYER_SKILL_MASTERY_GRANDMASTER = 4,

    PLAYER_SKILL_MASTERY_FIRST = PLAYER_SKILL_MASTERY_NOVICE,
    PLAYER_SKILL_MASTERY_LAST = PLAYER_SKILL_MASTERY_GRANDMASTER
};
using enum PLAYER_SKILL_MASTERY;

inline Segment<PLAYER_SKILL_MASTERY> SkillMasteries() {
    return Segment(PLAYER_SKILL_MASTERY_FIRST, PLAYER_SKILL_MASTERY_LAST);
}

enum CLASS_SKILL: uint8_t {
    CLASS_SKILL_DENIED = 0,
    CLASS_SKILL_AVAILABLE = 1,
    CLASS_SKILL_PRIMARY = 2
};
using enum CLASS_SKILL;

inline PLAYER_SKILL_LEVEL GetSkillLevel(PLAYER_SKILL skill_value) {
    return skill_value & 0x3F;
}

/**
 * @offset 0x00458244.
 */
inline PLAYER_SKILL_MASTERY GetSkillMastery(PLAYER_SKILL skill_value) {
    // PLAYER_SKILL_MASTERY_NONE equal PLAYER_SKILL_MASTERY_NOVICE with skill level 0.
    //if (skill_value == 0)
    //    return PLAYER_SKILL_MASTERY_NONE;

    switch (skill_value & 0x1C0) {
        case 0x100:
            return PLAYER_SKILL_MASTERY_GRANDMASTER;
        case 0x80:
            return PLAYER_SKILL_MASTERY_MASTER;
        case 0x40:
            return PLAYER_SKILL_MASTERY_EXPERT;
        case 0x00:
            return PLAYER_SKILL_MASTERY_NOVICE;
    }

    assert(false); // should not get here
    return PLAYER_SKILL_MASTERY_NONE;
}

inline void SetSkillLevel(PLAYER_SKILL *skill_value, PLAYER_SKILL_LEVEL level) {
    *skill_value = (*skill_value & ~0x3F) | (level & 0x3F);
}

inline void SetSkillMastery(PLAYER_SKILL *skill_value, PLAYER_SKILL_MASTERY mastery) {
    assert(mastery <= PLAYER_SKILL_MASTERY_GRANDMASTER);

    *skill_value &= 0x3F;

    switch (mastery) {
        case(PLAYER_SKILL_MASTERY_EXPERT):
            *skill_value |= 0x40;
            break;
        case(PLAYER_SKILL_MASTERY_MASTER):
            *skill_value |= 0x80;
            break;
        case(PLAYER_SKILL_MASTERY_GRANDMASTER):
            *skill_value |= 0x100;
            break;
        default:
            return;
    }
}

/**
 * Construct player skill value using skill mastery and skill level
 */
inline PLAYER_SKILL ConstructSkillValue(PLAYER_SKILL_MASTERY mastery, PLAYER_SKILL_LEVEL level) {
    PLAYER_SKILL skill = 0;

    SetSkillMastery(&skill, mastery);
    SetSkillLevel(&skill, level);

    return skill;
}

#pragma warning(push)
#pragma warning(disable : 4341) // TODO(captainurist): msvc mis-warns here, just drop this warning altogether for msvc
/*  328 */
enum class PLAYER_SKILL_TYPE : int8_t {
    PLAYER_SKILL_INVALID = -1,
    PLAYER_SKILL_STAFF = 0,
    PLAYER_SKILL_SWORD = 1,
    PLAYER_SKILL_DAGGER = 2,
    PLAYER_SKILL_AXE = 3,
    PLAYER_SKILL_SPEAR = 4,
    PLAYER_SKILL_BOW = 5,
    PLAYER_SKILL_MACE = 6,
    PLAYER_SKILL_BLASTER = 7,
    PLAYER_SKILL_SHIELD = 8,
    PLAYER_SKILL_LEATHER = 9,
    PLAYER_SKILL_CHAIN = 10,
    PLAYER_SKILL_PLATE = 11,
    PLAYER_SKILL_FIRE = 12,
    PLAYER_SKILL_AIR = 13,
    PLAYER_SKILL_WATER = 14,
    PLAYER_SKILL_EARTH = 15,
    PLAYER_SKILL_SPIRIT = 16,
    PLAYER_SKILL_MIND = 17,
    PLAYER_SKILL_BODY = 18,
    PLAYER_SKILL_LIGHT = 19,
    PLAYER_SKILL_DARK = 20,
    PLAYER_SKILL_ITEM_ID = 21,
    PLAYER_SKILL_MERCHANT = 22,
    PLAYER_SKILL_REPAIR = 23,
    PLAYER_SKILL_BODYBUILDING = 24,
    PLAYER_SKILL_MEDITATION = 25,
    PLAYER_SKILL_PERCEPTION = 26,
    PLAYER_SKILL_DIPLOMACY = 27,
    PLAYER_SKILL_THIEVERY = 28,
    PLAYER_SKILL_TRAP_DISARM = 29,
    PLAYER_SKILL_DODGE = 30,
    PLAYER_SKILL_UNARMED = 31,
    PLAYER_SKILL_MONSTER_ID = 32,
    PLAYER_SKILL_ARMSMASTER = 33,
    PLAYER_SKILL_STEALING = 34,
    PLAYER_SKILL_ALCHEMY = 35,
    PLAYER_SKILL_LEARNING = 36,
    PLAYER_SKILL_CLUB = 37, // In vanilla clubs are using separate hidden & non-upgradable skill.
    PLAYER_SKILL_MISC = 38, // Hidden skill that's always 1. Used for wetsuits, for example.

    PLAYER_SKILL_FIRST_VISIBLE = PLAYER_SKILL_STAFF,
    PLAYER_SKILL_LAST_VISIBLE = PLAYER_SKILL_LEARNING,

    PLAYER_SKILL_FIRST = PLAYER_SKILL_STAFF,
    PLAYER_SKILL_LAST = PLAYER_SKILL_MISC,
};
using enum PLAYER_SKILL_TYPE;

inline Segment<PLAYER_SKILL_TYPE> AllSkills() {
    return Segment(PLAYER_SKILL_FIRST, PLAYER_SKILL_LAST);
}

/**
 * @return                              List of skills that are visible to the player and that are stored in a savegame.
 */
inline Segment<PLAYER_SKILL_TYPE> VisibleSkills() {
    return Segment(PLAYER_SKILL_FIRST_VISIBLE, PLAYER_SKILL_LAST_VISIBLE);
}

/**
 * @return                              List of skills that are drawn in the "Armor" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<PLAYER_SKILL_TYPE> ArmorSkills() {
    static constexpr std::initializer_list<PLAYER_SKILL_TYPE> result = {
        PLAYER_SKILL_LEATHER, PLAYER_SKILL_CHAIN, PLAYER_SKILL_PLATE,
        PLAYER_SKILL_SHIELD,  PLAYER_SKILL_DODGE
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Weapons" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<PLAYER_SKILL_TYPE> WeaponSkills() {
    static constexpr std::initializer_list<PLAYER_SKILL_TYPE> result = {
        PLAYER_SKILL_AXE,   PLAYER_SKILL_BOW,     PLAYER_SKILL_DAGGER,
        PLAYER_SKILL_MACE,  PLAYER_SKILL_SPEAR,   PLAYER_SKILL_STAFF,
        PLAYER_SKILL_SWORD, PLAYER_SKILL_UNARMED, PLAYER_SKILL_BLASTER
        // PLAYER_SKILL_CLUB is not displayed in skills.
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Misc" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<PLAYER_SKILL_TYPE> MiscSkills() {
    static constexpr std::initializer_list<PLAYER_SKILL_TYPE> result = {
        PLAYER_SKILL_ALCHEMY,      PLAYER_SKILL_ARMSMASTER,
        PLAYER_SKILL_BODYBUILDING, PLAYER_SKILL_ITEM_ID,
        PLAYER_SKILL_MONSTER_ID,   PLAYER_SKILL_LEARNING,
        PLAYER_SKILL_TRAP_DISARM,  PLAYER_SKILL_MEDITATION,
        PLAYER_SKILL_MERCHANT,     PLAYER_SKILL_PERCEPTION,
        PLAYER_SKILL_REPAIR,       PLAYER_SKILL_STEALING
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Magic" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<PLAYER_SKILL_TYPE> MagicSkills() {
    static constexpr std::initializer_list<PLAYER_SKILL_TYPE> result = {
        PLAYER_SKILL_FIRE,  PLAYER_SKILL_AIR,    PLAYER_SKILL_WATER,
        PLAYER_SKILL_EARTH, PLAYER_SKILL_SPIRIT, PLAYER_SKILL_MIND,
        PLAYER_SKILL_BODY,  PLAYER_SKILL_LIGHT,  PLAYER_SKILL_DARK
    };

    return result;
}

#pragma warning(pop)

/*  329 */
// TODO(Nik-RE-dev): turn it into enum class
enum PLAYER_CLASS_TYPE : uint8_t {
    PLAYER_CLASS_KNIGHT = 0,
    PLAYER_CLASS_CHEVALIER = 1,
    PLAYER_CLASS_CHAMPION = 2,
    PLAYER_CLASS_BLACK_KNIGHT = 3,
    PLAYER_CLASS_THIEF = 4,
    PLAYER_CLASS_ROGUE = 5,
    PLAYER_CLASS_SPY = 6,
    PLAYER_CLASS_ASSASSIN = 7,
    PLAYER_CLASS_MONK = 8,
    PLAYER_CLASS_INITIATE = 9,
    PLAYER_CLASS_MASTER = 10,
    PLAYER_CLASS_NINJA = 11,
    PLAYER_CLASS_PALADIN = 12,
    PLAYER_CLASS_CRUSADER = 13,
    PLAYER_CLASS_HERO = 14,
    PLAYER_CLASS_VILLIAN = 15,
    PLAYER_CLASS_ARCHER = 16,
    PLAYER_CLASS_WARRIOR_MAGE = 17,
    PLAYER_CLASS_MASTER_ARCHER = 18,
    PLAYER_CLASS_SNIPER = 19,
    PLAYER_CLASS_RANGER = 20,
    PLAYER_CLASS_HUNTER = 21,
    PLAYER_CLASS_RANGER_LORD = 22,
    PLAYER_CLASS_BOUNTY_HUNTER = 23,
    PLAYER_CLASS_CLERIC = 24,
    PLAYER_CLASS_PRIEST = 25,
    PLAYER_CLASS_PRIEST_OF_SUN = 26,
    PLAYER_CLASS_PRIEST_OF_MOON = 27,
    PLAYER_CLASS_DRUID = 28,
    PLAYER_CLASS_GREAT_DRUID = 29,
    PLAYER_CLASS_ARCH_DRUID = 30,
    PLAYER_CLASS_WARLOCK = 31,
    PLAYER_CLASS_SORCERER = 32,
    PLAYER_CLASS_WIZARD = 33,
    PLAYER_CLASS_ARCHMAGE = 34,
    PLAYER_CLASS_LICH = 35,

    PLAYER_CLASS_FIRST = PLAYER_CLASS_KNIGHT,
    PLAYER_CLASS_LAST = PLAYER_CLASS_LICH
};

inline PLAYER_CLASS_TYPE getTier1Class(PLAYER_CLASS_TYPE classType) {
    int tier = classType % 4;
    return (PLAYER_CLASS_TYPE)(classType - tier);
}

inline PLAYER_CLASS_TYPE getTier2Class(PLAYER_CLASS_TYPE classType) {
    int tier = classType % 4;
    return (PLAYER_CLASS_TYPE)(classType - tier + 1);
}

inline PLAYER_CLASS_TYPE getTier3LightClass(PLAYER_CLASS_TYPE classType) {
    int tier = classType % 4;
    return (PLAYER_CLASS_TYPE)(classType - tier + 2);
}

inline PLAYER_CLASS_TYPE getTier3DarkClass(PLAYER_CLASS_TYPE classType) {
    int tier = classType % 4;
    return (PLAYER_CLASS_TYPE)(classType - tier + 3);
}

/**
 * Get priomotions of higher tier class relative to given one.
 *
 * Base class is of tier 1.
 * After initial promotion class becomes tier 2.
 * Tier 2 class is promoted through light or dark path to tier 3 class.
 *
 * @param classType     Character class.
 */
inline Segment<PLAYER_CLASS_TYPE> getClassPromotions(PLAYER_CLASS_TYPE classType) {
    int tier = classType % 4;

    if (tier == 0) {
        return {getTier2Class(classType), getTier3DarkClass(classType)};
    } else if (tier == 1) {
        return {getTier3LightClass(classType), getTier3DarkClass(classType)};
    } else {
        return {}; // tier 3 max
    }
}

// TODO(pskelton): decipher enum
enum CHARACTER_EXPRESSION_ID : uint16_t {
    CHARACTER_EXPRESSION_INVALID = 0,
    CHARACTER_EXPRESSION_NORMAL = 1,
    CHARACTER_EXPRESSION_CURSED = 2,
    CHARACTER_EXPRESSION_WEAK = 3,
    CHARACTER_EXPRESSION_SLEEP = 4,
    CHARACTER_EXPRESSION_FEAR = 5,
    CHARACTER_EXPRESSION_DRUNK = 6,
    CHARACTER_EXPRESSION_INSANE = 7,
    CHARACTER_EXPRESSION_POISONED = 8,
    CHARACTER_EXPRESSION_DISEASED = 9,
    CHARACTER_EXPRESSION_PARALYZED = 10,
    CHARACTER_EXPRESSION_UNCONCIOUS = 11,
    CHARACTER_EXPRESSION_PERTIFIED = 12,
    CHARACTER_EXPRESSION_BLINK = 13,
    CHARACTER_EXPRESSION_WINK = 14, // some faces wink, some shrug with eyebrows
    CHARACTER_EXPRESSION_MOUTH_OPEN_RANDOM = 15, // used for random expression, slightly opens mouth
    CHARACTER_EXPRESSION_PURSE_LIPS_RANDOM = 16, // used for random expression
    CHARACTER_EXPRESSION_LOOK_UP = 17,
    CHARACTER_EXPRESSION_LOOK_RIGHT = 18,
    CHARACTER_EXPRESSION_LOOK_LEFT = 19,
    CHARACTER_EXPRESSION_LOOK_DOWN = 20,
    CHARACTER_EXPRESSION_TALK = 21,
    CHARACTER_EXPRESSION_MOUTH_OPEN_WIDE = 22,
    CHARACTER_EXPRESSION_MOUTH_OPEN_A = 23,
    CHARACTER_EXPRESSION_MOUTH_OPEN_O = 24,
    CHARACTER_EXPRESSION_NO = 25,
    CHARACTER_EXPRESSION_26 = 26,
    CHARACTER_EXPRESSION_YES = 27,
    CHARACTER_EXPRESSION_28 = 28,
    CHARACTER_EXPRESSION_PURSE_LIPS_1 = 29, // these 3 seems to produce same expression
    CHARACTER_EXPRESSION_PURSE_LIPS_2 = 30,
    CHARACTER_EXPRESSION_PURSE_LIPS_3 = 31,
    CHARACTER_EXPRESSION_32 = 32,
    CHARACTER_EXPRESSION_AVOID_DAMAGE = 33,
    CHARACTER_EXPRESSION_DMGRECVD_MINOR = 34,
    CHARACTER_EXPRESSION_DMGRECVD_MODERATE = 35,
    CHARACTER_EXPRESSION_DMGRECVD_MAJOR = 36,
    CHARACTER_EXPRESSION_SMILE = 37, // not drowning
    CHARACTER_EXPRESSION_WIDE_SMILE = 38,
    CHARACTER_EXPRESSION_SAD = 39,
    CHARACTER_EXPRESSION_CAST_SPELL = 40,

    // ?

    CHARACTER_EXPRESSION_SCARED = 46,  // like falling

    CHARACTER_EXPRESSION_54 = 54,
    CHARACTER_EXPRESSION_55 = 55,
    CHARACTER_EXPRESSION_56 = 56,
    CHARACTER_EXPRESSION_57 = 57,
    CHARACTER_EXPRESSION_FALLING = 58,

    // ?

    CHARACTER_EXPRESSION_DEAD = 98,
    CHARACTER_EXPRESSION_ERADICATED = 99,
};

enum PLAYER_SEX : uint8_t {
    SEX_MALE = 0,
    SEX_FEMALE = 1
};

enum CHARACTER_ATTRIBUTE_TYPE {
    CHARACTER_ATTRIBUTE_STRENGTH = 0,
    CHARACTER_ATTRIBUTE_INTELLIGENCE = 1,
    CHARACTER_ATTRIBUTE_WILLPOWER = 2,
    CHARACTER_ATTRIBUTE_ENDURANCE = 3,
    CHARACTER_ATTRIBUTE_ACCURACY = 4,
    CHARACTER_ATTRIBUTE_SPEED = 5,
    CHARACTER_ATTRIBUTE_LUCK = 6,
    CHARACTER_ATTRIBUTE_HEALTH = 7,
    CHARACTER_ATTRIBUTE_MANA = 8,
    CHARACTER_ATTRIBUTE_AC_BONUS = 9,

    CHARACTER_ATTRIBUTE_RESIST_FIRE = 10,
    CHARACTER_ATTRIBUTE_RESIST_AIR = 11,
    CHARACTER_ATTRIBUTE_RESIST_WATER = 12,
    CHARACTER_ATTRIBUTE_RESIST_EARTH = 13,
    CHARACTER_ATTRIBUTE_RESIST_MIND = 14,
    CHARACTER_ATTRIBUTE_RESIST_BODY = 15,

    CHARACTER_ATTRIBUTE_SKILL_ALCHEMY = 16,
    CHARACTER_ATTRIBUTE_SKILL_STEALING = 17,
    CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM = 18,
    CHARACTER_ATTRIBUTE_SKILL_ITEM_ID = 19,
    CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID = 20,
    CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER = 21,
    CHARACTER_ATTRIBUTE_SKILL_DODGE = 22,
    CHARACTER_ATTRIBUTE_SKILL_UNARMED = 23,

    CHARACTER_ATTRIBUTE_LEVEL = 24,
    CHARACTER_ATTRIBUTE_ATTACK = 25,
    CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS = 26,
    CHARACTER_ATTRIBUTE_MELEE_DMG_MIN = 27,
    CHARACTER_ATTRIBUTE_MELEE_DMG_MAX = 28,
    CHARACTER_ATTRIBUTE_RANGED_ATTACK = 29,
    CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS = 30,
    CHARACTER_ATTRIBUTE_RANGED_DMG_MIN = 31,
    CHARACTER_ATTRIBUTE_RANGED_DMG_MAX = 32,
    CHARACTER_ATTRIBUTE_RESIST_SPIRIT = 33,

    CHARACTER_ATTRIBUTE_SKILL_FIRE = 34,
    CHARACTER_ATTRIBUTE_SKILL_AIR = 35,
    CHARACTER_ATTRIBUTE_SKILL_WATER = 36,
    CHARACTER_ATTRIBUTE_SKILL_EARTH = 37,
    CHARACTER_ATTRIBUTE_SKILL_SPIRIT = 38,
    CHARACTER_ATTRIBUTE_SKILL_MIND = 39,
    CHARACTER_ATTRIBUTE_SKILL_BODY = 40,
    CHARACTER_ATTRIBUTE_SKILL_LIGHT = 41,
    CHARACTER_ATTRIBUTE_SKILL_DARK = 42,
    CHARACTER_ATTRIBUTE_SKILL_MEDITATION = 43,
    CHARACTER_ATTRIBUTE_SKILL_BOW = 44,
    CHARACTER_ATTRIBUTE_SKILL_SHIELD = 45,
    CHARACTER_ATTRIBUTE_SKILL_LEARNING = 46
};
