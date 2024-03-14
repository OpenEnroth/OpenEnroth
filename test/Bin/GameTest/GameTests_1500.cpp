#include <unordered_set>

#include "Testing/Game/GameTest.h"

#include "Engine/Engine.h"
#include "Engine/mm7_data.h"
#include "Engine/Party.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIPartyCreation.h"

#include "Utility/Exception.h"

// 1500

GAME_TEST(Issues, Issue1503) {
    // Can start new game without 4 skills selected
    test.playTraceFromTestData("issue_1503.mm7", "issue_1503.json");
    EXPECT_EQ(current_screen_type, SCREEN_PARTY_CREATION);
    EXPECT_FALSE(PlayerCreation_Choose4Skills());
}

GAME_TEST(Issues, Issue1510) {
    // Enemies not attacking the party when in melee range
    auto partyHealth = tapes.totalHp();
    auto actorDistTape = actorTapes.custom(2, [](const Actor& a) { return (a.pos - pParty->pos).length(); });
    test.playTraceFromTestData("issue_1510.mm7", "issue_1510.json");
    EXPECT_LT(partyHealth.back(), partyHealth.front());
    EXPECT_LE(actorDistTape.max(), meleeRange);
}

// TODO(pskelton): Rough test idea but need to prove this will catch the issue first
/*
GAME_TEST(Issues, Issue1535a) {
    // Queue some messages then throw to 'fail' test
    auto queueMessageAndThrow = [] {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_ShowGameOverWindow, 0, 0);
        pushSpellOrRangedAttack(SPELL_LIGHT_DAY_OF_THE_GODS, 0, CombinedSkillValue(10, CHARACTER_SKILL_MASTERY_GRANDMASTER), 0, 0);
        throw Exception("Failing test on purpose");
        };
    EXPECT_ANY_THROW(test.playTraceFromTestData("issue_1510.mm7", "issue_1510.json", queueMessageAndThrow););
}
GAME_TEST(Issues, Issue1535b) {
    // Queued messages should be cleared if test fails
    EXPECT_FALSE(engine->_messageQueue.get()->haveMessages());
    EXPECT_FALSE(CastSpellInfoHelpers::hasQueuedSpells());
}
*/

