#include <unordered_set>
#include <string>
#include <algorithm>
#include <utility>
#include <vector>

#include "Testing/Game/GameTest.h"

#include "Arcomage/Arcomage.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIProgressBar.h"

#include "Engine/Tables/ItemTable.h"
#include "Engine/SaveLoad.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Party.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Spells/CastSpellInfo.h"

#include "Utility/ScopeGuard.h"

#include "GameTestCommon.h"


// 100

GAME_TEST(Issues, Issue123) {
    // Party falls when flying
    test.playTraceFromTestData("issue_123.mm7", "issue_123.json");
    // check party is still in the air
    EXPECT_GT(pParty->pos.z, 512);
}

GAME_TEST(Issues, Issue125) {
    // check that fireballs hurt party
    auto healthTape = tapes.totalHp();
    test.playTraceFromTestData("issue_125.mm7", "issue_125.json");
    EXPECT_LT(healthTape.delta(), 0);
}

GAME_TEST(Issues, Issue159) {
    // Exception when entering Tidewater Caverns
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_159.mm7", "issue_159.json");
    EXPECT_EQ(mapTape, tape(MAP_TATALIA, MAP_TIDEWATER_CAVERNS, MAP_TATALIA));
}

GAME_TEST(Issues, Issue163) {
    // Testing that pressing the Load Game button doesn't crash even if the 'saves' folder doesn't exist.
    ufs->remove("saves");
    EXPECT_FALSE(ufs->exists("saves"));

    game.pressGuiButton("MainMenu_LoadGame"); // Shouldn't crash.
    game.tick(10);
    EXPECT_TRUE(saveLoadMenu()->slots().empty()); // No saves listed.

    game.pressGuiButton("LoadMenu_Load");
    game.tick(10);
    EXPECT_EQ(current_screen_type, SCREEN_LOADGAME);
    EXPECT_FALSE(pGameLoadingUI_ProgressBar->IsActive()); // Load button shouldn't do anything.
}

GAME_TEST(Issues, Issue198) {
    // Check that items can't end up out of bounds of character's inventory.
    test.playTraceFromTestData("issue_198.mm7", "issue_198.json");

    for (const Character &character : pParty->pCharacters) {
        for (InventoryConstEntry entry : character.inventory.entries()) {
            Recti geometry = entry.geometry();

            EXPECT_LE(geometry.x + geometry.w, character.inventory.gridSize().w);
            EXPECT_LE(geometry.y + geometry.h, character.inventory.gridSize().h);
        }
    }
}

// 200

GAME_TEST(Issues, Issue201) {
    // Unhandled EVENT_ShowMovie in Event Processor
    auto healthTape = tapes.totalHp();
    auto mapTape = tapes.map();
    auto daysTape = tapes.custom([] { return pParty->GetPlayingTime().toDays(); });
    test.playTraceFromTestData("issue_201.mm7", "issue_201.json");
    EXPECT_GT(healthTape.delta(), 0); // Party should heal.
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_HARMONDALE)); // Emerald isle to Harmondale.
    EXPECT_EQ(daysTape.delta(), 7); // Time should advance by a week.
}

GAME_TEST(Issues, Issue202) {
    // Judge doesn't move to house and stays with the party.
    auto hirelingsTape = tapes.custom([] { return pParty->CountHirelings(); });
    auto alignmentTape = tapes.custom([] { return pParty->alignment; });
    test.playTraceFromTestData("issue_202.mm7", "issue_202.json");
    EXPECT_EQ(hirelingsTape.delta(), -1); // Judge shouldn't be with party anymore.
    EXPECT_EQ(alignmentTape, tape(PartyAlignment_Neutral, PartyAlignment_Evil)); // Party should turn evil.
}

GAME_TEST(Issues, Issue203) {
    // Judge's "I lost it" shouldn't crash.
    test.playTraceFromTestData("issue_203.mm7", "issue_203.json");
}

GAME_TEST(Issues, Issue211) {
    // Crash during accidental ok double click
    test.playTraceFromTestData("issue_211.mm7", "issue_211.json");
}

GAME_TEST(Issues, Issue223) {
    // Fire and air resistance not resetting between games
    auto fireTape = charTapes.resistances(ATTRIBUTE_RESIST_FIRE);
    auto airTape = charTapes.resistances(ATTRIBUTE_RESIST_AIR);
    test.playTraceFromTestData("issue_223.mm7", "issue_223.json");
    // expect normal resistances after restart 55-00-00-00.
    EXPECT_EQ(fireTape.frontBack(), tape({280, 262, 390, 241}, {5, 0, 0, 0}));
    EXPECT_EQ(airTape.frontBack(), tape({389, 385, 385, 381}, {5, 0, 0, 0}));
}

GAME_TEST(Issues, Issue238) {
    // Party vertical flight speed doesnt use frame pacing
    test.playTraceFromTestData("issue_238.mm7", "issue_238.json");
    EXPECT_LT(pParty->pos.z, 2500);
}

GAME_TEST(Issues, Issue248) {
    // Crash in NPC dialog.
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_248.mm7", "issue_248.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_NPC_DIALOGUE, SCREEN_GAME));
}

