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
#include "MonsterEnums.h"

class Blob;

struct MonsterInfo {
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
    MonsterHostility uHostilityType = HOSTILITY_FRIENDLY;
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
    MonsterId uID = MONSTER_INVALID;
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
    MonsterId FindMonsterByTextureName(const std::string &Str2);

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
    MonsterId GetMonsterIDByName(const std::string &pMonsterName);
    bool FromFileTxt(const char *Args);

    IndexedArray<MonsterDesc, MONSTER_FIRST, MONSTER_LAST> pMonsters;
};

extern struct MonsterStats *pMonsterStats;
extern struct MonsterList *pMonsterList;
