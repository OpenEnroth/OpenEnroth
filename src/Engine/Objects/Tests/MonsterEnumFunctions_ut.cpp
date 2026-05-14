#include "Testing/Game/GameTest.h"

#include "Engine/Objects/CombinedSkillValue.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Spells/SpellEnums.h"

void parseSpellEntry(std::string_view cell, SpellId &outSpellId, CombinedSkillValue &outMastery);

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

GAME_TEST(MonsterStats, ParseSpellEntry) {
    SpellId spellId = SPELL_NONE;
    CombinedSkillValue mastery = CombinedSkillValue::none();

    // Canonical well-formed cell: spell name, mastery, skill split across three comma-separated fields.
    parseSpellEntry("\"Fire Bolt,N,3\"", spellId, mastery);
    EXPECT_EQ(spellId, SPELL_FIRE_FIRE_BOLT);
    EXPECT_EQ(mastery.level(), 3);
    EXPECT_EQ(mastery.mastery(), MASTERY_NOVICE);

    // Issue #2507: Efreet's spell1 in monsters.txt is the malformed `"Lightning Bolt,M10"` — the comma
    // between mastery and skill is missing. The parser must recover by splitting `M10` into mastery `M`
    // and skill `10` so the lightning bolt deals its intended 10d8 instead of 0d8.
    parseSpellEntry("\"Lightning Bolt,M10\"", spellId, mastery);
    EXPECT_EQ(spellId, SPELL_AIR_LIGHTNING_BOLT);
    EXPECT_EQ(mastery.level(), 10);
    EXPECT_EQ(mastery.mastery(), MASTERY_MASTER);

    // All four mastery letters survive the same recovery path.
    parseSpellEntry("\"Fire Bolt,G7\"", spellId, mastery);
    EXPECT_EQ(mastery.level(), 7);
    EXPECT_EQ(mastery.mastery(), MASTERY_GRANDMASTER);
    parseSpellEntry("\"Fire Bolt,E4\"", spellId, mastery);
    EXPECT_EQ(mastery.level(), 4);
    EXPECT_EQ(mastery.mastery(), MASTERY_EXPERT);
    parseSpellEntry("\"Fire Bolt,N5\"", spellId, mastery);
    EXPECT_EQ(mastery.level(), 5);
    EXPECT_EQ(mastery.mastery(), MASTERY_NOVICE);

    // Empty cell yields no spell and no mastery.
    parseSpellEntry("", spellId, mastery);
    EXPECT_EQ(spellId, SPELL_NONE);
    EXPECT_EQ(mastery, CombinedSkillValue::none());

    // A bare mastery letter with no skill digits leaves mastery unset (matches original behavior).
    parseSpellEntry("\"Fire Bolt,M\"", spellId, mastery);
    EXPECT_EQ(spellId, SPELL_FIRE_FIRE_BOLT);
    EXPECT_EQ(mastery, CombinedSkillValue::none());
}
