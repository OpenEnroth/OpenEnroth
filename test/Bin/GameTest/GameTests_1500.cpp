#include <unordered_set>
#include <ranges>

#include "Testing/Game/GameTest.h"

#include "Engine/Tables/ItemTable.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Engine.h"
#include "Engine/mm7_data.h"
#include "Engine/Party.h"
#include "Engine/Objects/SpriteObject.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIStatusBar.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Events/EventInterpreter.h"

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

GAME_TEST(Issues, Issue1536) {
    // Unable to climb out of the boat in Castle Gloaming
    test.playTraceFromTestData("issue_1536.mm7", "issue_1536.json");
    EXPECT_GT(pParty->pos.z, -2235.0f); // party has escape boat and is on dock
}

GAME_TEST(Issues, Issue1547) {
    // Crash when attacking when no actors are around
    engine->config->debug.NoActors.setValue(true);

    auto actorsTape = actorTapes.totalCount();
    auto activeCharTape = tapes.activeCharacterIndex();
    game.startNewGame();
    test.startTaping();
    game.tick();
    for (int i = 0; i < 4; i++) {
        game.pressKey(PlatformKey::KEY_A); // Attack with each char - this shouldn't crash.
        game.tick();
        game.releaseKey(PlatformKey::KEY_A);
        game.tick();
    }
    test.stopTaping();

    EXPECT_EQ(actorsTape, tape(0));
    EXPECT_EQ(activeCharTape, tape(1, 2, 3, 4, 0)); // All chars attacked.
}

