#include "Testing/Unit/UnitTest.h"

#include "Engine/Objects/CharacterEnumFunctions.h"

// MM8 races & classes.

UNIT_TEST(CharacterEnumFunctions, Mm8StartableRaceSetIsSevenExcludingDragon) {
    int count = 0;
    for (Race race : allMm8StartableRaces()) {
        EXPECT_TRUE(isMm8Race(race));
        EXPECT_NE(race, RACE_MM8_DRAGON);
        ++count;
    }
    EXPECT_EQ(count, 7);

    EXPECT_FALSE(isMm8StartableRace(RACE_MM8_DRAGON));
    EXPECT_TRUE(isMm8Race(RACE_MM8_DRAGON));
}

UNIT_TEST(CharacterEnumFunctions, Mm8AllRacesAreEightIncludingDragon) {
    int count = 0;
    for ([[maybe_unused]] Race race : allMm8Races())
        ++count;
    EXPECT_EQ(count, 8);
}

UNIT_TEST(CharacterEnumFunctions, Mm8RacesDontOverlapMm7Races) {
    // MM7 numbering must be untouched by the MM8 extension.
    EXPECT_FALSE(isMm8Race(RACE_HUMAN));
    EXPECT_FALSE(isMm8Race(RACE_ELF));
    EXPECT_FALSE(isMm8Race(RACE_GOBLIN));
    EXPECT_FALSE(isMm8Race(RACE_DWARF));

    for (Race race : allMm8Races())
        EXPECT_GT(std::to_underlying(race), std::to_underlying(RACE_LAST));
}

UNIT_TEST(CharacterEnumFunctions, Mm8ClassesDontOverlapMm7Classes) {
    EXPECT_FALSE(isMm8Class(CLASS_KNIGHT));
    EXPECT_FALSE(isMm8Class(CLASS_CLERIC));
    EXPECT_FALSE(isMm8Class(CLASS_LICH));

    for (Race race : allMm8Races()) {
        EXPECT_TRUE(isMm8BaseClass(mm8BaseClassForRace(race)));
        EXPECT_FALSE(isMm8PromotedClass(mm8BaseClassForRace(race)));
        EXPECT_TRUE(isMm8PromotedClass(mm8PromotedClassForRace(race)));
        EXPECT_FALSE(isMm8BaseClass(mm8PromotedClassForRace(race)));

        EXPECT_GT(std::to_underlying(mm8BaseClassForRace(race)), std::to_underlying(CLASS_LAST));
        EXPECT_GT(std::to_underlying(mm8PromotedClassForRace(race)), std::to_underlying(CLASS_LAST));
    }
}

UNIT_TEST(CharacterEnumFunctions, Mm8RaceClassRoundTrip) {
    for (Race race : allMm8Races()) {
        EXPECT_EQ(mm8RaceForClass(mm8BaseClassForRace(race)), race);
        EXPECT_EQ(mm8RaceForClass(mm8PromotedClassForRace(race)), race);
    }
}

UNIT_TEST(CharacterEnumFunctions, Mm8PromotionNames) {
    EXPECT_EQ(mm8PromotedClassForRace(RACE_MM8_NECROMANCER), CLASS_MM8_LICH);
    EXPECT_EQ(mm8PromotedClassForRace(RACE_MM8_CLERIC), CLASS_MM8_PRIEST_OF_LIGHT);
    EXPECT_EQ(mm8PromotedClassForRace(RACE_MM8_KNIGHT), CLASS_MM8_CHAMPION);
    EXPECT_EQ(mm8PromotedClassForRace(RACE_MM8_TROLL), CLASS_MM8_WAR_TROLL);
    EXPECT_EQ(mm8PromotedClassForRace(RACE_MM8_MINOTAUR), CLASS_MM8_MINOTAUR_LORD);
    EXPECT_EQ(mm8PromotedClassForRace(RACE_MM8_DARK_ELF), CLASS_MM8_PATRIARCH);
    EXPECT_EQ(mm8PromotedClassForRace(RACE_MM8_VAMPIRE), CLASS_MM8_NOSFERATU);
    EXPECT_EQ(mm8PromotedClassForRace(RACE_MM8_DRAGON), CLASS_MM8_GREAT_WYRM);
}

