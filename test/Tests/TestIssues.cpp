#include "Testing/Game/GameTest.h"

#include "Arcomage/Arcomage.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIProgressBar.h"

#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/SaveLoad.h"
#include "Engine/Graphics/Indoor.h"

#include "Utility/DataPath.h"
#include "Utility/ScopeGuard.h"

static int totalPartyHealth() {
    int result = 0;
    for (const Player &player : pParty->pPlayers)
        result += player.health;
    return result;
}

static int partyItemCount() {
    int result = 0;
    for (const Player &player : pParty->pPlayers)
        for (const ItemGen &item : player.pOwnItems)
            result += item.uItemID != ITEM_NULL;
    return result;
}

static std::initializer_list<PLAYER_BUFFS> allPotionBuffs() {
    static constexpr std::initializer_list<PLAYER_BUFFS> result = {
        PLAYER_BUFF_RESIST_AIR,
        PLAYER_BUFF_BLESS,
        PLAYER_BUFF_RESIST_BODY,
        PLAYER_BUFF_RESIST_EARTH,
        PLAYER_BUFF_RESIST_FIRE,
        PLAYER_BUFF_HASTE,
        PLAYER_BUFF_HEROISM,
        PLAYER_BUFF_RESIST_MIND,
        PLAYER_BUFF_PRESERVATION,
        PLAYER_BUFF_SHIELD,
        PLAYER_BUFF_STONESKIN,
        PLAYER_BUFF_ACCURACY,
        PLAYER_BUFF_ENDURANCE,
        PLAYER_BUFF_INTELLIGENCE,
        PLAYER_BUFF_LUCK,
        PLAYER_BUFF_STRENGTH,
        PLAYER_BUFF_WILLPOWER,
        PLAYER_BUFF_SPEED,
        PLAYER_BUFF_RESIST_WATER,
        PLAYER_BUFF_WATER_WALK
    };
    return result;
}

// 100

GAME_TEST(Issues, Issue123) {
    // Party falls when flying
    test->playTraceFromTestData("issue_123.mm7", "issue_123.json");
    // check party is still in the air
    EXPECT_GT(pParty->vPosition.z, 512);
}

GAME_TEST(Issues, Issue125) {
    // check that fireballs hurt party
    int oldHealth = 0;
    test->playTraceFromTestData("issue_125.mm7", "issue_125.json", [&] { oldHealth = totalPartyHealth(); });
    int newHealth = totalPartyHealth();
    EXPECT_LT(newHealth, oldHealth);
}

GAME_TEST(Issues, Issue159) {
    // Exception when entering Tidewater Caverns
    test->playTraceFromTestData("issue_159.mm7", "issue_159.json");
}

GAME_TEST(Issues, Issue163) {
    // Testing that pressing the Load Game button doesn't crash even if the 'saves' folder doesn't exist.
    std::string savesDir = MakeDataPath("saves");
    std::string savesDirMoved;

    MM_AT_SCOPE_EXIT({
        if (!savesDirMoved.empty()) {
            std::error_code ec;
            std::filesystem::rename(savesDirMoved, savesDir, ec); // Using std::error_code here, so can't throw.
        }
    });

    if (std::filesystem::exists(savesDir)) {
        savesDirMoved = savesDir + "_moved_for_testing";
        ASSERT_FALSE(std::filesystem::exists(savesDirMoved)); // Throws on failure.
        std::filesystem::rename(savesDir, savesDirMoved);
    }

    game->pressGuiButton("MainMenu_LoadGame"); // Shouldn't crash.
    game->tick(10);
    for (bool used : pSavegameList->pSavegameUsedSlots)
        EXPECT_FALSE(used); // All slots unused.

    game->pressGuiButton("LoadMenu_Load");
    game->tick(10);
    EXPECT_EQ(current_screen_type, CURRENT_SCREEN::SCREEN_LOADGAME);
    EXPECT_FALSE(pGameLoadingUI_ProgressBar->IsActive()); // Load button shouldn't do anything.
}

GAME_TEST(Issues, Issue198) {
    // Check that items can't end up out of bounds of player's inventory.
    test->playTraceFromTestData("issue_198.mm7", "issue_198.json");

    auto forEachInventoryItem = [](auto &&callback) {
        for (const Player &player : pParty->pPlayers) {
            for (int inventorySlot = 0; inventorySlot < Player::INVENTORY_SLOT_COUNT; inventorySlot++) {
                int itemIndex = player.pInventoryMatrix[inventorySlot];
                if (itemIndex <= 0)
                    continue; // Empty or non-primary cell.

                int x = inventorySlot % Player::INVENTORY_SLOTS_WIDTH;
                int y = inventorySlot / Player::INVENTORY_SLOTS_WIDTH;

                callback(player.pInventoryItemList[itemIndex - 1], x, y);
            }
        }
    };

    // Preload item images in the main thread first.
    game->runGameRoutine([&] {
        forEachInventoryItem([](const ItemGen &item, int /*x*/, int /*y*/) {
            // Calling GetWidth forces the texture to be created.
            assets->GetImage_ColorKey(pItemTable->pItems[item.uItemID].iconName)->GetWidth();
        });
    });

    // Then can safely check everything.
    forEachInventoryItem([](const ItemGen &item, int x, int y) {
        Texture *image = assets->GetImage_ColorKey(pItemTable->pItems[item.uItemID].iconName);
        int width = GetSizeInInventorySlots(image->GetWidth());
        int height = GetSizeInInventorySlots(image->GetHeight());

        EXPECT_LE(x + width, Player::INVENTORY_SLOTS_WIDTH);
        EXPECT_LE(y + height, Player::INVENTORY_SLOTS_HEIGHT);
    });
}