GAME_TEST(Issues, Issue268_939) {
    // Crash in ODM_GetFloorLevel.
    test.playTraceFromTestData("issue_268.mm7", "issue_268.json");

    // #939: Quick reference doesn't match vanilla.
    // hp
    EXPECT_EQ(pParty->pCharacters[0].GetHealth(), 71);
    EXPECT_EQ(pParty->pCharacters[1].GetHealth(), 80);
    EXPECT_EQ(pParty->pCharacters[2].GetHealth(), 154);
    EXPECT_EQ(pParty->pCharacters[3].GetHealth(), 169);
    // sp
    EXPECT_EQ(pParty->pCharacters[0].GetMana(), 0);
    EXPECT_EQ(pParty->pCharacters[1].GetMana(), 0);
    EXPECT_EQ(pParty->pCharacters[2].GetMana(), 55);
    EXPECT_EQ(pParty->pCharacters[3].GetMana(), 19);
    // ac
    EXPECT_EQ(pParty->pCharacters[0].GetActualAC(), 126);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAC(), 77);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAC(), 82);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAC(), 66);
    // attack
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(false), 30);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAttack(false), 37);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAttack(false), 29);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAttack(false), 9);
    // dmg
    EXPECT_EQ(pParty->pCharacters[0].GetMeleeDamageString(), "35 - 41");
    EXPECT_EQ(pParty->pCharacters[1].GetMeleeDamageString(), "39 - 55");
    EXPECT_EQ(pParty->pCharacters[2].GetMeleeDamageString(), "35 - 39");
    EXPECT_EQ(pParty->pCharacters[3].GetMeleeDamageString(), "Wand");
    // shoot
    EXPECT_EQ(pParty->pCharacters[0].GetRangedAttack(), 18);
    EXPECT_EQ(pParty->pCharacters[1].GetRangedAttack(), 23);
    EXPECT_EQ(pParty->pCharacters[2].GetRangedAttack(), 21);
    EXPECT_EQ(pParty->pCharacters[3].GetRangedAttack(), 9); // was 17 in vanilla see #1927
    // dmg
    EXPECT_EQ(pParty->pCharacters[0].GetRangedDamageString(), "9 - 14");
    EXPECT_EQ(pParty->pCharacters[1].GetRangedDamageString(), "11 - 16");
    EXPECT_EQ(pParty->pCharacters[2].GetRangedDamageString(), "11 - 16");
    EXPECT_EQ(pParty->pCharacters[3].GetRangedDamageString(), "Wand");
    // skills
    auto checkSkills = [](std::initializer_list<std::pair<int, int>> numSkillPairs) {
        for (auto pair : numSkillPairs) {
            int pSkillsCount = 0;
            for (Skill j : allVisibleSkills()) {
                if (pParty->pCharacters[pair.first].pActiveSkills[j]) {
                    ++pSkillsCount;
                }
            }
            EXPECT_EQ(pSkillsCount, pair.second);
        }
    };
    // NB vanilla gets wrong count for characters - doesnt count learning
    checkSkills({ {0, 13}, {1, 11}, {2, 20}, {3, 11} });
}

GAME_TEST(Issues, Issue271) {
    // Party shouldn't yell when landing from flight.
    auto expressionTape = charTapes.portrait(1);
    auto landingTape = tapes.custom([] { return !!(pParty->uFlags & PARTY_FLAG_LANDING); });
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("issue_271.mm7", "issue_271.json");
    EXPECT_MISSES(expressionTape, PORTRAIT_FEAR);
    EXPECT_EQ(landingTape, tape(false, true));
    EXPECT_LT(zTape.delta(), -1000);
}

GAME_TEST(Issues, Issue272a) {
    // Controls menu bugs - resetting controls doesn't work.
    auto rightTape = tapes.config(engine->config->keybindings.Right);
    test.playTraceFromTestData("issue_272a.mm7", "issue_272a.json");
    EXPECT_EQ(rightTape, tape(PlatformKey::KEY_RIGHT, PlatformKey::KEY_H, PlatformKey::KEY_RIGHT)); // Pressing 'default' resets keys.
}

GAME_TEST(Issues, Issue272b) {
    // Check you cant leave menu with conflicting keys.
    test.playTraceFromTestData("issue_272b.mm7", "issue_272b.json");
    EXPECT_EQ(current_screen_type, SCREEN_KEYBOARD_OPTIONS);
    EXPECT_EQ(engine->_statusBar->get(), "Please resolve all key conflicts!");
}

GAME_TEST(Issues, Issue289a) {
    // Collisions climbing walls in dragon cave.
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("issue_289a.mm7", "issue_289a.json");
    // Shouldnt climb too high
    EXPECT_LT(zTape.max(), 350);
}

GAME_TEST(Issues, Issue289b) {
    // Collisions clipping through surfaces.
    float rx = 12032.0f, ry = 3720.0f;
    float rr = 12224.0f, rt = 3912.0f;
    auto distTape = tapes.custom([&] {
        // test party distance to square of pillar
        float dx = std::max({ rx - pParty->pos.x, 0.0f, pParty->pos.x - rr });
        float dy = std::max({ ry - pParty->pos.y, 0.0f, pParty->pos.y - rt });
        return (dx*dx + dy*dy);
    });
    test.playTraceFromTestData("issue_289b.mm7", "issue_289b.json");
    EXPECT_GT(distTape.min(), pParty->radius * pParty->radius);
}

GAME_TEST(Issues, Issue290) {
    // Town Hall bugs.
    auto fineTape = tapes.custom([] { return pParty->GetFine(); });
    auto goldTape = tapes.gold();
    test.playTraceFromTestData("issue_290.mm7", "issue_290.json");
    EXPECT_EQ(fineTape.delta(), -1000);
    EXPECT_EQ(goldTape.delta(), -1000);
}

