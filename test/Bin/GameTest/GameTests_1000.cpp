#include <unordered_set>

#include "Testing/Game/GameTest.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIHouses.h"

#include "Engine/Graphics/TextureFrameTable.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/ParticleEngine.h"

#include "Media/Audio/AudioPlayer.h"

static bool characterHasJar(int charIndex, int jarIndex) {
    for (const ItemGen &item : pParty->pCharacters[charIndex].pInventoryItemList)
        if (item.uItemID == ITEM_QUEST_LICH_JAR_FULL && item.uHolderPlayer == jarIndex)
            return true;
    return false;
}

// 1000

GAME_TEST(Issues, Issues1004) {
    // Collisions: Can walk right through the bridge on Emerald Isle
    auto xTape = tapes.custom([] { return pParty->pos.x; });
    test.playTraceFromTestData("issue_1004.mm7", "issue_1004.json");
    EXPECT_LT(xTape.max(), 12552 + 1);
}

GAME_TEST(Prs, Pr1005) {
    // Testing collisions - stairs should work. In this test case the party is walking onto a wooden paving in Tatalia.
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("pr_1005.mm7", "pr_1005.json");
    EXPECT_EQ(zTape.frontBack(), tape(154, 193)); // Paving is at z=192, party z should be this value +1.
}

GAME_TEST(Issues, Issue1020) {
    // Test finishing the scavenger hunt quest. The game should not crash when there is no dialogue options.
    test.playTraceFromTestData("issue_1020.mm7", "issue_1020.json"); // Should not assert
}

GAME_TEST(Issues, Issue1034) {
    // Crash when casting telekinesis outdoors.
    auto houseTape = tapes.house();
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_1034.mm7", "issue_1034.json");
    EXPECT_TRUE(statusTape.contains("Select Target")); // Telekinesis message.
    EXPECT_EQ(houseTape, tape(HOUSE_INVALID, HOUSE_WEAPON_SHOP_EMERALD_ISLAND)); // We have entered into the shop.
}

GAME_TEST(Issues, Issue1036) {
    // Test that elemental magic guilds teach Learning skill and self magic guilds teach Meditation skill.
    test.playTraceFromTestData("issue_1036.mm7", "issue_1036.json");
    EXPECT_TRUE(pParty->pCharacters[2].pActiveSkills[CHARACTER_SKILL_LEARNING]);
    EXPECT_TRUE(pParty->pCharacters[2].pActiveSkills[CHARACTER_SKILL_MEDITATION]);
}

GAME_TEST(Issues, Issue1038) {
    // Crash while fighting Eyes in Nighon Tunnels
    auto conditionsTape = charTapes.conditions();
    test.playTraceFromTestData("issue_1038.mm7", "issue_1038.json");
    EXPECT_EQ(conditionsTape.frontBack(), tape({CONDITION_GOOD, CONDITION_INSANE, CONDITION_GOOD, CONDITION_INSANE},
                                               {CONDITION_SLEEP, CONDITION_INSANE, CONDITION_UNCONSCIOUS, CONDITION_UNCONSCIOUS}));
}

GAME_TEST(Issues, Issue1040) {
    // Crash when talking to 4-th dark advisor
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_1040.mm7", "issue_1040.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE, SCREEN_GAME));
}

GAME_TEST(Issues, Issue1051) {
    // Collision code asserts when fighting Magogs in Nighon Tunnels.
    // Note that the bug only reproduces on high fps, the trace is shot at 15ms per frame.
    auto frameTimeTape = tapes.config(engine->config->debug.TraceFrameTimeMs);
    test.playTraceFromTestData("issue_1051.mm7", "issue_1051.json");
    EXPECT_EQ(frameTimeTape, tape(15)); // Don't redo this at different FPS, the problem won't reproduce.
}

GAME_TEST(Issues, Issue1068) {
    // Kills assert if characters don't have learning skill, but party has an npc that gives learning boost.
    auto expTape = charTapes.experiences();
    test.playTraceFromTestData("issue_1068.mm7", "issue_1068.json");
    EXPECT_EQ(expTape.frontBack(), tape({158039, 156727, 157646, 157417}, {158518, 157206, 158125, 157896}));
}

GAME_TEST(Issues, Issue1093) {
    // Town Portal on master can be cast near enemies
    auto screenTape = tapes.screen();
    auto manaTape = charTapes.mp(3);
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_1093.mm7", "issue_1093.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_SPELL_BOOK, SCREEN_GAME));
    EXPECT_EQ(manaTape, tape(355, 356)); // +1 mana from mana regen, no mana spent on spells.
    EXPECT_TRUE(statusTape.contains("Cast Town Portal"));
    EXPECT_TRUE(statusTape.contains("Spell failed"));
}

// 1100

GAME_TEST(Issues, Issue1115) {
    // Entering Arena on level 21 should not crash the game
    auto mapTape = tapes.map();
    auto dialogueTape = tapes.dialogueType();
    auto levelTape = charTapes.levels();
    test.playTraceFromTestData("issue_1115.mm7", "issue_1115.json");
    EXPECT_EQ(mapTape, tape(MAP_HARMONDALE, MAP_ARENA)); // Harmondale -> Arena.
    EXPECT_TRUE(dialogueTape.contains(DIALOGUE_ARENA_SELECT_LORD));
    EXPECT_EQ(levelTape, tape({21, 21, 21, 21}));
}

GAME_TEST(Issues, Issue1155) {
    // Crash when pressing [Game Options] while talking to NPCs
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_1155.mm7", "issue_1155.json");
    EXPECT_FALSE(screenTape.contains(SCREEN_SPELL_BOOK));
    EXPECT_FALSE(screenTape.contains(SCREEN_REST));
    EXPECT_FALSE(screenTape.contains(SCREEN_QUICK_REFERENCE));
    EXPECT_FALSE(screenTape.contains(SCREEN_OPTIONS));
    EXPECT_FALSE(screenTape.contains(SCREEN_BOOKS));
    EXPECT_TRUE(screenTape.contains(SCREEN_CHARACTERS));
    EXPECT_TRUE(screenTape.contains(SCREEN_BRANCHLESS_NPC_DIALOG));
}

