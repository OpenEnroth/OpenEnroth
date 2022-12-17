#include "Testing/Game/GameTest.h"

#include "GUI/GUIWindow.h"

GAME_TEST(Menu, PartyCreation) {
    EXPECT_EQ(GetCurrentMenuID(), MENU_MAIN);

    game->PressGuiButton("MainMenu_NewGame");
    game->Tick(2);

    EXPECT_EQ(GetCurrentMenuID(), MENU_CREATEPARTY);
    EXPECT_EQ(pParty->pPlayers[0].uMight, 30);

    game->PressGuiButton("PartyCreation_Clear"); // This shouldn't crash.
    game->Tick();

    EXPECT_EQ(pParty->pPlayers[0].classType, PLAYER_CLASS_KNIGHT);
    EXPECT_EQ(pParty->pPlayers[0].uMight, 14);
}

GAME_TEST(Menu, SomethingElse) {
    EXPECT_EQ(GetCurrentMenuID(), MENU_MAIN);
}