UNIT_TEST(CharacterEnumFunctions, Mm8StartingSkillAffinity_Necromancer) {
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_NECROMANCER, SKILL_STAFF), SKILL_AFFINITY_PRIMARY);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_NECROMANCER, SKILL_DARK), SKILL_AFFINITY_PRIMARY);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_NECROMANCER, SKILL_FIRE), SKILL_AFFINITY_AVAILABLE);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_NECROMANCER, SKILL_SWORD), SKILL_AFFINITY_DENIED);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_NECROMANCER, SKILL_PLATE), SKILL_AFFINITY_DENIED);
}

UNIT_TEST(CharacterEnumFunctions, Mm8StartingSkillAffinity_Knight) {
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_KNIGHT, SKILL_SWORD), SKILL_AFFINITY_PRIMARY);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_KNIGHT, SKILL_LEATHER), SKILL_AFFINITY_PRIMARY);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_KNIGHT, SKILL_CHAIN), SKILL_AFFINITY_AVAILABLE);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_KNIGHT, SKILL_FIRE), SKILL_AFFINITY_DENIED);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_KNIGHT, SKILL_DARK), SKILL_AFFINITY_DENIED);
}

UNIT_TEST(CharacterEnumFunctions, Mm8StartingSkillAffinity_DarkElf) {
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_DARK_ELF, SKILL_BOW), SKILL_AFFINITY_PRIMARY);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_DARK_ELF, SKILL_TRAP_DISARM), SKILL_AFFINITY_AVAILABLE);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_DARK_ELF, SKILL_ITEM_ID), SKILL_AFFINITY_AVAILABLE);
    EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_DARK_ELF, SKILL_MACE), SKILL_AFFINITY_DENIED);
}

UNIT_TEST(CharacterEnumFunctions, Mm8StartingSkillAffinity_DragonHasNoTrainedSkills) {
    for (Skill skill : allVisibleSkills())
        EXPECT_EQ(mm8StartingSkillAffinity(RACE_MM8_DRAGON, skill), SKILL_AFFINITY_DENIED);
}

UNIT_TEST(CharacterEnumFunctions, Mm8EveryStartableRaceHasAtLeastOnePrimarySkill) {
    for (Race race : allMm8StartableRaces()) {
        bool hasPrimary = false;
        for (Skill skill : allVisibleSkills()) {
            if (mm8StartingSkillAffinity(race, skill) == SKILL_AFFINITY_PRIMARY)
                hasPrimary = true;
        }
        EXPECT_TRUE(hasPrimary) << "Race #" << std::to_underlying(race) << " has no primary starting skill.";
    }
}

// The char-create seam - the startable-set exposed to char-create must match the game type.

UNIT_TEST(CharacterEnumFunctions, StartableRacesForGameTypeMm8IsTheSevenStartableMm8Races) {
    int count = 0;
    for (Race race : startableRacesForGameType(GAME_TYPE_MM8)) {
        EXPECT_TRUE(isMm8StartableRace(race));
        ++count;
    }
    EXPECT_EQ(count, 7);
}

UNIT_TEST(CharacterEnumFunctions, StartableRacesForGameTypeMm7IsTheFourMm7Races) {
    int count = 0;
    for (Race race : startableRacesForGameType(GAME_TYPE_MM7)) {
        EXPECT_FALSE(isMm8Race(race));
        ++count;
    }
    EXPECT_EQ(count, 4);
}