GAME_TEST(Issues, Issue1164) {
    // CHARACTER_EXPRESSION_NO animation ending abruptly - should show the character moving his/her head to the left,
    // then to the right.
    auto expressionTape = tapes.custom([] { return std::pair(pParty->pCharacters[0].expression, pEventTimer->time()); });
    auto frameTimeTape = tapes.config(engine->config->debug.TraceFrameTimeMs);
    test.playTraceFromTestData("issue_1164.mm7", "issue_1164.json");
    EXPECT_EQ(frameTimeTape, tape(15)); // Don't redo at other frame rates.

    auto isNo = [] (const auto &pair) { return pair.first == CHARACTER_EXPRESSION_NO; };
    auto begin = std::find_if(expressionTape.begin(), expressionTape.end(), isNo);
    auto end = std::find_if_not(begin, expressionTape.end(), isNo);
    ASSERT_NE(end, expressionTape.end());

    // CHARACTER_EXPRESSION_NO should take 144 ticks, minus one frame. This one frame is an implementation artifact,
    // shouldn't really be there, but for now we test it the way it actually works.
    auto ticks = end->second - begin->second;
    Duration frameTicks = Duration::fromRealtimeMilliseconds(15 + (1_ticks).realtimeMilliseconds() - 1 /* Round up! */);
    EXPECT_GE(ticks, 144_ticks - frameTicks);
}

GAME_TEST(Issues, Issue1175) {
    // Enemies not using ranged attacks in turn based mode indoors
    auto healthTape = tapes.totalHp();
    auto turnTape = tapes.turnBasedMode();
    test.playTraceFromTestData("issue_1175.mm7", "issue_1175.json");
    EXPECT_LT(healthTape.back(), healthTape.front());
    EXPECT_TRUE(turnTape.back());
}

GAME_TEST(Issues, Issue1191) {
    auto foodTape = tapes.food();
    test.playTraceFromTestData("issue_1191.mm7", "issue_1191.json");

    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_FIRE).level(), 10);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_AIR).level(), 4);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_WATER).level(), 4);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_EARTH).level(), 4);

    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_SPIRIT).level(), 4);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_MIND).level(), 9); // 4, +3 dragon, +2 Ruler's ring
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_BODY).level(), 4);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_DARK).level(), 0);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_LIGHT).level(), 0);

    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_FIRE).level(), 4);
    EXPECT_LE(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_AIR).level(), 3); // She has no skill. 0 or 3 skill level is fine
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_WATER).level(), 4);
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_EARTH).level(), 18); // 10, +3 dragon, +5 ring

    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_SPIRIT).level(), 13);
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_MIND).level(), 4);
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_BODY).level(), 18); // 10, +3 dragon, +5 ring
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_DARK).level(), 0);
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_LIGHT).level(), 0);

    // TODO(captainurist): Uncomment when food issues (1226) resolved
    // EXPECT_EQ(foodTape.delta(), -3);
    // EXPECT_EQ(pParty->GetFood(), 7);
}

GAME_TEST(Issues, Issue1196) {
    // Assert fails in Character::playEmotion when character looks down
    auto expr = tapes.custom([] { return pParty->activeCharacter().expression; });
    test.playTraceFromTestData("issue_1196.mm7", "issue_1196.json");
    EXPECT_FALSE(expr.contains(CHARACTER_EXPRESSION_32));
    EXPECT_TRUE(expr.contains(CHARACTER_EXPRESSION_LOOK_UP));
    EXPECT_TRUE(expr.contains(CHARACTER_EXPRESSION_LOOK_DOWN));
}

GAME_TEST(Issues, Issue1197) {
    // Assert on party death
    auto loc = tapes.map();
    auto deaths = tapes.deaths();
    test.playTraceFromTestData("issue_1197.mm7", "issue_1197.json");
    EXPECT_TRUE(loc.contains(MAP_EMERALD_ISLAND)); // make it back to emerald
    EXPECT_EQ(deaths.delta(), 1);
}

// 1200

GAME_TEST(Issues, Issue1251a) {
    // Part A - test that wand damage matches vanilla damage should be in range (d6 per skill) 8-48 for novice 8 fireball wand
    auto dragonhealth = tapes.custom([] { return pActors[0].currentHP; });
    test.playTraceFromTestData("issue_1251a.mm7", "issue_1251a.json");

    auto damageRange = dragonhealth.reversed().adjacentDeltas().minMax();
    EXPECT_GE(damageRange.front(), 8);
    EXPECT_LE(damageRange.back(), 48);
}

GAME_TEST(Issues, Issue1251b) {
    // Make sure charm wand doesn't assert
    auto charmedActors = actorTapes.countByBuff(ACTOR_BUFF_CHARM);
    auto charmWands = tapes.hasItem(ITEM_ALACORN_WAND_OF_CHARMS);
    test.playTraceFromTestData("issue_1251b.mm7", "issue_1251b.json");
    EXPECT_EQ(charmedActors.delta(), 3);
    EXPECT_EQ(charmWands, tape(true));
}

GAME_TEST(Issues, Issue1253) {
    // Right clicking on a hireling asserts if there is no active character
    auto messageBoxesTape = tapes.messageBoxes();
    auto messageBoxesBody = tapes.allGUIWindowsText();
    test.playTraceFromTestData("issue_1253.mm7", "issue_1253.json");
    // message box text was displayed.
    auto flatMessageBoxes = messageBoxesTape.flattened();
    auto flatMessageBoxesBody = messageBoxesBody.flattened();
    EXPECT_GT(flatMessageBoxes.size(), 0);
    EXPECT_GT(flatMessageBoxesBody.filtered([](const auto& s) { return s.starts_with("Perception skill is increased by"); }).size(), 0);
    EXPECT_FALSE(pParty->hasActiveCharacter());
    EXPECT_EQ(current_screen_type, SCREEN_GAME);
}

