#include "Testing/Game/GameTest.h"

#include "Engine/Objects/MonsterEnumFunctions.h"

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
