#include "QuestTable.h"

#include <array>
#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

IndexedArray<std::string, QBIT_FIRST, QBIT_LAST> pQuestTable;

void initializeQuests(const Blob &quests) {
    // quests.txt table structure: quest bit | text (localized) | dev notes (not used) |
    //                             quest giver name (not localized, not used).
    pQuestTable.fill({});

    for (TsvLine line : TsvReader(quests).drop(1).skip(&TsvLine::isBlank)) {
        QuestBit qbit = static_cast<QuestBit>(line[0].as<int>());
        pQuestTable[qbit] = line[1];
    }
}
