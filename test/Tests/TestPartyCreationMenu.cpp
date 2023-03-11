#include "Testing/Game/GameTest.h"

#include "Arcomage/Arcomage.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIProgressBar.h"

#include "Engine/Objects/ItemTable.h"
#include "Engine/SaveLoad.h"

#include "Utility/DataPath.h"
#include "Utility/ScopeGuard.h"

GAME_TEST(Items, GenerateItem) {
    // Calling GenerateItem 100 times shouldn't assert.
    ItemGen item;
    for (int i = 0; i < 100; i++)
        pItemTable->GenerateItem(ITEM_TREASURE_LEVEL_6, 0, &item);
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
    for (bool used : pSavegameUsedSlots)
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
        forEachInventoryItem([] (const ItemGen &item, int /*x*/, int /*y*/) {
            // Calling GetWidth forces the texture to be created.
            assets->GetImage_ColorKey(pItemTable->pItems[item.uItemID].pIconName)->GetWidth();
        });
    });

    // Then can safely check everything.
    forEachInventoryItem([] (const ItemGen &item, int x, int y) {
        Texture *image = assets->GetImage_ColorKey(pItemTable->pItems[item.uItemID].pIconName);
        int width = GetSizeInInventorySlots(image->GetWidth());
        int height = GetSizeInInventorySlots(image->GetHeight());

        EXPECT_LE(x + width, Player::INVENTORY_SLOTS_WIDTH);
        EXPECT_LE(y + height, Player::INVENTORY_SLOTS_HEIGHT);
    });
}

GAME_TEST(Issues, Issue203) {
    // Judge's "I lost it" shouldn't crash.
    test->playTraceFromTestData("issue_203.mm7", "issue_203.json");
}

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

static int partyItemCount() {
    int result = 0;
    for (int i = 0; i < 4; i++)
        for (const ItemGen& item : pParty->pPlayers[i].pOwnItems)
            result += item.uItemID != ITEM_NULL;
    return result;
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
        EXPECT_FALSE(pParty->pPlayers[0].HasItem(ITEM_LEATHER_ARMOR, false));
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
    EXPECT_TRUE(pParty->pPlayers[0].HasItem(ITEM_LEATHER_ARMOR, false)); // That's the item from the trash pile.
    for (int i = 0; i < 4; i++)
        EXPECT_EQ(pParty->pPlayers[i].GetMajorConditionIdx(), Condition_Disease_Weak);
}

GAME_TEST(Issues, Issue293b) {
    // Test that table food in castle Harmondale is pickable only once and gives apples.
    test->playTraceFromTestData("issue_293b.mm7", "issue_293b.json", [] {
        EXPECT_EQ(pParty->uNumFoodRations, 7);
        EXPECT_EQ(partyItemCount(), 18);
        EXPECT_FALSE(pParty->HasItem(ITEM_RED_APPLE));
    });

    EXPECT_EQ(pParty->uNumFoodRations, 7); // No change.
    EXPECT_EQ(partyItemCount(), 19); // +1
    EXPECT_TRUE(pParty->HasItem(ITEM_RED_APPLE)); // That's the table food item.
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
            result += player.uExperience;
        return result;
    };

    // Testing that party auto-casting shrapnel successfully targets rats & kills them, gaining experience.
    engine->config->debug.AllMagic.Set(true);
    engine->config->debug.NoDamage.Set(true);

    uint64_t oldExperience = 0;
    test->playTraceFromTestData("issue_294.mm7", "issue_294.json", [&] { oldExperience = partyExperience(); });
    uint64_t newExperience = partyExperience();
    // EXPECT_GT(newExperience, oldExperience); // Expect the giant rat to be dead after four shrapnel casts from character #4.
    // TODO(captainurist): ^fails now
}

GAME_TEST(Issues, Issue315) {
    test->loadGameFromTestData("issue_315.mm7");
    game->goToMainMenu();
    game->pressGuiButton("MainMenu_NewGame");
    game->tick(2);
    game->pressGuiButton("PartyCreation_OK");
    game->skipLoadingScreen(); // This shouldn't crash.
}

