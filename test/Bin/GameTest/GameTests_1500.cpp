#include <algorithm>
#include <unordered_set>
#include <ranges>
#include <string>
#include <regex>

#include "Testing/Game/GameTest.h"

#include "Engine/Tables/ItemTable.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Engine.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/mm7_data.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Objects/SpriteObject.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIStatusBar.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Evt/EvtInterpreter.h"
#include "Engine/Objects/Chest.h"

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
    EXPECT_CONTAINS(soundsTape.flattened(), SOUND_RechargeItem); // dispel magic
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

GAME_TEST(Issues, Issue1521) {
    // Enemies spawned in mob pack stuck
    test.playTraceFromTestData("issue_1521.mm7", "issue_1521.json");

    // make sure actors are not stuck
    constexpr Vec3f mobPos(9120, -25168, 703);
    auto mobDist = [&mobPos](const Actor& a) { return (a.pos - mobPos).length(); };
    auto mobToMobDist = [](const Actor& a, const Actor& b) { return (a.pos - b.pos).length(); };

    // make sure actors have moved from their spawn point
    EXPECT_GT(mobDist(pActors[37]), 512.0f);
    EXPECT_GT(mobDist(pActors[38]), 512.0f);
    EXPECT_GT(mobDist(pActors[39]), 512.0f);
    // and that they are not stuck to each other
    EXPECT_GT(mobToMobDist(pActors[37], pActors[38]), 512.0f);
    EXPECT_GT(mobToMobDist(pActors[38], pActors[39]), 512.0f);
}

GAME_TEST(Issues, Issue1522) {
    // Wizards not summoning light elementals
    auto actorsCount = actorTapes.totalCount();
    auto lightElem = tapes.custom([]() { return std::ranges::count_if(pActors, [](const Actor& a) { return a.name.contains("Light Elemental"); }); });
    test.playTraceFromTestData("issue_1522.mm7", "issue_1522.json");
    EXPECT_GT(actorsCount.back(), actorsCount.front());
    EXPECT_GT(lightElem.back(), lightElem.front());
    auto summoned = std::ranges::count_if(pActors, [](const Actor& a) { return a.summonerId.type() == OBJECT_Actor; });
    EXPECT_GT(summoned, 0);
}

GAME_TEST(Issues, Issue1524) {
    // More enemy spells without sound
    auto soundsTape = tapes.sounds();
    test.playTraceFromTestData("issue_1524.mm7", "issue_1524.json");
    EXPECT_CONTAINS(soundsTape.flattened(), SOUND_Sacrifice2); // pain reflection sound
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
    Item item;
    int attrEnchantmentsNum = 0;
    int specialEnchantmentsNum = 0;
    for (int i = 0; i < 100; i++) {
        pItemTable->generateItem(ITEM_TREASURE_LEVEL_5, RANDOM_ITEM_AMULET, &item);
        if (item.standardEnchantment)
            attrEnchantmentsNum++;
        if (item.specialEnchantment != ITEM_ENCHANTMENT_NULL)
            specialEnchantmentsNum++;
    }
    EXPECT_NE(attrEnchantmentsNum, 0);
    EXPECT_NE(specialEnchantmentsNum, 0);
}

// 1600

GAME_TEST(Issues, Issue1655) {
    // Assertion in CalcSpellDamage failed b/c an actor is trying to cast SPELL_NONE.
    auto stateTape = actorTapes.aiState(73);
    auto expressionsTape = charTapes.portraits();
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
        return expressions.containsAny(PORTRAIT_DMGRECVD_MINOR, PORTRAIT_DMGRECVD_MODERATE, PORTRAIT_DMGRECVD_MAJOR);
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
    EXPECT_CONTAINS(screenTape, SCREEN_INPUT_BLV);
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
    auto expressionTape = charTapes.portrait(2);
    auto modelTape = tapes.custom([]() {bool on_water = false; int bmodel_pid = 0;
        float floor_level = ODM_GetFloorLevel(pParty->pos, &on_water, &bmodel_pid);
        return bmodel_pid; });
    test.playTraceFromTestData("issue_1671.mm7", "issue_1671.json");
    EXPECT_LT(health.back(), health.front()); // party has taken damage from fall
    EXPECT_CONTAINS(expressionTape, PORTRAIT_FEAR);
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

    Item jar1;
    jar1.itemId = ITEM_QUEST_LICH_JAR_FULL;
    jar1.lichJarCharacterIndex = 0;

    Item jar2;
    jar2.itemId = ITEM_QUEST_LICH_JAR_FULL;
    jar2.lichJarCharacterIndex = 1;

    pParty->pCharacters[0].AddItem2(-1, &jar1);
    pParty->pCharacters[1].AddItem2(-1, &jar2);

    EXPECT_EQ(jar1.GetIdentifiedName(), "Kolya's Jar");
    EXPECT_EQ(jar2.GetIdentifiedName(), "Nicholas' Jar");
}

