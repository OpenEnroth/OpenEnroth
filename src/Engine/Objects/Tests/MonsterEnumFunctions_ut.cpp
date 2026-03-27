#include "Testing/Game/GameTest.h"

#include "Engine/Objects/MonsterEnumFunctions.h"

// MM7 v1.1 fix: "No invisible monsters will spawn in the Arena."
// Manticore was in the game files but had no sprites in v1.0, making it invisible in the arena.
GAME_TEST(MonsterEnumFunctions, ArenaDoesNotContainManticore) {
    auto arena = allArenaMonsters();
    for (MonsterId monsterId : arena) {
        EXPECT_NE(monsterTypeForMonsterId(monsterId), MONSTER_TYPE_MANTICORE)
            << "Manticore should not appear in the Arena (v1.1 fix: was invisible due to missing sprites).";
    }
}

GAME_TEST(MonsterEnumFunctions, MonsterTierForMonsterId) {
    // Angel A/B/C map to tiers A/B/C.
    EXPECT_EQ(monsterTierForMonsterId(MONSTER_ANGEL_A), MONSTER_TIER_A);
    EXPECT_EQ(monsterTierForMonsterId(MONSTER_ANGEL_B), MONSTER_TIER_B);
    EXPECT_EQ(monsterTierForMonsterId(MONSTER_ANGEL_C), MONSTER_TIER_C);

    // Spot check some other monsters.
    EXPECT_EQ(monsterTierForMonsterId(MONSTER_GHOST_A), MONSTER_TIER_A);
    EXPECT_EQ(monsterTierForMonsterId(MONSTER_GHOST_B), MONSTER_TIER_B);
    EXPECT_EQ(monsterTierForMonsterId(MONSTER_GHOST_C), MONSTER_TIER_C);
}
