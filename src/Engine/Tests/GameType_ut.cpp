#include "Testing/Unit/UnitTest.h"

#include "Engine/GameType.h"

// engine-wide runtime MM8 game-version seam.

UNIT_TEST(GameType, DefaultsToMm7) {
    // `gameType()` must default to MM7 so that every existing MM7 code path is unaffected until boot explicitly
    // detects otherwise.
    EXPECT_EQ(gameType(), GAME_TYPE_MM7);
}

UNIT_TEST(GameType, SetGameTypeIsQueryableEngineWide) {
    setGameType(GAME_TYPE_MM8);
    EXPECT_EQ(gameType(), GAME_TYPE_MM8);

    setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
    EXPECT_EQ(gameType(), GAME_TYPE_MM7);
}
