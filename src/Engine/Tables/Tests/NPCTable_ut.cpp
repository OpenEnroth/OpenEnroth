#include <memory>
#include <string>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Tables/NPCTable.h"
#include "Library/Logger/BufferLogSink.h"
#include "Library/Logger/Logger.h"
#include "Utility/Memory/Blob.h"

// MM8's npcdata.txt uses the exact same column layout as MM7's, and its NPC/hireling roster runs up to
// row 527, past MM7's 500-slot cap.
// `pOriginalNPCData`/`pNPCData` (and the `npcdata.bin` save-game format they round-trip through) have
// 528 slots to fit all of MM8's real rows - this test checks that rows up to the boundary parse correctly, and
// the still-further filler placeholder rows (528+) are dropped without crashing. Row ids and numeric
// values below follow MM8's npcdata.txt (in `data/EnglishT.lod`), with placeholder names and the notes column trimmed.

namespace {
Blob mm8NpcDataFixture() {
    std::string text =
        "NPC Data (Special)\t\t\tGroup\t\t\tCurrent\tProfession\tGreet\tJoin\tEvent\tEvent\tEvent\tEvent\tEvent\tEvent\t\r\n"
        "#\tName\tPic\tA\tB\tC\t2D Location\t 1 - 76\t#\tY / N\t# A\t# B\t# C\t# D\t# E\t# F\tNotes\r\n";

    for (int i = 1; i <= 528; i++) {
        switch (i) {
        case 1:
            text += "1\tTest Npc One\t833\t0\t0\t0\t173\t0\t3\t0\t7\t8\t9\t0\t0\t0\r\n";
            break;
        case 500:
            text += "500\tTest Npc FiveHundred\t1257\t0\t0\t0\t468\t0\t70\t0\t687\t0\t0\t0\t0\t0\r\n";
            break;
        case 501:
            text += "501\tTest Npc FiveHundredOne\t1080\t0\t0\t0\t0\t0\t0\t0\t704\t0\t0\t0\t0\t0\r\n";
            break;
        case 527:
            text += "527\tTest Npc FiveHundredTwentySeven\t1044\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\r\n";
            break;
        case 528: // First filler placeholder row - real data ends at 527, this must still be dropped.
            text += "528\tFiller\t884\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\r\n";
            break;
        default:
            text += std::to_string(i) + "\t\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\r\n";
            break;
        }
    }

    return Blob::fromString(std::move(text));
}
} // namespace

UNIT_TEST(NPCTable, InitializeNPCDataMm8RealRowsFitFillerRowsAreSkipped) {
    // `InitializeNPCData` logs a warning for every row past the array bound, which needs a logger.
    BufferLogSink sink;
    Logger testLogger(LOG_WARNING, &sink);

    auto stats = std::make_unique<NPCStats>();
    stats->pOriginalNPCData.fill(NPCData());
    stats->InitializeNPCData(mm8NpcDataFixture()); // Shouldn't throw - filler row 528 is past our array bound.

    EXPECT_EQ(stats->pOriginalNPCData[1].name, "Test Npc One");
    EXPECT_EQ(stats->pOriginalNPCData[1].portraitId, 833u);

    EXPECT_EQ(stats->pOriginalNPCData[500].name, "Test Npc FiveHundred");
    EXPECT_EQ(stats->pOriginalNPCData[500].portraitId, 1257u);

    // Rows 501-527 are past MM7's 500-slot cap and fit in the 528-slot table.
    EXPECT_EQ(stats->pOriginalNPCData[501].name, "Test Npc FiveHundredOne");
    EXPECT_EQ(stats->pOriginalNPCData[501].portraitId, 1080u);

    EXPECT_EQ(stats->pOriginalNPCData[527].name, "Test Npc FiveHundredTwentySeven"); // Last real (non-filler) row.
    EXPECT_EQ(stats->pOriginalNPCData[527].portraitId, 1044u);

    // Total count is capped at the new array size (528, including the unused slot 0): all 527 real MM8 rows
    // fit, and the first filler placeholder row (528) got dropped by the bounds check above.
    EXPECT_EQ(stats->uNumNewNPCs, 528);
}

