#include "Testing/Unit/UnitTest.h"

#include "Engine/GameType.h"
#include "Engine/Seasons.h"

// MM8's Dagger Wound Island is a permanently-tropical setting - a fresh MM8 new game
// starts in month 0 ("winter" under MM7's month->season mapping), yet the retail exe applies
// neither the grass->snow terrain reskin nor the matching decoration-sprite reskin (flowers
// nulled out, some trees swapped) there. Both `tilesetForSeason` and `spriteIdForSeason` must be
// no-ops for MM8, same MM7-only mechanic, same gate.

namespace {

class SeasonsTest : public testing::Test {
 protected:
    void TearDown() override {
        setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
    }
};

} // namespace

TEST_F(SeasonsTest, TilesetForSeasonIsNoOpForMm8) {
    setGameType(GAME_TYPE_MM8);

    EXPECT_EQ(tilesetForSeason(TILESET_GRASS, 0), TILESET_GRASS); // Winter would reskin to snow under MM7.
    EXPECT_EQ(tilesetForSeason(TILESET_GRASS, 9), TILESET_GRASS); // Autumn would reskin to dirt under MM7.
}

TEST_F(SeasonsTest, SpriteIdForSeasonIsNoOpForMm8) {
    setGameType(GAME_TYPE_MM8);

    // Winter under MM7 would null out these flower sprites and swap these tree sprites.
    EXPECT_EQ(spriteIdForSeason(539, 0), 539);
    EXPECT_EQ(spriteIdForSeason(541, 0), 541);
    EXPECT_EQ(spriteIdForSeason(547, 0), 547);
    EXPECT_EQ(spriteIdForSeason(548, 0), 548);
    EXPECT_EQ(spriteIdForSeason(483, 0), 483);
    EXPECT_EQ(spriteIdForSeason(486, 0), 486);
    EXPECT_EQ(spriteIdForSeason(492, 0), 492);

    // Autumn under MM7 would do the same with a different tree offset.
    EXPECT_EQ(spriteIdForSeason(539, 9), 539);
    EXPECT_EQ(spriteIdForSeason(483, 9), 483);
}

TEST_F(SeasonsTest, SpriteIdForSeasonStillAppliesForMm7) {
    setGameType(GAME_TYPE_MM7);

    EXPECT_EQ(spriteIdForSeason(539, 0), 0); // flower01 nulled out in winter.
    EXPECT_EQ(spriteIdForSeason(483, 0), 485); // tree01 swapped to its winter variant.
}
