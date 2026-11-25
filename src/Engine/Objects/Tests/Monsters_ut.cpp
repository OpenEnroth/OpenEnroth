#include <memory>
#include <string>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Objects/Monsters.h"
#include "Engine/Spells/SpellEnums.h"
#include "Library/Logger/BufferLogSink.h"
#include "Library/Logger/Logger.h"
#include "Utility/Memory/Blob.h"

// MM8's monsters.txt uses the same column layout, header-line count and row-index range as MM7's (both fit
// within `MONSTER_LAST`). The rows below are in MM8's monsters.txt layout (`data/EnglishT.lod`).

namespace {
Blob mm8MonstersFixture() {
    std::string text =
        "Default Monster Data\t\t\t\t\tStats\t\t\t\t\t\t\t\tMovement and Combat\t\t\t\t\tAttack 1\t\t\tAttack 2\t\t\t    Spell Attack\t\t    Spell Attack 2\t\tResistances\t\t\t\t\t\t\t\t\t\tMisc\r\n"
        "#\tName\tPicture\tLVL\t HP \tAC\t EXP \tTreasure\tQuest\tFly\tMove\tAI Type\tHst\tSpd\tRec\tPref\tBonus\tType\tDamage\tMiss\tAtt%\tType\tDamage\tMiss\tUse%\t\"Spl,Mas,Skil\"\t"
        "Use%\t\"Spl,Mas,Skil\"\tFire\tAir\tWater\tEarth\tMind\tSpirit\tBody\tLight\tDark\tPhys\tSpecial\r\n"
        "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\r\n"
        "\tA\t\t0\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\r\n";

    for (int i = 1; i <= 198; i++) {
        switch (i) {
        case 1:
            text += "1\tLizardman Peasant\tLizardmen Male (Peasant) A\t4\t13\t0\t5\t1d3\t1\tN\tShort\tWimp\t4\t200\t100\t0\t0\tPhys\t3D4+2\t0\t0\t"
                "0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\r\n";
            break;
        // MM8-only monster spell names: "Firebolt" (no space, unlike MM7's "Fire Bolt"),
        // "Poison Spray", "Implosion", "Mass Distortion" via spell 1/2 cells below, "Paralyze" and
        // "Day of the Gods" via spell 1/2 cells on row 198, "Spirit Lash"/"Heal" via row 197.
        case 2:
            text += "2\tSpellTest Firebolt Poison Spray\t\t1\t1\t0\t0\t0\t0\tN\t\t\t0\t0\t0\t\t\t\t\t\t0\t\t\t\t40\t\"Firebolt,M,9\"\t40\t\"Poison Spray,M,9\"\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\r\n";
            break;
        case 3:
            text += "3\tSpellTest Implosion Mass Distortion\t\t1\t1\t0\t0\t0\t0\tN\t\t\t0\t0\t0\t\t\t\t\t\t0\t\t\t\t40\t\"Implosion,M,9\"\t40\t"
                "\"Mass Distortion,M,9\"\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\r\n";
            break;
        case 197:
            text += "197\tSpellTest Spirit Lash Heal\t\t1\t1\t0\t0\t0\t0\tN\t\t\t0\t0\t0\t\t\t\t\t\t0\t\t\t\t40\t\"Spirit Lash,M,9\"\t40\t\"Heal,E,5\"\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\r\n";
            break;
        case 198:
            text += "198\tChaos Overlord\tWimpy Plane Guardian C\t90\t1080\t100\t9000\t30%300D10+L6\t0\tN\tShort\tSuicidal\t2\t200\t70\t0\t0\tPhys\t25D4\t0\t0\t0\t0\t0\t"
                "40\t\"Paralyze,M,9\"\t40\t\"Day of the Gods,M,9\"\t30\t30\t30\t30\t30\t30\t30\t30\t30\t0\t0\r\n";
            break;
        default:
            text += std::to_string(i) + "\t\t\t0\t0\t0\t0\t0\t0\tN\t\t\t0\t0\t0\t\t\t\t\t\t0\t\t\t\t0\t\t0\t\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t\r\n";
            break;
        }
    }

    return Blob::fromString(std::move(text));
}
} // namespace

UNIT_TEST(Monsters, InitializeMm8Roster) {
    // `Initialize` logs a warning for unrecognized spell names, which needs a live logger.
    BufferLogSink sink;
    Logger testLogger(LOG_WARNING, &sink);

    auto stats = std::make_unique<MonsterStats>();
    stats->Initialize(mm8MonstersFixture()); // Shouldn't throw.

    const MonsterInfo &peasant = stats->infos[static_cast<MonsterId>(1)];
    EXPECT_EQ(peasant.name, "Lizardman Peasant");
    EXPECT_EQ(peasant.level, 4);
    EXPECT_EQ(peasant.hp, 13u);
    EXPECT_FALSE(peasant.flying);

    const MonsterInfo &overlord = stats->infos[static_cast<MonsterId>(198)];
    EXPECT_EQ(overlord.name, "Chaos Overlord");
    EXPECT_EQ(overlord.level, 90);
    EXPECT_EQ(overlord.hp, 1080u);
}

// MM8's monsters.txt uses monster spell names not present in MM7's data (e.g. "Firebolt" instead of
// "Fire Bolt"). They used to be dropped to SPELL_NONE with an "Unknown monster spell" warning.
UNIT_TEST(Monsters, InitializeMm8RosterMapsMm8OnlySpellNames) {
    BufferLogSink sink;
    Logger testLogger(LOG_WARNING, &sink);

    auto stats = std::make_unique<MonsterStats>();
    stats->Initialize(mm8MonstersFixture());

    const MonsterInfo &firebolt = stats->infos[static_cast<MonsterId>(2)];
    EXPECT_EQ(firebolt.spell1Id, SPELL_FIRE_FIRE_BOLT);
    EXPECT_EQ(firebolt.spell2Id, SPELL_WATER_POISON_SPRAY);

    const MonsterInfo &implosion = stats->infos[static_cast<MonsterId>(3)];
    EXPECT_EQ(implosion.spell1Id, SPELL_AIR_IMPLOSION);
    EXPECT_EQ(implosion.spell2Id, SPELL_EARTH_MASS_DISTORTION);

    const MonsterInfo &spiritLash = stats->infos[static_cast<MonsterId>(197)];
    EXPECT_EQ(spiritLash.spell1Id, SPELL_SPIRIT_SPIRIT_LASH);
    // MM8's spells.txt names this spell #68 "Heal" even though the engine's enum constant, inherited
    // from MM6/7 naming, is SPELL_BODY_FIRST_AID.
    EXPECT_EQ(spiritLash.spell2Id, SPELL_BODY_FIRST_AID);

    const MonsterInfo &overlord = stats->infos[static_cast<MonsterId>(198)];
    EXPECT_EQ(overlord.spell1Id, SPELL_LIGHT_PARALYZE);
    EXPECT_EQ(overlord.spell2Id, SPELL_LIGHT_DAY_OF_THE_GODS);
}