UNIT_TEST(CharacterEnumFunctions, StartableRacesForGameTypeMm6FallsBackToMm7Races) {
    // MM6 isn't a supported game yet, but it must not be confused for MM8 - it should get the same (MM7) set as
    // an unrecognized/ambiguous game type would.
    EXPECT_EQ(startableRacesForGameType(GAME_TYPE_MM6).front(), startableRacesForGameType(GAME_TYPE_MM7).front());
    EXPECT_EQ(startableRacesForGameType(GAME_TYPE_MM6).back(), startableRacesForGameType(GAME_TYPE_MM7).back());
}

UNIT_TEST(CharacterEnumFunctions, Mm8RaceNameCoversEveryMm8Race) {
    for (Race race : allMm8Races())
        EXPECT_FALSE(mm8RaceName(race).empty());
}

// MM8 char-create balance tables ("Class Starting Stats.txt" & "Class HP SP.txt").

UNIT_TEST(CharacterEnumFunctions, Mm8StartingStats_Necromancer) {
    // Necromancer/Cleric/Knight share the uniform 11/25 range across all 7 stats.
    for (Attribute attribute : allStatAttributes()) {
        Mm8StartingStatRange range = mm8StartingStats(CLASS_MM8_NECROMANCER, attribute);
        EXPECT_EQ(range.base, 11);
        EXPECT_EQ(range.max, 25);
        EXPECT_EQ(range.stepBelowBase, 1);
        EXPECT_EQ(range.stepAtOrAboveBase, 1);
    }
}

UNIT_TEST(CharacterEnumFunctions, Mm8StartingStats_Troll) {
    EXPECT_EQ(mm8StartingStats(CLASS_MM8_TROLL, ATTRIBUTE_MIGHT), (Mm8StartingStatRange{14, 35, 2, 2}));
    EXPECT_EQ(mm8StartingStats(CLASS_MM8_TROLL, ATTRIBUTE_INTELLIGENCE), (Mm8StartingStatRange{7, 15, 1, 2}));
    EXPECT_EQ(mm8StartingStats(CLASS_MM8_TROLL, ATTRIBUTE_ACCURACY), (Mm8StartingStatRange{11, 25, 1, 1}));
}

UNIT_TEST(CharacterEnumFunctions, Mm8StartingStats_DarkElf) {
    EXPECT_EQ(mm8StartingStats(CLASS_MM8_DARK_ELF, ATTRIBUTE_ACCURACY), (Mm8StartingStatRange{14, 30, 2, 2}));
    EXPECT_EQ(mm8StartingStats(CLASS_MM8_DARK_ELF, ATTRIBUTE_ENDURANCE), (Mm8StartingStatRange{7, 20, 1, 2}));
}

UNIT_TEST(CharacterEnumFunctions, Mm8StartingStats_Vampire) {
    for (Attribute attribute : allStatAttributes())
        EXPECT_EQ(mm8StartingStats(CLASS_MM8_VAMPIRE, attribute), (Mm8StartingStatRange{11, 30, 1, 1}));
}

UNIT_TEST(CharacterEnumFunctions, Mm8StartingStats_BaseAndPromotedClassMatch) {
    // The table gives identical ranges for a race's base & promoted class.
    for (Race race : allMm8Races()) {
        for (Attribute attribute : allStatAttributes()) {
            EXPECT_EQ(mm8StartingStats(mm8BaseClassForRace(race), attribute),
                      mm8StartingStats(mm8PromotedClassForRace(race), attribute));
        }
    }
}

UNIT_TEST(CharacterEnumFunctions, Mm8BaseHpSpPerLevel_Necromancer) {
    EXPECT_EQ(mm8BaseHpSpPerLevel(CLASS_MM8_NECROMANCER), (Mm8HpSpPerLevel{20, 2, 25, 3, ATTRIBUTE_INTELLIGENCE}));
    EXPECT_EQ(mm8BaseHpSpPerLevel(CLASS_MM8_LICH), (Mm8HpSpPerLevel{20, 3, 25, 5, ATTRIBUTE_INTELLIGENCE}));
}

