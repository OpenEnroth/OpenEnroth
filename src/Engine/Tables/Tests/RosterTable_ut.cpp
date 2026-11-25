#include <string>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Tables/RosterTable.h"

#include "Utility/Memory/Blob.h"

namespace {

// roster.txt's header, trimmed to the columns the parser reads.
// Header row 1 is a lone tab (category labels, unused) so `.skip("")` doesn't drop it as blank.
constexpr const char *kMm8RosterHeader = "\t\r\n#\tName\tClass\tBirthdate\tPicture\tVoice\tExperience\tLevel\tMight\tIntelligence\tPersonality\tEndurance\tSpeed\tAccuracy\tLuck\tFire resist"
"\tAir Resist\tWater Resist\tEarth resist\tMind Resist\tBody resist\tSkillpoints\tStaff\t\tSword\t\tDagger\t\tAxe\t\tSpear\t\tBow\t\tMace\t\tBlaster\t\tShield\t\tLeather\t\tChain"
"\t\tPlate\t\tFire\t\tAir\t\tWater\t\tEarth\t\tSpirit\t\tMind\t\tBody\t\tLight\t\tDark\t\tDarkElf\t\tVampire\t\tDragon\t\tID Item\t\tMerchant\t\tRepair\t\tB Building"
"\t\tMeditate\t\tPerception\t\tRegeneration\t\tDisarm Trap\t\tDodging\t\tUnarmed\t\tIdent Mon\t\tArmsmaster\t\tStealing\t\tAlchemy\t\tLearning\t\tQuestBit\tEquipment"
"\tBlurb\tEnd\r\n";

// Row 0: Placeholder (the champion slot, not a recruitable companion).
constexpr const char *kMm8RosterRow0 = "0\tPlaceholder\tknight\t1150\t1\t1\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t20\t\t0\t"
"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t404\t \tblurb\tX\r\n";

// Row 4: Test Companion Four, troll, level 15, real stat spread, Staff=Expert4.
constexpr const char *kMm8RosterRow4 = "4\tTest Companion Four\ttroll\t1150\t22\t22\t100500\t15\t30\t7\t7\t24\t18\t20\t16\t0\t0\t0\t0\t0\t0\t20\tE\t4\t"
"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t404\t2078\tblurb\tX\r\n";

// Row 34: Test Companion ThirtyFour, cleric, level 15, Staff=Novice4. QuestBit=434 (distinct from row 4's 404,
// to catch a copy-paste column-index bug the two rows sharing "404" wouldn't have caught).
constexpr const char *kMm8RosterRow34 = "34\tTest Companion ThirtyFour\tcleric\t1150\t6\t6\t105000\t15\t15\t15\t22\t14\t13\t12\t13\t0\t0\t0\t0\t0\t0\t20\tB\t4\t"
"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t434\t2068\tblurb\tX\r\n";

// Name, Class, QuestBit and Equipment cells. Stats and skills stay blank, Level and
// Picture are numeric placeholders, and the 93 tabs put QuestBit at column 100 of this fixture's header.
std::string fixtureRow(int id, const char *name, const char *className, int questBit, const char *equipment, int experience = 0) {
    std::string row = std::to_string(id) + "\t" + name + "\t" + className + "\t1150\t1\t1\t" + std::to_string(experience) + "\t5";
    row += std::string(93, '\t');
    return row + std::to_string(questBit) + "\t" + equipment + "\tblurb\tX\r\n";
}

Blob mm8RosterFixture() {
    std::string text = kMm8RosterHeader;
    text += kMm8RosterRow0;
    text += fixtureRow(1, "Test Member One", "necromancer", 401, "79", 10000);
    text += fixtureRow(2, "Test Member Two", "cleric", 402, "66");
    text += kMm8RosterRow4;
    text += fixtureRow(7, "Test Member Seven", "vampire", 407, "21");
    text += kMm8RosterRow34;
    return Blob::fromString(std::move(text));
}

} // namespace

UNIT_TEST(RosterTable, InitializeRosterMm8ParsesRealColumnLayout) {
    initializeRosterMm8(mm8RosterFixture());

    ASSERT_GT(rosterMm8.size(), 34u);
    EXPECT_EQ(rosterMm8[4].name, "Test Companion Four");
    EXPECT_EQ(rosterMm8[4].className, "troll");
    EXPECT_EQ(rosterMm8[4].pictureId, 22);
    EXPECT_EQ(rosterMm8[4].level, 15);
    EXPECT_EQ(rosterMm8[4].experience, 100500);
    EXPECT_EQ(rosterMm8[34].experience, 105000);
    EXPECT_EQ(rosterMm8[1].experience, 10000);
    EXPECT_EQ(rosterMm8[4].stats[ATTRIBUTE_MIGHT], 30);
    EXPECT_EQ(rosterMm8[4].stats[ATTRIBUTE_SPEED], 18);
    EXPECT_EQ(rosterMm8[4].stats[ATTRIBUTE_ACCURACY], 20);
    EXPECT_EQ(rosterMm8[4].skills[SKILL_STAFF], CombinedSkillValue(4, MASTERY_EXPERT));
    EXPECT_EQ(rosterMm8[4].questBit, 404);

    EXPECT_EQ(rosterMm8[34].name, "Test Companion ThirtyFour");
    EXPECT_EQ(rosterMm8[34].className, "cleric");
    EXPECT_EQ(rosterMm8[34].pictureId, 6);
    EXPECT_EQ(rosterMm8[34].skills[SKILL_STAFF], CombinedSkillValue(4, MASTERY_NOVICE));
    EXPECT_EQ(rosterMm8[34].questBit, 434);
}