GAME_TEST(Issues, Issue1569) {
    // Armorer offer chain mail skill learning.
    auto screenTape = tapes.screen();
    auto chainTape = charTapes.hasSkill(0, CHARACTER_SKILL_CHAIN);
    auto goldTape = tapes.gold();
    test.playTraceFromTestData("issue_1569.mm7", "issue_1569.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE, SCREEN_GAME)); // Visited the shop.
    EXPECT_EQ(chainTape, tape(false, true)); // Learned chain mail.
    EXPECT_EQ(goldTape.delta(), -500); // And paid for it.
}

GAME_TEST(Issues, Issue1597) {
    // Test that generated amulets with high enough treasure level have enchantment on them
    ItemGen item;
    int attrEnchantmentsNum = 0;
    int specialEnchantmentsNum = 0;
    for (int i = 0; i < 100; i++) {
        pItemTable->generateItem(ITEM_TREASURE_LEVEL_5, RANDOM_ITEM_AMULET, &item);
        if (item.attributeEnchantment)
            attrEnchantmentsNum++;
        if (item.special_enchantment != ITEM_ENCHANTMENT_NULL)
            specialEnchantmentsNum++;
    }
    EXPECT_NE(attrEnchantmentsNum, 0);
    EXPECT_NE(specialEnchantmentsNum, 0);
}

// 1600

GAME_TEST(Issues, Issue1655) {
    // Assertion in CalcSpellDamage failed b/c an actor is trying to cast SPELL_NONE.
    auto stateTape = actorTapes.aiState(73);
    auto expressionsTape = charTapes.expressions();
    test.playTraceFromTestData("issue_1655.mm7", "issue_1655.json");
    EXPECT_EQ(stateTape, tape(AttackingMelee));

    // Check that we have received quite a beating.
    //
    // The bug here was that the blasterguy had SPELL_NONE as one of the special attacks with a probability of 15%. So
    // we need to roll attack enough times for this code path to trigger even upon retracing. 0.85^25 = ~2% chance not
    // to trigger the relevant codepath.
    EXPECT_EQ(pActors[73].monsterInfo.id, MONSTER_BLASTERGUY_C);
    EXPECT_EQ(pActors[73].monsterInfo.spell1Id, SPELL_NONE);
    EXPECT_EQ(pActors[73].monsterInfo.spell1UseChance, 15);
    auto beatingsTape = expressionsTape.filtered([] (const auto &expressions) {
        return expressions.containsAny(CHARACTER_EXPRESSION_DMGRECVD_MINOR, CHARACTER_EXPRESSION_DMGRECVD_MODERATE, CHARACTER_EXPRESSION_DMGRECVD_MAJOR);
    });
    EXPECT_GE(beatingsTape.size(), 25);
}

GAME_TEST(Issues, Issue1657) {
    // Party can be placed at wrong position after canceling indoor transfer and changing map after it
    auto screenTape = tapes.screen();
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_1657.mm7", "issue_1657.json");
    EXPECT_EQ(pParty->pos.toInt(), Vec3i(12552, 800, 193)); // party is back at new game start position
    EXPECT_EQ(mapTape, tape(MAP_ERATHIA, MAP_EMERALD_ISLAND));
    EXPECT_TRUE(screenTape.contains(SCREEN_INPUT_BLV));
}

GAME_TEST(Issues, Issue1665) {
    // Secret Room under the bed in Castle Harmondale Dungeon. Can't enter room
    auto xpos = tapes.custom([]() { return static_cast<int>(pParty->pos.x); });
    auto zpos = tapes.custom([]() { return static_cast<int>(pParty->pos.z); });
    test.playTraceFromTestData("issue_1665.mm7", "issue_1665.json");
    EXPECT_GT(xpos.max(), 2200);
    EXPECT_LT(zpos.min(), -1700); // weve made it into the hole
    EXPECT_LT(xpos.back(), 1500);
    EXPECT_GT(zpos.back(), -1550); // And made it back out
}

GAME_TEST(Issues, Issue1666) {
    // Sound 220 'splash' getting spammed
    auto mapTape = tapes.map();
    auto soundsTape = tapes.sounds();
    test.playTraceFromTestData("issue_1666.mm7", "issue_1666.json");
    EXPECT_EQ(mapTape.size(), 2);
    int count = soundsTape.flattened().filtered([](const auto& sound) { return sound == SOUND_splash; }).size();
    EXPECT_EQ(count, 1); // jump splash at start
}

GAME_TEST(Issues, Issue1671) {
    // Falling from height outdoors onto models doesnt cause damage
    auto health = tapes.totalHp();
    auto expressionTape = charTapes.expression(2);
    auto modelTape = tapes.custom([]() {bool on_water = false; int bmodel_pid = 0;
        float floor_level = ODM_GetFloorLevel(pParty->pos, 0, &on_water, &bmodel_pid, false);
        return bmodel_pid; });
    test.playTraceFromTestData("issue_1671.mm7", "issue_1671.json");
    EXPECT_LT(health.back(), health.front()); // party has taken damage from fall
    EXPECT_TRUE(expressionTape.contains(CHARACTER_EXPRESSION_FEAR));
    EXPECT_NE(modelTape.back(), 0); // landed on a model
}

GAME_TEST(Issues, Issue1673) {
    // Actors can spawn in "NoActor" debug mode
    auto recordOnce = [&] {
        engine->config->debug.WizardEye.setValue(true);
        auto actorsTape = actorTapes.totalCount();
        game.startNewGame();
        test.startTaping();
        game.tick();
        for (int i = 0; i < 5; i++) {
            spawnMonsters(1, 0, 5, pParty->pos + Vec3f(0, 1000, 0), 0, i); // Spawning dragonflies in front of the party.
            game.tick();
        }
        test.stopTaping();
        return actorsTape;
    };

    auto enabledTape = recordOnce();
    test.prepareForNextTest();
    engine->config->debug.NoActors.setValue(true);
    auto disabledTape = recordOnce();

    EXPECT_EQ(enabledTape.delta(), +25); // Monster spawning works.
    EXPECT_EQ(disabledTape, tape(0)); // But not when actors are disabled.
}

GAME_TEST(Issues, Issue1685) {
    // Lich jar item name was "Kolya' jar" / "Nicholas's jar" instead of "Kolya's jar" / "Nicholas' jar"
    game.startNewGame();
    pParty->pCharacters[0].name = "Kolya";
    pParty->pCharacters[1].name = "Nicholas";

    ItemGen jar1;
    jar1.uItemID = ItemId::ITEM_QUEST_LICH_JAR_FULL;
    jar1.uHolderPlayer = 0;

    ItemGen jar2;
    jar2.uItemID = ItemId::ITEM_QUEST_LICH_JAR_FULL;
    jar2.uHolderPlayer = 1;

    game.runGameRoutine([&] {
        // This code needs to be run in game thread b/c AddItem2 is loading textures...
        pParty->pCharacters[0].AddItem2(-1, &jar1);
        pParty->pCharacters[1].AddItem2(-1, &jar2);
    });

    EXPECT_EQ(jar1.GetIdentifiedName(), "Kolya's Jar");
    EXPECT_EQ(jar2.GetIdentifiedName(), "Nicholas' Jar");
}

// 1700

GAME_TEST(Issues, Issue1706) {
    // Collisions - Getting out of bounds in The temple of the Light
    auto ypos = tapes.custom([]() { return static_cast<int>(pParty->pos.y); });
    test.playTraceFromTestData("issue_1706.mm7", "issue_1706.json");
    EXPECT_GT(ypos.min(), -2555); // make sure we dont get past the wall
    EXPECT_LE(ypos.min(), -2550); // but we should get right up against it
}

GAME_TEST(Issues, Issue1708) {
    // Collisions - Paralyzed Angel falling perpetually
    auto zPos = actorTapes.custom(7, [](const Actor &a) { return static_cast<int>(a.pos.z); });
    auto parTape = actorTapes.hasBuff(7, ACTOR_BUFF_PARALYZED);
    test.playTraceFromTestData("issue_1708.mm7", "issue_1708.json");
    EXPECT_EQ(parTape, tape(true, false)); // paralysed
    EXPECT_EQ(zPos.max(), zPos.front()); // highest position at the start
    EXPECT_LT(zPos.size(), 5); // no bobbling around
}

GAME_TEST(Issues, Issue1710) {
    // Fall damage indoors
    auto health = tapes.totalHp();
    auto expressionTape = charTapes.expression(2);
    auto zpos = tapes.custom([]() { return static_cast<int>(pParty->pos.z); });
    auto noFallDamageTape = tapes.config(engine->config->gameplay.NoIndoorFallDamage);
    test.playTraceFromTestData("issue_1710.mm7", "issue_1710.json");
    EXPECT_EQ(noFallDamageTape, tape(false)); // Fall damage was actually possible
    EXPECT_LT(health.back(), health.front()); // party has taken damage from fall
    EXPECT_TRUE(uCurrentlyLoadedLevelType == LEVEL_INDOOR);
    EXPECT_TRUE(expressionTape.contains(CHARACTER_EXPRESSION_FEAR));
    EXPECT_GT(zpos.max(), zpos.min() + 1000);
}

GAME_TEST(Issues, Issue1716) {
    // Status protections not working
    auto specialAttack = tapes.specialAttacks();
    auto pmCountTape = tapes.custom([]() { return pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].power; });
    test.playTraceFromTestData("issue_1716.mm7", "issue_1716.json");
    EXPECT_TRUE(specialAttack.flattened().contains(SPECIAL_ATTACK_PARALYZED)); // Paralysis attacks were made
    int paraCount = std::ranges::count_if(pParty->pCharacters, [](Character& ch) { return ch.IsParalyzed(); });
    EXPECT_EQ(paraCount, 0); // No one ended up paralysed
    EXPECT_LT(pmCountTape.back(), pmCountTape.front()); // PM saved us
}

