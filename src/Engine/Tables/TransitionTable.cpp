#include "TransitionTable.h"

#include <array>
#include <cassert>
#include <string>

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

std::array<std::string, 465> pTransitionStrings;

void initializeTransitions(const Blob &transitions) {
    // trans.txt table structure: index | description (localized) | name (not localized, not used).
    pTransitionStrings.fill({});

    for (std::string_view line : split(transitions.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t'); // Some rows have no description, so it defaults to "".
        int i = fromString<int>(tokens[0]);
        pTransitionStrings[i] = unquote(tokens[1]);
    }
}
