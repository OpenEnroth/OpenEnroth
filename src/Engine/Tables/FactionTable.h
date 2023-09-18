#pragma once

class Blob;

struct FactionTable {
    void Initialize(const Blob &factions);

    char relations[89][89]; // TODO(captainurist): index is 1 + MONSTER_TYPE / 3?
};

extern FactionTable *pFactionTable;
