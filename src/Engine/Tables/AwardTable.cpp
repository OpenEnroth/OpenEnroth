#include "AwardTable.h"

#include <array>
#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

IndexedArray<AwardData, AWARD_FIRST, AWARD_LAST> pAwards;

void initializeAwards(const Blob &awards) {
    // awards.txt table structure: index | text (localized) | priority.
    for (TsvLine line : TsvReader(awards).drop(1).skip(&TsvLine::isBlank)) {
        if (line[2].empty())
            continue; // Truncated lines with just the index exist in the file.

        AwardId awardId = static_cast<AwardId>(line[0].as<int>());
        pAwards[awardId].pText = line[1];
        pAwards[awardId].uPriority = line[2].as<int>();
    }
}
