#include "QuestTable.h"

#include <cassert>
#include <string>
#include <vector>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<std::string, QBIT_FIRST, QBIT_LAST> pQuestTable;

void initializeQuests(const Blob &quests) {
    // quests.txt table structure: quest bit | text (localized) | dev notes (not used) |
    //                             quest giver name (not localized, not used).
    pQuestTable.fill({});

    std::vector<std::string_view> tokens;
    for (std::string_view line : split(quests.str()).by("\r\n").drop(1).skip("")) {
        split(line).by('\t').to(&tokens);
        assert(tokens.size() >= 2 && "Invalid number of tokens"); // TODO(captainurist): should not be an assert.

        QuestBit qbit = static_cast<QuestBit>(fromString<int>(tokens[0]));
        pQuestTable[qbit] = removeQuotes(tokens[1]);
    }
}