// 200

GAME_TEST(Issues, Issue201) {
    // Unhandled EVENT_ShowMovie in Event Processor
    int oldHealth = 0;
    uint64_t oldTime = 0;
    test->playTraceFromTestData("issue_201.mm7", "issue_201.json", [&] { oldHealth = totalPartyHealth(); oldTime = pParty->GetPlayingTime().GetDays(); });

    int newHealth = totalPartyHealth();
    uint64_t newTime = pParty->GetPlayingTime().GetDays();
    EXPECT_GT(newHealth, oldHealth); // party should heal
    EXPECT_EQ(pCurrentMapName, "Out02.odm"); // we should be teleported to harmondale
    EXPECT_EQ((oldTime + GameTime(0, 0, 0, 0, 1).GetDays()), newTime); // time should advance by a week
}

GAME_TEST(Issues, Issue202) {
    //Judge doesn't move to house and stays with the party
    int oldhirecount{};
    test->playTraceFromTestData("issue_202.mm7", "issue_202.json", [&]() {oldhirecount = pParty->CountHirelings(); });
    EXPECT_EQ(oldhirecount - 1, pParty->CountHirelings()); // judge shouldn't be with party anymore
    EXPECT_EQ(pParty->alignment, PartyAlignment_Evil); // party align evil
}

GAME_TEST(Issues, Issue203) {
    // Judge's "I lost it" shouldn't crash.
    test->playTraceFromTestData("issue_203.mm7", "issue_203.json");
}

GAME_TEST(Issues, Issue211) {
    // Crash during accidental ok double click
    test->playTraceFromTestData("issue_211.mm7", "issue_211.json");
}

GAME_TEST(Issues, Issue223) {
    // Fire and air resistance not resetting between games
    auto checkResistances = [](CHARACTER_ATTRIBUTE_TYPE resistance, std::initializer_list<std::pair<int, int>> resistancePairs) {
        for (auto pair : resistancePairs) {
            EXPECT_EQ(pParty->pPlayers[pair.first].GetActualResistance(resistance), pair.second);
        }
    };

    test->playTraceFromTestData("issue_223.mm7", "issue_223.json");
    // expect normal resistances 55-00-00-00
    checkResistances(CHARACTER_ATTRIBUTE_RESIST_FIRE, { {0, 5}, {1, 0}, {2, 0}, {3, 0} });
    checkResistances(CHARACTER_ATTRIBUTE_RESIST_AIR, { {0, 5}, {1, 0}, {2, 0}, {3, 0} });
}

GAME_TEST(Issues, Issue238) {
    // Party vertical flight speed doesnt use frame pacing
    test->playTraceFromTestData("issue_238.mm7", "issue_238.json");
    EXPECT_LT(pParty->vPosition.z, 2500);
}

GAME_TEST(Issues, Issue248) {
    // Crash in NPC dialog
    test->playTraceFromTestData("issue_248.mm7", "issue_248.json");
}

GAME_TEST(Issues, Issue268) {
    // Crash in ODM_GetFloorLevel
    test->playTraceFromTestData("issue_268.mm7", "issue_268.json");
}

GAME_TEST(Issues, Issue271) {
    // Party shouldn't yell when landing from flight
    test->playTraceFromTestData("issue_271.mm7", "issue_271.json");
    EXPECT_NE(pParty->pPlayers[1].expression, CHARACTER_EXPRESSION_FEAR);
}

GAME_TEST(Issues, Issue272) {
    // Controls menu bugs
    // Check default resets keys
    test->playTraceFromTestData("issue_272a.mm7", "issue_272a.json");
    EXPECT_EQ(engine->config->keybindings.Right.value(), engine->config->keybindings.Right.defaultValue());
    // Check you cant leave menu with conflicting keys
    test->playTraceFromTestData("issue_272b.mm7", "issue_272b.json");
    EXPECT_EQ(current_screen_type, CURRENT_SCREEN::SCREEN_KEYBOARD_OPTIONS);
}

GAME_TEST(Issues, Issue293a) {
    // Test that barrels in castle Harmondale work and can be triggered only once, and that trash piles work,
    // give an item once, but give disease indefinitely.
    test->playTraceFromTestData("issue_293a.mm7", "issue_293a.json", [] {
        EXPECT_EQ(pParty->pPlayers[0].uMight, 30);
        EXPECT_EQ(pParty->pPlayers[0].uIntelligence, 5);
        EXPECT_EQ(pParty->pPlayers[0].uWillpower, 5);
        EXPECT_EQ(pParty->pPlayers[0].uEndurance, 13);
        EXPECT_EQ(pParty->pPlayers[0].uSpeed, 14);
        EXPECT_EQ(pParty->pPlayers[0].uAccuracy, 13);
        EXPECT_EQ(pParty->pPlayers[0].uLuck, 7);
        EXPECT_EQ(partyItemCount(), 18);
        EXPECT_FALSE(pParty->pPlayers[0].hasItem(ITEM_LEATHER_ARMOR, false));
        for (int i = 0; i < 4; i++)
            EXPECT_EQ(pParty->pPlayers[i].GetMajorConditionIdx(), Condition_Good);
    });

    EXPECT_EQ(pParty->pPlayers[0].uMight, 30);
    EXPECT_EQ(pParty->pPlayers[0].uIntelligence, 7); // +2
    EXPECT_EQ(pParty->pPlayers[0].uWillpower, 5);
    EXPECT_EQ(pParty->pPlayers[0].uEndurance, 13);
    EXPECT_EQ(pParty->pPlayers[0].uSpeed, 14);
    EXPECT_EQ(pParty->pPlayers[0].uAccuracy, 15); // +2
    EXPECT_EQ(pParty->pPlayers[0].uLuck, 7);
    EXPECT_EQ(partyItemCount(), 19); // +1
    EXPECT_TRUE(pParty->pPlayers[0].hasItem(ITEM_CHAIN_MAIL, false)); // That's the item from the trash pile.
    EXPECT_EQ(pParty->pPlayers[0].GetMajorConditionIdx(), Condition_Disease_Weak);
    EXPECT_EQ(pParty->pPlayers[1].GetMajorConditionIdx(), Condition_Good); // Good roll here, didn't get sick.
    EXPECT_EQ(pParty->pPlayers[2].GetMajorConditionIdx(), Condition_Disease_Weak);
    EXPECT_EQ(pParty->pPlayers[3].GetMajorConditionIdx(), Condition_Disease_Weak);
}

