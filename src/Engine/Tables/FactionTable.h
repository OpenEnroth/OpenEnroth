#pragma once

#include "Engine/Objects/MonsterEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

struct FactionTable {
    void Initialize(const Blob &factions);

    IndexedArray<IndexedArray<MonsterHostility, MONSTER_TYPE_INVALID, MONSTER_TYPE_LAST>, MONSTER_TYPE_INVALID, MONSTER_TYPE_LAST> relations;
};

extern FactionTable *pFactionTable;
