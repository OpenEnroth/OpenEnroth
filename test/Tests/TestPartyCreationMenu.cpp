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

GAME_TEST(Issues, Issue125) {
    // check that fireballs hurt party
    auto partyHealth = [&] {
        uint64_t result = 0;
        for (const Player& player : pParty->pPlayers)
            result += player.sHealth;
        return result;
    };

    engine->config->debug.AllMagic.Set(true);

    uint64_t oldHealth = 0;
    test->playTraceFromTestData("issue_125.mm7", "issue_125.json", [&] { oldHealth = partyHealth(); });
    uint64_t newHealth = partyHealth();
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

GAME_TEST(Prs, Pr469) {
    // Assert when using Quick Spell button when spell is not set
    test->playTraceFromTestData("pr_469.mm7", "pr_469.json");
}
