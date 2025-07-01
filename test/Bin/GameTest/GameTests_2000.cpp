#include "Testing/Game/GameTest.h"

#include "Engine/Engine.h"
#include "Engine/MapEnums.h"
#include "Engine/Party.h"
#include "Engine/Objects/Chest.h"


// 2000

GAME_TEST(Issues, Issue2002) {
    // Character recovery is carried over when loading a saved game in turn based mode
    // start game and enter turn based mode
    game.startNewGame();
    game.pressAndReleaseKey(PlatformKey::KEY_RETURN);
    game.tick(15);
    for (int i = 0; i < 3; ++i) {
        game.pressKey(PlatformKey::KEY_A); // Attack with 3 chars
        game.tick();
        game.releaseKey(PlatformKey::KEY_A);
        game.tick();
    }

    // check recovery
    EXPECT_TRUE(pParty->bTurnBasedModeOn);
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(pParty->pCharacters[i].timeToRecovery > 0_ticks);
    }
    EXPECT_FALSE(pParty->pCharacters[3].timeToRecovery > 0_ticks);

    // now load a saved game
    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_LoadGame");
    game.tick(10);
    game.pressGuiButton("LoadMenu_Slot0");
    game.tick(2);
    game.pressGuiButton("LoadMenu_Load");
    game.tick(2);
    game.skipLoadingScreen();
    game.tick(2);

    // check recovery again
    for (int i = 0; i < 4; i++) {
        EXPECT_FALSE(pParty->pCharacters[i].timeToRecovery > 0_ticks);
    }
    EXPECT_FALSE(pParty->bTurnBasedModeOn);
}

GAME_TEST(Issues, Issue2017) {
    // Bats move through closed doors in Barrow XII
    test.playTraceFromTestData("issue_2017.mm7", "issue_2017.json");
    // Make sure all the monsters have stayed contained
    constexpr std::array<int, 3> monsterIds = { 12, 13, 20 };
    for (auto ids : monsterIds) {
        EXPECT_LT(pActors[ids].pos.y, -100);
    }
}

GAME_TEST(Issues, Issue2018) {
    // Scrolls of Town Portal and Lloyd's Beacon did consume mana or assert when cast by a character with insufficient mana.
    auto mapTape = tapes.map();
    auto scrollsLBTape = tapes.totalItemCount(ITEM_SCROLL_LLOYDS_BEACON);
    auto scrollsTPTape = tapes.totalItemCount(ITEM_SCROLL_TOWN_PORTAL);
    auto recoveryTape = charTapes.areRecovering();
    auto mpTape = charTapes.mps();
    test.playTraceFromTestData("issue_2018.mm7", "issue_2018.json");
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_ERATHIA, MAP_EMERALD_ISLAND));
    EXPECT_EQ(scrollsLBTape.frontBack(), tape(6, 4)); // Used 2 Lloyd's out of 6, ignore intervening steps from pickup and r-click.
    EXPECT_EQ(scrollsTPTape.frontBack(), tape(4, 2)); // Also used 2 Town Portal because it fails once on the Thief.
    EXPECT_MISSES(recoveryTape.slice(0), true); // Char 0 didn't do anything.
    EXPECT_CONTAINS(recoveryTape.slice(1), true); // Char 1 did cast a spell.
    EXPECT_LT(mpTape.slice(1).max(), 20); // Char 1 didn't have enough mana for the spells cast.
    EXPECT_MISSES(recoveryTape.slice(2), true); // Char 2 didn't do anything.
    EXPECT_CONTAINS(recoveryTape.slice(3), true); // Char 3 did cast a spell.
    EXPECT_LT(mpTape.slice(3).max(), 20); // Char 3 didn't have enough mana for the spells cast.
    EXPECT_EQ(mpTape.back(), mpTape.front()); // No mana was spent.
}

GAME_TEST(Issues, Issue2021_2022) {
    // Lloyd's Beacon did not keep beacons in the player-selected slot.
    // Also, OE did allow characters in recovery to cast from spell scrolls.
    // The trace does a similar portal to Erathia and back as Issue2018, but selects the center slot to do so.
    // Additionally, it tries to cast a Protection from Magic scroll on a 'greyed' character - should _not_ succeed.
    auto mapTape = tapes.map();
    auto scrollsPMTape = tapes.totalItemCount(ITEM_SCROLL_PROTECTION_FROM_MAGIC);
    auto soundsTape = tapes.sounds();
    auto statusTape = tapes.statusBar();
    auto pmBuffTape = tapes.custom([] { return pParty->pPartyBuffs[PARTY_BUFF_PROTECTION_FROM_MAGIC].Active(); });
    auto lloydSlot1Tape = tapes.custom([] { return static_cast<bool>(pParty->pCharacters[3].vBeacons[0]); });
    test.playTraceFromTestData("issue_2021_2022.mm7", "issue_2021_2022.json");
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_ERATHIA, MAP_EMERALD_ISLAND));
    EXPECT_EQ(scrollsPMTape.delta(), 0); // No Protection from Magic scroll used
    EXPECT_CONTAINS(soundsTape.flatten(), SOUND_error);
    EXPECT_CONTAINS(statusTape, "That player is not active");
    EXPECT_EQ(lloydSlot1Tape, tape(false)); // Top left slot stayed empty
    EXPECT_EQ(pmBuffTape, tape(false)); // Not Prot Mg buff received
}