GAME_TEST(Prs, Pr1694) {
    // Having a dir ending with .mm7 in /saves shouldn't trip the engine.
    ufs->write("saves/dir.mm7/1.txt", Blob());

    game.pressGuiButton("MainMenu_LoadGame");
    game.tick(5);
    game.pressGuiButton("LoadMenu_Slot0"); // Should not crash.
    game.tick(1);

    for (bool used : pSavegameList->pSavegameUsedSlots)
        EXPECT_FALSE(used); // All slots unused.
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
    auto expressionTape = charTapes.portrait(2);
    auto zpos = tapes.custom([]() { return static_cast<int>(pParty->pos.z); });
    auto noFallDamageTape = tapes.config(engine->config->gameplay.NoIndoorFallDamage);
    test.playTraceFromTestData("issue_1710.mm7", "issue_1710.json");
    EXPECT_EQ(noFallDamageTape, tape(false)); // Fall damage was actually possible
    EXPECT_LT(health.back(), health.front()); // party has taken damage from fall
    EXPECT_EQ(uCurrentlyLoadedLevelType, LEVEL_INDOOR);
    EXPECT_CONTAINS(expressionTape, PORTRAIT_FEAR);
    EXPECT_GT(zpos.max(), zpos.min() + 1000);
}

GAME_TEST(Issues, Issue1716) {
    // Status protections not working
    auto specialAttack = tapes.specialAttacks();
    auto pmCountTape = tapes.custom([]() { return pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].power; });
    test.playTraceFromTestData("issue_1716.mm7", "issue_1716.json");
    EXPECT_CONTAINS(specialAttack.flattened(), SPECIAL_ATTACK_PARALYZED); // Paralysis attacks were made
    int paraCount = std::ranges::count_if(pParty->pCharacters, [](Character& ch) { return ch.IsParalyzed(); });
    EXPECT_EQ(paraCount, 0); // No one ended up paralysed
    EXPECT_LT(pmCountTape.back(), pmCountTape.front()); // PM saved us
}

GAME_TEST(Issues, Issue1717) {
    // Immolation incorrect damage message.
    auto statusBar = tapes.statusBar();
    auto immoBuff = tapes.custom([]() { return pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Active(); });
    test.playTraceFromTestData("issue_1717.mm7", "issue_1717.json");
    EXPECT_EQ(immoBuff, tape(false, true));
    EXPECT_EQ(pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].caster, 4);

    std::regex regex("Immolation deals [0-9]+ damage to [0-9]+ target\\(s\\)");
    EXPECT_CONTAINS(statusBar, [&](const std::string &message) { return std::regex_match(message, regex); });
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
    EXPECT_CONTAINS(textTape.flattened(), "Exit Building"); // And can exit it
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
    EXPECT_CONTAINS(textTape.flattened(), "You don't meet the requirements, and cannot be taught until you do."); // but we dont meet them
}

