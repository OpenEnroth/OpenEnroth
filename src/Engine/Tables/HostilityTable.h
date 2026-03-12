#pragma once

#include "Engine/Objects/MonsterEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

/**
 * Table of hostility levels between monster types. Indexed by two `MonsterType` values,
 * returns the `MonsterHostility` (aggro range) between them. `MONSTER_TYPE_INVALID` (index 0)
 * is the party's row/column.
 */
struct HostilityTable {
    void Initialize(const Blob &factions);

    // Original table was 89x89 elements, in OE it was expanded to include unused monster types.
    IndexedArray<IndexedArray<MonsterHostility, MONSTER_TYPE_INVALID, MONSTER_TYPE_LAST>, MONSTER_TYPE_INVALID, MONSTER_TYPE_LAST> relations;
};

extern HostilityTable *pHostilityTable;
