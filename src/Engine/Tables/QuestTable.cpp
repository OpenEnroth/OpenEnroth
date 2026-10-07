#include "QuestTable.h"

#include <array>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<std::string, QBIT_FIRST, QBIT_LAST> pQuestTable;

void initializeQuests(const Blob &quests) {
    // quests.txt table structure: quest bit | text (localized) | dev notes (not used) |
    //                             quest giver name (not localized, not used).
    pQuestTable.fill({});

    for (std::string_view line : split(quests.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        QuestBit qbit = static_cast<QuestBit>(fromString<int>(tokens[0]));
        // Rows without a quest have "0" for text. The English and Russian data have it for bit 271 too, which a script
        // sets, and vanilla MM7 then lists a quest that reads "0". The German and French data leave that text empty.
        pQuestTable[qbit] = tokens[1] == "0" ? "" : unquote(tokens[1]);
    }
}