GAME_TEST(Issues, Issue1786) {
    // Casting a quick spell that's not in spellbook asserts
    auto sprites = tapes.sprites();
    game.startNewGame();
    test.startTaping();
    pParty->pCharacters[3].uQuickSpell = SPELL_FIRE_FIRE_BOLT;
    EXPECT_FALSE(pParty->pCharacters[3].bHaveSpell[SPELL_FIRE_FIRE_BOLT]);

    engine->config->debug.AllMagic.setValue(true);
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_4); // Select char 4.
    game.tick();
    EXPECT_EQ(pParty->activeCharacterIndex(), 4);

    game.pressKey(PlatformKey::KEY_S); // Quick cast fire bolt.
    game.tick();
    game.releaseKey(PlatformKey::KEY_S);
    game.tick();
    EXPECT_CONTAINS(sprites.back(), SPRITE_SPELL_FIRE_FIRE_BOLT);

    while (pParty->activeCharacterIndex() != 4) {
        game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_4);
        game.tick();
    }

    engine->config->debug.AllMagic.setValue(false);
    game.pressKey(PlatformKey::KEY_S); // Try to quick cast fire bolt.
    game.tick();
    game.releaseKey(PlatformKey::KEY_S);
    game.tick();
    EXPECT_MISSES(sprites.back(), SPRITE_SPELL_FIRE_FIRE_BOLT);
}

// 1800

GAME_TEST(Issues, Issue1807) {
    // Opening arcomage menu in a tavern while not carrying a deck asserts.
    auto deckTape = tapes.hasItem(ITEM_QUEST_ARCOMAGE_DECK);
    auto houseTape = tapes.house();
    auto textTape = tapes.allGUIWindowsText();
    test.playTraceFromTestData("issue_1807.mm7", "issue_1807.json");
    EXPECT_EQ(deckTape, tape(false)); // No deck.
    EXPECT_CONTAINS(houseTape, HOUSE_TAVERN_HARMONDALE); // We've visited the Harmondale tavern.
    EXPECT_CONTAINS(textTape.flattened(), "Victory Conditions"); // We've seen the Arcomage dialog.
    EXPECT_MISSES(textTape.flattened(), "Play"); // But there was no "Play" option.
}

GAME_TEST(Issues, Issue1808) {
    // Vase quest item stays in inventory.
    auto activeCharTape = tapes.activeCharacterIndex();
    auto classTape = charTapes.clazz(1);
    auto vaseTape = charTapes.hasItem(3, ITEM_QUEST_VASE);
    auto vasesTape = tapes.hasItem(ITEM_QUEST_VASE);
    auto goldTape = tapes.gold();
    test.playTraceFromTestData("issue_1808.mm7", "issue_1808.json");
    EXPECT_EQ(activeCharTape, tape(1)); // First character was active.
    EXPECT_EQ(vaseTape, tape(true, false)); // Vase was taken from 3rd char.
    EXPECT_EQ(vasesTape, tape(true, false)); // And it was the only vase we had.
    EXPECT_EQ(classTape, tape(CLASS_THIEF, CLASS_ROGUE)); // 2nd char was promoted.
    EXPECT_EQ(goldTape.delta(), +5000); // Quest reward.
}

GAME_TEST(Issues, Issue1849_1831) {
    // 1849 - Sectors get seen and activated on save load despite being behind closed doors
    // 1831 - Engine too eager to reveal indoor map
    auto nearDoor = tapes.custom([]() {return pParty->pos.x > -7800 && pParty->pos.x < -7400 && pParty->pos.y < -470; });
    auto sectors = tapes.custom([]() {return pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom; });
    test.playTraceFromTestData("issue_1849.mm7", "issue_1849.json"); // this loads a save
    auto flat = sectors.flattened();
    EXPECT_MISSES(flat, 58); // sectors not exposed
    EXPECT_MISSES(flat, 32);
    EXPECT_CONTAINS(nearDoor, true); // position take us close to wall
}

GAME_TEST(Issues, Issue1851a) {
    // Collisions: stair climbing - hall of the pit - using catch all
    auto zPos = tapes.custom([]() { return static_cast<int>(pParty->pos.z); });
    auto jumpTape = tapes.custom([]() { return !!(pParty->uFlags & PARTY_FLAG_JUMPING); });
    test.playTraceFromTestData("issue_1851a.mm7", "issue_1851a.json");
    EXPECT_LT(zPos.front(), -190);
    EXPECT_GE(zPos.back(), 0);
    EXPECT_MISSES(jumpTape, true);
}

GAME_TEST(Issues, Issue1851b) {
    // Collisions: stair climbing - castle gloaming - using face exclusion
    auto zPos = tapes.custom([]() { return static_cast<int>(pParty->pos.z); });
    auto jumpTape = tapes.custom([]() { return !!(pParty->uFlags & PARTY_FLAG_JUMPING); });
    test.playTraceFromTestData("issue_1851b.mm7", "issue_1851b.json");
    EXPECT_LT(zPos.front(), -90);
    EXPECT_GT(zPos.back(), 525);
    EXPECT_MISSES(jumpTape, true);
}

