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

    for (TsvLine cells : TsvReader(quests).drop(1).skip(&TsvLine::isBlank)) {
        QuestBit qbit = static_cast<QuestBit>(cells[0].as<int>());
        pQuestTable[qbit] = cells[1];
    }
}