GAME_TEST(Issues, Issue1255) {
    // Cant buy green wand
    auto wandTape = tapes.hasItem(ITEM_FAIRY_WAND_OF_LASHING);
    test.playTraceFromTestData("issue_1255.mm7", "issue_1255.json");
    EXPECT_EQ(wandTape, tape(false, true));
}

GAME_TEST(Issues, Issue1272) {
    // Game (but not UI) is frozen after death, until click.
    auto deathsTape = tapes.deaths();
    auto screenTape = tapes.screen();
    auto actorWiggleAfterDeath = tapes.custom([] {
        if (pParty->uNumDeaths == 0 || current_screen_type != SCREEN_GAME)
            return 0;

        int result = 0;
        for (const Actor &actor : pActors)
            result += static_cast<int>(actor.pos.x);
        return result;
    });
    test.playTraceFromTestData("issue_1272.mm7", "issue_1272.json");
    EXPECT_EQ(deathsTape.delta(), +1); // Party did die.
    EXPECT_TRUE(screenTape.contains(SCREEN_VIDEO)); // Party death video should have played.
    EXPECT_GT(actorWiggleAfterDeath.size(), 2); // Time did flow after respawn, and actors did move around.
}

GAME_TEST(Issues, Issue1273) {
    // Assert when clicking on shop video area
    auto dialogueTape = tapes.dialogueType();
    test.playTraceFromTestData("issue_1273.mm7", "issue_1273.json");
    EXPECT_EQ(dialogueTape, tape(DIALOGUE_NULL, DIALOGUE_MAIN));
}