GAME_TEST(Issues, Issue292a) {
    // Collisions - slip through signs
    auto yTape = tapes.custom([] { return pParty->pos.y; });
    test.playTraceFromTestData("issue_292a.mm7", "issue_292a.json");
    // Shouldnt pass through sign
    EXPECT_LT(yTape.max(), 4855.0f);
}

GAME_TEST(Issues, Issue292b) {
    // Collisions - becoming stuck
    float rx = -3028.0f, ry = 5815.0f;
    float rr = -2894.0f, rt = 6090.0f;
    auto distTape = tapes.custom([&] {
        // test party distance to square of altar
        float dx = std::max({ rx - pParty->pos.x, 0.0f, pParty->pos.x - rr });
        float dy = std::max({ ry - pParty->pos.y, 0.0f, pParty->pos.y - rt });
        return (dx * dx + dy * dy);
        });
    test.playTraceFromTestData("issue_292b.mm7", "issue_292b.json");
    EXPECT_GT(distTape.min(), pParty->radius * pParty->radius);
}

GAME_TEST(Issues, Issue293a) {
    // Test that barrels in castle Harmondale work and can be triggered only once, and that trash piles work,
    // give an item once, but give disease indefinitely.
    auto totalItemsTape = tapes.totalItemCount();
    auto conditionsTape = charTapes.conditions();
    test.playTraceFromTestData("issue_293a.mm7", "issue_293a.json", [] {
        EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_MIGHT], 30);
        EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_INTELLIGENCE], 5);
        EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_PERSONALITY], 5);
        EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_ENDURANCE], 13);
        EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_SPEED], 14);
        EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_ACCURACY], 13);
        EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_LUCK], 7);
    });

    EXPECT_EQ(totalItemsTape.delta(), +1);
    EXPECT_EQ(conditionsTape.frontBack(), tape({CONDITION_GOOD, CONDITION_GOOD, CONDITION_GOOD, CONDITION_GOOD},
                                               {CONDITION_DISEASE_WEAK, CONDITION_DISEASE_WEAK, CONDITION_DISEASE_WEAK, CONDITION_DISEASE_WEAK}));
    EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_MIGHT], 30);
    EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_INTELLIGENCE], 7); // +2
    EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_PERSONALITY], 5);
    EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_ENDURANCE], 13);
    EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_SPEED], 14);
    EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_ACCURACY], 15); // +2
    EXPECT_EQ(pParty->pCharacters[0]._stats[ATTRIBUTE_LUCK], 7);
}

GAME_TEST(Issues, Issue293b) {
    // Test that table food in castle Harmondale is pickable only once and gives apples.
    auto foodTape = tapes.food();
    auto totalItemsTape = tapes.totalItemCount();
    auto hasAppleTape = tapes.hasItem(ITEM_RED_APPLE);
    test.playTraceFromTestData("issue_293b.mm7", "issue_293b.json");
    EXPECT_EQ(foodTape, tape(7));
    EXPECT_EQ(totalItemsTape, tape(18, 19));
    EXPECT_EQ(hasAppleTape, tape(false, true)); // Got an apple!
}

GAME_TEST(Issues, Issue293c) {
    // Test that cauldrons work, and work only once. The cauldron tested is in the Barrow Downs.
    test.playTraceFromTestData("issue_293c.mm7", "issue_293c.json", [] {
        EXPECT_EQ(pParty->pCharacters[0].sResAirBase, 230); // An interesting save we have here.
        EXPECT_EQ(pParty->pCharacters[1].sResAirBase, 50);
        EXPECT_EQ(pParty->pCharacters[2].sResAirBase, 24);
        EXPECT_EQ(pParty->pCharacters[3].sResAirBase, 18);
    });

    EXPECT_EQ(pParty->pCharacters[0].sResAirBase, 230);
    EXPECT_EQ(pParty->pCharacters[1].sResAirBase, 52); // +2
    EXPECT_EQ(pParty->pCharacters[2].sResAirBase, 24);
    EXPECT_EQ(pParty->pCharacters[3].sResAirBase, 18);
}

GAME_TEST(Issues, Issue294) {
    // Testing that party auto-casting shrapnel successfully targets rats & kills them, gaining experience.
    auto deadActorsTape = actorTapes.countByState(Dead);
    auto ratStateTape = actorTapes.aiState(79);
    auto ratPositionTape = tapes.custom([] { return pActors[79].pos; });
    auto recoveringTape = charTapes.areRecovering();
    auto spritesTape = tapes.sprites();
    test.playTraceFromTestData("issue_294.mm7", "issue_294.json");

    // Only the 4th char acted.
    EXPECT_EQ(recoveringTape.slice(0).unique(), tape(false));
    EXPECT_EQ(recoveringTape.slice(1).unique(), tape(false));
    EXPECT_EQ(recoveringTape.slice(2).unique(), tape(false));
    EXPECT_EQ(recoveringTape.slice(3).unique(), tape(false, true, false));

    // Sharpmetal was cast.
    EXPECT_CONTAINS(spritesTape.flatten(), SPRITE_SPELL_DARK_SHARPMETAL_IMPACT);

    // Giant rat died after a sharpmetal cast from character #4.
    EXPECT_EQ(deadActorsTape.delta(), +1);
    EXPECT_EQ(ratStateTape.frontBack(), tape(Standing, Dead));

    // Rat didn't move much.
    Vec3f positionJitter = BBoxf::forPoints(ratPositionTape).size();
    EXPECT_LT(positionJitter.x, 100);
    EXPECT_LT(positionJitter.y, 100);
    EXPECT_LT(positionJitter.z, 100);
}

// 300

