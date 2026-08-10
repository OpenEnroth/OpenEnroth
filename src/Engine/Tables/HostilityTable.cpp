#include "HostilityTable.h"

#include <string>

#include "Library/Tsv/TsvReader.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Segment.h"

HostilityTable *pHostilityTable;

//----- (004547E4) --------------------------------------------------------
void HostilityTable::Initialize(const Blob &factions) {
    // hostile.txt table structure: monster name (localized, not used) | hostility values per monster group...
    for (auto &line : relations)
        line.fill(HOSTILITY_FRIENDLY);

    for (auto [line, row] : TsvReader(factions).drop(1).skip(&TsvLine::isEmpty).zip(Segment(0, 88))) {
        for (auto [cell, col] : line.cells().drop(1).zip(Segment(0, 88)))
            relations[static_cast<MonsterType>(col)][static_cast<MonsterType>(row)] = static_cast<MonsterHostility>(fromString<int>(cell));
    }
}
