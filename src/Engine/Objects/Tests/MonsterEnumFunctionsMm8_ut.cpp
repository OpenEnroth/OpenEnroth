#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/GameType.h"
#include "Engine/Objects/MonsterEnumFunctions.h"

// MM8's monsters.txt (data/EnglishT.lod)
// keeps MM7's 3-tier-per-species chunking, but completely reshuffles which species sits at which `MonsterId`,
// so `monsterTypeForMonsterId`'s MM7 row-position arithmetic doesn't carry over. Ids below are pinned against
// real roster rows: id 1 is "Lizardman Peasant" (no MM7 equivalent), ids 28-33/58-63/91-93/106-108 are the
// Ogre Peasant/Ogre Warrior/Troll/Troll Peasant/Cyclops/Ogre Mage kinds, ids 73-84 are the four classic
// elementals, and ids 103-105/115-117/127-129/130-132/157-159 are Efreeti/Thunderbird/Salamander/Phoenix/
// Juggernaut respectively (see `Monsters_ut.cpp` for the same roster's row 1 & row 198 pinned).

UNIT_TEST(MonsterEnumFunctionsMm8, MonsterTypeMatchesRealMm7Equivalents) {
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(46)), MONSTER_TYPE_CLERIC_SUN); // Acolyte of the Sun
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(58)), MONSTER_TYPE_TROLL);       // Troll Clubman
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(70)), MONSTER_TYPE_DRAGON);      // Dragon
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(73)), MONSTER_TYPE_ELEMENTAL_FIRE);  // Lesser Fire Elemental
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(76)), MONSTER_TYPE_ELEMENTAL_WATER); // Lesser Water Elemental
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(79)), MONSTER_TYPE_ELEMENTAL_EARTH); // Lesser Earth Elemental
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(82)), MONSTER_TYPE_ELEMENTAL_AIR);   // Lesser Air Elemental
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(121)), MONSTER_TYPE_WYVERN);     // Wyvern
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(160)), MONSTER_TYPE_GOG);        // Smoke Gog
}

UNIT_TEST(MonsterEnumFunctionsMm8, MonsterTypeFallsBackToUnusedRatForMm8OnlySpecies) {
    // Real MM8 species with no MM7 `MonsterType` counterpart, including both required-for-slaying kinds
    // (Ogre, Cyclops) and others (Lizardman, Couatl).
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(1)), MONSTER_TYPE_UNUSED_RAT);   // Lizardman Peasant
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(7)), MONSTER_TYPE_UNUSED_RAT);   // Young Couatl
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(28)), MONSTER_TYPE_UNUSED_RAT);  // Ogre Peasant
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(91)), MONSTER_TYPE_UNUSED_RAT);  // Cyclops
}

UNIT_TEST(MonsterEnumFunctionsMm8, SupertypeCoversFullOgreKindIdRanges) {
    // Ogre Peasant (28-30), Ogre Warrior (31-33), Troll (58-60), Troll Peasant (61-63), Cyclops (91-93),
    // Ogre Mage (106-108) - both range endpoints for each, pinning against real roster rows.
    for (int id : {28, 30, 31, 33, 58, 60, 61, 63, 91, 93, 106, 108})
        EXPECT_EQ(mm8SupertypeForMonsterId(static_cast<MonsterId>(id)), MONSTER_SUPERTYPE_MM8_OGRE) << "id=" << id;

    // Ids immediately outside those ranges must not be classified as ogre-kind.
    EXPECT_NE(mm8SupertypeForMonsterId(static_cast<MonsterId>(27)), MONSTER_SUPERTYPE_MM8_OGRE); // Dark Elven Crusader
    EXPECT_NE(mm8SupertypeForMonsterId(static_cast<MonsterId>(34)), MONSTER_SUPERTYPE_MM8_OGRE); // Animalist (Wererat)
    EXPECT_NE(mm8SupertypeForMonsterId(static_cast<MonsterId>(64)), MONSTER_SUPERTYPE_MM8_OGRE); // Minotaur Guard
    EXPECT_NE(mm8SupertypeForMonsterId(static_cast<MonsterId>(109)), MONSTER_SUPERTYPE_MM8_OGRE); // Unicorn Foal
}