GAME_TEST(Prs, Pr314_742) {
    // Check that character creating menu works.
    // Trace pretty much presses all the buttons and opens all the popups possible.
    test.playTraceFromTestData("pr_314.mm7", "pr_314.json");

    for (int i = 0; i < 4; i++)
        EXPECT_EQ(pParty->pCharacters[i]._stats[ATTRIBUTE_LUCK], 20);

    EXPECT_EQ(pParty->pCharacters[0].classType, CLASS_MONK);
    EXPECT_EQ(pParty->pCharacters[1].classType, CLASS_THIEF);
    EXPECT_EQ(pParty->pCharacters[2].classType, CLASS_RANGER);
    EXPECT_EQ(pParty->pCharacters[3].classType, CLASS_CLERIC);
    EXPECT_EQ(pParty->pCharacters[0].GetRace(), RACE_ELF);
    EXPECT_EQ(pParty->pCharacters[1].GetRace(), RACE_ELF);
    EXPECT_EQ(pParty->pCharacters[2].GetRace(), RACE_GOBLIN);
    EXPECT_EQ(pParty->pCharacters[3].GetRace(), RACE_ELF);

    // #742: Check that party qbits are set even if we press Clear when creating a party.
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_RED_POTION_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_SEASHELL_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_LONGBOW_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_PLATE_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_LUTE_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_HAT_ACTIVE));
}

GAME_TEST(Issues, Issue315) {
    test.loadGameFromTestData("issue_315.mm7");
    game.startNewGame(); // This shouldn't crash.
}

GAME_TEST(Issues, Issue331_679) {
    // Assert when traveling by horse caused by out of bound access to pObjectList->pObjects.
    auto goldTape = tapes.gold();
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_331.mm7", "issue_331.json");
    EXPECT_EQ(mapTape, tape(MAP_TULAREAN_FOREST, MAP_HARMONDALE, MAP_TULAREAN_FOREST)); // We did travel.

    // #679: Loading autosave after travelling by stables / boat results in gold loss.
    EXPECT_EQ(goldTape.delta(), 0);
    EXPECT_LT(goldTape.min(), goldTape.front()); // We did spend money.
}

GAME_TEST(Prs, Pr347) {
    // Testing that shops work.
    auto itemsTape = tapes.totalItemCount();
    auto goldTape = tapes.gold();
    test.playTraceFromTestData("pr_347.mm7", "pr_347.json");
    EXPECT_GT(itemsTape.delta(), 0); // Bought smth.
    EXPECT_LT(goldTape.delta(), 0); // Spent on items.
}

GAME_TEST(Issues, Issue355_2520) {
    // EVENT_CastSpell damage to characters (fire bolts in temple of the moon for example) doesnt match GOG.
    // GOG: 6-2. OpenEnroth: 9-5.
    auto healthTape = charTapes.hps();
    auto fireboltColZ = tapes.custom([]() {
        std::vector<float> zValues;
        for (const auto& obj : pSpriteObjects) {
            if (obj.spriteId == SPRITE_SPELL_FIRE_FIRE_BOLT_IMPACT) {
                zValues.push_back(obj.vPosition.z);
            }
        }
        return zValues;
    });

    test.playTraceFromTestData("issue_355.mm7", "issue_355.json");
    auto damageRange = healthTape.reverse().adjacentDeltas().flatten().filter([] (int damage) { return damage > 0; }).minMax();
    // 2d3+0 with a sequential engine can't roll 2 or 6, so all values should be in [3, 5]. Luck roll can drop this to 1/2...
    EXPECT_EQ(damageRange, tape(2 /*1*/, 5));

    // 2520: Fire traps in Temple of the Moon aim at the floor instead of the opposite walls
    // floor level is -96 so make sure everything is above that
    EXPECT_GE(fireboltColZ.flatten().min(), -95);
}

GAME_TEST(Issues, Issue388) {
    // TODO(captainurist): this one doesn't obey the unlimited FPS when retracing. Implement properly.
    // Testing that Arcomage works.
    // Trace enters tavern, plays arcomage, plays a couple of cards then exits and leaves tavern.
    int oldfpslimit = pArcomageGame->_targetFPS;
    pArcomageGame->_targetFPS = 500;

    auto arcomageTape = tapes.custom([] { return !!pArcomageGame->_gameInProgress; });
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_388.mm7", "issue_388.json");
    EXPECT_EQ(arcomageTape, tape(false, true, false)); // We've played arcomage.
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE, SCREEN_GAME)); // And returned to game screen.
    EXPECT_EQ(pArcomageGame->_gameOver, 1); // With arcomage exit flag.

    pArcomageGame->_targetFPS = oldfpslimit;
}

GAME_TEST(Issues, Issue395) {
    // Check that learning skill works as intended.
    auto expTape = charTapes.experiences();
    auto learningTape = charTapes.skillLevels(SKILL_LEARNING);
    test.playTraceFromTestData("issue_395.mm7", "issue_395.json");
    EXPECT_EQ(expTape.frontBack(), tape({100, 100, 100, 100}, {214, 228, 237, 258}));
    EXPECT_EQ(learningTape, tape({0, 4, 6, 10}));
}

// 400

GAME_TEST(Issues, Issue402) {
    // Attacking while wearing wetsuits shouldn't assert.
    auto checkCharactersWearWetsuits = [] {
        for (int i = 0; i < 4; i++)
            EXPECT_TRUE(pParty->pCharacters[i].wearsItem(ITEM_QUEST_WETSUIT));
    };

    test.playTraceFromTestData("issue_402.mm7", "issue_402.json", [&] {
        checkCharactersWearWetsuits();
    });
    checkCharactersWearWetsuits();
}

