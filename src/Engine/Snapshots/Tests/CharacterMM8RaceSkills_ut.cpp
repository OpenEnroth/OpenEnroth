#include "Testing/Unit/UnitTest.h"

#include "Engine/GameType.h"
#include "Engine/Objects/Character.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Snapshots/EntitySnapshots.h"

static CombinedSkillValue withMastery(int level, Mastery mastery) {
    CombinedSkillValue result = CombinedSkillValue::novice(level);
    for (int i = std::to_underlying(MASTERY_NOVICE) + 1; i <= std::to_underlying(mastery); i++)
        result = CombinedSkillValue::increaseMastery(result, Mastery(i));
    return result;
}

static Character roundTrip(const Character &character) {
    Character_MM7 snapshotted;
    snapshot(character, &snapshotted);
    Character result;
    reconstruct(snapshotted, &result, tags::context(0));
    return result;
}

UNIT_TEST(CharacterMM8RaceSkills, RoundTripKeepsLevelAndMastery) {
    GameType oldType = gameType();
    setGameType(GAME_TYPE_MM8);

    Character character;
    character.pActiveSkills[SKILL_DARK_ELF] = withMastery(5, MASTERY_EXPERT);
    character.pActiveSkills[SKILL_VAMPIRE] = withMastery(7, MASTERY_MASTER);
    character.pActiveSkills[SKILL_DRAGON] = withMastery(9, MASTERY_GRANDMASTER);
    character.pActiveSkills[SKILL_REGENERATION] = CombinedSkillValue::novice(3);
    character.pActiveSkills[SKILL_LEARNING] = CombinedSkillValue::novice(2);

    Character loaded = roundTrip(character);
    setGameType(oldType);

    for (Skill skill : {SKILL_DARK_ELF, SKILL_VAMPIRE, SKILL_DRAGON, SKILL_REGENERATION, SKILL_LEARNING}) {
        EXPECT_EQ(loaded.pActiveSkills[skill].level(), character.pActiveSkills[skill].level());
        EXPECT_EQ(loaded.pActiveSkills[skill].mastery(), character.pActiveSkills[skill].mastery());
    }
    EXPECT_EQ(loaded.pActiveSkills[SKILL_DRAGON].level(), 9);
    EXPECT_EQ(loaded.pActiveSkills[SKILL_DRAGON].mastery(), MASTERY_GRANDMASTER);
}

UNIT_TEST(CharacterMM8RaceSkills, MaxMasteryPerClass) {
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_MM8_DARK_ELF, SKILL_DARK_ELF), MASTERY_MASTER);
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_MM8_PATRIARCH, SKILL_DARK_ELF), MASTERY_GRANDMASTER);
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_MM8_VAMPIRE, SKILL_VAMPIRE), MASTERY_MASTER);
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_MM8_GREAT_WYRM, SKILL_DRAGON), MASTERY_GRANDMASTER);
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_MM8_TROLL, SKILL_REGENERATION), MASTERY_MASTER);
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_MM8_KNIGHT, SKILL_REGENERATION), MASTERY_NONE);
    EXPECT_EQ(skillMaxMasteryForClass(CLASS_KNIGHT, SKILL_DRAGON), MASTERY_NONE);
}
