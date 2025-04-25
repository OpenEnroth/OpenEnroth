#include "Testing/Game/GameTest.h"

#include "Engine/MapEnums.h"

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
