#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <vector>

#include "Utility/IndexedArray.h"

#include "Library/Color/Color.h"

#include "Engine/Spells/SpellEnums.h"
#include "Engine/Objects/CombinedSkillValue.h"

#include "Utility/Segment.h"

#include "ActorEnums.h"
#include "ItemEnums.h"

class Blob;

// TODO(captainurist): Rename to MonsterId?
// TODO(captainurist): codegen enum values.
enum class MONSTER_TYPE {
    MONSTER_0 = 0, // TODO(captainurist): MONSTER_INVALID?
    MONSTER_1 = 1,
    MONSTER_DEVIL_1 = 22,
    MONSTER_DEVIL_2 = 23,
    MONSTER_DEVIL_3 = 24,
    MONSTER_DRAGON_1 = 25,
    MONSTER_DRAGON_2 = 26,
    MONSTER_DRAGON_3 = 27,
    MONSTER_45 = 45,
    MONSTER_ELEMENTAL_WATER_1 = 46,
    MONSTER_ELEMENTAL_WATER_3 = 48,
    MONSTER_ELF_ARCHER_1 = 49,
    MONSTER_ELF_ARCHER_3 = 51,
    MONSTER_ELF_SPEARMAN_1 = 52,
    MONSTER_ELF_SPEARMAN_3 = 54,
    MONSTER_GHOST_1 = 70,
    MONSTER_GHOST_3 = 72,
    MONSTER_HARPY_1 = 85,
    MONSTER_HARPY_2 = 86,
    MONSTER_HARPY_3 = 87,
    MONSTER_LICH_1 = 91,
    MONSTER_LICH_3 = 93,
    MONSTER_OOZE_1 = 112,
    MONSTER_OOZE_2 = 113,
    MONSTER_OOZE_3 = 114,
    MONSTER_115 = 115,
    MONSTER_PEASANT_ELF_FEMALE_1_1 = 133,
    MONSTER_PEASANT_ELF_MALE_3_3 = 150,
    MONSTER_186 = 186,
    MONSTER_SKELETON_1 = 199,
    MONSTER_SKELETON_3 = 201,
    MONSTER_TITAN_1 = 211,
    MONSTER_TITAN_3 = 213,
    MONSTER_VAMPIRE_1 = 217,
    MONSTER_VAMPIRE_3 = 219,
    MONSTER_WIGHT_1 = 223,
    MONSTER_WIGHT_3 = 225,
    MONSTER_ZOMBIE_1 = 229,
    MONSTER_ZOMBIE_3 = 231,
    MONSTER_232 = 232,
    MONSTER_PEASANT_GOBLIN_MALE_3_3 = 249,
    MONSTER_TROLL_1 = 250,
    MONSTER_TROLL_2 = 251,
    MONSTER_TROLL_3 = 252,
    MONSTER_TREANT_1 = 253,
    MONSTER_TREANT_3 = 255,
    MONSTER_GHOUL_1 = 256,
    MONSTER_GHOUL_3 = 258,

    MONSTER_FIRST = MONSTER_1,
    MONSTER_LAST = 277,

    MONSTER_FIRST_ARENA = MONSTER_1,
    MONSTER_LAST_ARENA = MONSTER_GHOUL_3
};
using enum MONSTER_TYPE;

inline Segment<MONSTER_TYPE> allArenaMonsters() {
    return {MONSTER_FIRST_ARENA, MONSTER_LAST_ARENA};
}

/*  335 */
enum class MONSTER_SPECIAL_ABILITY_TYPE {
    MONSTER_SPECIAL_ABILITY_NONE = 0x0,
    MONSTER_SPECIAL_ABILITY_SHOT = 0x1,
    MONSTER_SPECIAL_ABILITY_SUMMON = 0x2,
    MONSTER_SPECIAL_ABILITY_EXPLODE = 0x3,
};
using enum MONSTER_SPECIAL_ABILITY_TYPE;