GAME_TEST(Issues, Issue293b) {
    // Test that table food in castle Harmondale is pickable only once and gives apples.
    test->playTraceFromTestData("issue_293b.mm7", "issue_293b.json", [] {
        EXPECT_EQ(pParty->uNumFoodRations, 7);
        EXPECT_EQ(partyItemCount(), 18);
        EXPECT_FALSE(pParty->hasItem(ITEM_RED_APPLE));
    });

    EXPECT_EQ(pParty->uNumFoodRations, 7); // No change.
    EXPECT_EQ(partyItemCount(), 19); // +1
    EXPECT_TRUE(pParty->hasItem(ITEM_RED_APPLE)); // That's the table food item.
}

GAME_TEST(Issues, Issue293c) {
    // Test that cauldrons work, and work only once. The cauldron tested is in the Barrow Downs.
    test->playTraceFromTestData("issue_293c.mm7", "issue_293c.json", [] {
        EXPECT_EQ(pParty->pPlayers[0].sResAirBase, 230); // An interesting save we have here.
        EXPECT_EQ(pParty->pPlayers[1].sResAirBase, 50);
        EXPECT_EQ(pParty->pPlayers[2].sResAirBase, 24);
        EXPECT_EQ(pParty->pPlayers[3].sResAirBase, 18);
    });

    EXPECT_EQ(pParty->pPlayers[0].sResAirBase, 230);
    EXPECT_EQ(pParty->pPlayers[1].sResAirBase, 52); // +2
    EXPECT_EQ(pParty->pPlayers[2].sResAirBase, 24);
    EXPECT_EQ(pParty->pPlayers[3].sResAirBase, 18);
}

GAME_TEST(Issues, Issue294) {
    auto partyExperience = [&] {
        uint64_t result = 0;
        for (const Player &player : pParty->pPlayers)
            result += player.experience;
        return result;
    };

    // Testing that party auto-casting shrapnel successfully targets rats & kills them, gaining experience.
    uint64_t oldExperience = 0;
    test->playTraceFromTestData("issue_294.mm7", "issue_294.json", [&] { oldExperience = partyExperience(); });
    uint64_t newExperience = partyExperience();
    // EXPECT_GT(newExperience, oldExperience); // Expect the giant rat to be dead after four shrapnel casts from character #4.
    // TODO(captainurist): ^passes now, but for the wrong reason - the rat decided to move after recent patches
}

// 300

GAME_TEST(Prs, Pr314) {
    // Check that character creating menu works.
    // Trace pretty much presses all the buttons and opens all the popups possible.
    test->playTraceFromTestData("pr_314.mm7", "pr_314.json");

    for (int i = 0; i < 4; i++)
        EXPECT_EQ(pParty->pPlayers[i].uLuck, 20);

    EXPECT_EQ(pParty->pPlayers[0].classType, PLAYER_CLASS_MONK);
    EXPECT_EQ(pParty->pPlayers[1].classType, PLAYER_CLASS_THIEF);
    EXPECT_EQ(pParty->pPlayers[2].classType, PLAYER_CLASS_RANGER);
    EXPECT_EQ(pParty->pPlayers[3].classType, PLAYER_CLASS_CLERIC);
    EXPECT_EQ(pParty->pPlayers[0].GetRace(), CHARACTER_RACE_ELF);
    EXPECT_EQ(pParty->pPlayers[1].GetRace(), CHARACTER_RACE_ELF);
    EXPECT_EQ(pParty->pPlayers[2].GetRace(), CHARACTER_RACE_GOBLIN);
    EXPECT_EQ(pParty->pPlayers[3].GetRace(), CHARACTER_RACE_ELF);
}

GAME_TEST(Issues, Issue315) {
    test->loadGameFromTestData("issue_315.mm7");
    game->goToMainMenu();
    game->pressGuiButton("MainMenu_NewGame");
    game->tick(2);
    game->pressGuiButton("PartyCreation_OK");
    game->skipLoadingScreen(); // This shouldn't crash.
}

GAME_TEST(Issues, Issue331) {
    // Assert when traveling by horse caused by out of bound access to pObjectList->pObjects
    test->playTraceFromTestData("issue_331.mm7", "issue_331.json");
}

GAME_TEST(Prs, Pr347) {
    // Testing that shops work.
    int oldGold = 0;
    test->playTraceFromTestData("pr_347.mm7", "pr_347.json", [&] { oldGold = pParty->uNumGold; });
    EXPECT_NE(oldGold, pParty->uNumGold); // Spent on items.
}