GAME_TEST(Issues, Issue1274) {
    // Right clicking in guild when not member brings up skill tooltips
    auto screenTape = tapes.screen();
    auto messageBoxesTape = tapes.messageBoxes();
    test.playTraceFromTestData("issue_1274.mm7", "issue_1274.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE)); // Entered a house.
    EXPECT_EQ(messageBoxesTape.size(), 1);
    EXPECT_TRUE(messageBoxesTape.front().empty()); // No message boxes.
}

GAME_TEST(Issues, Issue1275) {
    // Clicking a store button while holding item causes black screen
    auto heldTape = tapes.custom([] {return pParty->pPickedItem.uItemID; });
    auto dialoTape = tapes.custom([] {if (window_SpeakInHouse != nullptr) return window_SpeakInHouse->getCurrentDialogue(); return DIALOGUE_NULL; });
    test.playTraceFromTestData("issue_1275.mm7", "issue_1275.json");
    // make sure item is returned to inventory
    EXPECT_EQ(heldTape.frontBack(), tape(ITEM_NULL, ITEM_NULL));
    EXPECT_TRUE(heldTape.contains(ITEM_LEATHER_ARMOR));
    // and we reach sell dialog
    EXPECT_TRUE(dialoTape.contains(DIALOGUE_SHOP_SELL));
}

GAME_TEST(Issues, Issue1277) {
    // Crash when press enter on character skills tab
    test.playTraceFromTestData("issue_1277.mm7", "issue_1277.json");
    EXPECT_EQ(current_screen_type, SCREEN_CHARACTERS);
}

GAME_TEST(Issues, Issue1281) {
    // Assert when drinking from THE WELL in Eofol.
    auto acTape = charTapes.ac(0);
    test.playTraceFromTestData("issue_1281.mm7", "issue_1281.json");
    EXPECT_EQ(acTape.delta(), -50); // We've hit the -50 AC branch in the script that used to trigger the assertion.
}

GAME_TEST(Issues, Issue1282) {
    // Picking up an item asserts.
    auto itemTape = tapes.hasItem(ITEM_LEATHER_ARMOR);
    auto totalObjectsTape = tapes.mapItemCount();
    test.playTraceFromTestData("issue_1282.mm7", "issue_1282.json");
    EXPECT_EQ(itemTape, tape(false, true));
    EXPECT_EQ(totalObjectsTape.delta(), -1);
}

GAME_TEST(Issues, Issue1294_1389) {
    // Bow and Blaster recovery times
    // Character::GetAttackRecoveryTime assert when character is using blaster
    auto windowTape = tapes.custom([] { return current_character_screen_window; });
    test.playTraceFromTestData("issue_1294.mm7", "issue_1294.json");

    // Check that we get back to stats screen without asserting
    EXPECT_TRUE(windowTape.contains(WindowType::WINDOW_CharacterWindow_Inventory));
    EXPECT_EQ(windowTape.back(), WindowType::WINDOW_CharacterWindow_Stats);
    // Check min values are used
    EXPECT_EQ(pParty->pCharacters[0].GetAttackRecoveryTime(false), Duration::fromTicks(engine->config->gameplay.MinRecoveryBlasters.value()));
    EXPECT_EQ(pParty->pCharacters[2].GetAttackRecoveryTime(true), Duration::fromTicks(engine->config->gameplay.MinRecoveryRanged.value()));
}

// 1300

GAME_TEST(Issues, Issue1315) {
    // Dying in turn-based mode asserts.
    auto deathsTape = tapes.deaths();
    auto mapTape = tapes.map();
    auto stateTape = tapes.custom([] { return std::tuple(pParty->bTurnBasedModeOn, uGameState); });
    test.playTraceFromTestData("issue_1315.mm7", "issue_1315.json");
    EXPECT_EQ(deathsTape.delta(), +1);
    EXPECT_EQ(mapTape, tape(MAP_LAND_OF_THE_GIANTS, MAP_HARMONDALE)); // Land of the Giants -> Harmondale.
    EXPECT_EQ(stateTape, tape(std::tuple(false, GAME_STATE_PLAYING),
                              std::tuple(true, GAME_STATE_PLAYING),
                              std::tuple(false, GAME_STATE_PARTY_DIED), // Instant switch from turn-based & alive into realtime & dead,
                              std::tuple(false, GAME_STATE_PLAYING)));  // meaning that the party died in turn-based mode.
}

GAME_TEST(Prs, Pr1325) {
    // Trolls drop vials of troll blood.
    auto vialsTape = tapes.mapItemCount(ITEM_REAGENT_VIAL_OF_TROLL_BLOOD);
    auto deadTape = actorTapes.countByState(AIState::Dead);
    test.playTraceFromTestData("pr_1325.mm7", "pr_1325.json");
    EXPECT_EQ(vialsTape.delta(), +6);
    EXPECT_EQ(deadTape.delta(), +84); // Too much armageddon...
}

GAME_TEST(Issues, Issue1331) {
    // "of David" enchanted bows should do double damage against Titans.
    auto hpsTape = actorTapes.hps({31, 33});
    auto deadTape = actorTapes.indicesByState(AIState::Dead);
    auto rngTape = tapes.config(engine->config->debug.TraceRandomEngine);
    test.playTraceFromTestData("issue_1331.mm7", "issue_1331.json");
    EXPECT_EQ(deadTape.frontBack(), tape(std::initializer_list<int>{}, {33})); // One of the titans is dead.

    // Damage as stated in the character sheet is 41-45. Crossbow is 4d2+7. We're using sequential rng, so
    // 4d2+7 will always roll 13, and thus the 41-45 range is effectively compressed into 43-43. With the "of David"
    // enchantment, the 4d2+7 part of the damage is doubled, so max damage is now 43+13=56. And then we also have to
    // multiply the damage by two because the character shoots two arrows at a time.
    //
    // Min damage is so low because titans have physical resistance. Min damage can change between 2 and 3 on retrace.
    // This just means that the Titans' physical resistance was never "lucky enough" to roll the damage down to 1 two
    // times in a row.
    EXPECT_EQ(rngTape, tape(RANDOM_ENGINE_SEQUENTIAL));
    EXPECT_EQ(pParty->pCharacters[2].GetBowItem()->special_enchantment, ITEM_ENCHANTMENT_TITAN_SLAYING);
    EXPECT_EQ(pParty->pCharacters[2].GetRangedDamageString(), "41 - 45");
    auto damageRange = hpsTape.reversed().adjacentDeltas().flattened().filtered([] (int damage) { return damage > 0; }).minMax();
    EXPECT_EQ(damageRange, tape(3, (43 + 13) * 2));
    auto totalDamages = hpsTape.reversed().delta();
    EXPECT_TRUE(std::ranges::all_of(totalDamages, [](int damage) { return damage > 300; })); // Both titans are now pin cushions.
}

GAME_TEST(Issues, Issue1338) {
    // Casting telepathy on an actor and then killing it results in the actor not dropping any gold.
    auto deadTape = actorTapes.indicesByState(AIState::Dead);
    auto statusTape = tapes.statusBar();
    auto goldTape = tapes.gold();
    auto peasantGoldTape = tapes.custom([] { return pActors[18].items[3].goldAmount; });
    test.playTraceFromTestData("issue_1338.mm7", "issue_1338.json");
    EXPECT_EQ(deadTape, tape(std::initializer_list<int>{}, {18}, std::initializer_list<int>{})); // Alive -> Dead -> corpse picked up.
    EXPECT_GT(peasantGoldTape.max(), 0); // Peasant should have had gold generated.
    EXPECT_EQ(goldTape.delta(), peasantGoldTape.max());
    EXPECT_TRUE(statusTape.contains(fmt::format("{} gold", peasantGoldTape.max()))); // Telepathy status message.
    EXPECT_TRUE(statusTape.contains(fmt::format("You found {} gold!", peasantGoldTape.max()))); // Corpse pickup message.
}

GAME_TEST(Issues, Issue1340) {
    // Gold piles in chests are generated with 0 gold.
    auto goldTape = tapes.gold();
    auto mapTape = tapes.map();
    auto statusTape = tapes.statusBar();
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_1340.mm7", "issue_1340.json", [] {
        // Harmondale should not have been visited - check that the dlv data is the same as what's in games.lod.
        Blob saveHarmondale = pSave_LOD->read("d29.dlv");
        Blob origHarmondale = pGames_LOD->read("d29.dlv");
        EXPECT_EQ(saveHarmondale.string_view(), origHarmondale.string_view());
    });
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_CASTLE_HARMONDALE)); // Emerald Isle -> Castle Harmondale. Map change is important because
                                                      // we want to trigger map respawn on first visit.
    EXPECT_TRUE(screenTape.contains(SCREEN_CHEST));
    EXPECT_GT(goldTape.delta(), 0); // Party should have picked some gold from the chest.
    EXPECT_FALSE(statusTape.contains("You found 0 gold!")); // No piles of 0 size.
    for (int gold : goldTape.adjacentDeltas())
        EXPECT_TRUE(statusTape.contains(fmt::format("You found {} gold!", gold)));
}

GAME_TEST(Issues, Issue1341) {
    // Can't steal gold from peasants.
    auto goldTape = tapes.gold();
    auto statusTape = tapes.statusBar();
    auto deadTape = actorTapes.countByState(AIState::Dead);
    test.playTraceFromTestData("issue_1341.mm7", "issue_1341.json");
    EXPECT_GT(goldTape.delta(), 0); // We did steal some gold.
    EXPECT_TRUE(statusTape.contains("Roderick failed to steal anything!")); // We have tried many times.
    EXPECT_TRUE(statusTape.contains(fmt::format("Roderick stole {} gold!", goldTape.delta()))); // And succeeded.
    EXPECT_EQ(deadTape, tape(0)); // No one died in the process.
}