GAME_TEST(Issues, Issue1837) {
    // Checking if we still fall down from the sky in Barrow Downs
    test.playTraceFromTestData("issue_1837.mm7", "issue_1837.json");
    EXPECT_FLOAT_EQ(pParty->pos.z, 1.0f);
}

GAME_TEST(Issues, Issue1869) {
    // Face overflow in Tunnels to Eeofol
    auto faceCount = tapes.custom([]() { return pBspRenderer->num_faces; });
    test.playTraceFromTestData("issue_1869.mm7", "issue_1869.json");
    EXPECT_EQ(engine->_currentLoadedMapId, MAP_TUNNELS_TO_EEOFOL);
    EXPECT_LT(faceCount.max(), 1400);
}

GAME_TEST(Issues, Issue1898) {
    // Doors in School of Sorcery hurt all party members instead of the active one
    auto totalHealth = tapes.totalHp();
    auto charHealth = charTapes.hp(0);
    test.playTraceFromTestData("issue_1898.mm7", "issue_1898.json");
    EXPECT_EQ(engine->_currentLoadedMapId, MAP_SCHOOL_OF_SORCERY);
    EXPECT_LT(totalHealth.delta(), 0);
    EXPECT_EQ(charHealth.delta(), totalHealth.delta()); // all health lost from one character
}

GAME_TEST(Issues, Issue1890) {
    // Stuck *in* stairs when leaving the Mercenary Guild
    auto yPos = tapes.custom([]() { return static_cast<int>(pParty->pos.y); });
    test.playTraceFromTestData("issue_1890.mm7", "issue_1890.json");
    EXPECT_EQ(engine->_currentLoadedMapId, MAP_TATALIA);
    EXPECT_CONTAINS(yPos, 16803); // starting point
    EXPECT_LT(yPos.back(), 16700); // moved forwards
}

// 1900

GAME_TEST(Issues, Issue1910) {
    // Insane condition doesn't affect character attributes when there is Weak condition.
    // We test this for both vanilla & grayface condition priorities.
    for (bool useAlternativePriorities : {true, false}) {
        test.prepareForNextTest();
        engine->config->gameplay.AlternativeConditionPriorities.setValue(useAlternativePriorities);

        auto intTape = charTapes.stat(0, ATTRIBUTE_INTELLIGENCE);
        auto strTape = charTapes.stat(0, ATTRIBUTE_MIGHT);

        game.startNewGame();
        test.startTaping();
        game.tick();
        pParty->pCharacters[0].SetCondition(CONDITION_WEAK, 0);
        pParty->pCharacters[0].SetCondition(CONDITION_INSANE, 0);
        game.tick();

        EXPECT_EQ(intTape, tape(5, 0)); // Int -90% b/c insane.
        EXPECT_EQ(strTape, tape(30, 60)); // Str +100% b/c insane.
    }
}

GAME_TEST(Issues, Issue1911) {
    // Unarmed attack bonus is not applied when Monk is unarmed, but applied when wearing a staff.
    game.startNewGame();
    EXPECT_TRUE(pParty->pCharacters[0].IsUnarmed());

    pParty->pCharacters[0].pActiveSkills[CHARACTER_SKILL_UNARMED] = CombinedSkillValue(1, CHARACTER_SKILL_MASTERY_NOVICE);
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(true), 1);

    pParty->pCharacters[0].pActiveSkills[CHARACTER_SKILL_UNARMED] = CombinedSkillValue(4, CHARACTER_SKILL_MASTERY_NOVICE);
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(true), 4);

    // Equip staff.
    Item staff;
    staff.itemId = ITEM_STAFF;
    pParty->pPickedItem = staff;
    pParty->pCharacters[0].EquipBody(ITEM_TYPE_TWO_HANDED);
    pParty->pCharacters[0].pActiveSkills[CHARACTER_SKILL_STAFF] = CombinedSkillValue(1, CHARACTER_SKILL_MASTERY_NOVICE);
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(true), 1); // +1 from staff skill.

    // Check that master staff is not affected by unarmed.
    pParty->pCharacters[0].pActiveSkills[CHARACTER_SKILL_STAFF] = CombinedSkillValue(7, CHARACTER_SKILL_MASTERY_MASTER);
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(true), 7); // +7 from staff skill.

    // Check that GM staff works with unarmed.
    pParty->pCharacters[0].pActiveSkills[CHARACTER_SKILL_STAFF] = CombinedSkillValue(10, CHARACTER_SKILL_MASTERY_GRANDMASTER);
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(true), 14); // +10 from staff, +4 from unarmed.
}