GAME_TEST(Issues, Issue388) {
    // Testing that Arcomage works
    int oldfpslimit = pArcomageGame->_targetFPS;
    pArcomageGame->_targetFPS = 500;
    // Trace enters tavern, plays arcomage, plays a couple of cards then exits and leaves tavern
    CURRENT_SCREEN oldscreen = CURRENT_SCREEN::SCREEN_GAME;
    test->playTraceFromTestData("issue_388.mm7", "issue_388.json", [&] { oldscreen = current_screen_type; });
    // we should return to game screen
    EXPECT_EQ(oldscreen, current_screen_type);
    // with arcomage exit flag
    EXPECT_EQ(pArcomageGame->GameOver, 1);
    pArcomageGame->_targetFPS = oldfpslimit;
}

GAME_TEST(Issues, Issue395) {
    // Check that learning skill works as intended
    auto checkExperience = [](std::initializer_list<std::pair<int, int>> experiencePairs) {
        for (auto pair : experiencePairs) {
            EXPECT_EQ(pParty->pPlayers[pair.first].experience, pair.second);
        }
    };

    test->playTraceFromTestData("issue_395.mm7", "issue_395.json", [&] { checkExperience({ {0, 100}, {1, 100}, {2, 100}, {3, 100} }); });
    checkExperience({ {0, 214}, {1, 228}, {2, 237}, {3, 258} });
}

// 400

GAME_TEST(Issues, Issue402) {
    // Attacking while wearing wetsuits shouldn't assert.
    test->playTraceFromTestData("issue_402.mm7", "issue_402.json");
}

GAME_TEST(Issues, Issue403) {
    // Entering Lincoln shouldn't crash.
    test->playTraceFromTestData("issue_403.mm7", "issue_403.json");
}

GAME_TEST(Issues, Issue405) {
    // FPS affects effective recovery time.
    auto runTrace = [&] {
        test->loadGameFromTestData("issue_405.mm7");
        game->pressGuiButton("Game_Character1");
        game->tick(1);
        game->pressGuiButton("Game_CastSpell");
        game->tick(1);
        game->pressGuiButton("SpellBook_Spell7"); // 7 is immolation.
        game->tick(1);
        game->pressGuiButton("SpellBook_Spell7"); // Confirm.
        game->tick(1);
    };
    engine->config->debug.AllMagic.setValue(true);

    // 30ms/frame
    test->startDeterministicSegment(30);
    runTrace();
    game->tick(10);
    EXPECT_TRUE(pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Active());
    int firstRemainingRecovery = pPlayers[1]->timeToRecovery;

    // 2ms/frame
    test->startDeterministicSegment(2);
    runTrace();
    game->tick(150);
    EXPECT_TRUE(pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Active());
    int secondRemainingRecovery = pPlayers[1]->timeToRecovery;

    EXPECT_EQ(firstRemainingRecovery - 1, secondRemainingRecovery); // TODO(captainurist): where is this -1 coming from???
}

GAME_TEST(Issues, Issue408) {
    // testing that the gameover loop works
    CURRENT_SCREEN oldscreen = CURRENT_SCREEN::SCREEN_GAME;
    // enters throne room - resurecta - final task and exits gameover loop
    test->playTraceFromTestData("issue_408.mm7", "issue_408.json", [&] { oldscreen = current_screen_type; });
    // we should return to game screen
    EXPECT_EQ(oldscreen, current_screen_type);
    // windowlist size should be 1
    EXPECT_EQ(lWindowList.size(), 1);
    // should have save a winner cert tex
    EXPECT_NE(assets->WinnerCert, nullptr);
    // we should be teleported to harmondale
    EXPECT_EQ(pCurrentMapName, "out02.odm");
}

GAME_TEST(Issues, Issue417) {
    // testing that portal nodes looping doesnt assert
    test->playTraceFromTestData("issue_417a.mm7", "issue_417a.json");
    test->playTraceFromTestData("issue_417b.mm7", "issue_417b.json");
}

static void check427Buffs(const char *ctx, std::initializer_list<int> players, bool hasBuff) {
    for (int player : players) {
        for (PLAYER_BUFFS buff : {PLAYER_BUFF_BLESS, PLAYER_BUFF_PRESERVATION, PLAYER_BUFF_HAMMERHANDS, PLAYER_BUFF_PAIN_REFLECTION}) {
            EXPECT_EQ(pParty->pPlayers[player].pPlayerBuffs[buff].Active(), hasBuff)
                << "(with ctx=" << ctx << ", player=" << player << ", buff=" << buff << ")";
        }
    }
}

GAME_TEST(Issues, Issue427) {
    // Test that some of the buff spells that start to affect whole party starting from certain mastery work correctly.

    // In this test mastery is not enough for the whole party buff
    test->playTraceFromTestData("issue_427a.mm7", "issue_427a.json");

    // Check that spell targeting works correctly - 1st char is getting the buffs.
    check427Buffs("a", { 0 }, true);
    check427Buffs("a", { 1, 2, 3 }, false);

    // In this test mastery is enough for the whole party
    test->playTraceFromTestData("issue_427b.mm7", "issue_427b.json");

    // Check that all character have buffs
    check427Buffs("b", { 0, 1, 2, 3 }, true);
}

GAME_TEST(Issues, Issue442) {
    // Test that regular UI is blocked on spell cast
    test->playTraceFromTestData("issue_442.mm7", "issue_442.json");
    EXPECT_EQ(pParty->pPlayers[1].pPlayerBuffs[PLAYER_BUFF_BLESS].Active(), true);
}

GAME_TEST(Prs, Pr469) {
    // Assert when using Quick Spell button when spell is not set
    test->playTraceFromTestData("pr_469.mm7", "pr_469.json");
}

GAME_TEST(Issues, Issue488) {
    // Test that Mass Distortion spell works
    test->playTraceFromTestData("issue_488.mm7", "issue_488.json", [] { EXPECT_EQ(pActors[24].sCurrentHP, 3); });
    EXPECT_EQ(pActors[24].sCurrentHP, 2);
}