GAME_TEST(Issues, Issue1342) {
    // Gold piles are generated with 0 gold.
    auto goldTape = tapes.gold();
    auto pilesTape = tapes.mapItemCount(ITEM_GOLD_SMALL);
    auto statusTape = tapes.statusBar();
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_1342.mm7", "issue_1342.json");

    // Emerald Isle -> Dragon Cave. Map change is important here because we need to trigger map respawn on first visit.
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_DRAGONS_LAIR));

    EXPECT_GT(goldTape.delta(), 0); // We picked up some gold.
    EXPECT_EQ(pilesTape.max() - pilesTape.back(), 3); // Minus three small gold piles.
    EXPECT_FALSE(statusTape.contains("You found 0 gold!")); // No piles of 0 size.
    for (int gold : goldTape.adjacentDeltas())
        EXPECT_TRUE(statusTape.contains(fmt::format("You found {} gold!", gold)));
}

GAME_TEST(Issues, Issue1362) {
    // HP/SP hint doesn't show when hovering over the SP bar
    game.startNewGame();
    game.tick(1);
    engine->_statusBar->clearEvent();

    game.moveMouse(104, 426);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "45 / 45 Hit Points    0 / 0 Spell Points");
    game.moveMouse(219, 426);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "39 / 39 Hit Points    0 / 0 Spell Points");
    game.moveMouse(333, 426);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "35 / 35 Hit Points    22 / 22 Spell Points");
    game.moveMouse(449, 426);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "22 / 22 Hit Points    36 / 36 Spell Points");
}

GAME_TEST(Issues, Issue1364) {
    // Saving in Arena should display an appropriate status message.
    auto mapTape = tapes.map();
    auto statusTape = tapes.statusBar();
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_1364.mm7", "issue_1364.json");
    EXPECT_EQ(mapTape, tape(MAP_HARMONDALE, MAP_ARENA)); // Harmondale -> Arena.
    EXPECT_TRUE(statusTape.contains("No saving in the Arena")); // Clicking the save button didn't work.
    EXPECT_TRUE(screenTape.contains(SCREEN_HOUSE)); // We have visited the stables.
    EXPECT_TRUE(screenTape.contains(SCREEN_MENU)); // Opened the game menu while in the Arena.
    EXPECT_FALSE(screenTape.contains(SCREEN_SAVEGAME)); // But save menu didn't open on click.
}

GAME_TEST(Issues, Issue1368) {
    // maybeWakeSoloSurvivor() error
    auto canActTape = tapes.custom([] { return pParty->canActCount(); });
    auto sleepTape = tapes.custom([] { return pParty->pCharacters[0].conditions.Has(CONDITION_SLEEP); });
    test.playTraceFromTestData("issue_1368.mm7", "issue_1368.json");
    // No one can act - try waking
    EXPECT_EQ(canActTape.min(), 0);
    // Shouldve been asleep
    EXPECT_TRUE(sleepTape.contains(true));
    // But awake at the end
    EXPECT_EQ(sleepTape.back(), false);
}

GAME_TEST(Issues, Issue1370) {
    // CHARACTER_EXPRESSION_TALK doesn't work
    EXPECT_TRUE(fuzzyEquals(2.53f, pAudioPlayer->getSoundLength(SOUND_EndTurnBasedMode), 0.001f));
    EXPECT_TRUE(fuzzyEquals(2.49f, pAudioPlayer->getSoundLength(static_cast<SoundId>(6480)), 0.001f));

    // Can be any character selected to talk on map change
    auto someonesTalking = tapes.custom([] { for (const auto& ch : pParty->pCharacters) if (ch.expression == CHARACTER_EXPRESSION_TALK) return true; return false; });
    auto talkExprTimeTape = tapes.custom([] { for (const auto& ch : pParty->pCharacters) if (ch.expression == CHARACTER_EXPRESSION_TALK) return ch.uExpressionTimeLength; return Duration(); });
    test.playTraceFromTestData("issue_1370.mm7", "issue_1370.json", [] { engine->config->settings.VoiceLevel.setValue(1); });
    EXPECT_TRUE(someonesTalking.contains(true));
    EXPECT_GT(talkExprTimeTape.max(), 128_ticks);  // Check that we have at least a second of speech to cover all
    EXPECT_EQ(someonesTalking.back(), false);
}

GAME_TEST(Issues, Issue1371) {
    // Collisions - Party struggles to climb stairs to quarter deck
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("issue_1371.mm7", "issue_1371.json");
    EXPECT_GT(zTape.max(), 448);
}

GAME_TEST(Issues, Issue1383) {
    // Negative buying prices for characters with GM Merchant.
    uCurrentlyLoadedLevelType = LEVEL_INDOOR;
    pIndoor->dlv.reputation = 0; // Reputation is used for price calculations.

    Character character;
    character.pActiveSkills[CHARACTER_SKILL_MERCHANT] = CombinedSkillValue(10, CHARACTER_SKILL_MASTERY_GRANDMASTER);
    ItemGen item;
    item.uItemID = ITEM_SPELLBOOK_ARMAGEDDON;
    int gmPrice = PriceCalculator::itemBuyingPriceForPlayer(&character, item.GetValue(), 10.0f);
    EXPECT_EQ(gmPrice, 7500);
    EXPECT_EQ(item.GetValue(), 7500);

    // Also check prices w/o skill, just in case.
    character.pActiveSkills[CHARACTER_SKILL_MERCHANT] = CombinedSkillValue();
    int noobPrice = PriceCalculator::itemBuyingPriceForPlayer(&character, item.GetValue(), 10.0f);
    EXPECT_EQ(noobPrice, 75000);

    // Restore level type.
    uCurrentlyLoadedLevelType = LEVEL_NULL;
}

// 1400