GAME_TEST(Issues, Issue1912) {
    // Trading a Red Potion for a Wealthy Hat doesn't remove the potion from the inventory.
    auto potionTape = charTapes.haveItem(ITEM_POTION_CURE_WOUNDS);
    auto hatTape = charTapes.haveItem(ITEM_QUEST_WEALTHY_HAT);
    auto activeCharTape = tapes.activeCharacterIndex();
    test.playTraceFromTestData("issue_1912.mm7", "issue_1912.json");
    EXPECT_EQ(activeCharTape, tape(1)); // First char was talking.
    EXPECT_EQ(potionTape, tape({false, true, false, false}, {false, false, false, false})); // But 2nd char had the potion.
    EXPECT_EQ(hatTape, tape({false, false, false, false}, {true, false, false, false})); // We passed the hat to the 1st char.
}

GAME_TEST(Issues, Issue1925) {
    // Test for wand behaviour.
    for (bool wandsDisappear : {true, false}) {
        test.prepareForNextTest();
        engine->config->gameplay.DestroyDischargedWands.setValue(wandsDisappear);

        auto wandTape = tapes.hasItem(ITEM_WAND_OF_FIRE);
        auto spritesTape = tapes.sprites();

        game.startNewGame();
        test.startTaping();
        game.tick();

        // Equip wand.
        Item wand;
        wand.itemId = ITEM_WAND_OF_FIRE;
        wand.numCharges = wand.maxCharges = 1;
        pParty->pPickedItem = wand;
        pParty->pCharacters[0].EquipBody(ITEM_TYPE_WAND);
        game.tick();

        // Attack.
        game.pressKey(PlatformKey::KEY_A);
        game.tick();
        game.releaseKey(PlatformKey::KEY_A);
        game.tick();

        EXPECT_EQ(wandTape, wandsDisappear ? tape(false, true, false) : tape(false, true));
        if (!wandsDisappear) {
            const Item *dischargedWand = pParty->pCharacters[0].GetItem(ITEM_SLOT_MAIN_HAND);
            EXPECT_NE(dischargedWand, nullptr);
            EXPECT_EQ(dischargedWand->itemId, ITEM_WAND_OF_FIRE);
            EXPECT_EQ(dischargedWand->numCharges, 0);
            EXPECT_EQ(dischargedWand->maxCharges, 1);
        }
        EXPECT_CONTAINS(spritesTape.flattened(), SPRITE_SPELL_FIRE_FIRE_BOLT);
    }
}

GAME_TEST(Issues, Issue1927) {
    // Having bow equipped increases ranged attack bonus with wands
    test.prepareForNextTest();
    auto rangeAttackTape = tapes.custom([] { return pParty->pCharacters[0].GetRangedAttack(); });
    game.startNewGame();
    test.startTaping();
    game.tick();

    // Equip a bow
    Item bow;
    bow.itemId = ITEM_GRIFFIN_BOW;
    pParty->pPickedItem = bow;
    pParty->pCharacters[0].EquipBody(ITEM_TYPE_BOW);
    game.tick();

    // Equip wand.
    Item wand;
    wand.itemId = ITEM_ALACORN_WAND_OF_FIREBALLS;
    wand.numCharges = wand.maxCharges = 30;
    pParty->pPickedItem = wand;
    pParty->pCharacters[0].EquipBody(ITEM_TYPE_WAND);
    game.tick();

    EXPECT_EQ(rangeAttackTape.size(), 3); // nothing, bow, bow and wand
    EXPECT_EQ(rangeAttackTape.front(), rangeAttackTape.back()); // range bonus between nothing equipped and wand should be the same
    EXPECT_EQ(rangeAttackTape.back(), pParty->pCharacters[0].GetActualAttack(false)); // should match melee
}