GAME_TEST(Issues, Issue1717) {
    // Immolation incorrect damage message
    auto statusBar = tapes.statusBar();
    auto immoBuff = tapes.custom([]() { return pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Active(); });
    test.playTraceFromTestData("issue_1717.mm7", "issue_1717.json");
    EXPECT_EQ(immoBuff, tape( false, true ));
    EXPECT_EQ(pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].caster, 4);
    EXPECT_TRUE(statusBar.contains("Immolation deals 77 damage to 2 target(s)"));
}

GAME_TEST(Issues, Issue1724) {
    // Enemies killed by immolation in turn based mode come back alive
    auto statusBar = tapes.statusBar();
    auto partyXP = tapes.totalExperience();
    auto tbState = tapes.turnBasedMode();
    auto zombieActor = tapes.custom([]() {return std::ranges::count_if(pActors, [](const Actor &act) { return (act.currentHP < 1) && act.CanAct(); }); } );
    test.playTraceFromTestData("issue_1724.mm7", "issue_1724.json");

     EXPECT_GT(statusBar.filtered([](const auto &s) { return s.starts_with("Immolation deals"); }).size(), 0);// test for immolation message
    EXPECT_GT(partyXP.back(), partyXP.front());
    EXPECT_EQ(tbState.back(), true);
    EXPECT_EQ(zombieActor.max(), 0);
}

GAME_TEST(Issues, Issue1725) {
    // Finishing Strike the Devils quest on dark path glitches out game menus
    auto screenTape = tapes.screen();
    auto textTape = tapes.allGUIWindowsText();
    auto bit120Tape = tapes.questBit(QBIT_120);
    auto bit123Tape = tapes.questBit(QBIT_123);
    test.playTraceFromTestData("issue_1725.mm7", "issue_1725.json");
    EXPECT_EQ(screenTape.back(), SCREEN_HOUSE); // Make sure we end up back in the throne room
    EXPECT_GT(textTape.flattened().filtered([](const auto &s) { return s.starts_with("THAT WAS AWESOME!"); }).size(), 0);
    EXPECT_TRUE(textTape.flattened().contains("Exit Building")); // And can exit it
    EXPECT_EQ(bit120Tape, tape(false, true));
    EXPECT_EQ(bit123Tape, tape(true, false));
}

GAME_TEST(Issues, Issue1726) {
    // Blaster trainers do not check requirements and crash the game
    auto textTape = tapes.allGUIWindowsText();
    test.playTraceFromTestData("issue_1726.mm7", "issue_1726.json");
    int GMcount = std::ranges::count_if(pParty->pCharacters, [](const Character &ch) { return ch.getActualSkillValue(CHARACTER_SKILL_BLASTER).mastery() == CHARACTER_SKILL_MASTERY_GRANDMASTER; });
    EXPECT_EQ(GMcount, 0); // no one ends up grand master
    EXPECT_GT(textTape.flattened().filtered([](const auto& s) { return s.starts_with("Your skills improve!  If your Skill with the Blaster"); }).size(), 0); // blaster requirements shown
    EXPECT_TRUE(textTape.flattened().contains("You don't meet the requirements, and cannot be taught until you do.")); // but we dont meet them
}