UNIT_TEST(MonsterEnumFunctionsMm8, SupertypeCoversFullElementalKindIdRanges) {
    // Fire/Water/Earth/Air Elemental (73-84), Efreeti (103-105), Thunderbird (115-117), Salamander (127-129),
    // Phoenix (130-132), Juggernaut (157-159) - both range endpoints for each.
    for (int id : {73, 84, 103, 105, 115, 117, 127, 129, 130, 132, 157, 159})
        EXPECT_EQ(mm8SupertypeForMonsterId(static_cast<MonsterId>(id)), MONSTER_SUPERTYPE_MM8_ELEMENTAL) << "id=" << id;

    // Ids immediately outside those ranges must not be classified as elemental-kind.
    EXPECT_NE(mm8SupertypeForMonsterId(static_cast<MonsterId>(72)), MONSTER_SUPERTYPE_MM8_ELEMENTAL);  // Great Wyrm
    EXPECT_NE(mm8SupertypeForMonsterId(static_cast<MonsterId>(85)), MONSTER_SUPERTYPE_MM8_ELEMENTAL);  // Dire Wolf Yearling
    EXPECT_NE(mm8SupertypeForMonsterId(static_cast<MonsterId>(133)), MONSTER_SUPERTYPE_MM8_ELEMENTAL); // Triton
}

UNIT_TEST(MonsterEnumFunctionsMm8, SupertypeIsNoneForOutOfScopeSpecies) {
    EXPECT_EQ(mm8SupertypeForMonsterId(static_cast<MonsterId>(1)), MONSTER_SUPERTYPE_NONE); // Lizardman Peasant
    EXPECT_EQ(mm8SupertypeForMonsterId(static_cast<MonsterId>(52)), MONSTER_SUPERTYPE_NONE); // Vampire Minion
}

// `mm8MonsterTypeForMonsterId`'s MM7-semantic value is NOT safe to use for hostility - most of
// it falls outside `hostile.txt`'s real 0-66 column range (see HostilityTable.cpp) and silently
// reads the all-friendly default, or lands on a different, unrelated species' real column.
// Every hostility-mapped id must resolve inside the real column range.
UNIT_TEST(MonsterEnumFunctionsMm8, HostilityMonsterTypeStaysWithinRealHostileTxtColumnRange) {
    constexpr int kRealHostileTxtColumnCount = 67; // hostile.txt has real data in columns 0-66 only.

    for (int id : {46, 52, 55, 58, 64, 70, 73, 76, 79, 82, 121, 160, // Species with an MM7 counterpart
                   10, 13, 16, 181, 184, 187}) {                    // Pirate entries
        MonsterType hostilityType = mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(id));
        EXPECT_LT(std::to_underlying(hostilityType), kRealHostileTxtColumnCount) << "id=" << id;
    }
}

// Pins each entry's hostile.txt column (data/EnglishT.lod).
UNIT_TEST(MonsterEnumFunctionsMm8, HostilityMonsterTypeMatchesRealHostileTxtColumns) {
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(46)), static_cast<MonsterType>(16));  // Cleric of the Sun
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(52)), static_cast<MonsterType>(18));  // Vampire Minion
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(55)), static_cast<MonsterType>(19));  // Dark Path Journeyman (Necromancer)
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(58)), static_cast<MonsterType>(20));  // Troll Clubman
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(64)), static_cast<MonsterType>(22));  // Minotaur Guard
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(70)), static_cast<MonsterType>(24));  // Dragon
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(73)), static_cast<MonsterType>(25));  // Lesser Fire Elemental
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(76)), static_cast<MonsterType>(26));  // Lesser Water Elemental
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(79)), static_cast<MonsterType>(27));  // Lesser Earth Elemental
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(82)), static_cast<MonsterType>(28));  // Lesser Air Elemental
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(121)), static_cast<MonsterType>(41)); // Wyvern
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(160)), static_cast<MonsterType>(54)); // Smoke Gog

    // Pirate entries.
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(10)), static_cast<MonsterType>(4));
    EXPECT_EQ(mm8HostilityMonsterTypeForMonsterId(static_cast<MonsterId>(181)), static_cast<MonsterType>(61));
}

// Pirates have no genuine MM7 `MonsterType` counterpart, so the *semantic* classifier (item drop / sex /
// race / bounty / arena / peasant) must fall back to the shared placeholder, same as any other MM8-only
// species - even though the *hostility* classifier gives them a real, distinct column (tested above).
UNIT_TEST(MonsterEnumFunctionsMm8, SemanticMonsterTypeFallsBackToUnusedRatForPirates) {
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(10)), MONSTER_TYPE_UNUSED_RAT);  // Pirate Warrior Male
    EXPECT_EQ(mm8MonsterTypeForMonsterId(static_cast<MonsterId>(181)), MONSTER_TYPE_UNUSED_RAT);  // Wimpy Pirate Warrior Male
}

