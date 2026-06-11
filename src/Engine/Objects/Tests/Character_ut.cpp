#include "Testing/Game/GameTest.h"

#include "Engine/Objects/Character.h"

// Regression test for issue #2502 - dead/petrified/eradicated party members should be excluded from
// triggered status checks (e.g. the Haste Pedestal on Emerald Island).

GAME_TEST(Character, IsAliveByDefault) {
    // A freshly constructed character has no conditions and is considered alive.
    Character character;
    EXPECT_TRUE(character.isAlive());
}

GAME_TEST(Character, IsAliveWithMinorConditions) {
    // Transient conditions like weakness, sleep or curse do not make the character "not alive".
    Character character;

    character.conditions.set(CONDITION_WEAK, Time::fromTicks(1));
    EXPECT_TRUE(character.isAlive());

    character.conditions.set(CONDITION_CURSED, Time::fromTicks(1));
    EXPECT_TRUE(character.isAlive());

    character.conditions.set(CONDITION_SLEEP, Time::fromTicks(1));
    EXPECT_TRUE(character.isAlive());

    character.conditions.set(CONDITION_PARALYZED, Time::fromTicks(1));
    EXPECT_TRUE(character.isAlive());

    character.conditions.set(CONDITION_UNCONSCIOUS, Time::fromTicks(1));
    EXPECT_TRUE(character.isAlive()); // Unconscious is recoverable, party members can still be revived by rest.
}

GAME_TEST(Character, IsAliveExcludesDead) {
    // Dead characters are not alive, even if they also carry a stacked minor condition like weakness.
    Character character;

    character.conditions.set(CONDITION_DEAD, Time::fromTicks(1));
    EXPECT_FALSE(character.isAlive());

    // The bug from issue #2502: a character that became weak before dying still carries CONDITION_WEAK
    // alongside CONDITION_DEAD because resting does not clear conditions on dead members.
    character.conditions.set(CONDITION_WEAK, Time::fromTicks(1));
    EXPECT_FALSE(character.isAlive());
}

GAME_TEST(Character, IsAliveExcludesPetrified) {
    Character character;
    character.conditions.set(CONDITION_PETRIFIED, Time::fromTicks(1));
    EXPECT_FALSE(character.isAlive());
}

GAME_TEST(Character, IsAliveExcludesEradicated) {
    Character character;
    character.conditions.set(CONDITION_ERADICATED, Time::fromTicks(1));
    EXPECT_FALSE(character.isAlive());
}

GAME_TEST(Character, IsAliveZombie) {
    // Zombie characters are functional party members, they remain alive for the purposes of status checks.
    Character character;
    character.conditions.set(CONDITION_ZOMBIE, Time::fromTicks(1));
    EXPECT_TRUE(character.isAlive());
}

GAME_TEST(Character, CompareWeakSkipsDeadCharacter) {
    // Per-character predicate that the pedestal event loop relies on. A character carrying both
    // CONDITION_DEAD and CONDITION_WEAK still reports as weak through CompareVariable - the bug fix
    // skips such characters at the event-loop iteration level (see EvtInterpreter::EVENT_Compare).
    Character character;
    character.conditions.set(CONDITION_WEAK, Time::fromTicks(1));
    character.conditions.set(CONDITION_DEAD, Time::fromTicks(1));

    EXPECT_TRUE(character.CompareVariable(VAR_Weak, 0));
    EXPECT_FALSE(character.isAlive());
}