GAME_TEST(Issues, Issue403_970) {
    // Entering Lincoln shouldn't crash.
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_403.mm7", "issue_403.json");
    EXPECT_EQ(mapTape, tape(MAP_SHOALS, MAP_LINCOLN)); // Shoals -> Lincoln.

    // #970: Armor Class is wrong.
    EXPECT_EQ(pParty->pCharacters[0].GetActualAC(), 10);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAC(), 5);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAC(), 10);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAC(), 7);
}

GAME_TEST(Issues, Issue405) {
    // FPS affects effective recovery time.
    auto runTrace = [&] {
        engine->config->debug.AllMagic.setValue(true);
        test.loadGameFromTestData("issue_405.mm7");
        game.castSpell(1, SPELL_FIRE_IMMOLATION);
    };

    // 100ms/frame
    test.prepareForNextTest(100, RANDOM_ENGINE_SEQUENTIAL);
    runTrace();
    game.tick(10);
    EXPECT_TRUE(pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Active());
    Duration firstRemainingRecovery = pParty->pCharacters[0].timeToRecovery;

    // 10ms/frame
    test.prepareForNextTest(10, RANDOM_ENGINE_SEQUENTIAL);
    runTrace();
    game.tick(100);
    EXPECT_TRUE(pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Active());
    Duration secondRemainingRecovery = pParty->pCharacters[0].timeToRecovery;

    EXPECT_EQ(firstRemainingRecovery, secondRemainingRecovery);
}

GAME_TEST(Issues, Issue408_939_970_996) {
    // Testing that the gameover loop works.
    // Trace enters throne room - resurecta - final task and exits gameover loop.
    auto screenTape = tapes.screen();
    auto mapTape = tapes.map();
    auto certTape = tapes.custom([] { return ufs->exists("MM7_Win.Pcx"); });
    auto messageBoxesTape = tapes.messageBoxes();
    test.playTraceFromTestData("issue_408.mm7", "issue_408.json");
    // we should return to game screen
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE, SCREEN_GAMEOVER_WINDOW, SCREEN_GAME));
    // windowlist size should be 1
    EXPECT_EQ(lWindowList.size(), 1);
    // should have saved a winner cert tex
    EXPECT_GT(certTape.size(), 1);
    // we should be teleported to harmondale
    EXPECT_EQ(mapTape, tape(MAP_CASTLE_LAMBENT, MAP_HARMONDALE));
    // ending message box was displayed.
    auto flatMessageBoxes = messageBoxesTape.flatten();
    EXPECT_EQ(flatMessageBoxes.size(), 1);
    EXPECT_TRUE(flatMessageBoxes.front().starts_with("Congratulations Adventurer"));

    // #970: Armor Class is wrong.
    // #939: Quick reference doesnt match vanilla.
    // #996: Wrong attack damage when dual wielding blaster and offhand weapon.
    // hp
    EXPECT_EQ(pParty->pCharacters[0].GetHealth(), 1240);
    EXPECT_EQ(pParty->pCharacters[1].GetHealth(), 397);
    EXPECT_EQ(pParty->pCharacters[2].GetHealth(), 307);
    EXPECT_EQ(pParty->pCharacters[3].GetHealth(), 285);
    // sp
    EXPECT_EQ(pParty->pCharacters[0].GetMana(), 0);
    EXPECT_EQ(pParty->pCharacters[1].GetMana(), 77);
    EXPECT_EQ(pParty->pCharacters[2].GetMana(), 57);
    EXPECT_EQ(pParty->pCharacters[3].GetMana(), 543);
    // ac
    EXPECT_EQ(pParty->pCharacters[0].GetActualAC(), 137);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAC(), 128);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAC(), 87);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAC(), 92);
    // attack
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(false), 75);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAttack(false), 100);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAttack(false), 96);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAttack(false), 97);
    // dmg
    EXPECT_EQ(pParty->pCharacters[0].GetMeleeDamageString(), "32 - 61");
    EXPECT_EQ(pParty->pCharacters[1].GetMeleeDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[2].GetMeleeDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[3].GetMeleeDamageString(), "17 - 37");
    // shoot
    EXPECT_EQ(pParty->pCharacters[0].GetRangedAttack(), 63);
    EXPECT_EQ(pParty->pCharacters[1].GetRangedAttack(), 100);
    EXPECT_EQ(pParty->pCharacters[2].GetRangedAttack(), 96);
    EXPECT_EQ(pParty->pCharacters[3].GetRangedAttack(), 97);
    // dmg
    EXPECT_EQ(pParty->pCharacters[0].GetRangedDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[1].GetRangedDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[2].GetRangedDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[3].GetRangedDamageString(), "17 - 37");
    // skills
    auto checkSkills = [](std::initializer_list<std::pair<int, int>> numSkillPairs) {
        for (auto pair : numSkillPairs) {
            int pSkillsCount = 0;
            for (Skill j : allVisibleSkills()) {
                if (pParty->pCharacters[pair.first].pActiveSkills[j]) {
                    ++pSkillsCount;
                }
            }
            EXPECT_EQ(pSkillsCount , pair.second);
        }
    };
    // NB vanilla gets wrong count for character index 2 (13) - doesnt count learning
    checkSkills({ {0, 10}, {1, 11}, {2, 14}, {3, 9} });
}

