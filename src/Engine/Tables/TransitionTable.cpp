#include "TransitionTable.h"

#include <array>
#include <cassert>
#include <string>

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

std::array<std::string, 465> pTransitionStrings;

void initializeTransitions(const Blob &transitions) {
    // trans.txt table structure: index | description (localized) | name (not localized, not used).
    pTransitionStrings.fill({});

    for (TsvLine cells : TsvReader(transitions).drop(1).skip(&TsvLine::isBlank)) {
        int i = cells[0].as<int>();
        pTransitionStrings[i] = cells[1];
    }
}