GAME_TEST(Issues, Issue1429a) {
    // Lich regen is broken. Check that having a lich jar regens SP.
    auto mpTape = charTapes.mp(3);
    auto hpTape = charTapes.hp(3);
    auto timeTape = tapes.time();
    test.playTraceFromTestData("issue_1429a.mm7", "issue_1429a.json", [] {
        EXPECT_TRUE(characterHasJar(3, 3)); // Has own jar.
    });
    EXPECT_GE(timeTape.delta(), Duration::fromHours(3));
    EXPECT_EQ(mpTape.delta(), +36); // Wait three hours => +36 sp.
    EXPECT_EQ(hpTape.delta(), 0);
}

GAME_TEST(Issues, Issue1429b) {
    // Lich regen is broken. Check that having another char's lich jar doesn't help with regen.
    auto mpTape = charTapes.mp(3);
    auto hpTape = charTapes.hp(3);
    auto timeTape = tapes.time();
    test.playTraceFromTestData("issue_1429b.mm7", "issue_1429b.json", [] {
        EXPECT_TRUE(!characterHasJar(3, 3)); // Doesn't have own jar.
        EXPECT_TRUE(characterHasJar(3, 2)); // Has #2's jar.
        EXPECT_TRUE(characterHasJar(2, 3)); // #2 has #3's jar.
    });
    EXPECT_GE(timeTape.delta(), Duration::fromHours(1));
    EXPECT_EQ(mpTape.delta(), -24); // Wait an hour => -24 sp.
    EXPECT_EQ(hpTape.delta(), -24); // Wait an hour => -24 hp.
}

GAME_TEST(Issues, Issue1429c) {
    // Lich regen is broken. HP/MP is drained down to half of max HP/MP if the char doesn't have his/her own jar.
    auto mpTape = charTapes.mp(3);
    auto hpTape = charTapes.hp(3);
    auto timeTape = tapes.time();
    auto jarsTape = tapes.hasItem(ITEM_QUEST_LICH_JAR_FULL);
    test.playTraceFromTestData("issue_1429c.mm7", "issue_1429c.json");
    EXPECT_EQ(jarsTape, tape(false)); // No jars.
    EXPECT_GE(timeTape.delta(), Duration::fromHours(23));
    EXPECT_EQ(mpTape.front(), pParty->pCharacters[3].GetMaxMana());
    EXPECT_EQ(mpTape.back(), pParty->pCharacters[3].GetMaxMana() / 2);
    EXPECT_EQ(hpTape.front(), pParty->pCharacters[3].GetMaxHealth());
    EXPECT_EQ(hpTape.back(), pParty->pCharacters[3].GetMaxHealth() / 2);
}

GAME_TEST(Prs, Pr1440) {
    // Frame table search is off by 1 tick.
    TextureFrameTable table;

    TextureFrame frame0;
    frame0.name = "dec33b";
    frame0.animationDuration = 16_ticks;
    frame0.frameDuration = 8_ticks;
    frame0.flags = TEXTURE_FRAME_TABLE_MORE_FRAMES;
    GraphicsImage *tex0 = frame0.GetTexture();

    TextureFrame frame1;
    frame1.name = "dec33d";
    frame1.animationDuration = 0_ticks;
    frame1.frameDuration = 8_ticks;
    frame1.flags = 0;
    GraphicsImage *tex1 = frame1.GetTexture();

    table.textures.push_back(frame0);
    table.textures.push_back(frame1);

    for (int i = 0; i < 8; i++)
        EXPECT_EQ(table.GetFrameTexture(0, Duration::fromTicks(i)), tex0) << i;
    for (int i = 8; i < 16; i++)
        EXPECT_EQ(table.GetFrameTexture(0, Duration::fromTicks(i)), tex1) << i;
    for (int i = 16; i < 24; i++)
        EXPECT_EQ(table.GetFrameTexture(0, Duration::fromTicks(i)), tex0) << i;
    for (int i = 24; i < 32; i++)
        EXPECT_EQ(table.GetFrameTexture(0, Duration::fromTicks(i)), tex1) << i;
}

GAME_TEST(Issues, Issue1447A) {
    // Fire bolt doesn't emit particles in turn based mode
    auto particlesTape = tapes.custom([] { return std::ranges::count_if(engine->particle_engine.get()->pParticles,
                                        [](const Particle &par) { return par.type != ParticleType_Invalid; }); });
    auto turnBasedTape = tapes.custom([] { return pParty->bTurnBasedModeOn; });
    test.playTraceFromTestData("issue_1447A.mm7", "issue_1447A.json");
    EXPECT_EQ(turnBasedTape.back(), true);
    EXPECT_GT(particlesTape.max(), 10);
}

GAME_TEST(Issues, Issue1447B) {
    // Fireball doesn't emit particles in turn based mode
    auto particlesTape = tapes.custom([] { return std::ranges::count_if(engine->particle_engine.get()->pParticles,
                                        [](const Particle& par) { return par.type != ParticleType_Invalid; }); });
    auto turnBasedTape = tapes.custom([] { return pParty->bTurnBasedModeOn; });
    test.playTraceFromTestData("issue_1447B.mm7", "issue_1447B.json");
    EXPECT_EQ(turnBasedTape.back(), true);
    EXPECT_GT(particlesTape.max(), 10);
}

GAME_TEST(Issues, Issue1447C) {
    // Acid blast doesn't emit particles in turn based mode
    auto particlesTape = tapes.custom([] { return std::ranges::count_if(engine->particle_engine.get()->pParticles,
                                        [](const Particle& par) { return par.type != ParticleType_Invalid; }); });
    auto turnBasedTape = tapes.custom([] { return pParty->bTurnBasedModeOn; });
    test.playTraceFromTestData("issue_1447C.mm7", "issue_1447C.json");
    EXPECT_EQ(turnBasedTape.back(), true);
    EXPECT_GT(particlesTape.max(), 10);
}