GAME_TEST(Issues, Issue414) {
    // Spell failure due to curse should spend the same mana as a successful cast.
    test.prepareForNextTest(10, RANDOM_ENGINE_SEQUENTIAL);
    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();

    Character &character = pParty->pCharacters[0];
    character.pActiveSkills[SKILL_FIRE] = CombinedSkillValue(10, MASTERY_GRANDMASTER);
    character.mana = 100;

    // Cast while cursed — sequential RNG returns 0 for grng->random(100), which is < 50,
    // so the spell always fails due to curse.
    character.conditions.set(CONDITION_CURSED, Time::fromTicks(1));
    pushSpellOrRangedAttack(SPELL_FIRE_FIRE_SPIKE, 0, CombinedSkillValue::none(), 0, 0);
    game.tick(1);
    int manaSpentOnFailure = 100 - character.mana;
    EXPECT_EQ(engine->_statusBar->get(), "Spell failed"); // Status bar shows failure message.

    // Cast while not cursed — spell succeeds.
    character.conditions.reset(CONDITION_CURSED);
    character.mana = 100;
    pushSpellOrRangedAttack(SPELL_FIRE_FIRE_SPIKE, 0, CombinedSkillValue::none(), 0, 0);
    game.tick(1);
    int manaSpentOnSuccess = 100 - character.mana;
    int fireSpikeCount = std::ranges::count_if(pSpriteObjects, [](const SpriteObject &obj) {
        return obj.uSpellID == SPELL_FIRE_FIRE_SPIKE && obj.uObjectDescID != 0;
    });
    EXPECT_EQ(fireSpikeCount, 1); // One fire spike projectile was created.

    test.stopTaping();

    EXPECT_GT(manaSpentOnFailure, 0); // Mana was actually spent on failure.
    EXPECT_EQ(manaSpentOnFailure, manaSpentOnSuccess);
}