GAME_TEST(Issues, Issue403) {
    // Entering Lincoln shouldn't crash.
    test->playTraceFromTestData("issue_403.mm7", "issue_403.json");
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

GAME_TEST(Issues, Issue402) {
    // Attacking while wearing wetsuits shouldn't assert.
    test->playTraceFromTestData("issue_402.mm7", "issue_402.json");
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
    check427Buffs("a", {0}, true);
    check427Buffs("a", {1, 2, 3}, false);

    // In this test mastery is enough for the whole party
    test->playTraceFromTestData("issue_427b.mm7", "issue_427b.json");

    // Check that all character have buffs
    check427Buffs("b", {0, 1, 2, 3}, true);
}

uint64_t GetPartyHealth() {
    uint64_t result = 0;
    for (const Player& player : pParty->pPlayers)
        result += player.sHealth;
    return result;
}

GAME_TEST(Issues, Issue125) {
    // check that fireballs hurt party
    engine->config->debug.AllMagic.Set(true);

    uint64_t oldHealth = 0;
    test->playTraceFromTestData("issue_125.mm7", "issue_125.json", [&] { oldHealth = GetPartyHealth(); });
    uint64_t newHealth = GetPartyHealth();
    EXPECT_LT(newHealth, oldHealth);
}

GAME_TEST(Issues, Issue159) {
    // Exception when entering Tidewater Caverns
    test->playTraceFromTestData("issue_159.mm7", "issue_159.json");
}

GAME_TEST(Issue, Issue123) {
    // Party falls when flying
    test->playTraceFromTestData("issue_123.mm7", "issue_123.json");
    // check party is still in the air
    EXPECT_GT(pParty->vPosition.z, 512);
}

GAME_TEST(Issue, Issue201) {
    // Unhandled EVENT_ShowMovie in Event Processor
    uint64_t oldHealth = 0;
    uint64_t oldgametime{};
    test->playTraceFromTestData("issue_201.mm7", "issue_201.json", [&] { oldHealth = GetPartyHealth(); oldgametime = pParty->GetPlayingTime().GetDays(); });

    // party should heal
    uint64_t newHealth = GetPartyHealth();
    EXPECT_GT(newHealth, oldHealth);
    // we should be teleported to harmondale
    EXPECT_EQ(pCurrentMapName, "Out02.odm");
    // time should advance by a week
    uint64_t newtime = pParty->GetPlayingTime().GetDays();
    EXPECT_EQ((oldgametime + GameTime(0, 0, 0, 0, 1).GetDays()), newtime);
}

GAME_TEST(Issue, Issue202) {
    //Judge doesn't move to house and stays with the party
    int oldhirecount{};
    test->playTraceFromTestData("issue_202.mm7", "issue_202.json", [&]() {oldhirecount = pParty->CountHirelings(); });
    // judge shouldnt be with party anymore
    EXPECT_EQ(oldhirecount - 1, pParty->CountHirelings());
    // party align evil
    EXPECT_EQ(pParty->alignment, PartyAlignment_Evil);
}

GAME_TEST(Issue, Issue211) {
    // Crash during accidental ok double click
    test->playTraceFromTestData("issue_211.mm7", "issue_211.json");
}


static void check223res(CHARACTER_ATTRIBUTE_TYPE res, std::initializer_list<std::pair<int, int>> playerrespairs) {
    for (auto pair : playerrespairs) {
        EXPECT_EQ(pParty->pPlayers[pair.first].GetActualResistance(res), pair.second);
    }
}

GAME_TEST(Issue, Issue223) {
    // Fire and air resistance not resetting between games
    test->playTraceFromTestData("issue_223.mm7", "issue_223.json");
    // expect normal resistances 55-00-00-00
    check223res(CHARACTER_ATTRIBUTE_RESIST_FIRE, { {0, 5}, {1, 0}, {2, 0}, {3, 0} });
    check223res(CHARACTER_ATTRIBUTE_RESIST_AIR, { {0, 5}, {1, 0}, {2, 0}, {3, 0} });
}

GAME_TEST(Prs, Pr469) {
    // Assert when using Quick Spell button when spell is not set
    test->playTraceFromTestData("pr_469.mm7", "pr_469.json");
}

GAME_TEST(Issue, Issue331) {
    // Assert when traveling by horse caused by out of bound access to pObjectList->pObjects
    test->playTraceFromTestData("issue_331.mm7", "issue_331.json");
}

static void check395exp(std::initializer_list<std::pair<int, int>> playerexppairs) {
    for (auto pair : playerexppairs) {
        EXPECT_EQ(pParty->pPlayers[pair.first].uExperience, pair.second);
    }
}

GAME_TEST(Issue, Issue395) {
    // Check that learning skill works as intended
    test->playTraceFromTestData("issue_395.mm7", "issue_395.json", []() { check395exp({ {0, 100}, {1, 100}, {2, 100}, {3, 100} }); });
    check395exp({ {0, 214}, {1, 228}, {2, 237}, {3, 258} });
}

GAME_TEST(Issue, Issue490) {
    // Check that Poison Spray sprites are moving and doing damage
    test->playTraceFromTestData("issue_490.mm7", "issue_490.json", []() { EXPECT_EQ(pParty->pPlayers[0].uExperience, 279); });
    EXPECT_EQ(pParty->pPlayers[0].uExperience, 285);
}

GAME_TEST(Issue, Issue491) {
    // Check that opening and closing Lloyd book does not cause Segmentation Fault
    test->playTraceFromTestData("issue_491.mm7", "issue_491.json");
}

GAME_TEST(Issue, Issue492) {
    // Check that spells that target all visible actors work
    test->playTraceFromTestData("issue_492.mm7", "issue_492.json", []() { EXPECT_EQ(pParty->pPlayers[0].uExperience, 279); });
    EXPECT_EQ(pParty->pPlayers[0].uExperience, 287);
}

GAME_TEST(Issue, Issue502) {
    // Check that script face animation and voice indexes right characters
    test->playTraceFromTestData("issue_502.mm7", "issue_502.json");
}

static void check503health(std::initializer_list<std::pair<int, int>> playerhealthpairs) {
    for (auto pair : playerhealthpairs) {
        EXPECT_EQ(pParty->pPlayers[pair.first].sHealth, pair.second);
    }
}

GAME_TEST(Issue, Issue503) {
    // Check that town portal book actually pauses game
    test->playTraceFromTestData("issue_503.mm7", "issue_503.json", []() { check503health({ {0, 1147}, {1, 699}, {2, 350}, {3, 242} }); });
    check503health({ {0, 1147}, {1, 699}, {2, 350}, {3, 242} });
}

GAME_TEST(Issue, Issue506) {
    // Check that scroll use does not assert
    test->playTraceFromTestData("issue_506.mm7", "issue_506.json");
}

GAME_TEST(Issues, Issue268) {
    // Crash in ODM_GetFloorLevel
    test->playTraceFromTestData("issue_268.mm7", "issue_268.json");
}

GAME_TEST(Issues, Issue271) {
    // Party shouldnt yell when landing from flight
    test->playTraceFromTestData("issue_271.mm7", "issue_271.json");
    EXPECT_NE(pParty->pPlayers[1].expression, CHARACTER_EXPRESSION_FEAR);
}

GAME_TEST(Issues, Issue520) {
    // Party should take fall damage
    uint64_t oldHealth = 0;
    test->playTraceFromTestData("issue_520.mm7", "issue_520.json", [&] { oldHealth = GetPartyHealth(); });
    EXPECT_LT(GetPartyHealth(), oldHealth);
}

// This cant be tested properly using the current framework
//GAME_TEST(Issues, Issue405) {
//    // FPS affects effective recovery time
//    // play trace at 60fps
//    engine->config->debug.AllMagic.Set(true);
//    engine->config->graphics.FPSLimit.Set(63);
//    test->playTraceFromTestData("issue_405.mm7", "issue_405.json");
//    int remainingtime60{ pPlayers[1]->uTimeToRecovery };
//
//    // play trace at max fps
//    engine->config->debug.AllMagic.Set(true);
//    engine->config->graphics.FPSLimit.Set(0);
//    test->playTraceFromTestData("issue_405.mm7", "issue_405.json");
//    int remainingtimemax{ pPlayers[1]->uTimeToRecovery };
//
//    // recovered amount should match
//    EXPECT_EQ(remainingtime60, remainingtimemax);
//}
