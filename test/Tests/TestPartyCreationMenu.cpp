#include "Testing/Game/GameTest.h"

#include "GUI/GUIWindow.h"

GAME_TEST(Menu, PartyCreation) {
    game->Tick(100);

    EXPECT_EQ(GetCurrentMenuID(), MENU_MAIN);

    game->LClick(495, 172); // New game
    game->Tick(10);

    EXPECT_EQ(GetCurrentMenuID(), MENU_CREATEPARTY);

    EXPECT_EQ(pParty->pPlayers[0].uMight, 30);

    game->LClick(527, 431); // Clear, shouldn't crash
    game->Tick();

    EXPECT_EQ(pParty->pPlayers[0].classType, PLAYER_CLASS_KNIGHT);
    EXPECT_EQ(pParty->pPlayers[0].uMight, 14);
}
