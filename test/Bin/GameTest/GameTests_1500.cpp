#include <unordered_set>

#include "Testing/Game/GameTest.h"

#include "Engine/Engine.h"
#include "Engine/mm7_data.h"
#include "Engine/Party.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIPartyCreation.h"

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
    auto actorDistTape = actorTapes.custom(2, [](const Actor& a) { return (a.pos.toFloat() - pParty->pos).length(); });
    test.playTraceFromTestData("issue_1510.mm7", "issue_1510.json");
    EXPECT_LT(partyHealth.back(), partyHealth.front());
    EXPECT_LE(actorDistTape.max(), meleeRange);
}

GAME_TEST(Issues, Issue1535) {
    // Queued messages stay in the event queue and roll over between tests.
    game.startNewGame();
    engine->config->debug.AllMagic.setValue(true);

    game.pressGuiButton("Game_CastSpell");
    game.tick(1);
    game.pressGuiButton("SpellBook_Spell8"); // 8 is meteor shower.
    game.tick(1);
    game.pressGuiButton("SpellBook_Spell8"); // Confirm.
    game.tick(1);

    // Should have put the spell cast message to queue.
    UIMessageType message = UIMSG_Invalid;
    int spell = 0;
    engine->_messageQueue->peekMessage(&message, &spell, nullptr);
    EXPECT_EQ(message, UIMSG_CastSpellFromBook);
    EXPECT_EQ(spell, std::to_underlying(SPELL_FIRE_METEOR_SHOWER));

    // Then we start a new test.
    test.prepareForNextTest(); // This call used to assert.
    EXPECT_FALSE(engine->_messageQueue->haveMessages()); // Please don't roll over the messages between tests!
}
