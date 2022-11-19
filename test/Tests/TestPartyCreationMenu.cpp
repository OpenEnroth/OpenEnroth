#include "Testing/Game/GameTest.h"

#include "GUI/GUIWindow.h"

GAME_TEST(Menu, PartyCreation) {
    Game().Tick(100);

    EXPECT_EQ(GetCurrentMenuID(), MENU_MAIN);

    Game().LClick(495, 172); // New game
    Game().Tick(10);

    EXPECT_EQ(GetCurrentMenuID(), MENU_CREATEPARTY);

    Game().LClick(527, 431); // Clear, shouldn't crash
    Game().Tick();

    EXPECT_EQ(pParty->pPlayers[0].classType, PLAYER_CLASS_KNIGHT);
}