GAME_TEST(Issues, Issue1449) {
    // Turn-based overlays are broken. Opening hand isn't animated, hourglass isn't animated.
    test.prepareForNextTest(20, RANDOM_ENGINE_MERSENNE_TWISTER); // 50fps, so that we see the animations.
    auto iconsTape = tapes.hudTextures();
    game.startNewGame();
    test.startTaping();
    game.pressKey(PlatformKey::KEY_RETURN); // Enter turn-based mode.
    game.tick();
    game.releaseKey(PlatformKey::KEY_RETURN);
    game.tick(1000 / 20); // Wait 1s.
    for (int i = 0; i < 4; i++) {
        game.pressKey(PlatformKey::KEY_A); // Attack with each char.
        game.tick();
        game.releaseKey(PlatformKey::KEY_A);
        game.tick();
    }
    game.pressKey(PlatformKey::KEY_DOWN); // Walk.
    game.tick(500 / 20);
    game.releaseKey(PlatformKey::KEY_DOWN);
    game.tick(500 / 20); // Wait 0.5s.
    test.stopTaping();

    // This is a test for the taping framework itself. The first & last elements in a tape recorded from a call observer
    // shouldn't be empty.
    EXPECT_FALSE(iconsTape.front().empty());
    EXPECT_FALSE(iconsTape.back().empty());

    // Then we just check that the necessary animation frames were actually displayed.
    auto flatIcons = iconsTape.flattened();
    EXPECT_TRUE(flatIcons.containsAll("ia01-001", "ia01-002", "ia01-003", "ia01-004", "ia01-005", "ia01-006",
                                      "ia01-007", "ia01-008", "ia01-009", "ia01-010")); // Opening hand animation.
    EXPECT_TRUE(flatIcons.containsAll("ia01-011", "ia01-012", "ia01-013", "ia01-014")); // Fingers.

    // Hourglass animation is 10 frames long, we only see the first 5 frames.
    EXPECT_TRUE(flatIcons.containsAll("ia02-001", "ia02-002", "ia02-003", "ia02-004", "ia02-005"));
}

GAME_TEST(Issues, Issue1454) {
    // Map hotkey doesn't close the map
    game.startNewGame();
    game.tick(1);
    game.pressAndReleaseKey(PlatformKey::KEY_M);
    game.tick(1);
    EXPECT_EQ(current_screen_type, ScreenType::SCREEN_BOOKS);
    EXPECT_EQ(pGUIWindow_CurrentMenu->eWindowType, WindowType::WINDOW_MapsBook);
    game.pressAndReleaseKey(PlatformKey::KEY_M);
    game.tick(1);
    EXPECT_EQ(current_screen_type, ScreenType::SCREEN_GAME);
    EXPECT_EQ(pGUIWindow_CurrentMenu, nullptr);
}

GAME_TEST(Issues, Issue1457) {
    // Ghost items - able to pick up items across the map
    auto itemsTape = tapes.totalItemCount();
    auto mapItemsTape = tapes.mapItemCount();
    test.playTraceFromTestData("issue_1457.mm7", "issue_1457.json");
    EXPECT_EQ(itemsTape.size(), 1);
    EXPECT_EQ(mapItemsTape.size(), 1);
}

GAME_TEST(Issues, Issue1462) {
    // Can dark sacrifice the same npc 4 times.
    auto hirelingsTape = tapes.totalHirelings();
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_1462.mm7", "issue_1462.json");
    EXPECT_EQ(hirelingsTape, tape(1, 0)); // We did sacrifice the last one.
    EXPECT_EQ(statusTape, tape("", "Select Target", "", "Select Target", "Spell failed")); // Sacrifice was cast twice, second cast failed.
}

