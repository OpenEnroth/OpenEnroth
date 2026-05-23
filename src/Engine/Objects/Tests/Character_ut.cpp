#include "Testing/Game/GameTest.h"

#include "Engine/Party.h"
#include "GUI/UI/NPCTopics.h"

namespace {

void expectGrandmasterTrainingAvailable(Class classType, Skill skill) {
    Character &character = pParty->activeCharacter();
    character.classType = classType;
    character.setSkillValue(skill, CombinedSkillValue(10, MASTERY_MASTER));
    pParty->SetGold(100000);

    NPCData teacher;
    teacher.dialogue_1_evt_id = 200 + 3 * std::to_underlying(skill) + 2;
    ASSERT_EQ(handleScriptedNPCTopicSelection(DIALOGUE_SCRIPTED_LINE_1, &teacher),
              DIALOGUE_MASTERY_TEACHER_OFFER);

    npcDialogueOptionString(DIALOGUE_MASTERY_TEACHER_LEARN, &teacher);
    selectSpecialNPCTopicSelection(DIALOGUE_MASTERY_TEACHER_LEARN, &teacher);

    EXPECT_EQ(character.getSkillValue(skill).mastery(), MASTERY_GRANDMASTER);
}

}  // namespace

GAME_TEST(Issues, Issue987) {
    // Light and Dark grandmaster teachers should accept either corresponding final promotion.
    game.startNewGame();
    game.tick(2);

    expectGrandmasterTrainingAvailable(CLASS_ARCHAMGE, SKILL_LIGHT);
    expectGrandmasterTrainingAvailable(CLASS_PRIEST_OF_SUN, SKILL_LIGHT);
    expectGrandmasterTrainingAvailable(CLASS_LICH, SKILL_DARK);
    expectGrandmasterTrainingAvailable(CLASS_PRIEST_OF_MOON, SKILL_DARK);
}