GAME_TEST(Prs, Pr1934) {
    // Should be able to generate standard enchantments with +25 bonus.
    Item item;
    int maxStrength = 0;
    for (int i = 0; i < 100; i++) {
        pItemTable->generateItem(ITEM_TREASURE_LEVEL_6, RANDOM_ITEM_RING, &item);
        if (item.standardEnchantment)
            maxStrength = std::max(maxStrength, item.standardEnchantmentStrength);
    }

    EXPECT_EQ(maxStrength, 25);
}

GAME_TEST(Issues, Issue1947) {
    // Wand is generated with 0 charges in Tatalia.
    // The wand in question does in fact have 0 charges in the data files, and NWC never wrote the code to post-process
    // it on load like they did for wands in chests or wands carried by monsters. So we fixed that.
    auto mapTape = tapes.map();
    game.startNewGame();
    test.startTaping();
    engine->config->debug.TownPortal.setValue(true);
    engine->config->debug.AllMagic.setValue(true);

    game.pressAndReleaseKey(PlatformKey::KEY_C);
    game.tick();
    game.pressGuiButton("SpellBook_School2"); // Water magic.
    game.tick();
    game.pressGuiButton("SpellBook_Spell8"); // Town portal.
    game.tick();
    game.pressGuiButton("SpellBook_Spell8"); // Confirm.
    game.tick(3);
    game.pressGuiButton("TownPortalBook_Marker10"); // Tatalia.
    game.tick();
    game.skipLoadingScreen();

    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_TATALIA));
    EXPECT_EQ(pSpriteObjects[4].containing_item.itemId, ITEM_ALACORN_WAND_OF_FIREBALLS);
    EXPECT_GT(pSpriteObjects[4].containing_item.numCharges, 0);
    EXPECT_GT(pSpriteObjects[4].containing_item.maxCharges, 0);
}

GAME_TEST(Issues, Issue1956) {
    // Crash shortly after entering Land of Giants
    test.playTraceFromTestData("issue_1956.mm7", "issue_1956.json");
    // trace launches a load of items down the slope
    // test that they all slow down and all are in bounds
    const BBoxf limitBox = BBoxf::cubic(Vec3f(), 32768.0f);
    for (const auto& item : pSpriteObjects) {
        EXPECT_LT(item.vVelocity.length(), 100.0f);
        EXPECT_TRUE(limitBox.contains(item.vPosition));
    }
}

GAME_TEST(Issues, Issue1958) {
    // Assert failing when trying to claim an already claimed bounty.
    auto textsTape = tapes.allGUIWindowsText();
    auto goldTape = tapes.gold();
    test.playTraceFromTestData("issue_1958.mm7", "issue_1958.json");
    EXPECT_CONTAINS(textsTape.flattened(), [](std::string_view s) { return s.contains("Congratulations on defeating the"); }); // Bounty message.
    EXPECT_CONTAINS(textsTape.flattened(), [](std::string_view s) { return s.contains("Someone has already claimed the bounty this month."); }); // Bounty already claimed message.
    EXPECT_EQ(goldTape.delta(), +1400); // We got the bounty.
}

GAME_TEST(Issues, Issue1961) {
    // Enchant Item costs no SP.
    auto manaTape = charTapes.mp(3);
    game.startNewGame();
    test.startTaping();

    // Prepare an item to enchant.
    pParty->pCharacters[3].pInventoryItemList.fill(Item());
    pParty->pCharacters[3].pInventoryMatrix.fill(0);
    pParty->pCharacters[3].AddItem(-1, ITEM_GOLDEN_CHAIN_MAIL);
    const Item &chainmail = pParty->pCharacters[3].pInventoryItemList[0];
    EXPECT_EQ(chainmail.itemId, ITEM_GOLDEN_CHAIN_MAIL);

    // Learn enchant item.
    pParty->pCharacters[3].pActiveSkills[CHARACTER_SKILL_WATER] = CombinedSkillValue(10, CHARACTER_SKILL_MASTERY_GRANDMASTER);
    pParty->pCharacters[3].bHaveSpell[SPELL_WATER_ENCHANT_ITEM] = true;

    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_4); // Select 4th char.
    game.tick(1);
    game.pressGuiButton("Game_CastSpell");
    game.tick(1);
    game.pressGuiButton("SpellBook_School2"); // Water magic.
    game.tick(1);
    game.pressGuiButton("SpellBook_Spell7"); // Enchant item.
    game.tick(1);
    game.pressGuiButton("SpellBook_Spell7"); // Confirm.
    game.tick(2);
    game.pressAndReleaseButton(BUTTON_LEFT, 30, 30);
    game.tick(1); // Don't wait out the animation.

    EXPECT_EQ(manaTape.delta(), -15);
    EXPECT_TRUE(chainmail.standardEnchantment || chainmail.specialEnchantment != ITEM_ENCHANTMENT_NULL);
}

