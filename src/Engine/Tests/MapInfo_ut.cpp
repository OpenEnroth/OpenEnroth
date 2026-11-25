#include <string>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/GameType.h"
#include "Engine/Tables/MapTable.h"
#include "Utility/Memory/Blob.h"

// Rows in the layout of MM8's mapstats.txt (data/EnglishT.lod).
static Blob mm8MapStatsFixture() {
    std::string text =
        "Header line 1 (ignored)\r\n"
        "Header line 2 (ignored)\r\n"
        "Header line 3 (ignored)\r\n"
        "1\tDagger Wound Island\tOut01.odm\t0\t0\t0\t672\t0\t1\t0\t0\t0\t10\t0\t100\t0\tLizardmen Warrior\tLizardmen Warrior\t1\t 2-5\t"
        "Wimpy Pirate Warrior Male\tWimpy Pirate Warrior Male\t1\t 1-3\tCouatl (winged snake)\tCouatl (winged snake)\t1\t 1-3\t4\tFOREST\t0\t\t\tx\r\n"
        "13\tRegna\tOut13.odm\t0\t0\t10\t672\t0\t10\t10\t5\t4\t10\t34\t33\t33\tPirate Warrior Male\tPirate Warrior Male\t3\t 1-3\t"
        "Pirate Crossbowman (Female)\tPirate Crossbowman (Female)\t1\t 1-3\tPirate Mage Male\tPirate Mage Male\t3\t 1-3\t10\tPLAINS\t0\t\t\r\n"
        "61\tNWC\tD50.blv\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t1\t 1-3\t0\t0\t3\t 1-3\t0\t0\t1\t 1-3\t6\tCAVE\t0\t\t\r\n"
        "62\t\t\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\tGhoul\tGhoul\t2\t 1-3\tBat\tBat\t3\t 1-3\t0\t0\t1\t 1-3\t7\tCAVE\t0\r\n";
    return Blob::fromString(std::move(text));
}

UNIT_TEST(MapInfo, InitializeMm8RemapsRawIdsAndSkipsBlankFillerRows) {
    setGameType(GAME_TYPE_MM8);

    MapTable mapStats;
    mapStats.Initialize(mm8MapStatsFixture()); // Shouldn't throw/OOB - row 62 doesn't fit any real MapId.

    EXPECT_EQ(mapStats.pInfos[MAP_MM8_DAGGER_WOUND_ISLAND].name, "Dagger Wound Island");
    EXPECT_EQ(mapStats.pInfos[MAP_MM8_DAGGER_WOUND_ISLAND].fileName, "out01.odm");
    EXPECT_EQ(mapStats.pInfos[MAP_MM8_REGNA].name, "Regna");
    EXPECT_EQ(mapStats.pInfos[MAP_MM8_NWC].name, "NWC");
    EXPECT_EQ(mapStats.pInfos[MAP_MM8_NWC].fileName, "d50.blv");

    setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
}

UNIT_TEST(MapInfo, InitializeMm7UsesRawIdsDirectly) {
    // Regression check: MM7 map ids parse into their own (unshifted) `MapId` untouched by the MM8 remap.
    ASSERT_EQ(gameType(), GAME_TYPE_MM7);

    std::string text =
        "Header line 1 (ignored)\r\n"
        "Header line 2 (ignored)\r\n"
        "Header line 3 (ignored)\r\n"
        "1\tEmerald Island\tout00.odm\t0\t0\t0\t672\t0\t1\t0\t0\t0\t10\t0\t100\t0\tName\tName\t1\t 2-5\tName\tName\t1\t 1-3\tName\tName\t1\t 1-3\t4\tFOREST\t0\r\n";

    MapTable mapStats;
    mapStats.Initialize(Blob::view(text));

    EXPECT_EQ(mapStats.pInfos[MAP_EMERALD_ISLAND].name, "Emerald Island");
    EXPECT_EQ(mapStats.pInfos[MAP_EMERALD_ISLAND].fileName, "out00.odm");
}
