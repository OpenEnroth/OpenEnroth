#include "Testing/Game/GameTest.h"

#include "GUI/GUIWindow.h"

GAME_TEST(Menu, PartyCreation) {
    EXPECT_EQ(GetCurrentMenuID(), MENU_MAIN);

    game->pressGuiButton("MainMenu_NewGame");
    game->tick(2);

    EXPECT_EQ(GetCurrentMenuID(), MENU_CREATEPARTY);
    EXPECT_EQ(pParty->pPlayers[0].uMight, 30);

    game->pressGuiButton("PartyCreation_Clear"); // This shouldn't crash.
    game->tick();

    EXPECT_EQ(pParty->pPlayers[0].classType, PLAYER_CLASS_KNIGHT);
    EXPECT_EQ(pParty->pPlayers[0].uMight, 14);
}

GAME_TEST(Issues, Issue315) {
    EXPECT_EQ(GetCurrentMenuID(), MENU_MAIN);

    test->loadGameFromTestData("issue_315.mm7");
    game->goToMainMenu();
    game->pressGuiButton("MainMenu_NewGame");
    game->tick(2);
    game->pressGuiButton("PartyCreation_OK");
    game->skipLoadingScreen(); // This shouldn't crash.
}

GAME_TEST(Prs, Pr347) {
    // Testing that shops work.
    int oldGold = 0;
    test->playTraceFromTestData("pr_347.mm7", "pr_347.json", [&] { oldGold = pParty->uNumGold; });
    EXPECT_NE(oldGold, pParty->uNumGold); // Spent on items.
}