// `isPeasant(MonsterId)` used to route through `mm8MonsterTypeForMonsterId`'s MM7-semantic
// value, which maps every MM8-only peasant species to the shared `MONSTER_TYPE_UNUSED_RAT` placeholder -
// none of which is in `isPeasant(MonsterType)`'s dwarf/elf/human/goblin ranges, so these actors never
// got promoted to an individual `NPCData` record (`Engine::_461103_load_level_sub`) and clicking them
// fell through to the no-npcId catch-phrase/branchless-dialogue path instead of the dedicated NPC
// dialogue screen. Ids pinned against MM8 monsters.txt rows.
UNIT_TEST(MonsterEnumFunctionsMm8, IsPeasantRecognizesMm8OnlyPeasantSpecies) {
    setGameType(GAME_TYPE_MM8);
    for (int id : {1, 2, 3,             // Lizardman Peasant/Farmer/Villager
                   19, 20, 21, 22, 23, 24, // Dark Elf Peasant (Male A/B/C, Female A/B/C)
                   28, 29, 30,          // Ogre Peasant
                   49, 50, 51,          // Peasant (Vampire Female Peasant)
                   61, 62, 63,          // Troll Peasant
                   67, 68, 69,          // Minotaur Peasant
                   166, 167, 168}) {    // Human Peasant
        EXPECT_TRUE(isPeasant(static_cast<MonsterId>(id))) << "id=" << id;
    }

    // Ids immediately outside those ranges, and a non-peasant "warrior" tier sharing the same species,
    // must not be classified as peasants.
    for (int id : {4, 5, 6,             // Lizardman Soldier/Sergeant/Captain (warrior tier)
                   18, 25,              // just outside the Dark Elf Peasant range
                   27, 31,              // Dark Elven Crusader / Ogre Brawler
                   58, 64})             // Troll Clubman / Minotaur Guard (warrior tiers)
        EXPECT_FALSE(isPeasant(static_cast<MonsterId>(id))) << "id=" << id;

    setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
}

UNIT_TEST(MonsterEnumFunctionsMm8, DispatchersRouteThroughGameTypeAndMm7StaysByteIdentical) {
    ASSERT_EQ(gameType(), GAME_TYPE_MM7); // Precondition: default gametype, so this test cleans up after itself.

    // MM7 path is untouched: row-position arithmetic, same as before this change.
    EXPECT_EQ(monsterTypeForMonsterId(MONSTER_ANGEL_A), MONSTER_TYPE_ANGEL);
    EXPECT_EQ(monsterTypeForMonsterId(MONSTER_TROLL_A), MONSTER_TYPE_TROLL);
    EXPECT_EQ(supertypeForMonsterId(MONSTER_DEVIL_A), MONSTER_SUPERTYPE_KREEGAN);
    // MM7's own row-position arithmetic IS also its hostile.txt column, so the two dispatchers agree.
    EXPECT_EQ(hostilityMonsterTypeForMonsterId(MONSTER_TROLL_A), monsterTypeForMonsterId(MONSTER_TROLL_A));

    setGameType(GAME_TYPE_MM8);
    EXPECT_EQ(monsterTypeForMonsterId(static_cast<MonsterId>(58)), MONSTER_TYPE_TROLL);  // Troll Clubman, real MM8 data
    EXPECT_EQ(monsterTypeForMonsterId(static_cast<MonsterId>(1)), MONSTER_TYPE_UNUSED_RAT); // Lizardman Peasant
    EXPECT_EQ(supertypeForMonsterId(static_cast<MonsterId>(31)), MONSTER_SUPERTYPE_MM8_OGRE); // Ogre Brawler
    EXPECT_EQ(supertypeForMonsterId(static_cast<MonsterId>(73)), MONSTER_SUPERTYPE_MM8_ELEMENTAL); // Lesser Fire Elemental
    // On MM8 the two dispatchers diverge - Troll Clubman's real hostile.txt column (20) differs from its
    // MM7-semantic ordinal (84, MONSTER_TYPE_TROLL).
    EXPECT_EQ(hostilityMonsterTypeForMonsterId(static_cast<MonsterId>(58)), static_cast<MonsterType>(20));
    EXPECT_NE(hostilityMonsterTypeForMonsterId(static_cast<MonsterId>(58)), monsterTypeForMonsterId(static_cast<MonsterId>(58)));
    setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
}
