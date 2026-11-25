#include "Testing/Unit/UnitTest.h"

#include "Engine/Objects/CharacterMM8Abilities.h"

#include "Library/Random/SequentialRandomEngine.h"

// Vampire Lifedrain at skill 1: 3 + 1-3 damage, a third heals the Vampire (MM8 manual, Vampire Abilities).
UNIT_TEST(CharacterMM8Abilities, VampireLifedrainDamageIsWithinManualRange) {
    SequentialRandomEngine rng;
    for (int i = 0; i < 100; i++) {
        int damage = mm8VampireLifedrainDamage(rng);
        EXPECT_GE(damage, 3 + 1);
        EXPECT_LE(damage, 3 + 3);
    }
}

UNIT_TEST(CharacterMM8Abilities, VampireLifedrainHealIsAThirdOfDamageDealt) {
    EXPECT_EQ(mm8VampireLifedrainHeal(0), 0);
    EXPECT_EQ(mm8VampireLifedrainHeal(3), 1);
    EXPECT_EQ(mm8VampireLifedrainHeal(6), 2);
    EXPECT_EQ(mm8VampireLifedrainHeal(9), 3);
    EXPECT_EQ(mm8VampireLifedrainHeal(5), 1); // Rounds down, same as the game's other integer-division drains.
}

// Troll Regeneration at skill 1 (Trolls start with the skill per "Class Starting Skills.txt"). The rate is
// `CHARACTER_BUFF_REGENERATION`'s 5 hp per point of power, because the manual and skilldes.txt give no
// per-skill-point rate for the skill itself.
UNIT_TEST(CharacterMM8Abilities, TrollRegenerationHpPerTickMatchesUntrainedRegenerationRate) {
    EXPECT_EQ(mm8TrollRegenerationHpPerTick(), 5);
}

// MM8 race innate starting resistances, from the MM8 manual's "Starting Resistance"
// table (p.20): Dark Elf = Fire/Air/Water/Earth, Minotaur = Spirit/Mind/Body, Troll = Water/Body, all +5.
UNIT_TEST(CharacterMM8Abilities, RaceStartingResistanceBonusMatchesManualTable) {
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_DARK_ELF, ATTRIBUTE_RESIST_FIRE), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_DARK_ELF, ATTRIBUTE_RESIST_AIR), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_DARK_ELF, ATTRIBUTE_RESIST_WATER), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_DARK_ELF, ATTRIBUTE_RESIST_EARTH), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_DARK_ELF, ATTRIBUTE_RESIST_MIND), 0);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_DARK_ELF, ATTRIBUTE_RESIST_BODY), 0);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_DARK_ELF, ATTRIBUTE_RESIST_SPIRIT), 0);

    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_MINOTAUR, ATTRIBUTE_RESIST_SPIRIT), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_MINOTAUR, ATTRIBUTE_RESIST_MIND), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_MINOTAUR, ATTRIBUTE_RESIST_BODY), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_MINOTAUR, ATTRIBUTE_RESIST_FIRE), 0);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_MINOTAUR, ATTRIBUTE_RESIST_WATER), 0);

    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_TROLL, ATTRIBUTE_RESIST_WATER), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_TROLL, ATTRIBUTE_RESIST_BODY), 5);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_TROLL, ATTRIBUTE_RESIST_SPIRIT), 0);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_TROLL, ATTRIBUTE_RESIST_FIRE), 0);

    // Vampires, Dragons and MM7 races get no bonus from this function.
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_MM8_VAMPIRE, ATTRIBUTE_RESIST_MIND), 0);
    EXPECT_EQ(mm8RaceStartingResistanceBonus(RACE_HUMAN, ATTRIBUTE_RESIST_BODY), 0);
}

// Dragon Flame Blast at skill 1: 10 + 1-10 damage (MM8 manual, Dragon Abilities).
UNIT_TEST(CharacterMM8Abilities, DragonBreathDamageIsWithinManualRange) {
    SequentialRandomEngine rng;
    for (int i = 0; i < 100; i++) {
        int damage = mm8DragonBreathDamage(rng);
        EXPECT_GE(damage, 10 + 1);
        EXPECT_LE(damage, 10 + 10);
    }
}

// Levitate's duration per mastery, from spells.txt row 112.
UNIT_TEST(CharacterMM8Abilities, VampireLevitateDurationByMastery) {
    EXPECT_EQ(mm8VampireLevitateDuration(MASTERY_EXPERT, 3), Duration::fromMinutes(30));
    EXPECT_EQ(mm8VampireLevitateDuration(MASTERY_MASTER, 7), Duration::fromHours(1));
    EXPECT_EQ(mm8VampireLevitateDuration(MASTERY_GRANDMASTER, 12), Duration::fromHours(3));
}