// MM8's `EnglishT.lod` doesn't ship `npcnames.txt` at all - the name pool it backs in MM7
// (`NPCStats::pNPCNames`, consumed by `InitializeAdditionalNPCs`/`Character::RandomizeName` via
// `RandomEngine::randomSample`, which asserts on an empty pool) comes from `pcnames.txt`:
// 8 race/class name-pool columns instead of MM7's flat male|female pair, plus a constant "*end" spreadsheet
// artifact column, and a final sentinel row whose first column is "*end". The rows below mirror the shape of
// MM8's `pcnames.txt` (trimmed to a handful of rows).
UNIT_TEST(NPCTable, InitializeNPCNamesMm8ParsesPerRaceColumnsIntoSexPools) {
    std::string text =
        "Knight/Cleric Male\tKnight/Cleric Female\tDark Elf Pesant Male\tDark Elf Pesant Female\tTroll/Minotaur\t"
        "Vampire/Necro Female\tVampire/Necro Male\tDragon\tTo make excel work right\r\n"
        "NameA\tNameB\tNameC\tNameB\tNameD\tNameE\tNameC\tNameC\t*end\r\n"
        "NameF\tNameG\tNameH\tNameG\tNameI\tNameJ\tNameH\tNameH\t*end\r\n"
        "NameK\t\t\t\t\t\t\t\t*end\r\n" // Later row: only the longest-running column (0) still has an entry.
        "*end\t\t\t\t\t\t\t\t*end\r\n"; // Sentinel end-of-table row - must not be parsed as data.

    auto stats = std::make_unique<NPCStats>();
    stats->InitializeNPCNamesMM8(Blob::fromString(std::move(text)));

    // Male pool = Knight/Cleric Male + Dark Elf Peasant Male + Vampire/Necro Male + unisex Troll/Minotaur & Dragon.
    EXPECT_THAT(stats->pNPCNames[SEX_MALE], testing::ElementsAre(
        "NameA", "NameC", "NameD", "NameC", "NameC",
        "NameF", "NameH", "NameI", "NameH", "NameH",
        "NameK"));
    // Female pool = Knight/Cleric Female + Dark Elf Peasant Female + Vampire/Necro Female + unisex Troll/Minotaur & Dragon.
    EXPECT_THAT(stats->pNPCNames[SEX_FEMALE], testing::ElementsAre(
        "NameB", "NameB", "NameD", "NameE", "NameC",
        "NameG", "NameG", "NameI", "NameJ", "NameH"));
}

// MM8's `pcnames.txt` doesn't leave a column's cell empty the moment its name list
// runs out - the row where the list ends carries a literal "*end" in that column (same string as the sentinel/
// artifact column), and only rows *after* that go empty. A per-column "*end" from any of the other 7
// columns used to be parsed as a name, and a character could be named "*end". The rows below mirror that shape
// (columns ending on different rows, each with one "*end" row before going empty).
UNIT_TEST(NPCTable, InitializeNPCNamesMm8IgnoresPerColumnEndMarkers) {
    std::string text =
        "Knight/Cleric Male\tKnight/Cleric Female\tDark Elf Pesant Male\tDark Elf Pesant Female\tTroll/Minotaur\t"
        "Vampire/Necro Female\tVampire/Necro Male\tDragon\tTo make excel work right\r\n"
        "NameL\tNameM\t*end\tNameM\tNameN\tNameO\t*end\t*end\t*end\r\n"
        "NameP\tNameQ\t\tNameQ\t*end\tNameR\t\t\t*end\r\n"
        "NameS\t*end\t\t*end\t\tNameT\t\t\t*end\r\n"
        "NameU\t\t\t\t\t*end\t\t\t*end\r\n"
        "*end\t\t\t\t\t\t\t\t*end\r\n"; // Sentinel end-of-table row.

    auto stats = std::make_unique<NPCStats>();
    stats->InitializeNPCNamesMM8(Blob::fromString(std::move(text)));

    EXPECT_THAT(stats->pNPCNames[SEX_MALE], testing::Not(testing::Contains("*end")));
    EXPECT_THAT(stats->pNPCNames[SEX_FEMALE], testing::Not(testing::Contains("*end")));

    EXPECT_THAT(stats->pNPCNames[SEX_MALE], testing::ElementsAre(
        "NameL", "NameN",
        "NameP",
        "NameS",
        "NameU"));
    EXPECT_THAT(stats->pNPCNames[SEX_FEMALE], testing::ElementsAre(
        "NameM", "NameM", "NameN", "NameO",
        "NameQ", "NameQ", "NameR",
        "NameT"));
}
