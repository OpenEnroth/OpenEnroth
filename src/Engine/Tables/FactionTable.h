#pragma once

#include "Engine/Objects/MonsterEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

struct FactionTable {
    void Initialize(const Blob &factions);

    // Original table was 89x89 elements, in OE it was expanded to include unused monster types.
    IndexedArray<IndexedArray<MonsterHostility, MONSTER_TYPE_INVALID, MONSTER_TYPE_LAST>, MONSTER_TYPE_INVALID, MONSTER_TYPE_LAST> relations;
};

extern FactionTable *pFactionTable;
