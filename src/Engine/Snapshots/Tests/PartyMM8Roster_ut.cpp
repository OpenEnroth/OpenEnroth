#include <string>
#include <string_view>

#include "Testing/Unit/UnitTest.h"

#include "Engine/PartyMM8Roster.h"

// MM8's Adventurer's Inn bench - companions recruited but not in the active
// party, keyed by `Character::rosterId`.

static Character makeCharacter(int rosterId, std::string_view name) {
    Character result;
    result.rosterId = rosterId;
    result.name = std::string(name);
    return result;
}

UNIT_TEST(PartyMM8Roster, AddedCharacterIsInInn) {
    PartyMM8Roster roster;
    roster.addToInn(makeCharacter(4, "Test Companion Four"));

    EXPECT_TRUE(roster.isInInn(4));
    EXPECT_FALSE(roster.isInInn(34));
    EXPECT_EQ(roster.innCount(), 1);
}

UNIT_TEST(PartyMM8Roster, TakeFromInnRemovesAndReturnsCharacter) {
    PartyMM8Roster roster;
    roster.addToInn(makeCharacter(4, "Test Companion Four"));
    roster.addToInn(makeCharacter(34, "Test Companion ThirtyFour"));

    Character taken = roster.takeFromInn(4);

    EXPECT_EQ(taken.name, "Test Companion Four");
    EXPECT_FALSE(roster.isInInn(4));
    EXPECT_TRUE(roster.isInInn(34));
    EXPECT_EQ(roster.innCount(), 1);
}

UNIT_TEST(PartyMM8Roster, InnCountTracksMultipleMembers) {
    PartyMM8Roster roster;
    EXPECT_EQ(roster.innCount(), 0);

    roster.addToInn(makeCharacter(4, "Test Companion Four"));
    roster.addToInn(makeCharacter(34, "Test Companion ThirtyFour"));
    EXPECT_EQ(roster.innCount(), 2);

    roster.takeFromInn(34);
    EXPECT_EQ(roster.innCount(), 1);
}