GAME_TEST(Issues, Issue489) {
    // Test that AOE version of Shrinking Ray spell works
    auto countChibis = [] {
        return std::count_if(pActors.begin(), pActors.end(), [] (const Actor &actor) {
            return actor.pActorBuffs[ACTOR_BUFF_SHRINK].Active();
        });
    };

    test->playTraceFromTestData("issue_489.mm7", "issue_489.json", [&] { EXPECT_EQ(countChibis(), 0); });
    EXPECT_EQ(countChibis(), 21);
}

GAME_TEST(Issues, Issue490) {
    // Check that Poison Spray sprites are moving and doing damage
    test->playTraceFromTestData("issue_490.mm7", "issue_490.json", []() { EXPECT_EQ(pParty->pPlayers[0].experience, 279); });
    EXPECT_EQ(pParty->pPlayers[0].experience, 285);
}

GAME_TEST(Issues, Issue491) {
    // Check that opening and closing Lloyd book does not cause Segmentation Fault
    test->playTraceFromTestData("issue_491.mm7", "issue_491.json");
}

GAME_TEST(Issues, Issue492) {
    // Check that spells that target all visible actors work
    test->playTraceFromTestData("issue_492.mm7", "issue_492.json", []() { EXPECT_EQ(pParty->pPlayers[0].experience, 279); });
    EXPECT_EQ(pParty->pPlayers[0].experience, 287);
}

// 500

GAME_TEST(Issues, Issue502) {
    // Check that script face animation and voice indexes right characters
    test->playTraceFromTestData("issue_502.mm7", "issue_502.json");
}

static void check503health(std::initializer_list<std::pair<int, int>> playerhealthpairs) {
    for (auto pair : playerhealthpairs) {
        EXPECT_EQ(pParty->pPlayers[pair.first].health, pair.second);
    }
}

GAME_TEST(Issues, Issue503) {
    // Check that town portal book actually pauses game
    test->playTraceFromTestData("issue_503.mm7", "issue_503.json", []() { check503health({ {0, 1147}, {1, 699}, {2, 350}, {3, 242} }); });
    check503health({ {0, 1147}, {1, 699}, {2, 350}, {3, 242} });
}

GAME_TEST(Issues, Issue504) {
    // Going to prison doesn't recharge hirelings
    test->playTraceFromTestData("issue_504.mm7", "issue_504.json");
    EXPECT_TRUE(pParty->pPartyBuffs[PARTY_BUFF_HEROISM].Active());
}

GAME_TEST(Issues, Issue506) {
    // Check that scroll use does not assert
    test->playTraceFromTestData("issue_506.mm7", "issue_506.json");
}

GAME_TEST(Issue, Issue518) {
    // Armageddon yeets the actors way too far into the sky & actors take stops when falling down
    test->playTraceFromTestData("issue_518.mm7", "issue_518.json");
    for (auto &actor : pActors) {
        EXPECT_LT(actor.vPosition.z, 3500);
    }
}

GAME_TEST(Issues, Issue520) {
    // Party should take fall damage
    uint64_t oldHealth = 0;
    test->playTraceFromTestData("issue_520.mm7", "issue_520.json", [&] { oldHealth = totalPartyHealth(); });
    EXPECT_LT(totalPartyHealth(), oldHealth);
}

GAME_TEST(Issues, Issue521) {
    // 500 endurance leads to asserts in Player::SetRecoveryTime
    int oldActive{};
    test->playTraceFromTestData("issue_521.mm7", "issue_521.json", [&] { oldActive = pParty->activeCharacterIndex(); });
    EXPECT_EQ(oldActive, pParty->activeCharacterIndex());
}

GAME_TEST(Issues, Issue527) {
    // Check Cure Disease spell works
    test->playTraceFromTestData("issue_527.mm7", "issue_527.json", []() { EXPECT_TRUE(pParty->pPlayers[0].conditions.Has(Condition_Disease_Weak)); });
    EXPECT_FALSE(pParty->pPlayers[0].conditions.Has(Condition_Disease_Weak));
}

GAME_TEST(Issues, Issue528) {
    // Check that spells target single player or entire party depending on mastery drain mana
    // Use test for issue 427 which test the same spells
    test->playTraceFromTestData("issue_427b.mm7", "issue_427b.json", []() { EXPECT_EQ(pParty->pPlayers[2].mana, 100); });
    EXPECT_EQ(pParty->pPlayers[2].mana, 40);
}

GAME_TEST(Issues, Issue540) {
    // Check that Mass Distortion and Charm without target does not assert
    test->playTraceFromTestData("issue_540.mm7", "issue_540.json");
}

GAME_TEST(Issues, Issue558) {
    // Check that performing alchemy does not cause out of bound access
    test->playTraceFromTestData("issue_558.mm7", "issue_558.json");
    EXPECT_EQ(pParty->pPickedItem.uItemID, ITEM_POTION_CURE_WOUNDS);
}

GAME_TEST(Issues, Issue563) {
    // Check that drinking some buff potions does not cause assert
    test->playTraceFromTestData("issue_563.mm7", "issue_563.json");
}

GAME_TEST(Issues, Issue571) {
    // Check that item potion cannot be wastefully applied to unrelated item
    test->playTraceFromTestData("issue_571.mm7", "issue_571.json");
    EXPECT_NE(pParty->pPickedItem.uItemID, ITEM_NULL);
}

GAME_TEST(Issues, Issue574) {
    // Check that applying recharge item potion produces correct number of charges
    test->playTraceFromTestData("issue_574.mm7", "issue_574.json");
    EXPECT_EQ(pParty->pPickedItem.uMaxCharges, pParty->pPickedItem.uNumCharges);
}

