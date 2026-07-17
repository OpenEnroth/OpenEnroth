#include "MessageScrollTable.h"

#include <array>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

IndexedArray<std::string, ITEM_FIRST_MESSAGE_SCROLL, ITEM_LAST_MESSAGE_SCROLL> pMessageScrolls;

void initializeMessageScrolls(std::string_view scrolls) {
    // scroll.txt table structure: item index | message text (localized) | scroll title (localized, not used) | (empty).
    for (std::string_view line : split(scrolls).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        if (tokens[0].empty())
            continue; // Skip tab-only trailing lines.

        ItemId i = static_cast<ItemId>(fromString<int>(tokens[0]));
        pMessageScrolls[i] = unquote(tokens[1]);
    }
}
