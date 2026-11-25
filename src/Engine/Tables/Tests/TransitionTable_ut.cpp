#include <string>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Tables/TransitionTable.h"
#include "Utility/Memory/Blob.h"

// MM8's trans.txt has real rows up to index 500, past MM7's 465-slot ceiling - these used to be
// out-of-bounds writes into `pTransitionStrings`.
UNIT_TEST(TransitionTable, InitializeMm8FullRange) {
    std::string text =
        "2D#\tTransition Description\t\r\n"
        "1\tPlaceholder transition text.\t\"Placeholder place, 32\"\r\n"
        "500\tA far-flung MM8-only transition.\t\"Some place, 1\"\r\n";

    initializeTransitions(Blob::fromString(std::move(text))); // Shouldn't throw/OOB - row 500 used to be past MM7's ceiling.

    EXPECT_EQ(pTransitionStrings[1], "Placeholder transition text.");
    EXPECT_EQ(pTransitionStrings[500], "A far-flung MM8-only transition.");
}