GAME_TEST(Issues, Issue578) {
    // Check that rest & heal work after waiting
    test->playTraceFromTestData("issue_578.mm7", "issue_578.json", []() { EXPECT_EQ(pParty->pPlayers[0].health, 66); });
    EXPECT_EQ(pParty->pPlayers[0].health, 108);
}

GAME_TEST(Issues, Issue598) {
    // Assert when accessing character inventory from the shop screen
    test->playTraceFromTestData("issue_598.mm7", "issue_598.json");
    EXPECT_EQ(current_screen_type, CURRENT_SCREEN::SCREEN_SHOP_INVENTORY);
}

// 600

static void check601Conds(std::array<Condition, 4> conds, std::array<int, 4> health) {
    for (int i = 0; i < conds.size(); i++) {
        EXPECT_EQ(pParty->pPlayers[i].GetMajorConditionIdx(), conds[i]);
        EXPECT_EQ(pParty->pPlayers[i].health, health[i]);
    }
}

GAME_TEST(Issues, Issue601) {
    // Check that Master Healer NPC skill work and does not assert
    test->playTraceFromTestData("issue_601.mm7", "issue_601.json", []() { check601Conds({Condition_Sleep, Condition_Cursed, Condition_Fear, Condition_Dead}, {66, 128, 86, 70}); });
    check601Conds({Condition_Good, Condition_Good, Condition_Good, Condition_Good}, {126, 190, 96, 80});
}

GAME_TEST(Issues, Issue608) {
    // Check that using Gate Master ability does not deplete mana of character
    test->playTraceFromTestData("issue_608.mm7", "issue_608.json", []() { EXPECT_EQ(pParty->pPlayers[0].mana, 35); });
    EXPECT_EQ(pParty->pPlayers[0].mana, 35);
}

GAME_TEST(Issues, Issue611) {
    // Heal and reanimate dont work
    test->playTraceFromTestData("issue_611.mm7", "issue_611.json");
    // expect chars to be healed and zombies
    EXPECT_EQ(pParty->pPlayers[0].health, 45);
    EXPECT_EQ(pParty->pPlayers[1].health, 39);
    EXPECT_EQ(pParty->pPlayers[2].conditions.Has(Condition_Zombie), true);
    EXPECT_EQ(pParty->pPlayers[3].conditions.Has(Condition_Zombie), true);
}

GAME_TEST(Issues, Issue613) {
    // Check that maximum food cooked by NPC is 14
    test->playTraceFromTestData("issue_613a.mm7", "issue_613a.json", []() { EXPECT_EQ(pParty->GetFood(), 13); });
    EXPECT_EQ(pParty->GetFood(), 14);
    test->playTraceFromTestData("issue_613b.mm7", "issue_613b.json", []() { EXPECT_EQ(pParty->GetFood(), 13); });
    EXPECT_EQ(pParty->GetFood(), 14);
}

GAME_TEST(Issues, Issue615) {
    // test 1 - ensure that clicking between active portraits changes active character.
    test->playTraceFromTestData("issue_615a.mm7", "issue_615a.json", []() { EXPECT_EQ(pParty->activeCharacterIndex(), 1); });
    EXPECT_EQ(pParty->activeCharacterIndex(), 3);
    // Assert when clicking on character portrait when no active character is present
    test->playTraceFromTestData("issue_615b.mm7", "issue_615b.json", []() { EXPECT_EQ(pParty->activeCharacterIndex(), 1); });
    EXPECT_EQ(pParty->activeCharacterIndex(), 4);
}

GAME_TEST(Issues, Issue625) {
    // Every character getting club at the start of the game
    game->pressGuiButton("MainMenu_NewGame");
    game->tick(2);
    game->pressGuiButton("PartyCreation_OK");
    game->skipLoadingScreen();
    game->tick(2);
    EXPECT_FALSE(pParty->hasItem(ITEM_CLUB));
}

GAME_TEST(Issues, Issue626) {
    // Last loaded save is not remembered

    // Skipping random checks because we're essentially loading a random savegame inside this trace.
    test->playTraceFromTestData("issue_626.mm7", "issue_626.json", TRACE_PLAYBACK_SKIP_RANDOM_CHECKS);

    // TODO(captainurist): this will fail if we don't have any saves in saves folder
    EXPECT_EQ(pSavegameList->selectedSlot, 5);
}

GAME_TEST(Issue, Issue645) {
    // Characters does not enter unconscious state
    test->playTraceFromTestData("issue_645.mm7", "issue_645.json");
    EXPECT_EQ(pParty->pPlayers[0].conditions.Has(Condition_Unconscious), true);
    EXPECT_EQ(pParty->pPlayers[1].conditions.Has(Condition_Unconscious), false);
    EXPECT_EQ(pParty->pPlayers[2].conditions.Has(Condition_Unconscious), true);
    EXPECT_EQ(pParty->pPlayers[3].conditions.Has(Condition_Unconscious), true);
}

GAME_TEST(Issues, Issue661) {
    // HP/SP regen from items is too high
    int oldHealth = 0;
    int oldMana = 0;
    test->playTraceFromTestData("issue_661.mm7", "issue_661.json", [&] { oldHealth = pParty->pPlayers[0].health; oldMana = pParty->pPlayers[0].mana; });
    // two hour wait period is 24 blocks of 5 mins
    // one item that heals hp, three items heal mana
    EXPECT_EQ(pParty->pPlayers[0].health, oldHealth + 24);
    EXPECT_EQ(pParty->pPlayers[0].mana, oldMana + 24*3);
}

