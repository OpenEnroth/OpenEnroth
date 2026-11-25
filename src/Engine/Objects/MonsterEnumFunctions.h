#pragma once

#include <span>
#include <utility>

#include "Engine/Data/HouseEnums.h"
#include "Engine/GameType.h"

#include "Utility/Segment.h"
#include "Library/Color/ColorTable.h"

#include "ItemEnums.h"
#include "CharacterEnums.h"
#include "MonsterEnums.h"
#include "SpriteEnums.h"

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

/**
 * MM8's roster reshuffles which species sits at which `MonsterId`, so MM7's row-position arithmetic doesn't
 * carry over. See `mm8MonsterTypeForMonsterId` for the MM8-correct mapping, sourced from real MM8 data.
 *
 * This is a *semantic* classification (which MM7 species does this MM8 monster resemble) used for
 * item drop / sex / race / bounty-hunt / arena-eligibility / peasant classification. It is NOT safe to
 * use for hostility resolution on MM8 - see `mm8HostilityMonsterTypeForMonsterId`.
 */
MonsterType mm8MonsterTypeForMonsterId(MonsterId monsterId);

/**
 * MM8 `hostile.txt` column index per species, for hostility resolution ONLY
 * (`Actor::hostilityGroup`, `HostilityTable::relations` lookups). `hostile.txt`'s row/column
 * position for a species is unrelated to its MM7-legacy `MonsterType` ordinal - reusing
 * `mm8MonsterTypeForMonsterId`'s semantic value here either misses the table entirely (falls back to
 * the all-friendly default) or reads a different, unrelated species' real hostility value.
 */
MonsterType mm8HostilityMonsterTypeForMonsterId(MonsterId monsterId);

inline MonsterType monsterTypeForMonsterId(MonsterId monsterId) {
    if (gameType() == GAME_TYPE_MM8)
        return mm8MonsterTypeForMonsterId(monsterId);
    return static_cast<MonsterType>((std::to_underlying(monsterId) - 1) / 3 + 1);
}

/**
 * @return                              The `MonsterType` to use when resolving hostility (i.e. indexing
 *                                      into `HostilityTable::relations`) for the given monster. On MM7 this
 *                                      is the same value as `monsterTypeForMonsterId` (MM7's row-position
 *                                      arithmetic IS also its `hostile.txt` position); on MM8 the two diverge,
 *                                      see `mm8HostilityMonsterTypeForMonsterId`.
 */
inline MonsterType hostilityMonsterTypeForMonsterId(MonsterId monsterId) {
    if (gameType() == GAME_TYPE_MM8)
        return mm8HostilityMonsterTypeForMonsterId(monsterId);
    return monsterTypeForMonsterId(monsterId);
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

/**
 * MM8 `monsters.txt` id ranges for the "peasant"-tier variant of each species, not derived
 * from `mm8MonsterTypeForMonsterId`'s MM7-semantic value (which
 * maps every MM8-only peasant species - Lizardman, Dark Elf, Ogre, Vampire, Troll, Minotaur - to the
 * shared `MONSTER_TYPE_UNUSED_RAT` placeholder, so `isPeasant(MonsterType)`'s dwarf/elf/human/goblin
 * ranges never match them). Use this for MM8 peasant classification instead of `isPeasant(MonsterType)`.
 */
bool isMm8PeasantMonsterId(MonsterId monsterId);

inline bool isPeasant(MonsterId monsterId) {
    if (gameType() == GAME_TYPE_MM8)
        return isMm8PeasantMonsterId(monsterId);
    return isPeasant(monsterTypeForMonsterId(monsterId));
}

Sex sexForMonsterType(MonsterType monsterType);

Race raceForMonsterType(MonsterType monsterType);

bool isBountyHuntable(MonsterType monsterType, HouseId townHall);

ItemId itemDropForMonsterType(MonsterType monsterType);


//
// MonsterTier
//

inline MonsterTier monsterTierForMonsterId(MonsterId monsterId) {
    return static_cast<MonsterTier>((std::to_underlying(monsterId) - std::to_underlying(MONSTER_FIRST)) % 3);
}

inline MonsterId monsterIdForMonsterTypeAndTier(MonsterType monsterType, MonsterTier tier) {
    return static_cast<MonsterId>((std::to_underlying(monsterType) - 1) * 3 + 1 + std::to_underlying(tier));
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

/**
 * MM8-correct `MonsterId` -> `MonsterSupertype` mapping for the "kinds" MM8 actually needs a supertype for
 * (see `MONSTER_SUPERTYPE_MM8_OGRE`/`MONSTER_SUPERTYPE_MM8_ELEMENTAL`), sourced from real MM8 data rather
 * than from `monsterTypeForMonsterId` + `supertypeForMonsterType` (which don't carry over to MM8's roster).
 */
MonsterSupertype mm8SupertypeForMonsterId(MonsterId monsterId);

inline MonsterSupertype supertypeForMonsterId(MonsterId monsterId) {
    if (gameType() == GAME_TYPE_MM8)
        return mm8SupertypeForMonsterId(monsterId);
    return supertypeForMonsterType(monsterTypeForMonsterId(monsterId));
}


//
// MonsterAttackPreference
//

std::span<const MonsterAttackPreference> allMonsterAttackPreferences();


//
// MonsterProjectile
//

/**
 * @param projectile                    Monster projectile to get a sprite id for.
 * @return                              Sprite id to use for the given monster projectile.
 * @see isMonsterProjectileSprite
 */
SpriteId spriteForMonsterProjectile(MonsterProjectile projectile);