GAME_TEST(Issues, Issue1966) {
    // Assert crash casting Armageddon, happens when monsters are hit with Armageddon rocks.
    auto mapTape = tapes.map();
    auto itemCountTape = tapes.totalItemCount();
    auto spritesTape = tapes.sprites();
    test.playTraceFromTestData("issue_1966.mm7", "issue_1966.json");
    EXPECT_EQ(mapTape, tape(MAP_HARMONDALE, MAP_LAND_OF_THE_GIANTS));
    EXPECT_EQ(itemCountTape.delta(), -1); // Minus armageddon scroll.

    // Should have had a bunch of rocks in the air at some point due to Armageddon.
    EXPECT_GT(spritesTape.mapped([] (auto &&sprites) { return sprites.count(SPRITE_SPELL_EARTH_ROCK_BLAST); }).max(), 100);

    // Some rocks should have hit monsters - this is what was triggering the assertion.
    EXPECT_GT(spritesTape.flattened().count(SPRITE_SPELL_EARTH_ROCK_BLAST_IMPACT), 10);
}

GAME_TEST(Issues, Issue1972) {
    // Enemy AI meteor shower had a bad meteor distribution due to loop var overwrite.
    auto spritesTape = tapes.sprites();
    auto mapTape = tapes.map();
    auto hpTape = tapes.totalHp();
    test.playTraceFromTestData("issue_1972.mm7", "issue_1972.json", TRACE_PLAYBACK_SKIP_RANDOM_CHECKS);
    EXPECT_EQ(mapTape, tape(MAP_LAND_OF_THE_GIANTS));
    int meteorCount = spritesTape.mapped([] (auto &&sprites) { return sprites.count(SPRITE_SPELL_FIRE_METEOR_SHOWER); }).max();
    EXPECT_EQ(meteorCount, 24); // 2x meteor shower cast at master. Might change to 12 on retrace.
    EXPECT_LE(hpTape.delta(), -700); // Party should have received some damage. Checking this b/c retracing might break smth.
}

GAME_TEST(Issues, Issue1974) {
    // Monsters can climb vertical walls where in MM7 they could not
    test.playTraceFromTestData("issue_1974.mm7", "issue_1974.json");
    // Make sure all the monsters have stayed below
    constexpr std::array<int, 12> monsterIds = { 0, 31, 32, 33, 48, 49, 50, 51, 52, 53, 54, 55 };
    for (auto ids : monsterIds) {
        EXPECT_LT(pActors[ids].pos.z, -1375);
    }
}

GAME_TEST(Issues, Issue1977) {
    // Mix a potion of water resistance: explodes without fix.
    // Disabling RNG checks on playback to actually see results even if the explosion uses a `grng` call.
    // Pre-fix output still ignores the EXPECT's below, playTrace terminates comparing character 3's hp.
    auto resultTape = tapes.hasItem(ITEM_POTION_WATER_RESISTANCE);
    auto component1Tape = tapes.hasItem(ITEM_POTION_SHIELD);
    auto component2Tape = tapes.hasItem(ITEM_POTION_HARDEN_ITEM);
    test.playTraceFromTestData("issue_1977.mm7", "issue_1977.json");
    EXPECT_EQ(resultTape, tape(false, true));       // Got a white potion
    EXPECT_EQ(component1Tape, tape(true, false));   // Used up components
    EXPECT_EQ(component2Tape, tape(true, false));
}