UNIT_TEST(CharacterEnumFunctions, Mm8BaseHpSpPerLevel_Knight) {
    // Knight/Champion don't cast spells, so they have no SP-scaling attribute.
    EXPECT_EQ(mm8BaseHpSpPerLevel(CLASS_MM8_KNIGHT), (Mm8HpSpPerLevel{35, 5, 0, 0, std::nullopt}));
    EXPECT_EQ(mm8BaseHpSpPerLevel(CLASS_MM8_CHAMPION), (Mm8HpSpPerLevel{35, 8, 0, 0, std::nullopt}));
}

UNIT_TEST(CharacterEnumFunctions, Mm8BaseHpSpPerLevel_Dragon) {
    EXPECT_EQ(mm8BaseHpSpPerLevel(CLASS_MM8_DRAGON), (Mm8HpSpPerLevel{50, 10, 50, 5, ATTRIBUTE_INTELLIGENCE}));
    EXPECT_EQ(mm8BaseHpSpPerLevel(CLASS_MM8_GREAT_WYRM), (Mm8HpSpPerLevel{50, 10, 50, 10, ATTRIBUTE_INTELLIGENCE}));
}

// MM8 race-gated skill/mastery availability rules ("Class Skills.txt").

UNIT_TEST(CharacterEnumFunctions, Mm8SkillMaxMastery_Necromancer) {
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_NECROMANCER, SKILL_STAFF), MASTERY_MASTER);
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_NECROMANCER, SKILL_DARK), MASTERY_MASTER);
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_NECROMANCER, SKILL_SWORD), MASTERY_NONE);
    // Bow isn't a starting skill for Necromancer, but the class can still learn it up to Novice at a trainer.
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_NECROMANCER, SKILL_BOW), MASTERY_NOVICE);
}

UNIT_TEST(CharacterEnumFunctions, Mm8SkillMaxMastery_PromotionRaisesCaps) {
    // Lich's caps must be >= Necromancer's caps for every skill Necromancer can learn (promotion never lowers
    // a mastery cap), and strictly higher for at least the class' primary magic school.
    for (Skill skill : allVisibleSkills()) {
        EXPECT_GE(mm8SkillMaxMastery(CLASS_MM8_LICH, skill), mm8SkillMaxMastery(CLASS_MM8_NECROMANCER, skill));
    }
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_NECROMANCER, SKILL_DARK), MASTERY_MASTER);
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_LICH, SKILL_DARK), MASTERY_GRANDMASTER);
}

UNIT_TEST(CharacterEnumFunctions, Mm8SkillMaxMastery_KnightHasNoMagic) {
    for (Skill skill : allMagicSkills())
        EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_KNIGHT, skill), MASTERY_NONE);
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_KNIGHT, SKILL_SWORD), MASTERY_MASTER);
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_CHAMPION, SKILL_SWORD), MASTERY_GRANDMASTER);
}

UNIT_TEST(CharacterEnumFunctions, Mm8SkillMaxMastery_DragonOnlyLearnsMiscSkills) {
    // Dragons don't train weapon/armor/magic skills - they fight with claws & breath - but they can still
    // improve a handful of "misc" skills.
    for (Skill skill : allWeaponSkills())
        EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_DRAGON, skill), MASTERY_NONE);
    for (Skill skill : allArmorSkills())
        EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_DRAGON, skill), MASTERY_NONE);
    for (Skill skill : allMagicSkills())
        EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_DRAGON, skill), MASTERY_NONE);
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_DRAGON, SKILL_ITEM_ID), MASTERY_MASTER);
    EXPECT_EQ(mm8SkillMaxMastery(CLASS_MM8_GREAT_WYRM, SKILL_ITEM_ID), MASTERY_GRANDMASTER);
}