UNIT_TEST(RosterTable, InitializeRosterMm8ParsesEquipmentColumn) {
    // Cell values are item id + 1000 * enchantment level; the parser keeps the item id.
    initializeRosterMm8(mm8RosterFixture());

    EXPECT_EQ(rosterMm8[1].equipmentItemId, static_cast<ItemId>(79));
    EXPECT_EQ(rosterMm8[2].equipmentItemId, static_cast<ItemId>(66));
    EXPECT_EQ(rosterMm8[7].equipmentItemId, static_cast<ItemId>(21));
    EXPECT_EQ(rosterMm8[4].equipmentItemId, static_cast<ItemId>(78));
    EXPECT_EQ(rosterMm8[0].equipmentItemId, ITEM_NULL);
}

UNIT_TEST(RosterTable, InitializeRosterMm8EmptyBlobLeavesTableEmpty) {
    initializeRosterMm8(Blob());
    EXPECT_TRUE(rosterMm8.empty()); // MM7 (and any other non-MM8 install) doesn't ship roster.txt.
}

// `characterFromRosterRow()` itself (which calls `Character::ChangeClass()`, and through it
// `GetMaxHealth()`/`GetMaxMana()`) needs a running engine - `pParty`, item/skill bonus lookups,
// etc are all globals `ChangeClass()` reads unconditionally, not something a plain UNIT_TEST
// can fake. A game test covers it.

UNIT_TEST(RosterTable, InitializeRosterMm8MatchesSkillAndSpellColumnsByWholeHeaderCell) {
    // "Fire resist" and "Body resist" precede the skills, and each school name appears again for the spell know-counts.
    initializeRosterMm8(Blob::fromString(
        "\t\r\n#\tName\tClass\tPicture\tLevel\tFire resist\tBody resist\tFire\t\tBody\t\tFire\tBody\tQuestBit\tEquipment\tBlurb\tEnd\r\n"
        "1\tTest\tcleric\t1\t5\t0\t0\tB\t2\tE\t3\t2\t3\t401\t0\tblurb\tX\r\n"));

    ASSERT_EQ(rosterMm8.size(), 2u);
    EXPECT_EQ(rosterMm8[1].skills[SKILL_FIRE], CombinedSkillValue(2, MASTERY_NOVICE));
    EXPECT_EQ(rosterMm8[1].skills[SKILL_BODY], CombinedSkillValue(3, MASTERY_EXPERT));
    EXPECT_EQ(rosterMm8[1].knownSpellCounts[MAGIC_SCHOOL_FIRE], 2);
    EXPECT_EQ(rosterMm8[1].knownSpellCounts[MAGIC_SCHOOL_BODY], 3);
    EXPECT_EQ(rosterMm8[1].knownSpellCounts[MAGIC_SCHOOL_AIR], 0);
}

UNIT_TEST(RosterTable, InitializeRosterMm8ReadsSkillpointsAndBaseResistances) {
    initializeRosterMm8(Blob::fromString(
        "\t\r\n#\tName\tClass\tPicture\tLevel\tFire resist\tAir Resist\tWater Resist\tEarth resist\tMind Resist\tBody resist\tSkillpoints\tFire\t\tQuestBit\tEquipment\tBlurb\tEnd\r\n"
        "1\tTest\tcleric\t1\t5\t3\t4\t5\t6\t7\t8\t16\tB\t2\t402\t0\tblurb\tX\r\n"));

    ASSERT_EQ(rosterMm8.size(), 2u);
    EXPECT_EQ(rosterMm8[1].baseResistances[ATTRIBUTE_RESIST_FIRE], 3);
    EXPECT_EQ(rosterMm8[1].baseResistances[ATTRIBUTE_RESIST_AIR], 4);
    EXPECT_EQ(rosterMm8[1].baseResistances[ATTRIBUTE_RESIST_WATER], 5);
    EXPECT_EQ(rosterMm8[1].baseResistances[ATTRIBUTE_RESIST_EARTH], 6);
    EXPECT_EQ(rosterMm8[1].baseResistances[ATTRIBUTE_RESIST_MIND], 7);
    EXPECT_EQ(rosterMm8[1].baseResistances[ATTRIBUTE_RESIST_BODY], 8);
    EXPECT_EQ(rosterMm8[1].freeSkillPoints, 16);
    EXPECT_EQ(rosterMm8[1].skills[SKILL_FIRE], CombinedSkillValue(2, MASTERY_NOVICE));
}
