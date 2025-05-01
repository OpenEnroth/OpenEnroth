#include "Testing/Game/GameTest.h"

#include "Engine/MapEnums.h"
#include "Engine/Party.h"


// 2000

GAME_TEST(Issues, Issue2017) {
    // Bats move through closed doors in Barrow XII
    test.playTraceFromTestData("issue_2017.mm7", "issue_2017.json");
    // Make sure all the monsters have stayed contained
    constexpr std::array<int, 3> monsterIds = { 12, 13, 20 };
    for (auto ids : monsterIds) {
        EXPECT_LT(pActors[ids].pos.y, -100);
    }
}

GAME_TEST(Issues, Issue2018) {
    // Scrolls of Town Portal and Lloyd's Beacon did consume mana or assert when cast by a character with insufficient mana.
    // Not testing mana as the trace already compares before and after to recorded values.
    auto mapTape = tapes.map();
    auto scrollsLBTape = tapes.totalItemCount(ITEM_SCROLL_LLOYDS_BEACON);
    auto scrollsTPTape = tapes.totalItemCount(ITEM_SCROLL_TOWN_PORTAL);
    test.playTraceFromTestData("issue_2018.mm7", "issue_2018.json");
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_ERATHIA, MAP_EMERALD_ISLAND));
    EXPECT_EQ(scrollsLBTape.front(), 6); // Used 2 Lloyd's out of 6, ignore intervening steps from pickup and r-click.
    EXPECT_EQ(scrollsLBTape.back(), 4);
    EXPECT_EQ(scrollsTPTape.front(), 4); // Also used 2 Town Portal because it fails once on the Thief.
    EXPECT_EQ(scrollsTPTape.back(), 2);
}

GAME_TEST(Issues, Issue2021_2022) {
    // Lloyd's Beacon did not keep beacons in the player-selected slot.
    // Also, OE did allow characters in recovery to cast from spell scrolls.
    // The trace does a similar portal to Erathia and back as Issue2018, but selects the center slot to do so.
    // Additionally, it tries to cast a Protection from Magic scroll on a 'greyed' character - should _not_ succeed.
    auto mapTape = tapes.map();
    auto scrollsPMTape = tapes.totalItemCount(ITEM_SCROLL_PROTECTION_FROM_MAGIC);
    auto soundsTape = tapes.sounds();
    auto statusTape = tapes.statusBar();
    auto pmBuffTape = tapes.custom([] { return pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].Active(); });
    auto lloydSlot1Tape = tapes.custom([] { return static_cast<bool>(pParty->pCharacters[3].vBeacons[0]); });
    test.playTraceFromTestData("issue_2021_2022.mm7", "issue_2021_2022.json");
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_ERATHIA, MAP_EMERALD_ISLAND));
    EXPECT_EQ(scrollsPMTape.delta(), 0); // No Protection from Magic scroll used
    EXPECT_CONTAINS(soundsTape.flattened(), SOUND_error);
    EXPECT_CONTAINS(statusTape, "That player is not active");
    EXPECT_EQ(lloydSlot1Tape, tape(false)); // Top left slot stayed empty
    EXPECT_EQ(pmBuffTape, tape(false)); // Not Prot Mg buff received
}