enum class MONSTER_MOVEMENT_TYPE {
    MONSTER_MOVEMENT_TYPE_SHORT = 0x0,
    MONSTER_MOVEMENT_TYPE_MEDIUM = 0x1,
    MONSTER_MOVEMENT_TYPE_LONG = 0x2,
    MONSTER_MOVEMENT_TYPE_GLOBAL = 0x3,
    MONSTER_MOVEMENT_TYPE_FREE = 0x4,
    MONSTER_MOVEMENT_TYPE_STATIONARY = 0x5,
};
using enum MONSTER_MOVEMENT_TYPE;

/*  336 */
enum class MONSTER_SUPERTYPE {
    MONSTER_SUPERTYPE_UNDEAD = 0x1,
    MONSTER_SUPERTYPE_KREEGAN = 0x2,
    MONSTER_SUPERTYPE_DRAGON = 0x3,
    MONSTER_SUPERTYPE_ELF = 0x4,
    MONSTER_SUPERTYPE_WATER_ELEMENTAL = 0x5,
    MONSTER_SUPERTYPE_TREANT = 0x6,
    MONSTER_SUPERTYPE_TITAN = 0x7,
    MONSTER_SUPERTYPE_8 = 0x8,
};
using enum MONSTER_SUPERTYPE;

enum class SPECIAL_ATTACK_TYPE : uint8_t {
    SPECIAL_ATTACK_NONE = 0,
    SPECIAL_ATTACK_CURSE = 1,
    SPECIAL_ATTACK_WEAK = 2,
    SPECIAL_ATTACK_SLEEP = 3,
    SPECIAL_ATTACK_DRUNK = 4,
    SPECIAL_ATTACK_INSANE = 5,
    SPECIAL_ATTACK_POISON_WEAK = 6,
    SPECIAL_ATTACK_POISON_MEDIUM = 7,
    SPECIAL_ATTACK_POISON_SEVERE = 8,
    SPECIAL_ATTACK_DISEASE_WEAK = 9,
    SPECIAL_ATTACK_DISEASE_MEDIUM = 10,
    SPECIAL_ATTACK_DISEASE_SEVERE = 11,
    SPECIAL_ATTACK_PARALYZED = 12,
    SPECIAL_ATTACK_UNCONSCIOUS = 13,
    SPECIAL_ATTACK_DEAD = 14,
    SPECIAL_ATTACK_PETRIFIED = 15,
    SPECIAL_ATTACK_ERADICATED = 16,
    SPECIAL_ATTACK_BREAK_ANY = 17,
    SPECIAL_ATTACK_BREAK_ARMOR = 18,
    SPECIAL_ATTACK_BREAK_WEAPON = 19,
    SPECIAL_ATTACK_STEAL = 20,
    SPECIAL_ATTACK_AGING = 21,
    SPECIAL_ATTACK_MANA_DRAIN = 22,
    SPECIAL_ATTACK_FEAR = 23,
};
using enum SPECIAL_ATTACK_TYPE;

struct MonsterInfo {
    enum HostilityRadius {
        Hostility_Friendly = 0,
        Hostility_Close = 1,
        Hostility_Short = 2,
        Hostility_Medium = 3,
        Hostility_Long = 4
    };