GAME_TEST(Issues, Issue1983) {
    // It was possible to sell recipes at a magic shop, not only at the alchemist as intended.
    auto recipeTape = tapes.hasItem(ITEM_RECIPE_REJUVENATION);
    auto bookTape = tapes.hasItem(ITEM_SPELLBOOK_LIGHT_BOLT);
    auto letterTape = tapes.hasItem(ITEM_MESSAGE_LETTER_FROM_MR_STANTLEY_2);
    auto houseTape = tapes.house();
    auto goldTape = tapes.gold();
    auto soundsTape = tapes.sounds();
    auto textsTape = tapes.allGUIWindowsText();
    test.playTraceFromTestData("issue_1983.mm7", "issue_1983.json");
    EXPECT_EQ(recipeTape, tape(true)); // The recipe should still be there
    EXPECT_EQ(bookTape, tape(true, false)); // The spellbook should be gone
    EXPECT_EQ(letterTape, tape(true)); // The letter should still be there
    EXPECT_CONTAINS(houseTape, HOUSE_MAGIC_SHOP_EMERALD_ISLAND);
    EXPECT_EQ(goldTape.delta(), 365); // Sold the spellbook
    EXPECT_CONTAINS(soundsTape.flattened(), SOUND_error); // Tried to sell unsellable items

    // Merchant should have reacted properly to unsellable items.
    EXPECT_CONTAINS(textsTape.flattened(), [] (std::string_view text) { return text.contains("Body Resistance Recipe") && text.contains("is beyond my meager knowledge"); });
    EXPECT_CONTAINS(textsTape.flattened(), [] (std::string_view text) { return text.contains("Rejuvenation Recipe") && text.contains("is beyond my meager knowledge"); });
    EXPECT_CONTAINS(textsTape.flattened(), [] (std::string_view text) { return text.contains("Water Resistance Recipe") && text.contains("is beyond my meager knowledge"); });
    EXPECT_CONTAINS(textsTape.flattened(), [] (std::string_view text) { return text.contains("Letter from Mr. Stantley") && text.contains("is beyond my meager knowledge"); });
}

GAME_TEST(Issues, Issue1989) {
    // Crash entering Wormthrax' cave in turn mode
    auto mapTape = tapes.map();
    auto turnTape = tapes.turnBasedMode();

    test.playTraceFromTestData("issue_1989.mm7", "issue_1989.json", [] { EXPECT_TRUE([] {
        for (const auto& act : pActors)
            if ((pParty->pos - act.pos).lengthSqr() < 400) // troll is nearby
                return true;
        return false; });
    });
    EXPECT_EQ(mapTape, tape(MAP_TATALIA, MAP_WROMTHRAXS_CAVE));
    EXPECT_EQ(turnTape, tape(false, true)); // turn mode
    EXPECT_EQ(pActors.size(), 1); // here be dragon
}

GAME_TEST(Issues, Issue1990) {
    // Test opening the Tularean Forest half-hidden chest, which generates a black potion.
    auto screenTape = tapes.screen();
    auto potionTape = tapes.custom([] { return vChests[6].items[6].itemId; });
    auto powerTape = tapes.custom([] { return vChests[6].items[6].potionPower; });
    test.playTraceFromTestData("issue_1990.mm7", "issue_1990.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_CHEST)); // We have opened the chest.
    EXPECT_EQ(potionTape, tape(ITEM_POTION_PURE_MIGHT));
    EXPECT_EQ(powerTape.front(), 0); // Potion power started as uninitialized.
    EXPECT_GE(powerTape.back(), 5);
    EXPECT_LT(powerTape.back(), 20); // Potion power ended as initialized to 5-19.
}

GAME_TEST(Issues, Issue1997) {
    // Temple of Baa Clerics casting Spirit Lash did trigger assert.
    // We've replaced spirit lash with bless.
    auto blessTape = actorTapes.countByBuff(ACTOR_BUFF_BLESS);
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_1997.mm7", "issue_1997.json");
    EXPECT_EQ(mapTape, tape(MAP_TEMPLE_OF_BAA));
    EXPECT_EQ(blessTape.front(), 0);
    EXPECT_GT(blessTape.back(), 0); // Bless was cast at least once.
}
