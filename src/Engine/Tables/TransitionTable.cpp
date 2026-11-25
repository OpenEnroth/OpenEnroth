#include "TransitionTable.h"

#include <array>
#include <cassert>
#include <string>

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

std::array<std::string, 501> pTransitionStrings;

void initializeTransitions(const Blob &transitions) {
    // trans.txt table structure: index | description (localized) | name (not localized, not used).
    pTransitionStrings.fill({});

    for (std::string_view line : split(transitions.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t'); // Some rows have no description, so it defaults to "".
        if (tokens[0].empty())
            continue; // MM8's trans.txt also has blank-index tab-only trailing lines.
        int i = fromString<int>(tokens[0]);
        if (i < 0 || static_cast<size_t>(i) >= pTransitionStrings.size()) {
            MM_WARNING("TransitionTable: dropping transition #{}, doesn't fit in a {}-slot table", i, pTransitionStrings.size());
            continue;
        }
        pTransitionStrings[i] = unquote(tokens[1]);
    }
}
