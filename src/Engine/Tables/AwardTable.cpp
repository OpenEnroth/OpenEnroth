#include "AwardTable.h"

#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

IndexedArray<AwardData, AWARD_FIRST, AWARD_LAST> pAwards;

void initializeAwards(const Blob &awards) {
    // awards.txt table structure: index | text (localized) | priority.
    for (TsvLine cells : TsvReader(awards).drop(1).skip(&TsvLine::isBlank)) {
        if (cells[2].empty())
            continue; // Truncated lines with just the index exist in the file.

        AwardId awardId = static_cast<AwardId>(cells[0].as<int>());
        pAwards[awardId].pText = cells[1];
        pAwards[awardId].uPriority = cells[2].as<int>();
    }
}