UNIT_TEST(CharacterEnumFunctions, Mm8SkillMaxMastery_SkillsWithNoMm8TableEntryAreDenied) {
    // These skills aren't part of the MM8 "Class Skills" table at all (hidden MM7 skills, or skills no MM8
    // class ever learns), so they must be denied for every MM8 class.
    for (Class classType : allMm8Classes()) {
        EXPECT_EQ(mm8SkillMaxMastery(classType, SKILL_CLUB), MASTERY_NONE);
        EXPECT_EQ(mm8SkillMaxMastery(classType, SKILL_MISC), MASTERY_NONE);
        EXPECT_EQ(mm8SkillMaxMastery(classType, SKILL_DIPLOMACY), MASTERY_NONE);
        EXPECT_EQ(mm8SkillMaxMastery(classType, SKILL_THIEVERY), MASTERY_NONE);
        EXPECT_EQ(mm8SkillMaxMastery(classType, SKILL_DODGE), MASTERY_NONE);
        EXPECT_EQ(mm8SkillMaxMastery(classType, SKILL_UNARMED), MASTERY_NONE);
        EXPECT_EQ(mm8SkillMaxMastery(classType, SKILL_STEALING), MASTERY_NONE);
        EXPECT_EQ(mm8SkillMaxMastery(classType, SKILL_BLASTER), MASTERY_NONE);
    }
}

UNIT_TEST(CharacterEnumFunctions, Mm8SkillMaxMastery_EveryStartingSkillIsLearnable) {
    // Every skill a freshly-created MM8 character starts with must be at least learnable per the trainer table
    // - the two tables must agree on what's available, even though the trainer table also allows learning
    // skills the character didn't start with.
    for (Race race : allMm8Races()) {
        for (Skill skill : allVisibleSkills()) {
            if (mm8StartingSkillAffinity(race, skill) != SKILL_AFFINITY_DENIED) {
                EXPECT_NE(mm8SkillMaxMastery(mm8BaseClassForRace(race), skill), MASTERY_NONE)
                    << "Race #" << std::to_underlying(race) << " starts with skill #" << std::to_underlying(skill)
                    << " but can't learn it per the trainer table.";
            }
        }
    }
}

UNIT_TEST(CharacterEnumFunctions, SkillMaxMasteryForClass_RoutesMm8ClassesToMm8Table) {
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_MM8_KNIGHT, SKILL_SWORD), mm8SkillMaxMastery(CLASS_MM8_KNIGHT, SKILL_SWORD));
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_MM8_KNIGHT, SKILL_SWORD), MASTERY_MASTER);
}

UNIT_TEST(CharacterEnumFunctions, SkillMaxMasteryForClass_LeavesMm7ClassesUnchanged) {
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_KNIGHT, SKILL_SWORD), MASTERY_MASTER);
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_CHAMPION, SKILL_SWORD), MASTERY_GRANDMASTER);
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_KNIGHT, SKILL_FIRE), MASTERY_NONE);
}

// MM8 promotion quests - class advancement granted on quest completion.

UNIT_TEST(CharacterEnumFunctions, Mm8ClassName_CoversEveryMm8Class) {
    for (Class classType : allMm8Classes())
        EXPECT_FALSE(mm8ClassName(classType).empty());
}

UNIT_TEST(CharacterEnumFunctions, Mm8ClassName_BaseClassMatchesRaceName) {
    for (Race race : allMm8Races())
        EXPECT_EQ(mm8ClassName(mm8BaseClassForRace(race)), mm8RaceName(race));
}

UNIT_TEST(CharacterEnumFunctions, Mm8ClassName_PromotedClassDiffersFromBaseClass) {
    // Unlike the base class, a promotion has its own name, not just its race's name.
    for (Race race : allMm8Races())
        EXPECT_NE(mm8ClassName(mm8PromotedClassForRace(race)), mm8RaceName(race));

    EXPECT_EQ(mm8ClassName(CLASS_MM8_LICH), "Lich");
    EXPECT_EQ(mm8ClassName(CLASS_MM8_PRIEST_OF_LIGHT), "Priest of the Sun");
    EXPECT_EQ(mm8ClassName(CLASS_MM8_CHAMPION), "Champion");
    EXPECT_EQ(mm8ClassName(CLASS_MM8_WAR_TROLL), "War Troll");
    EXPECT_EQ(mm8ClassName(CLASS_MM8_MINOTAUR_LORD), "Minotaur Lord");
    EXPECT_EQ(mm8ClassName(CLASS_MM8_PATRIARCH), "Patriarch");
    EXPECT_EQ(mm8ClassName(CLASS_MM8_NOSFERATU), "Nosferatu");
    EXPECT_EQ(mm8ClassName(CLASS_MM8_GREAT_WYRM), "Great Wyrm");
}

