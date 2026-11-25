#include "CharacterMM8Abilities.h"

#include "Library/Random/RandomEngine.h"

int mm8VampireLifedrainDamage(RandomEngine &rng) {
    return 3 + rng.randomDice(1, 3);
}

int mm8VampireLifedrainHeal(int lifedrainDamage) {
    return lifedrainDamage / 3;
}

int mm8TrollRegenerationHpPerTick() {
    return 5;
}

int mm8RaceStartingResistanceBonus(Race race, Attribute resistance) {
    switch (race) {
        case RACE_MM8_DARK_ELF:
            switch (resistance) {
                case ATTRIBUTE_RESIST_FIRE:
                case ATTRIBUTE_RESIST_AIR:
                case ATTRIBUTE_RESIST_WATER:
                case ATTRIBUTE_RESIST_EARTH:
                    return 5;
                default:
                    return 0;
            }
        case RACE_MM8_MINOTAUR:
            switch (resistance) {
                case ATTRIBUTE_RESIST_SPIRIT:
                case ATTRIBUTE_RESIST_MIND:
                case ATTRIBUTE_RESIST_BODY:
                    return 5;
                default:
                    return 0;
            }
        case RACE_MM8_TROLL:
            switch (resistance) {
                case ATTRIBUTE_RESIST_WATER:
                case ATTRIBUTE_RESIST_BODY:
                    return 5;
                default:
                    return 0;
            }
        default:
            return 0;
    }
}

int mm8DragonBreathDamage(RandomEngine &rng) {
    return 10 + rng.randomDice(1, 10);
}

Duration mm8VampireLevitateDuration(Mastery mastery, int skillLevel) {
    switch (mastery) {
        case MASTERY_EXPERT:
            return Duration::fromMinutes(10 * skillLevel);
        case MASTERY_MASTER:
            return Duration::fromHours(1);
        case MASTERY_GRANDMASTER:
            return Duration::fromHours(3);
        default:
            return Duration::fromMinutes(5 * skillLevel); // Novice, same ratio as Water Walk's novice row.
    }
}
