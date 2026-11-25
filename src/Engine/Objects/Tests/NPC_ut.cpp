#include "Testing/Unit/UnitTest.h"

#include "Engine/GameType.h"
#include "Engine/Objects/NPC.h"

// MM8 renamed `icons.lod`'s NPC-portrait assets to a 4-digit-padded "npcNNNN" (e.g.
// "npc0816"), while MM7 keeps 3-digit "npcNNN" - every unguarded caller silently rendered
// LodTextureCache's "pending" placeholder glyph on MM8 (house proprietors/NPCs, dialogue
// portraits, hired-companion icons). Regression net for `npcPortraitTextureName()`, the shared
// helper that replaced 5 separate ad-hoc `fmt::format("npc{:03}", ...)` call sites.

UNIT_TEST(NPC, PortraitTextureNameUsesThreeDigitsOnMm7) {
    ASSERT_EQ(gameType(), GAME_TYPE_MM7); // Precondition: default gametype, so this test cleans up after itself.

    EXPECT_EQ(npcPortraitTextureName(1), "npc001");
    EXPECT_EQ(npcPortraitTextureName(816), "npc816");
}

UNIT_TEST(NPC, PortraitTextureNameUsesFourDigitsOnMm8) {
    setGameType(GAME_TYPE_MM8);

    EXPECT_EQ(npcPortraitTextureName(1), "npc0001");
    EXPECT_EQ(npcPortraitTextureName(816), "npc0816"); // Real MM8 asset name, confirmed via `LodTool ls icons.lod`.

    setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
}
