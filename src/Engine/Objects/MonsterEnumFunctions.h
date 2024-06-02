#pragma once

#include <span>

#include "GUI/UI/UIHouseEnums.h"

#include "Utility/Segment.h"

#include "ItemEnums.h"
#include "CharacterEnums.h"
#include "MonsterEnums.h"

//
// MonsterId
//

inline Segment<MonsterId> allMonsters() {
    return {MONSTER_FIRST, MONSTER_LAST};
}

/**
 * @return                              A span of all monsters that can appear in Arena.
 */
std::span<const MonsterId> allArenaMonsters();


//
// MonsterType
//

inline Segment<MonsterType> allMonsterTypes() {
    return {MONSTER_TYPE_FIRST, MONSTER_TYPE_LAST};
}

inline MonsterType monsterTypeForMonsterId(MonsterId monsterId) {
    return static_cast<MonsterType>((std::to_underlying(monsterId) - 1) / 3 + 1);
}

inline Segment<MonsterId> monsterIdsForMonsterType(MonsterType monsterType) {
    MonsterId first = static_cast<MonsterId>((std::to_underlying(monsterType) - 1) * 3 + 1);
    MonsterId last = static_cast<MonsterId>(std::to_underlying(first) + 2);
    return {first, last};
}

inline bool isPeasant(MonsterType monsterType) {
    return
        (monsterType >= MONSTER_TYPE_FIRST_PEASANT_DWARF && monsterType <= MONSTER_TYPE_LAST_PEASANT_DWARF) ||
        (monsterType >= MONSTER_TYPE_FIRST_PEASANT_ELF && monsterType <= MONSTER_TYPE_LAST_PEASANT_ELF) ||
        (monsterType >= MONSTER_TYPE_FIRST_PEASANT_HUMAN && monsterType <= MONSTER_TYPE_LAST_PEASANT_HUMAN) ||
        (monsterType >= MONSTER_TYPE_FIRST_PEASANT_GOBLIN && monsterType <= MONSTER_TYPE_LAST_PEASANT_GOBLIN);
}

inline bool isPeasant(MonsterId monsterId) {
    return isPeasant(monsterTypeForMonsterId(monsterId));
}

CharacterSex sexForMonsterType(MonsterType monsterType);

Race raceForMonsterType(MonsterType monsterType);

bool isBountyHuntable(MonsterType monsterType, HouseId townHall);

ItemId itemDropForMonsterType(MonsterType monsterType);


//
// MonsterTier
//

// TODO(captainurist): add a unit test here.
inline MonsterTier monsterTierForMonsterId(MonsterId monsterId) {
    return static_cast<MonsterTier>((std::to_underlying(monsterId) - std::to_underlying(MONSTER_FIRST)) % 3);
}


//
// MonsterSupertype
//

/**
 * @offset 0x00438BDF
 *
 * @param monsterType                   Monster type to check.
 * @return                              Supertype for the provided monster type.
 */
MonsterSupertype supertypeForMonsterType(MonsterType monsterType);

inline MonsterSupertype supertypeForMonsterId(MonsterId monsterId) {
    return supertypeForMonsterType(monsterTypeForMonsterId(monsterId));
}


//
// MonsterAttackPreference
//

std::span<const MonsterAttackPreference> allMonsterAttackPreferences();
