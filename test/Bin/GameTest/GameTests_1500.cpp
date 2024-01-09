#include <unordered_set>

#include "Testing/Game/GameTest.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIPartyCreation.h"

// 1500

GAME_TEST(Issues, Issue1503) {
    // Can start new game without 4 skills selected
    test.playTraceFromTestData("issue_1503.mm7", "issue_1503.json");
    EXPECT_EQ(current_screen_type, SCREEN_PARTY_CREATION);
    EXPECT_FALSE(PlayerCreation_Choose4Skills());
}
