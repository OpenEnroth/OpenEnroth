#include "Testing/Game/GameTest.h"

#include "Engine/Random/Random.h"
#include "Engine/Spells/Spells.h"

#include "Library/Random/MersenneTwisterRandomEngine.h"

// Tests for spell damage formulas from MM7 v1.1. Issue: #2055.

GAME_TEST(Spells, SpiritLashDamageFormula) {
    // Spirit Lash does 10 + 2-8 damage per SP (baseDamage=10, bonusSkillDamage=8, dice faces=7).
    // Formula: baseDamage + spellLevel + randomDice(spellLevel, bonusSkillDamage - 1).
    // At spellLevel=5: 10 + 5 + randomDice(5, 7) in [5, 35] => damage in [20, 50].
    MersenneTwisterRandomEngine engine;
    RandomEngine *savedGrng = grng;
    grng = &engine;
    for (int i = 0; i < 100; i++) {
        int damage = CalcSpellDamage(SPELL_SPIRIT_SPIRIT_LASH, 5, MASTERY_NOVICE, 0);
        EXPECT_GE(damage, 20);
        EXPECT_LE(damage, 50);
    }
    grng = savedGrng;
}

GAME_TEST(Spells, DeathBlossomGrandmasterDamage) {
    // Death Blossom at GM does 20 + 2 per SP (no randomness).
    // Formula: baseDamage + spellLevel * 2, where baseDamage=20.
    EXPECT_EQ(CalcSpellDamage(SPELL_EARTH_DEATH_BLOSSOM, 5, MASTERY_GRANDMASTER, 0), 30);   // 20 + 5*2
    EXPECT_EQ(CalcSpellDamage(SPELL_EARTH_DEATH_BLOSSOM, 10, MASTERY_GRANDMASTER, 0), 40);  // 20 + 10*2
    EXPECT_EQ(CalcSpellDamage(SPELL_EARTH_DEATH_BLOSSOM, 1, MASTERY_GRANDMASTER, 0), 22);   // 20 + 1*2
}

GAME_TEST(Spells, MeteorShowerDamageFormula) {
    // Meteor Shower does 1-8 per SP (baseDamage=0, bonusSkillDamage=8).
    // Formula: baseDamage + randomDice(spellLevel, bonusSkillDamage).
    // At spellLevel=5: randomDice(5, 8) in [5, 40] => damage in [5, 40].
    MersenneTwisterRandomEngine engine;
    RandomEngine *savedGrng = grng;
    grng = &engine;
    for (int i = 0; i < 100; i++) {
        int damage = CalcSpellDamage(SPELL_FIRE_METEOR_SHOWER, 5, MASTERY_MASTER, 0);
        EXPECT_GE(damage, 5);
        EXPECT_LE(damage, 40);
    }
    grng = savedGrng;
}

GAME_TEST(Spells, PsychicShockDamageFormula) {
    // Psychic Shock does 12 + 1-12 per SP (baseDamage=12, bonusSkillDamage=12).
    // Formula: baseDamage + randomDice(spellLevel, bonusSkillDamage).
    // At spellLevel=5: 12 + randomDice(5, 12) in [5, 60] => damage in [17, 72].
    MersenneTwisterRandomEngine engine;
    RandomEngine *savedGrng = grng;
    grng = &engine;
    for (int i = 0; i < 100; i++) {
        int damage = CalcSpellDamage(SPELL_MIND_PSYCHIC_SHOCK, 5, MASTERY_NOVICE, 0);
        EXPECT_GE(damage, 17);
        EXPECT_LE(damage, 72);
    }
    grng = savedGrng;
}
