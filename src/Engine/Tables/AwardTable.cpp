#include "AwardTable.h"

#include <array>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<AwardData, AWARD_FIRST, AWARD_LAST> pAwards;

void initializeAwards(std::string_view awards) {
    // awards.txt table structure: index | text (localized) | priority.
    for (std::string_view line : split(awards).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 3> chunks = split(line).by('\t');
        if (chunks[2].empty())
            continue; // Truncated lines with just the index exist in the file.

        AwardId awardId = static_cast<AwardId>(fromString<int>(chunks[0]));
        pAwards[awardId].pText = removeQuotes(chunks[1]);
        pAwards[awardId].uPriority = fromString<int>(chunks[2]);
    }
}