GAME_TEST(Issues, Issue1464) {
    // Can talk to the npc being dark-sacrificed.
    // Talking to the last NPC while he's being dark-sacrificed asserts.
    auto screenTape = tapes.screen();
    auto hirelingsTape = tapes.totalHirelings();
    test.playTraceFromTestData("issue_1464.mm7", "issue_1464.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME)); // No SCREEN_NPC_DIALOG.
    EXPECT_EQ(hirelingsTape, tape(1, 0)); // We did sacrifice the last one.
}

GAME_TEST(Issues, Issue1466) {
    // Assert in spellbook popups
    auto messageBoxesTape = tapes.messageBoxes();
    auto messageBoxesBody = tapes.allGUIWindowsText();
    test.playTraceFromTestData("issue_1466.mm7", "issue_1466.json");
    // message box body text was displayed.
    auto flatMessageBoxes = messageBoxesTape.flattened();
    auto flatMessageBoxesBody = messageBoxesBody.flattened();
    EXPECT_GT(flatMessageBoxes.size(), 0);
    EXPECT_GT(flatMessageBoxesBody.filtered([](const auto& s) { return s.starts_with("Inferno burns all"); }).size(), 0);
    EXPECT_FALSE(pParty->pCharacters[0].HasSkill(CHARACTER_SKILL_FIRE));
    EXPECT_EQ(current_screen_type, SCREEN_GAME);
}

GAME_TEST(Issues, Issue1467) {
    // Dark sacrificing an empty npc slot asserts.
    auto hirelingsTape = tapes.totalHirelings();
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_1467.mm7", "issue_1467.json");
    EXPECT_EQ(hirelingsTape, tape(1, 0)); // We did sacrifice the last one.
    EXPECT_EQ(statusTape, tape("", "Select Target", "", "Select Target", "")); // Sacrifice was cast twice. Also, no "Spell Failed".
}

GAME_TEST(Issues, Issue1471) {
    // Waiting until dawn doesn't recharge Armageddon.
    auto armageddonTape = tapes.custom([] { return pParty->pCharacters[0].uNumArmageddonCasts; });
    auto timeTape = tapes.time();
    test.playTraceFromTestData("issue_1471.mm7", "issue_1471.json");
    EXPECT_EQ(armageddonTape, tape(4, 0, 1)); // blocked/ reset/ cast
    EXPECT_GT(timeTape.back().toCivilTime().day, timeTape.front().toCivilTime().day); // Time should have passed 3am reset time
    EXPECT_GT(timeTape.back().toCivilTime().hour, 3);
}

GAME_TEST(Issues, Issue1475) {
    // Warlock mana regen from Baby Dragon regens mana for dead characters.
    auto timeTape = tapes.time();
    auto conditionTape = charTapes.condition(3);
    auto hpTape = charTapes.hp(3);
    auto mpTape = charTapes.mp(3);
    test.playTraceFromTestData("issue_1475.mm7", "issue_1475.json", [] {
        EXPECT_EQ(pParty->pCharacters[3].classType, CLASS_WARLOCK);
        EXPECT_TRUE(PartyHasDragon()); // Dragon provides mana regen.
    });
    EXPECT_GT(timeTape.delta(), Duration::fromHours(1));
    EXPECT_EQ(conditionTape, tape(CONDITION_DEAD));
    EXPECT_EQ(hpTape, tape(-44)); // Very dead.
    EXPECT_EQ(mpTape, tape(0)); // No mana regen.
}

GAME_TEST(Issues, Issue1476) {
    // Fireball self-damage doesn't always work
    auto hpTape = tapes.totalHp();
    auto exprTape = tapes.custom([] { return pParty->_delayedReactionSpeech; });
    test.playTraceFromTestData("issue_1476.mm7", "issue_1476.json");
    EXPECT_TRUE(exprTape.contains(SPEECH_DAMAGED_PARTY));
    EXPECT_LT(hpTape.back(), hpTape.front());
}

GAME_TEST(Issues, Issue1478) {
    // Invalid sprites are used for hireling left/right buttons in pressed state in dark skin.
    test.loadGameFromTestData("issue_1478.mm7");
    game.tick();
    EXPECT_EQ(pParty->alignment, PartyAlignment_Evil);
    // Check buttons have correct texture set.
    const auto quickRefButton = std::ranges::find_if(pPrimaryWindow->vButtons, [](const GUIButton* but) { return but->msg == UIMSG_QuickReference; });
    EXPECT_EQ((*quickRefButton)->vTextures[0]->GetName(), "ib-m3d-c");
    const auto npcLeftButton = std::ranges::find_if(pPrimaryWindow->vButtons, [](const GUIButton* but) { return but->msg == UIMSG_ScrollNPCPanel && but->msg_param == 0; });
    EXPECT_EQ((*npcLeftButton)->vTextures[0]->GetName(), "ib-npcld-c");
}

GAME_TEST(Issues, Issue1479) {
    // Crash when identifying Chaos Hydra with ID Monster skill.
    auto expressionTape = charTapes.expression(2);
    test.playTraceFromTestData("issue_1479.mm7", "issue_1479.json");
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_MONSTER_ID).mastery(), CHARACTER_SKILL_MASTERY_GRANDMASTER);
    EXPECT_TRUE(expressionTape.contains(CHARACTER_EXPRESSION_47)); // Reaction to strong monster id.
}

GAME_TEST(Issues, Issue1482) {
    // Regeneration sets HP to 15.
    auto regenTape = charTapes.hasBuff(0, CHARACTER_BUFF_REGENERATION);
    auto hpTape = charTapes.hp(0);
    auto timeTape = tapes.time();
    test.playTraceFromTestData("issue_1482.mm7", "issue_1482.json");
    EXPECT_EQ(regenTape, tape(true));
    EXPECT_EQ(hpTape.delta(), +50); // +50 hp every 5min from GM regeneration buff.
    EXPECT_GT(timeTape.delta(), Duration::fromMinutes(5));
    EXPECT_LT(timeTape.delta(), Duration::fromMinutes(10));
}

GAME_TEST(Issues, Issue1489) {
    // Cannot equip amulets or gauntlets
    auto bootTape = tapes.custom([] { auto item = pParty->pCharacters[0].GetBootItem(); if (!item) return ITEM_NULL; return item->uItemID; });
    auto helmetTape = tapes.custom([] {  auto item = pParty->pCharacters[0].GetHelmItem(); if (!item) return ITEM_NULL; return item->uItemID; });
    auto beltTape = tapes.custom([] {  auto item = pParty->pCharacters[0].GetBeltItem(); if (!item) return ITEM_NULL; return item->uItemID; });
    auto cloakTape = tapes.custom([] {  auto item = pParty->pCharacters[0].GetCloakItem(); if (!item) return ITEM_NULL; return item->uItemID; });
    auto gauntletTape = tapes.custom([] {  auto item = pParty->pCharacters[0].GetGloveItem(); if (!item) return ITEM_NULL; return item->uItemID;; });
    auto amuletTape = tapes.custom([] {  auto item = pParty->pCharacters[0].GetAmuletItem(); if (!item) return ITEM_NULL; return item->uItemID;; });
    test.playTraceFromTestData("issue_1489.mm7", "issue_1489.json");

    for (const auto& character : pParty->pCharacters) {
        EXPECT_TRUE(character.HasSkill(CHARACTER_SKILL_MISC));
    }
    // Check items were removed and re-equipped
    EXPECT_EQ(bootTape.front(), bootTape.back());
    EXPECT_TRUE(bootTape.contains(ITEM_NULL));
    EXPECT_EQ(helmetTape.front(), helmetTape.back());
    EXPECT_TRUE(helmetTape.contains(ITEM_NULL));
    EXPECT_EQ(beltTape.front(), beltTape.back());
    EXPECT_TRUE(beltTape.contains(ITEM_NULL));
    EXPECT_EQ(cloakTape.front(), cloakTape.back());
    EXPECT_TRUE(cloakTape.contains(ITEM_NULL));
    EXPECT_EQ(gauntletTape.front(), gauntletTape.back());
    EXPECT_TRUE(gauntletTape.contains(ITEM_NULL));
    EXPECT_EQ(amuletTape.front(), amuletTape.back());
    EXPECT_TRUE(amuletTape.contains(ITEM_NULL));
}
