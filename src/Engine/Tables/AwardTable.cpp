#include "AwardTable.h"

#include <ranges>
#include <string>
#include <vector>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

std::array<AwardData, 105> pAwards;

void initializeAwards(const Blob &awards) {
    std::vector<std::string_view> chunks;
    for (std::string_view line : split(awards.string_view()).byCrLf() | std::views::drop(1)) {
        split(line).by('\t').to(&chunks);
        if (chunks.size() < 3)
            continue;

        int awardId = fromString<int>(chunks[0]);
        pAwards[awardId].pText = removeQuotes(chunks[1]);
        pAwards[awardId].uPriority = fromString<int>(chunks[2]);
    }
}
