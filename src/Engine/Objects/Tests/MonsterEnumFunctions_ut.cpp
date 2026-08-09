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

GAME_TEST(MonsterEnumFunctions, MonsterIdForMonsterTypeAndTier) {
    // Spot checks.
    EXPECT_EQ(monsterIdForMonsterTypeAndTier(MONSTER_TYPE_ANGEL, MONSTER_TIER_A), MONSTER_ANGEL_A);
    EXPECT_EQ(monsterIdForMonsterTypeAndTier(MONSTER_TYPE_ANGEL, MONSTER_TIER_C), MONSTER_ANGEL_C);
    EXPECT_EQ(monsterIdForMonsterTypeAndTier(MONSTER_TYPE_GHOST, MONSTER_TIER_B), MONSTER_GHOST_B);

    // Splitting any monster id into type and tier and reassembling must give the same id back.
    for (MonsterId id : allMonsters())
        EXPECT_EQ(monsterIdForMonsterTypeAndTier(monsterTypeForMonsterId(id), monsterTierForMonsterId(id)), id);
}
