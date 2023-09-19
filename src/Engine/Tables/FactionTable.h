#pragma once

#include "Engine/Objects/Monsters.h" // TODO(captainurist): MonsterEnums.h

class Blob;

struct FactionTable {
    void Initialize(const Blob &factions);

    MonsterHostility relations[89][89]; // TODO(captainurist): index is 1 + MONSTER_TYPE / 3?
};

extern FactionTable *pFactionTable;
