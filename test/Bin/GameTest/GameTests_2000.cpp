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
    auto mapTape = tapes.map();
    auto scrollsLBTape = tapes.totalItemCount(ITEM_SCROLL_LLOYDS_BEACON);
    auto scrollsTPTape = tapes.totalItemCount(ITEM_SCROLL_TOWN_PORTAL);
    auto recoveryTape = charTapes.areRecovering();
    auto mpTape = charTapes.mps();
    test.playTraceFromTestData("issue_2018.mm7", "issue_2018.json");
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_ERATHIA, MAP_EMERALD_ISLAND));
    EXPECT_EQ(scrollsLBTape.frontBack(), tape(6, 4)); // Used 2 Lloyd's out of 6, ignore intervening steps from pickup and r-click.
    EXPECT_EQ(scrollsTPTape.frontBack(), tape(4, 2)); // Also used 2 Town Portal because it fails once on the Thief.
    EXPECT_MISSES(recoveryTape.sliced(0), true); // Char 0 didn't do anything.
    EXPECT_CONTAINS(recoveryTape.sliced(1), true); // Char 1 did cast a spell.
    EXPECT_LT(mpTape.sliced(1).max(), 20); // Char 1 didn't have enough mana for the spells cast.
    EXPECT_MISSES(recoveryTape.sliced(2), true); // Char 2 didn't do anything.
    EXPECT_CONTAINS(recoveryTape.sliced(3), true); // Char 3 did cast a spell.
    EXPECT_LT(mpTape.sliced(3).max(), 20); // Char 3 didn't have enough mana for the spells cast.
    EXPECT_EQ(mpTape.back(), mpTape.front()); // No mana was spent.
}