GAME_TEST(Issues, Issue2061) {
    // Game Crashes if you click the border of the inventory screen.
    game.startNewGame();
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_1);
    game.tick();
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_1);
    game.tick();
    game.pressAndReleaseKey(PlatformKey::KEY_I);
    game.tick();
    game.pressAndReleaseButton(BUTTON_LEFT, 3, 20); // This used to assert.
    game.tick();
    EXPECT_EQ(pParty->pPickedItem.itemId, ITEM_NULL); // Shouldn't pick anything.
}

GAME_TEST(Issues, Issue2066) {
    // No error sound when trying to place item outside of inventory boundaries.
    auto soundsTape = tapes.sounds();
    game.startNewGame();
    test.startTaping();

    pParty->pCharacters[0].inventory.clear();
    pParty->pCharacters[0].inventory.add(Pointi(0, 0), Item(ITEM_LEATHER_ARMOR)); // Add leather armor at (0, 0).

    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_1);
    game.tick();
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_1);
    game.tick();
    game.pressAndReleaseKey(PlatformKey::KEY_I);
    game.tick();
    game.pressAndReleaseButton(BUTTON_LEFT, 30, 30); // Pick up leather armor.
    game.tick();
    game.pressAndReleaseButton(BUTTON_LEFT, 30, 0); // Try to place outside inventory boundaries.
    game.tick(2); // Two ticks so that the taping engine doesn't merge frames with SOUND_error.
    game.pressAndReleaseButton(BUTTON_LEFT, 0, 30);
    game.tick(2);
    game.pressAndReleaseButton(BUTTON_LEFT, 476 - 60, 30);
    game.tick(2);
    game.pressAndReleaseButton(BUTTON_LEFT, 30, 345 - 60);
    game.tick(2);

    EXPECT_EQ(soundsTape.flatten().count(SOUND_error), 4); // Get 4 errors.
}

GAME_TEST(Issues, Issue2074) {
    // Re-entering castle gryphonheart causes NPCs to become hostile
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_2074.mm7", "issue_2074.json");
    EXPECT_EQ(mapTape, tape(MAP_ERATHIA, MAP_CASTLE_GRYPHONHEART, MAP_ERATHIA, MAP_CASTLE_GRYPHONHEART));

    for (const auto& actor : pActors)
        EXPECT_EQ(std::to_underlying(actor.attributes & ACTOR_AGGRESSOR), 0); // Check that the NPCs arent hostile
    EXPECT_EQ(engine->_persistentVariables.mapVars[4], 0); // check for persistant castle aggro var - 2 when angered
}

GAME_TEST(Issues, Issue2075) {
    // Paralyze works on dead enemies
    auto turnBased = tapes.custom([] { return pParty->bTurnBasedModeOn; });
    auto statusTape = tapes.statusBar();
    auto actorsTape = tapes.custom([] { return pActors[2].aiState; });
    test.playTraceFromTestData("issue_2075.mm7", "issue_2075.json");
    EXPECT_TRUE(turnBased.back());
    EXPECT_CONTAINS(statusTape, "Spell failed");
    EXPECT_EQ(actorsTape.back(), Dead);
    EXPECT_EQ(actorsTape.size(), 1);
}

GAME_TEST(Prs, Pr2083) {
    // Check that wands are generated with correct number of charges.
    for (int i = 0; i < 100; i++) {
        Item wand;
        wand.itemId = ITEM_MYSTIC_WAND_OF_SHRAPMETAL;
        wand.postGenerate(ITEM_SOURCE_MONSTER);
        EXPECT_EQ(wand.numCharges, wand.maxCharges);
        EXPECT_GE(wand.numCharges, 15+1);
        EXPECT_LE(wand.numCharges, 15+6);
    }
}

GAME_TEST(Issues, Issue2099) {
    // Opening chest asserts.
    // We had chests generating with `ITEM_RANDOM_LEVEL_1` and asserting on open, we're just restarting the game three
    // times (effectively with different random seeds), and checking the chests.
    for (int iteration = 0; iteration < 3; iteration++) {
        game.startNewGame();
        for (const Chest &chest : vChests)
            for (InventoryConstEntry entry : chest.inventory.entries())
                EXPECT_FALSE(isRandomItem(entry->itemId));
    }
}
