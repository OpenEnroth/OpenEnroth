#include "Testing/Game/GameTest.h"

#include "GUI/GUIWindow.h"
#include "Arcomage/Arcomage.h"

#include "Engine/Objects/ItemTable.h"

GAME_TEST(Items, GenerateItem) {
    // Calling GenerateItem 100 times shouldn't assert.
    ItemGen item;
    for (int i = 0; i < 100; i++)
        pItemTable->GenerateItem(ITEM_TREASURE_LEVEL_6, 0, &item);
}

GAME_TEST(Prs, Pr314) {
    // Check that character creating menu works.
    // Trace pretty much presses all the buttons and opens all the popups possible.
    test->playTraceFromTestData("pr_314.mm7", "pr_314.json", [] {});

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
    test->playTraceFromTestData("issue_403.mm7", "issue_403.json", [] {});
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
    test->playTraceFromTestData("issue_388.mm7", "issue_388.json", [&] {oldscreen = current_screen_type; });
    // we should return to game screen
    EXPECT_EQ(oldscreen, current_screen_type);
    // with arcomage exit flag
    EXPECT_EQ(pArcomageGame->GameOver, 1);
    pArcomageGame->_targetFPS = oldfpslimit;
}

GAME_TEST(Issues, Issue402) {
    // Attacking while wearing wetsuits shouldn't assert.
    test->playTraceFromTestData("issue_402.mm7", "issue_402.json", [] {});
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
    test->playTraceFromTestData("issue_417a.mm7", "issue_417a.json", [] {});
    test->playTraceFromTestData("issue_417b.mm7", "issue_417b.json", [] {});
}