    std::string pName;
    std::string pPictureName;
    uint8_t uLevel = 0;
    uint8_t uTreasureDropChance = 0;
    uint8_t uTreasureDiceRolls = 0;
    uint8_t uTreasureDiceSides = 0;
    ItemTreasureLevel uTreasureLevel = ITEM_TREASURE_LEVEL_INVALID;
    RandomItemType uTreasureType = RANDOM_ITEM_ANY;
    uint8_t uFlying = 0;
    MONSTER_MOVEMENT_TYPE uMovementType = MONSTER_MOVEMENT_TYPE_SHORT;
    uint8_t uAIType = 0;
    HostilityRadius uHostilityType = Hostility_Friendly;
    SPECIAL_ATTACK_TYPE uSpecialAttackType = SPECIAL_ATTACK_NONE;
    uint8_t uSpecialAttackLevel = 0;
    DAMAGE_TYPE uAttack1Type = DAMAGE_FIRE; // TODO(captainurist): that's... a weird default.
    uint8_t uAttack1DamageDiceRolls = 0;
    uint8_t uAttack1DamageDiceSides = 0;
    uint8_t uAttack1DamageBonus = 0;
    uint8_t uMissleAttack1Type = 0;
    uint8_t uAttack2Chance = 0;
    DAMAGE_TYPE uAttack2Type = DAMAGE_FIRE; // TODO(captainurist): and here, weird default.
    uint8_t uAttack2DamageDiceRolls = 0;
    uint8_t uAttack2DamageDiceSides = 0;
    uint8_t uAttack2DamageBonus = 0;
    uint8_t uMissleAttack2Type = 0;
    uint8_t uSpell1UseChance = 0;
    SpellId uSpell1ID = SPELL_NONE;
    uint8_t uSpell2UseChance = 0;
    SpellId uSpell2ID = SPELL_NONE;
    uint8_t uResFire = 0;
    uint8_t uResAir = 0;
    uint8_t uResWater = 0;
    uint8_t uResEarth = 0;
    uint8_t uResMind = 0;
    uint8_t uResSpirit = 0;
    uint8_t uResBody = 0;
    uint8_t uResLight = 0;
    uint8_t uResDark = 0;
    uint8_t uResPhysical = 0;
    MONSTER_SPECIAL_ABILITY_TYPE uSpecialAbilityType = MONSTER_SPECIAL_ABILITY_NONE;
    uint8_t uSpecialAbilityDamageDiceRolls = 0;
    uint8_t uSpecialAbilityDamageDiceSides = 0;
    uint8_t uSpecialAbilityDamageDiceBonus = 0;
    uint8_t uNumCharactersAttackedPerSpecialAbility = 0;
    MONSTER_TYPE uID = MONSTER_0;
    bool bBloodSplatOnDeath = 0;  // true for bloodsplat on death
    CombinedSkillValue uSpellSkillAndMastery1;
    CombinedSkillValue uSpellSkillAndMastery2;
    int16_t field_3C_some_special_attack = 0; // TODO(captainurist): what is this?
    int16_t field_3E = 0;
    unsigned int uHP = 0;
    unsigned int uAC = 0;
    unsigned int uExp = 0;
    unsigned int uBaseSpeed = 0;
    signed int uRecoveryTime = 0;
    unsigned int uAttackPreference = 0;
};

struct MonsterStats {
    void Initialize(const Blob &monsters);
    void InitializePlacements(const Blob &placements);
    MONSTER_TYPE FindMonsterByTextureName(const std::string &Str2);

    static bool BelongsToSupertype(MONSTER_TYPE uMonsterInfoID,
                                   MONSTER_SUPERTYPE eSupertype);

    IndexedArray<MonsterInfo, MONSTER_FIRST, MONSTER_LAST> pInfos;      // 0 - 5b18h
    std::array<std::string, 31> pPlaceStrings;      // 5B18h placement counts from 1
    unsigned int uNumMonsters;    // 5B94h // TODO(captainurist): can drop?
    unsigned int uNumPlacements;  // 5B98h
    int field_5B9C;
};

struct MonsterDesc {
    uint16_t uMonsterHeight;
    uint16_t uMonsterRadius;
    uint16_t uMovementSpeed;
    int16_t uToHitRadius;
    Color sTintColor;
    std::array<uint16_t, 4> pSoundSampleIDs; // TODO(captainurist): actually this one should also be an IndexedArray
    std::string pMonsterName;
    IndexedArray<std::string, ANIM_First, ANIM_Last> pSpriteNames;
};

struct MonsterList {
    MONSTER_TYPE GetMonsterIDByName(const std::string &pMonsterName);
    bool FromFileTxt(const char *Args);

    IndexedArray<MonsterDesc, MONSTER_FIRST, MONSTER_LAST> pMonsters;
};

extern struct MonsterStats *pMonsterStats;
extern struct MonsterList *pMonsterList;