GAME_TEST(Issues, Issue662) {
    // "of Air magic" should give floor(skill / 2) skill level bonus (like all other such bonuses)
    test->loadGameFromTestData("issue_662.mm7");
    // currently air magic is (expert) 6
    EXPECT_EQ(pParty->pPlayers[3].GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_AIR), 3);
    pParty->pPlayers[3].skillAir = 5;
    EXPECT_EQ(pParty->pPlayers[3].GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_AIR), 2);
}

GAME_TEST(Issues, Issue663) {
    // Cant switch between inactive char inventory in chests
    test->playTraceFromTestData("issue_663.mm7", "issue_663.json");
    // should switch to char 2 inv
    EXPECT_EQ(pParty->activeCharacterIndex(), 2);
    EXPECT_GT(pParty->activeCharacter().timeToRecovery, 0);
}

GAME_TEST(Issues, Issue664) {
    // Party sliding down shallow slopes outdoors
    test->playTraceFromTestData("issue_664.mm7", "issue_664.json");
    // shouldnt move
    EXPECT_EQ(pParty->vPosition.x, 7323);
    EXPECT_EQ(pParty->vPosition.y, 10375);
    EXPECT_EQ(pParty->vPosition.z, 309);
}

GAME_TEST(Issues, Issue674) {
    // Check that map timers are working
    int oldHealth;
    test->playTraceFromTestData("issue_674.mm7", "issue_674.json", [&] { oldHealth = pParty->pPlayers[0].health; });
    EXPECT_EQ(pParty->pPlayers[0].health, oldHealth + 5);
}

GAME_TEST(Issues, Issue675) {
    // generateItem used to generate invalid enchantments outside of the [0, 24] range in some cases.
    // Also, generateItem used to assert.
    std::initializer_list<ITEM_TREASURE_LEVEL> levels = {
        ITEM_TREASURE_LEVEL_1, ITEM_TREASURE_LEVEL_2, ITEM_TREASURE_LEVEL_3,
        ITEM_TREASURE_LEVEL_4, ITEM_TREASURE_LEVEL_5, ITEM_TREASURE_LEVEL_6
    };

    ItemGen item;
    for (int i = 0; i < 200; i++) {
        for (ITEM_TREASURE_LEVEL level : levels) {
            pItemTable->generateItem(level, 0, &item);
            if (isPotion(item.uItemID)) {
                // For potions, uEnchantmentType is potion strength.
                EXPECT_GE(item.uEnchantmentType, 1);
            } else {
                // For non-potions, it's a number in [0, 24].
                EXPECT_GE(item.uEnchantmentType, 0);
                EXPECT_LE(item.uEnchantmentType, 24);
            }
        }
    }
}

GAME_TEST(Issues, Issue676) {
    // Jump spell doesn't work
    test->playTraceFromTestData("issue_676.mm7", "issue_676.json");
    EXPECT_EQ(pParty->vPosition, Vec3i(11943, 11586, 857));
}

GAME_TEST(Issues, Issue677) {
    // Haste doesn't impose weakness after it ends
    test->playTraceFromTestData("issue_677.mm7", "issue_677.json");
    for (auto &player : pParty->pPlayers) {
        EXPECT_EQ(player.conditions.Has(Condition_Weak), true);
    }
}

GAME_TEST(Issues, Issue679) {
    // Loading autosave after travelling by stables / boat results in gold loss
    int prevGold = -1;
    test->playTraceFromTestData("issue_679.mm7", "issue_679.json", [&] { prevGold = pParty->GetGold(); });
    EXPECT_EQ(prevGold, pParty->GetGold());
}

GAME_TEST(Issues, Issue689) {
    // Testing that clicking on load game scroll is not crashing the game then there's small amount of saves present.
    std::string savesDir = MakeDataPath("saves");
    std::string savesDirMoved;

    MM_AT_SCOPE_EXIT({
        std::error_code ec;
        std::filesystem::remove_all(savesDir);
        if (!savesDirMoved.empty()) {
            std::filesystem::rename(savesDirMoved, savesDir, ec); // Using std::error_code here, so can't throw.
        }
    });

    if (std::filesystem::exists(savesDir)) {
        savesDirMoved = savesDir + "_moved_for_testing";
        ASSERT_FALSE(std::filesystem::exists(savesDirMoved)); // Throws on failure.
        std::filesystem::rename(savesDir, savesDirMoved);
    }

    std::filesystem::create_directory(savesDir);

    game->pressGuiButton("MainMenu_NewGame");
    game->tick(2);
    game->pressGuiButton("PartyCreation_OK");
    game->skipLoadingScreen();
    game->tick(2);

    game->pressAndReleaseKey(PlatformKey::Escape);
    game->tick(2);
    game->pressGuiButton("GameMenu_SaveGame");
    game->tick(10);
    game->pressGuiButton("SaveMenu_Slot0");
    game->tick(2);
    game->pressAndReleaseKey(PlatformKey::Digit0);
    game->tick(2);
    game->pressGuiButton("SaveMenu_Save");
    game->tick(10);

    game->pressAndReleaseKey(PlatformKey::Escape);
    game->tick(2);
    game->pressGuiButton("GameMenu_LoadGame");
    game->tick(10);
    game->pressGuiButton("LoadMenu_Scroll"); // Sould not crash
    game->tick(2);
    game->pressGuiButton("LoadMenu_Slot0");
    game->tick(2);
    game->pressGuiButton("LoadMenu_Load");
    game->tick(2);
    game->skipLoadingScreen();
    game->tick(2);

    game->pressAndReleaseKey(PlatformKey::Escape);
    game->tick(2);
    game->pressGuiButton("GameMenu_Quit");
    game->tick(2);
    game->pressGuiButton("GameMenu_Quit");
    game->tick(10);
    game->pressGuiButton("MainMenu_LoadGame"); // Should not crash because of last loaded save
    game->tick(10);
    game->pressGuiButton("LoadMenu_Scroll"); // Sould not crash
}

