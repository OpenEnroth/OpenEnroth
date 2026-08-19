#include "MessageScrollTable.h"

#include <array>
#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

IndexedArray<std::string, ITEM_FIRST_MESSAGE_SCROLL, ITEM_LAST_MESSAGE_SCROLL> pMessageScrolls;

void initializeMessageScrolls(const Blob &scrolls) {
    // scroll.txt table structure: item index | message text (localized) | scroll title (localized, not used) | (empty).
    for (TsvLine line : TsvReader(scrolls).drop(1).skip(&TsvLine::isBlank)) {
        if (line[0].empty())
            continue; // Skip tab-only trailing lines.

        ItemId i = static_cast<ItemId>(line[0].as<int>());
        pMessageScrolls[i] = line[1];
    }
}
