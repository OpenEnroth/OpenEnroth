#include <unordered_set>
#include <ranges>

#include "Testing/Game/GameTest.h"

#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Engine.h"
#include "Engine/mm7_data.h"
#include "Engine/Party.h"
#include "Engine/Objects/SpriteObject.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIStatusBar.h"

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

GAME_TEST(Issues, Issue1515) {
    // No dispel magic sound
    auto soundsTape = tapes.sounds();
    test.playTraceFromTestData("issue_1515.mm7", "issue_1515.json");
    EXPECT_TRUE(soundsTape.flattened().contains(SOUND_RechargeItem)); // dispel magic
}

GAME_TEST(Issues, Issue1519) {
    // Baby Dragon NPC description missing
    auto messageBoxesTape = tapes.messageBoxes();
    auto messageBoxesBody = tapes.allGUIWindowsText();
    test.playTraceFromTestData("issue_1519.mm7", "issue_1519.json");
    // message box body text was displayed.
    auto flatMessageBoxes = messageBoxesTape.flattened();
    auto flatMessageBoxesBody = messageBoxesBody.flattened();
    EXPECT_GT(flatMessageBoxes.size(), 0);
    EXPECT_GT(flatMessageBoxesBody.filtered([](const auto &s) { return s.starts_with("The Baby Dragon"); }).size(), 0);
}

GAME_TEST(Issues, Issue1524) {
    // More enemy spells without sound
    auto soundsTape = tapes.sounds();
    test.playTraceFromTestData("issue_1524.mm7", "issue_1524.json");
    EXPECT_TRUE(soundsTape.flattened().contains(SOUND_Sacrifice2)); // pain reflection sound
}

GAME_TEST(Issues, Issue1532) {
    // Can cast too many firespikes
    game.startNewGame();
    engine->config->debug.AllMagic.setValue(true);
    engine->config->debug.NoActors.setValue(true);
    auto statusTape = tapes.statusBar();

    // keep casting till the spell fails
    while (engine->_statusBar->get() != "Spell failed") {
        if (pParty->pCharacters[0].CanAct())
            pushSpellOrRangedAttack(SPELL_FIRE_FIRE_SPIKE, 0, CombinedSkillValue(10, CHARACTER_SKILL_MASTERY_GRANDMASTER), 0, 0);
        game.tick(1);
    }

    // count should be 9 for gm
    EXPECT_EQ(9, std::ranges::count_if(pSpriteObjects, [](const SpriteObject& object) {
        return object.uType != SPRITE_NULL &&
            object.uObjectDescID != 0 && // exploded fire spikes have no DescID
            object.uSpellID == SPELL_FIRE_FIRE_SPIKE &&
            object.spell_caster_pid == Pid(OBJECT_Character, 0);
        })
    );
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
