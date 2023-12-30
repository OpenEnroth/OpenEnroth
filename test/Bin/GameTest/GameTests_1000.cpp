#include <unordered_set>

#include "Testing/Game/GameTest.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Engine/Graphics/TextureFrameTable.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"
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
    EXPECT_EQ(mapTape, tape("out02.odm", "d05.blv")); // Harmondale -> Arena.
    EXPECT_TRUE(dialogueTape.contains(DIALOGUE_ARENA_SELECT_CHAMPION));
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
    Duration frameTicks = Duration::fromRealtimeMilliseconds(15 + (1_ticks).toRealtimeMilliseconds() - 1 /* Round up! */);
    EXPECT_GE(ticks, 144_ticks - frameTicks);
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
    EXPECT_TRUE(loc.contains("out01.odm")); // make it back to emerald
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
    test.playTraceFromTestData("issue_1251b.mm7", "issue_1251b.json");
    EXPECT_EQ(charmedActors.delta(), 3);
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
    EXPECT_EQ(mapTape, tape("out12.odm", "out02.odm")); // Land of the Giants -> Harmondale.
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
    test.playTraceFromTestData("issue_1331.mm7", "issue_1331.json");
    EXPECT_EQ(deadTape.frontBack(), tape(std::initializer_list<int>{}, {31, 33})); // Check that titans are dead.

    // Damage as stated in the character sheet is 41-45. Crossbow is 4d2+7. Because of how non-random engine works,
    // 4d2+7 will always roll 13, and thus the 41-45 range is effectively compressed into 43-43.
    //
    // With the "of David" enchantment, the 4d2+7 part of the damage is doubled, so max damage is now 43+13=56.
    //
    // Min damage is so low because titans have physical resistance. And then we also have to multiply the damage by
    // two because the character shoots two arrows at a time.
    EXPECT_EQ(pParty->pCharacters[2].GetBowItem()->special_enchantment, ITEM_ENCHANTMENT_TITAN_SLAYING);
    EXPECT_EQ(pParty->pCharacters[2].GetRangedDamageString(), "41 - 45");
    auto damageRange = hpsTape.reversed().adjacentDeltas().flattened().filtered([] (int damage) { return damage > 0; }).minMax();
    // 2 -> 3 change here can happen. This just means that the Titans' physical resistance was never "lucky enough" to roll the damage down to 1 two times in a row.
    EXPECT_EQ(damageRange, tape(/*1 * 2*/ 3, (43 + 13) * 2));
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
    test.playTraceFromTestData("issue_1340.mm7", "issue_1340.json");
    EXPECT_EQ(mapTape, tape("out01.odm", "d29.blv")); // Emerald Isle -> Castle Harmondale. Map change is important because
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
    auto peasantGoldTape = tapes.custom([] { return pActors[6].items[3].goldAmount; });
    auto statusTape = tapes.statusBar();
    auto deadTape = actorTapes.countByState(AIState::Dead);
    test.playTraceFromTestData("issue_1341.mm7", "issue_1341.json");
    EXPECT_GT(goldTape.delta(), 0); // We did steal some gold.
    EXPECT_EQ(peasantGoldTape.max(), goldTape.delta()); // And we did steal it from this peasant.
    EXPECT_TRUE(statusTape.contains("Roderick failed to steal anything!")); // We have tried many times.
    EXPECT_TRUE(statusTape.contains(fmt::format("Roderick stole {} gold!", peasantGoldTape.max()))); // And succeeded.
    EXPECT_EQ(deadTape, tape(0)); // No one died in the process.
}

GAME_TEST(Issues, Issue1342) {
    // Gold piles are generated with 0 gold.
    auto goldTape = tapes.gold();
    auto pilesTape = tapes.mapItemCount(ITEM_GOLD_SMALL);
    auto statusTape = tapes.statusBar();
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_1342.mm7", "issue_1342.json");
    EXPECT_EQ(mapTape, tape("out01.odm", "d28.blv")); // Emerald Isle -> Dragon Cave. Map change is important here
    // because we need to trigger map respawn on first visit.
    EXPECT_GT(goldTape.delta(), 0); // We picked up some gold.
    EXPECT_EQ(pilesTape, tape(0, 10, 9, 8, 7)); // Minus three small gold piles.
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
    EXPECT_EQ(mapTape, tape("out02.odm", "d05.blv")); // Harmondale -> Arena.
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

    auto exprTape = tapes.custom([] { return pParty->pCharacters[2].expression; });
    auto exprTimeTape = tapes.custom([] { return pParty->pCharacters[2].uExpressionTimeLength; });
    test.playTraceFromTestData("issue_1370.mm7", "issue_1370.json", [] { engine->config->settings.VoiceLevel.setValue(1); });
    EXPECT_TRUE(exprTape.contains(CHARACTER_EXPRESSION_TALK));
    EXPECT_TRUE(exprTimeTape.contains(318_ticks));  // 2.49 * 128
    EXPECT_EQ(exprTape.back(), CHARACTER_EXPRESSION_NORMAL);
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

GAME_TEST(Issues, Issue1467) {
    // Dark sacrificing an empty npc slot asserts.
    auto hirelingsTape = tapes.totalHirelings();
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_1467.mm7", "issue_1467.json");
    EXPECT_EQ(hirelingsTape, tape(1, 0)); // We did sacrifice the last one.
    EXPECT_EQ(statusTape, tape("", "Select Target", "", "Select Target", "")); // Sacrifice was cast twice. Also, no "Spell Failed".
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