GAME_TEST(Issues, Issue691) {
    // Test that hitting escape when in transition window does not crash
    test->playTraceFromTestData("issue_691.mm7", "issue_691.json");
    EXPECT_EQ(current_screen_type, CURRENT_SCREEN::SCREEN_GAME);
}

// 700

GAME_TEST(Issues, Issue700) {
    // Test that event check for killed monsters work
    test->playTraceFromTestData("issue_700.mm7", "issue_700.json", [] { EXPECT_EQ(pParty->uNumGold, 21541); });
    EXPECT_EQ(pParty->uNumGold, 21541);
}

GAME_TEST(Issues, Issue720) {
    // Test that quest book is opening fine
    test->playTraceFromTestData("issue_720.mm7", "issue_720.json");
    EXPECT_EQ(current_screen_type, CURRENT_SCREEN::SCREEN_BOOKS);
    EXPECT_EQ(pGUIWindow_CurrentMenu->eWindowType, WINDOW_QuestBook);
}

GAME_TEST(Issues, Issue724) {
    // Test that item potion can be applied to equipped items
    test->playTraceFromTestData("issue_724.mm7", "issue_724.json",
                                [] { EXPECT_NE((pParty->pPlayers[3].GetNthEquippedIndexItem(ITEM_SLOT_MAIN_HAND)->uAttributes & ITEM_HARDENED), ITEM_HARDENED); });
    EXPECT_EQ((pParty->pPlayers[3].GetNthEquippedIndexItem(ITEM_SLOT_MAIN_HAND)->uAttributes & ITEM_HARDENED), ITEM_HARDENED);
}

GAME_TEST(Issues, Issue728) {
    // mousing over facets with nonexisting events shouldn't crash the game
    test->playTraceFromTestData("issue_728.mm7", "issue_728.json");
}

GAME_TEST(Issues, Issue730) {
    // Thrown items are throwing a party of their own
    test->playTraceFromTestData("issue_730.mm7", "issue_730.json");
    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
        EXPECT_EQ(pSpriteObjects[i].vVelocity, Vec3s(0, 0, 0));
    }
}

GAME_TEST(Issues, Issue741) {
    // Game crashing when walking into a wall in Temple of the moon
    test->playTraceFromTestData("issue_741.mm7", "issue_741.json");
    EXPECT_NE(pBLVRenderParams->uPartySectorID, 0);
}

GAME_TEST(Issues, Issue760) {
    // Check that mixing potions when character inventory is full does not discards empty bottle
    test->playTraceFromTestData("issue_760.mm7", "issue_760.json");
    EXPECT_EQ(pParty->pPickedItem.uItemID, ITEM_POTION_BOTTLE);
}

void check783784Buffs(bool haveBuffs) {
    for (PLAYER_BUFFS buff : allPotionBuffs())
        EXPECT_EQ(pParty->pPlayers[0].pPlayerBuffs[buff].Active(), haveBuffs) << "buff=" << static_cast<int>(buff);
}

GAME_TEST(Issues, Issue783) {
    // Check that all player buffs expire after rest.
    GameTime startTime;
    test->playTraceFromTestData("issue_783.mm7", "issue_783.json", [&] {
        startTime = pParty->GetPlayingTime();
        check783784Buffs(true); // Should have all buffs at start.

        // And all buffs should expire way in the future.
        for (PLAYER_BUFFS buff : allPotionBuffs())
            EXPECT_GT(pParty->pPlayers[0].pPlayerBuffs[buff].expireTime, startTime + GameTime::FromHours(10));
    });

    GameTime endTime = pParty->GetPlayingTime();
    EXPECT_GT(endTime, startTime + GameTime::FromHours(8)); // Check that we did rest.
    EXPECT_LT(endTime, startTime + GameTime::FromHours(10)); // Check that we didn't wait out the buff expire times.
    check783784Buffs(false); // Check that the buffs still expired.
}

GAME_TEST(Issues, Issue784) {
    // Check that buff potions actually work.
    test->playTraceFromTestData("issue_784.mm7", "issue_784.json", [] {
        check783784Buffs(false);
    });
    check783784Buffs(true);

    // Check that buffs have effect.
    // Potions were at power 75, that's 75*3=225 points for attribute bonuses.
    const Player &player0 = pParty->pPlayers[0];

    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_AIR));
    EXPECT_EQ(5, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_ATTACK)); // PLAYER_BUFF_BLESS.
    EXPECT_EQ(5, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RANGED_ATTACK)); // PLAYER_BUFF_BLESS.
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_BODY));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_EARTH));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_FIRE));
    EXPECT_EQ(58, player0.GetAttackRecoveryTime(false)); // PLAYER_BUFF_HASTE.
    EXPECT_EQ(59, player0.GetAttackRecoveryTime(true)); // PLAYER_BUFF_HASTE.
    EXPECT_EQ(5, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS)); // PLAYER_BUFF_HEROISM.
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_MIND));
    // No check for PLAYER_BUFF_PRESERVATION.
    // No check for PLAYER_BUFF_SHIELD.
    EXPECT_EQ(5, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_AC_BONUS)); // PLAYER_BUFF_STONESKIN.
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_ACCURACY));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_ENDURANCE));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_INTELLIGENCE));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_LUCK));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_STRENGTH));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_WILLPOWER));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_SPEED));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_WATER));
    // No check for PLAYER_BUFF_WATER_WALK
}