UNIT_TEST(CharacterEnumFunctions, ClassTierForClass_RoutesMm8ClassesToTheTwoTierScheme) {
    for (Race race : allMm8Races()) {
        EXPECT_EQ(classTierForClass(mm8BaseClassForRace(race)), 1);
        EXPECT_EQ(classTierForClass(mm8PromotedClassForRace(race)), 2);
    }
}

UNIT_TEST(CharacterEnumFunctions, ClassTierForClass_LeavesMm7ClassesUnchanged) {
    EXPECT_EQ(classTierForClass(CLASS_KNIGHT), getClassTier(CLASS_KNIGHT));
    EXPECT_EQ(classTierForClass(CLASS_CAVALIER), getClassTier(CLASS_CAVALIER));
    EXPECT_EQ(classTierForClass(CLASS_CHAMPION), getClassTier(CLASS_CHAMPION));
    EXPECT_EQ(classTierForClass(CLASS_BLACK_KNIGHT), getClassTier(CLASS_BLACK_KNIGHT));
}

// Raw ids confirmed against MM8's real `global.evt`: e.g. the Knight-promotion event compares raw id 4
// (Knight) then sets raw id 5 (Champion).
UNIT_TEST(CharacterEnumFunctions, Mm8ClassFromRawId_MatchesRealPromotionQuestEvtValues) {
    EXPECT_EQ(mm8ClassFromRawId(0), CLASS_MM8_NECROMANCER);
    EXPECT_EQ(mm8ClassFromRawId(1), CLASS_MM8_LICH);
    EXPECT_EQ(mm8ClassFromRawId(2), CLASS_MM8_CLERIC);
    EXPECT_EQ(mm8ClassFromRawId(3), CLASS_MM8_PRIEST_OF_LIGHT);
    EXPECT_EQ(mm8ClassFromRawId(4), CLASS_MM8_KNIGHT);
    EXPECT_EQ(mm8ClassFromRawId(5), CLASS_MM8_CHAMPION);
    EXPECT_EQ(mm8ClassFromRawId(6), CLASS_MM8_TROLL);
    EXPECT_EQ(mm8ClassFromRawId(7), CLASS_MM8_WAR_TROLL);
    EXPECT_EQ(mm8ClassFromRawId(8), CLASS_MM8_MINOTAUR);
    EXPECT_EQ(mm8ClassFromRawId(9), CLASS_MM8_MINOTAUR_LORD);
    EXPECT_EQ(mm8ClassFromRawId(10), CLASS_MM8_DARK_ELF);
    EXPECT_EQ(mm8ClassFromRawId(11), CLASS_MM8_PATRIARCH);
    EXPECT_EQ(mm8ClassFromRawId(12), CLASS_MM8_VAMPIRE);
    EXPECT_EQ(mm8ClassFromRawId(13), CLASS_MM8_NOSFERATU);
    EXPECT_EQ(mm8ClassFromRawId(14), CLASS_MM8_DRAGON);
    EXPECT_EQ(mm8ClassFromRawId(15), CLASS_MM8_GREAT_WYRM);
}

UNIT_TEST(CharacterEnumFunctions, Mm8RawIdForClass_IsTheInverseOfMm8ClassFromRawId) {
    for (int rawId = 0; rawId <= 15; ++rawId)
        EXPECT_EQ(mm8RawIdForClass(mm8ClassFromRawId(rawId)), rawId);
}
