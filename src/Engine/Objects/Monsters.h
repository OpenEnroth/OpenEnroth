#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <vector>

#include "Utility/IndexedArray.h"

#include "Library/Color/Color.h"

#include "Engine/Spells/SpellEnums.h"
#include "Engine/Objects/CombinedSkillValue.h"

#include "Media/Audio/SoundEnums.h"

#include "Utility/Segment.h"

#include "ActorEnums.h"
#include "ItemEnums.h"
#include "MonsterEnums.h"

class Blob;

struct MonsterInfo {
    std::string name;
    std::string textureName;
    uint8_t level = 0;
    uint8_t treasureDropChance = 0; // Chance to drop an item, in [0, 100].
    ItemTreasureLevel treasureLevel = ITEM_TREASURE_LEVEL_INVALID;
    RandomItemType treasureType = RANDOM_ITEM_ANY;
    uint8_t goldDiceRolls = 0;
    uint8_t goldDiceSides = 0;
    bool flying = false;
    MonsterMovementType movementType = MONSTER_MOVEMENT_TYPE_SHORT;
    MonsterAiType aiType = MONSTER_AI_SUICIDE;
    MonsterHostility hostilityType = HOSTILITY_FRIENDLY;
    SPECIAL_ATTACK_TYPE specialAttackType = SPECIAL_ATTACK_NONE;
    uint8_t specialAttackLevel = 0;
    DamageType attack1Type = DAMAGE_FIRE; // TODO(captainurist): that's... a weird default.
    uint8_t attack1DamageDiceRolls = 0;
    uint8_t attack1DamageDiceSides = 0;
    uint8_t attack1DamageBonus = 0;
    uint8_t attack1MissileType = 0; // TODO(captainurist): #enum see Actor::AI_RangedAttack.
    uint8_t attack2Chance = 0;
    DamageType attack2Type = DAMAGE_FIRE; // TODO(captainurist): and here, weird default.
    uint8_t attack2DamageDiceRolls = 0;
    uint8_t attack2DamageDiceSides = 0;
    uint8_t attack2DamageBonus = 0;
    uint8_t attack2MissileType = 0;
    uint8_t spell1UseChance = 0; // In [0, 100].
    SpellId spell1Id = SPELL_NONE;
    CombinedSkillValue spell1SkillMastery;
    uint8_t spell2UseChance = 0; // In [0, 100].
    SpellId spell2Id = SPELL_NONE;
    CombinedSkillValue spell2SkillMastery;
    uint8_t resFire = 0;
    uint8_t resAir = 0;
    uint8_t resWater = 0;
    uint8_t resEarth = 0;
    uint8_t resMind = 0;
    uint8_t resSpirit = 0;
    uint8_t resBody = 0;
    uint8_t resLight = 0;
    uint8_t resDark = 0;
    uint8_t resPhysical = 0;
    MonsterSpecialAbility specialAbilityType = MONSTER_SPECIAL_ABILITY_NONE;
    uint8_t specialAbilityDamageDiceRolls = 0;
    uint8_t specialAbilityDamageDiceSides = 0;
    uint8_t specialAbilityDamageDiceBonus = 0;
    uint8_t numCharactersAttackedPerSpecialAbility = 0;
    MonsterId id = MONSTER_INVALID;
    bool bloodSplatOnDeath = 0;  // true for bloodsplat on death
    int16_t field_3C_some_special_attack = 0; // TODO(captainurist): what is this?
    int16_t field_3E = 0;
    unsigned int hp = 0;
    unsigned int ac = 0;
    unsigned int exp = 0; // Amount of experience the party gains upon killing this monster.
    unsigned int baseSpeed = 0;
    signed int recoveryTime = 0;
    MonsterAttackPreferences attackPreferences;
};

struct MonsterStats {
    void Initialize(const Blob &monsters);
    void InitializePlacements(const Blob &placements);
    MonsterId FindMonsterByTextureName(const std::string &Str2);

    IndexedArray<MonsterInfo, MONSTER_FIRST, MONSTER_LAST> infos;
    std::array<std::string, 31> uniqueNames; // Names of unique monsters, see Actor::uniqueNameIndex. Element 0 is unused.
};

struct MonsterDesc {
    uint16_t monsterHeight;
    uint16_t monsterRadius;
    uint16_t movementSpeed;
    int16_t toHitRadius;
    Color tintColor;
    IndexedArray<SoundId, ACTOR_SOUND_FIRST, ACTOR_SOUND_LAST> soundSampleIds;
    std::string monsterName;
    IndexedArray<std::string, ANIM_First, ANIM_Last> spriteNames;
};

struct MonsterList {
    MonsterId GetMonsterIDByName(const std::string &pMonsterName);

    IndexedArray<MonsterDesc, MONSTER_FIRST, MONSTER_LAST> monsters;
};

extern struct MonsterStats *pMonsterStats;
extern struct MonsterList *pMonsterList;