GAME_TEST(Issues, Issue415a) {
    // Paralysis prevents monsters from receiving damage. Check armageddon damage & knockback.
    // Also check that stoned monsters stay unaffected.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);

    engine->config->debug.NoActors.setValue(true);
    engine->config->debug.AllMagic.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest();
    engine->config->debug.NoActors.setValue(false);

    // Spawn a paralyzed titan & a stoned one.
    auto hpTape = actorTapes.hps({0, 1});
    auto paralyzedTape = actorTapes.hasBuff(0, ACTOR_BUFF_PARALYZED);
    auto stonedTape = actorTapes.hasBuff(1, ACTOR_BUFF_STONED);
    auto stateTape = actorTapes.aiStates({0, 1});
    Time tomorrow = pParty->GetPlayingTime() + Duration::fromDays(1);
    Actor *paralyzedTitan = game.spawnMonster(pParty->pos + Vec3f(0, 1000, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    paralyzedTitan->buffs[ACTOR_BUFF_PARALYZED].Apply(tomorrow, MASTERY_GRANDMASTER, 0, 0, 0);
    EXPECT_FALSE(paralyzedTitan->CanAct()); // Paralyzed monsters can't act...
    EXPECT_TRUE(paralyzedTitan->CanBeDamaged()); // ...but can be damaged.
    Actor *stonedTitan = game.spawnMonster(pParty->pos + Vec3f(0, 1200, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    stonedTitan->buffs[ACTOR_BUFF_STONED].Apply(tomorrow, MASTERY_GRANDMASTER, 0, 0, 0);
    EXPECT_FALSE(stonedTitan->CanAct()); // Stoned monsters can't act...
    EXPECT_FALSE(stonedTitan->CanBeDamaged()); // ...and are invulnerable statues.

    // Armageddon deals damage when its 256-tick timer runs out, wait it out.
    game.castSpell(1, SPELL_DARK_ARMAGEDDON);
    game.tick(30);
    test.stopTaping();

    EXPECT_EQ(hpTape.slice(0).delta(), -60); // Armageddon damage at GM is 60.
    EXPECT_EQ(hpTape.slice(1).delta(), 0); // The stoned titan is unfazed.
    EXPECT_CONTAINS(stateTape.slice(0), AIState::InPain); // Armageddon knockback should also affect paralyzed monsters.
    EXPECT_MISSES(stateTape.slice(1), AIState::InPain); // But not the stoned ones.
    EXPECT_EQ(paralyzedTape, tape(true)); // Stayed paralyzed the whole time.
    EXPECT_EQ(stonedTape, tape(true)); // Stayed stoned the whole time.
}

GAME_TEST(Issues, Issue415b) {
    // Paralysis prevents monsters from receiving damage from AoE spells.
    // Also check that stoned monsters stay unaffected.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);

    engine->config->debug.NoActors.setValue(true);
    engine->config->debug.AllMagic.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest();
    engine->config->debug.NoActors.setValue(false);

    // Titan #0 is the fireball's auto-picked target, paralyzed titan #1 behind it and stoned titan #2 to the side
    // only get the AoE splash.
    auto hpTape = actorTapes.hps({0, 1, 2});
    Time tomorrow = pParty->GetPlayingTime() + Duration::fromDays(1);
    game.spawnMonster(pParty->pos + Vec3f(0, 800, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    Actor *paralyzed = game.spawnMonster(pParty->pos + Vec3f(0, 1100, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    paralyzed->buffs[ACTOR_BUFF_PARALYZED].Apply(tomorrow, MASTERY_GRANDMASTER, 0, 0, 0);
    Actor *stoned = game.spawnMonster(pParty->pos + Vec3f(0, 900, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    stoned->buffs[ACTOR_BUFF_STONED].Apply(tomorrow, MASTERY_GRANDMASTER, 0, 0, 0);

    game.castQuickSpell(1, SPELL_FIRE_FIREBALL);
    game.tick(30);
    test.stopTaping();

    EXPECT_LT(hpTape.slice(0).delta(), 0); // Direct hit.
    EXPECT_LT(hpTape.slice(1).delta(), 0); // Splash damaged the paralyzed titan.
    EXPECT_EQ(hpTape.slice(2).delta(), 0); // The stoned titan is unfazed.
}

GAME_TEST(Issues, Issue415c) {
    // Paralysis prevents monsters from receiving damage for shrinking ray AoE damage.
    // Also check that stoned monsters stay unaffected.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);

    engine->config->debug.NoActors.setValue(true);
    engine->config->debug.AllMagic.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest();
    engine->config->debug.NoActors.setValue(false);

    // Same layout as in Issue415b - shrinking ray hits titan #0, paralyzed titan #1 and stoned titan #2 are in AoE range.
    auto shrinkTape = actorTapes.haveBuffs({0, 1, 2}, ACTOR_BUFF_SHRINK);
    Time tomorrow = pParty->GetPlayingTime() + Duration::fromDays(1);
    game.spawnMonster(pParty->pos + Vec3f(0, 800, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    Actor *paralyzed = game.spawnMonster(pParty->pos + Vec3f(0, 1000, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    paralyzed->buffs[ACTOR_BUFF_PARALYZED].Apply(tomorrow, MASTERY_GRANDMASTER, 0, 0, 0);
    Actor *stoned = game.spawnMonster(pParty->pos + Vec3f(0, 900, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    stoned->buffs[ACTOR_BUFF_STONED].Apply(tomorrow, MASTERY_GRANDMASTER, 0, 0, 0);

    game.castQuickSpell(1, SPELL_DARK_SHRINKING_RAY);
    game.tick(30);
    test.stopTaping();

    EXPECT_CONTAINS(shrinkTape.slice(0), true); // Direct hit shrunk titan #0.
    EXPECT_CONTAINS(shrinkTape.slice(1), true); // AoE shrunk the paralyzed titan.
    EXPECT_MISSES(shrinkTape.slice(2), true); // The stoned titan is unfazed.
}

GAME_TEST(Issues, Issue415d) {
    // Paralyzed monsters couldn't be targeted by melee attacks. Also check stoned monsters.
    for (ActorBuff buff : {ACTOR_BUFF_PARALYZED, ACTOR_BUFF_STONED}) {
        test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);

        engine->config->debug.NoActors.setValue(true);
        game.startNewGame();
        test.startTaping();
        prepareForBattleTest();
        engine->config->debug.NoActors.setValue(false);

        Character &char0 = pParty->pCharacters[0];
        char0.inventory.equip(ITEM_SLOT_MAIN_HAND, Item(ITEM_CHAMPION_SWORD));
        char0.setSkillValue(SKILL_SWORD, CombinedSkillValue(10, MASTERY_EXPERT));

        // Titan #0 to the left of the party is the closest-monster fallback bait, buffed titan #1 stands to the
        // right, slightly further away but still within melee reach.
        auto hpTape = actorTapes.hps({0, 1});
        game.spawnMonster(pParty->pos + Vec3f(-200, 300, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
        Actor *buffed = game.spawnMonster(pParty->pos + Vec3f(220, 380, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
        buffed->buffs[buff].Apply(pParty->GetPlayingTime() + Duration::fromDays(1), MASTERY_GRANDMASTER, 0, 0, 0);
        game.pointMouseAtActor(1);

        // Swing at the titan under the cursor until one of the titans is hit.
        for (int i = 0; i < 30 && pActors[0].hp == pActors[0].monsterInfo.hp && pActors[1].hp == pActors[1].monsterInfo.hp; i++) {
            game.pressAndReleaseKey(PlatformKey::KEY_A);
            game.tick(5);
        }
        test.stopTaping();

        if (buff == ACTOR_BUFF_PARALYZED) {
            EXPECT_LT(hpTape.slice(1).delta(), 0); // Paralyzed titan under the cursor was hit. Before the fix melee went for titan #0.
            EXPECT_EQ(hpTape.slice(0).delta(), 0); // The closest titan wasn't touched.
        } else {
            EXPECT_EQ(hpTape.slice(1).delta(), 0); // Stoned titan under the cursor is not a valid target...
            EXPECT_LT(hpTape.slice(0).delta(), 0); // ...so melee went for the closest titan.
        }
    }
}

GAME_TEST(Issues, Issue415e) {
    // Paralyzed monsters couldn't be targeted by targeted spells. Also check stoned monsters.
    for (ActorBuff buff : {ACTOR_BUFF_PARALYZED, ACTOR_BUFF_STONED}) {
        test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);

        engine->config->debug.NoActors.setValue(true);
        engine->config->debug.AllMagic.setValue(true);
        game.startNewGame();
        test.startTaping();
        prepareForBattleTest();
        engine->config->debug.NoActors.setValue(false);

        // Same layout as in Issue415d - the titans are on the opposite sides of the view, so the bolt's flight path
        // to one always clears the other.
        auto hpTape = actorTapes.hps({0, 1});
        game.spawnMonster(pParty->pos + Vec3f(-200, 300, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
        Actor *buffed = game.spawnMonster(pParty->pos + Vec3f(220, 380, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
        buffed->buffs[buff].Apply(pParty->GetPlayingTime() + Duration::fromDays(1), MASTERY_GRANDMASTER, 0, 0, 0);
        game.pointMouseAtActor(1);

        game.castQuickSpell(1, SPELL_WATER_ICE_BOLT);
        game.tick(30);
        test.stopTaping();

        if (buff == ACTOR_BUFF_PARALYZED) {
            EXPECT_LT(hpTape.slice(1).delta(), 0); // Ice bolt hit the paralyzed titan under the cursor. Before the fix it flew at titan #0.
            EXPECT_EQ(hpTape.slice(0).delta(), 0); // The closest titan wasn't hit.
        } else {
            EXPECT_EQ(hpTape.slice(1).delta(), 0); // Stoned titan under the cursor is not a valid target...
            EXPECT_LT(hpTape.slice(0).delta(), 0); // ...so the bolt flew at the closest titan.
        }
    }
}

GAME_TEST(Issues, Issue416) {
    // Area in The Dragon Caves is inaccessible
    test.playTraceFromTestData("issue_416.mm7", "issue_416.json");
    // Make sure all dragons are above the ground
    for (auto& actor : pActors) {
        EXPECT_GE(actor.pos.z, -1.0f);
    }

    // Same issue in Lincoln
    test.playTraceFromTestData("issue_416b.mm7", "issue_416b.json");
    // Make sure all droids are above the ground
    for (auto& actor : pActors) {
        EXPECT_GE(actor.pos.z, 224);
    }
}

GAME_TEST(Issues, Issue417a) {
    // Testing that portal nodes looping doesnt assert.
    test.playTraceFromTestData("issue_417a.mm7", "issue_417a.json");
}

GAME_TEST(Issues, Issue417b) {
    // Testing that portal nodes looping doesnt assert.
    test.playTraceFromTestData("issue_417b.mm7", "issue_417b.json");
}

static void check427Buffs(const char *ctx, std::initializer_list<int> players, bool hasBuff) {
    for (int character : players) {
        for (CharacterBuff buff : {CHARACTER_BUFF_BLESS, CHARACTER_BUFF_PRESERVATION, CHARACTER_BUFF_HAMMERHANDS, CHARACTER_BUFF_PAIN_REFLECTION}) {
            EXPECT_EQ(pParty->pCharacters[character].pCharacterBuffs[buff].Active(), hasBuff)
                << "(with ctx=" << ctx << ", character=" << character << ", buff=" << std::to_underlying(buff) << ")";
        }
    }
}

GAME_TEST(Issues, Issue427a) {
    // Test that some of the buff spells that start to affect whole party starting from certain mastery work correctly.
    // In this test mastery is not enough for the whole party buff.
    test.playTraceFromTestData("issue_427a.mm7", "issue_427a.json");

    // Check that spell targeting works correctly - 1st char is getting the buffs.
    check427Buffs("a", {0}, true);
    check427Buffs("a", {1, 2, 3}, false);
}

GAME_TEST(Issues, Issue427b_528) {
    // Test that some of the buff spells that start to affect whole party starting from certain mastery work correctly.
    // In this test mastery is enough for the whole party.
    auto manaTape = charTapes.mp(2);
    test.playTraceFromTestData("issue_427b.mm7", "issue_427b.json");

    // Check that all character have buffs.
    check427Buffs("b", {0, 1, 2, 3}, true);

    // #528: Check that spells that target single character or entire party depending on mastery drain mana.
    EXPECT_EQ(manaTape.delta(), -60);
}

GAME_TEST(Issues, Issue442) {
    // Test that regular UI is blocked on spell cast.
    auto blessTape = charTapes.hasBuff(1, CHARACTER_BUFF_BLESS);
    test.playTraceFromTestData("issue_442.mm7", "issue_442.json");
    EXPECT_EQ(blessTape, tape(false, true));
}

GAME_TEST(Prs, Pr469) {
    // Assert when using Quick Spell button when spell is not set.
    test.playTraceFromTestData("pr_469.mm7", "pr_469.json", [] {
        for (int i = 0; i < 4; i++)
            EXPECT_EQ(pParty->pCharacters[i].uQuickSpell, SPELL_NONE);
    });
}

GAME_TEST(Issues, Issue488) {
    // Test that Mass Distortion spell works.
    auto actorHpTape = actorTapes.hp(24);
    test.playTraceFromTestData("issue_488.mm7", "issue_488.json");
    EXPECT_EQ(actorHpTape, tape(3, 2));
}

GAME_TEST(Issues, Issue489) {
    // Test that AOE version of Shrinking Ray spell works.
    auto chibisTape = actorTapes.countByBuff(ACTOR_BUFF_SHRINK);
    test.playTraceFromTestData("issue_489.mm7", "issue_489.json");
    EXPECT_EQ(chibisTape, tape(0, 20));
}

GAME_TEST(Issues, Issue490) {
    // Check that Poison Spray sprites are moving and doing damage.
    auto experienceTape = charTapes.experience(0);
    test.playTraceFromTestData("issue_490.mm7", "issue_490.json");
    EXPECT_EQ(experienceTape, tape(279, 285));
}

GAME_TEST(Issues, Issue491) {
    // Check that opening and closing Lloyd book does not cause Segmentation Fault.
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_491.mm7", "issue_491.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_SPELL_BOOK, SCREEN_GAME, SCREEN_BOOKS, SCREEN_GAME));
}

GAME_TEST(Issues, Issue492) {
    // Check that spells that target all visible actors work.
    auto experienceTape = charTapes.experiences();
    test.playTraceFromTestData("issue_492.mm7", "issue_492.json");
    EXPECT_EQ(experienceTape.frontBack(), tape({279, 311, 266, 260}, {287, 319, 274, 268}));
}
