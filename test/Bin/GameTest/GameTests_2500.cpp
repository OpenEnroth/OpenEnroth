#include <algorithm>
#include <string>
#include <utility>

#include "Testing/Game/GameTest.h"

#include "Engine/Engine.h"
#include "Engine/MapEnums.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Data/HouseEnums.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Tables/DecorationTable.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UISaveLoad.h"

#include "Media/Audio/SoundList.h"

#include "GameTestCommon.h"

static AccessibleVector<std::string> soundNames(const TestMultiTape<SoundId> &soundsTape) {
    return soundsTape.flatten().map([](SoundId id) { return pSoundList->soundInfo(id)->name; });
}

// 2500

GAME_TEST(Issues, Issue2500a) {
    // Attack preferences are broken. Some monsters attack archers while they should have no attack pref.
    // Issue2500b covers a class preference, Issue2500c a race one, and Issue2146 promoted classes.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);
    auto hp0Tape = charTapes.hp(0);
    auto hp1Tape = charTapes.hp(1);

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest({{CLASS_KNIGHT, RACE_HUMAN}, {CLASS_ARCHER, RACE_HUMAN}});

    // Spawn dragonflies in waves so we get plenty of attacks.
    engine->config->debug.NoActors.setValue(false);
    for (int i = 0; i < 8; i++) {
        game.tick(7);
        game.spawnMonster(pParty->pos + Vec3f(0, 200, 0), MONSTER_DRAGONFLY_B);
    }
    game.tick(200);

    EXPECT_LT(hp0Tape.back(), hp0Tape.front()); // Knight took damage too - no spurious archer preference.
    EXPECT_LT(hp1Tape.back(), hp1Tape.front()); // Sanity: archer also took damage.
}

GAME_TEST(Issues, Issue2500b) {
    // Attack preferences are broken. Archers are missing archer attack preference.
    // Issue2500a covers having no preference, Issue2500c a race one, and Issue2146 promoted classes.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);
    auto hp0Tape = charTapes.hp(0);
    auto hp1Tape = charTapes.hp(1);

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest({{CLASS_KNIGHT, RACE_HUMAN}, {CLASS_ARCHER, RACE_HUMAN}});

    engine->config->debug.NoActors.setValue(false);
    for (int i = 0; i < 4; i++) {
        game.tick(7);
        game.spawnMonster(pParty->pos + Vec3f(0, 1500, 0), MONSTER_ARCHER_A, SPAWN_STATIONARY); // Stay in place & shoot.
    }
    game.tick(200);

    EXPECT_EQ(hp0Tape.back(), hp0Tape.front()); // Knight is untouched - archer monsters target archers only.
    EXPECT_LT(hp1Tape.back(), hp1Tape.front()); // Archer took the damage.
}

GAME_TEST(Issues, Issue2500c) {
    // Attack preferences are broken. Dwarven Commanders are missing goblin attack preference.
    // Issue2500a covers having no preference, Issue2500b a class one, and Issue2146 promoted classes.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);
    auto hp0Tape = charTapes.hp(0);
    auto hp1Tape = charTapes.hp(1);

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest({{CLASS_KNIGHT, RACE_HUMAN}, {CLASS_KNIGHT, RACE_GOBLIN}});

    engine->config->debug.NoActors.setValue(false);
    for (int i = 0; i < 4; i++) {
        game.tick(7);
        game.spawnMonster(pParty->pos + Vec3f(0, 200, 0), MONSTER_DWARF_C);
    }
    game.tick(200);

    EXPECT_EQ(hp0Tape.back(), hp0Tape.front()); // Human is untouched - Dwarven Commander targets goblins.
    EXPECT_LT(hp1Tape.back(), hp1Tape.front()); // Goblin took the damage.
}

GAME_TEST(Issues, Issue2503) {
    //A new party getting weak mechanic differs from Vanilla
    game.startNewGame();
    game.tick();
    game.pressAndReleaseKey(PlatformKey::KEY_R);
    game.tick();
    game.pressGuiButton("Rest_WaitTillDawn");
    game.tick(30);
    for (const auto& charac : pParty->pCharacters) {
        EXPECT_TRUE(charac.IsWeak());
    }
}

GAME_TEST(Issues, Issue2505) {
    // Water elementals supposed to cast ice blast, casting ice bolt.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);
    auto spritesTape = tapes.sprites();

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest();

    engine->config->debug.NoActors.setValue(false);
    game.spawnMonster(pParty->pos + Vec3f(0, 1500, 0), MONSTER_ELEMENTAL_WATER_C, SPAWN_STATIONARY);
    game.tick(300); // Cast chance is 30%, shall be enough.

    auto flat = spritesTape.flatten();
    EXPECT_CONTAINS(flat, SPRITE_SPELL_WATER_ICE_BLAST);
    EXPECT_MISSES(flat, SPRITE_SPELL_WATER_ICE_BOLT);
}

GAME_TEST(Issues, Issue2507) {
    // Efreet lightning bolt deals 0 damage.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);
    auto hpTape = charTapes.hp(0);
    auto spritesTape = tapes.sprites();

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest();

    engine->config->debug.NoActors.setValue(false);
    Actor *efreet = game.spawnMonster(pParty->pos + Vec3f(0, 1500, 0), MONSTER_GENIE_C, SPAWN_STATIONARY);
    EXPECT_EQ(efreet->monsterInfo.spell1Id, SPELL_AIR_LIGHTNING_BOLT);
    EXPECT_EQ(efreet->monsterInfo.spell1SkillMastery.level(), 10);
    EXPECT_EQ(efreet->monsterInfo.spell1SkillMastery.mastery(), MASTERY_MASTER);

    // Force the Efreet's spell to fire every time.
    efreet->monsterInfo.spell1UseChance = 100;
    game.tick(200);

    EXPECT_CONTAINS(spritesTape.flatten(), SPRITE_SPELL_AIR_LIGHTNING_BOLT); // Lightning Bolt was cast.
    auto damageRange = hpTape.reverse().adjacentDeltas().minMax();
    EXPECT_GE(damageRange[0], 10);
    EXPECT_LE(damageRange[1], 80); // Per-hit damage is within the 10d8 range.
}

GAME_TEST(Issues, Issue2551a) {
    // Save list was capped at 45 files, and saving past the cap produced saves that weren't listed in the load menu.
    game.startNewGame();
    game.tick(2);

    // Fill the saves folder well past the old cap.
    ufs->remove("saves/autosave.mm7");
    Blob save = game.saveGame();
    for (int i = 0; i < 50; i++)
        ufs->write(fmt::format("saves/save{:03}.mm7", i), save);

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(2);
    GUIWindow_SaveLoad *saveMenu = saveLoadMenu();
    ASSERT_EQ(saveMenu->slots().size(), 51); // All 50 saves are listed, plus the new save slot...
    ASSERT_TRUE(saveMenu->selectedSlot().fileName.empty()); // ...which is selected, as this game wasn't loaded from a save.

    // Pressing the save button right away shouldn't save, we want a save name typed in first.
    game.pressGuiButton("SaveMenu_Save");
    game.tick(2);
    EXPECT_EQ(ufs->ls("saves").size(), 50);
    game.pressAndReleaseKey(PlatformKey::KEY_A);
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_RETURN); // Confirm the name with Enter.
    game.tick(10);
    EXPECT_EQ(ufs->ls("saves").size(), 51); // New save went into a new file.
    EXPECT_TRUE(ufs->exists("saves/save050.mm7"));

    // The new save shows up in the load menu, sorted by display name - the other saves have blank titles & fall
    // back to "saveNNN" display names, so "a" sorts first.
    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_LoadGame");
    game.tick(3);
    GUIWindow_SaveLoad *loadMenu = saveLoadMenu();
    EXPECT_EQ(loadMenu->slots().size(), 51);
    EXPECT_EQ(loadMenu->slots()[0].fileName, "save050.mm7");
    EXPECT_EQ(loadMenu->slots()[0].header.name, "a");
}

GAME_TEST(Issues, Issue2551b) {
    // Quickload is handled from inside menus, check that a failed quickload doesn't break the open save menu.
    game.startNewGame();
    game.tick(2);

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_F9); // Try to quickload, there is no quicksave so this should fail.
    game.tick(3);
    ASSERT_EQ(ufs->ls("saves").size(), 1); // Just the autosave...
    GUIWindow_SaveLoad *saveMenu = saveLoadMenu();
    ASSERT_EQ(saveMenu->slots().size(), 1); // ...which is not shown in the save menu, so it's just the new save slot.

    // And the menu is still fully functional.
    game.doubleClickGuiButton("SaveMenu_Slot0"); // Double click the new save slot to start the name input.
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_A);
    game.tick(2);
    game.pressGuiButton("SaveMenu_Save");
    game.tick(10);
    EXPECT_TRUE(ufs->exists("saves/save000.mm7"));

    // Quicksaves are also hidden from the save menu.
    game.pressAndReleaseKey(PlatformKey::KEY_F5); // Quicksave.
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(2);
    ASSERT_EQ(ufs->ls("saves").size(), 3); // Autosave, quicksave & our save...
    GUIWindow_SaveLoad *saveMenu2 = saveLoadMenu();
    ASSERT_EQ(saveMenu2->slots().size(), 2); // ...but only the new save slot & our save are shown.
}

GAME_TEST(Issues, Issue2551c) {
    // Quickload works from the game, from the in-game save menu & from the main menu.
    game.startNewGame();
    game.tick(2);
    engine->config->gameplay.QuickSavesCount.setValue(4); // Quicksave counter wraps 4 -> 0.
    game.pressAndReleaseKey(PlatformKey::KEY_F5); // Quicksave.
    game.tick(2);
    std::string quickSaveName = getCurrentQuickSave();
    ASSERT_EQ(quickSaveName, "quicksave0.mm7");

    auto checkQuickLoaded = [&] {
        game.skipLoadingScreen();
        game.tick(2);
        EXPECT_EQ(current_screen_type, SCREEN_GAME);
        EXPECT_EQ(engine->_lastLoadedSaveFileName, quickSaveName);
        engine->_lastLoadedSaveFileName.clear(); // Make sure the next check is not vacuous.
    };

    // Quickload in-game.
    game.pressAndReleaseKey(PlatformKey::KEY_F9);
    checkQuickLoaded();

    // Quickload from the in-game save menu.
    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(2);
    ASSERT_EQ(current_screen_type, SCREEN_SAVEGAME);
    game.pressAndReleaseKey(PlatformKey::KEY_F9);
    checkQuickLoaded();

    // Quickload from the main menu.
    game.goToMainMenu();
    game.pressAndReleaseKey(PlatformKey::KEY_F9);
    checkQuickLoaded();

    // Quickload from the load menu in the main menu.
    game.goToMainMenu();
    game.pressGuiButton("MainMenu_LoadGame");
    game.tick(3);
    ASSERT_EQ(current_screen_type, SCREEN_LOADGAME);
    game.pressAndReleaseKey(PlatformKey::KEY_F9);
    checkQuickLoaded();
}

GAME_TEST(Prs, Pr2599) {
    // Leaving the Hidden Tomb while looking up used to leave the party staring at the sky in Erathia. Every shipped
    // MoveToMap carries pitch 0, and the original game applies a script's pitch only when it's non-zero, the view
    // was leveled by the target map's party start decoration instead, and Erathia has none - only the four
    // directional ones for foot travel. So this is vanilla behaviour, and we deliberately apply the script's pitch
    // as is, like on every other arrival.
    auto mapTape = tapes.map();
    game.startNewGame();
    game.teleportTo(MAP_HIDDEN_TOMB, Vec3f(-111, -25, 1), 0); // Just inside the entrance, facing the exit.
    test.startTaping();
    game.pressKey(PlatformKey::KEY_PAGEDOWN); // Look up.
    game.tick(10);
    game.releaseKey(PlatformKey::KEY_PAGEDOWN);
    EXPECT_EQ(pParty->_viewPitch, 125);
    game.pressKey(PlatformKey::KEY_UP);
    game.tick(10);
    game.releaseKey(PlatformKey::KEY_UP);
    game.pressAndReleaseKey(PlatformKey::KEY_SPACE);
    game.tick();
    game.pressGuiButton("Transition_Yes");
    game.tick();
    game.skipLoadingScreen();
    EXPECT_EQ(mapTape, tape(MAP_HIDDEN_TOMB, MAP_ERATHIA));
    EXPECT_EQ(pParty->pos.x, 14207);
    EXPECT_EQ(pParty->pos.y, -21526);
    EXPECT_EQ(pParty->_viewYaw, 1536);
    EXPECT_EQ(pParty->_viewPitch, 0);
}

// 2600

GAME_TEST(Prs, Pr2615a) {
    // A decoration's clickable reach extends past the mouse interaction depth by the decoration's radius, so a big
    // one is clickable from further away than its center. This campfire is at depth 560 with radius 52, clickable
    // at the default reach of 512.
    auto foodTape = tapes.food();
    auto statusTape = tapes.statusBar();
    game.startNewGame();
    const LevelDecoration &campfire = pLevelDecorations[7]; // The campfire on the beach.
    ASSERT_EQ(pDecorationTable->decoration(campfire.uDecorationDescID)->hint, "campfire");
    ASSERT_EQ(pDecorationTable->decoration(campfire.uDecorationDescID)->uRadius, 52);
    game.teleportTo(MAP_EMERALD_ISLAND, campfire.vPosition - Vec3f(535, 0, 0), 0); // The pick depth comes out at 560.
    test.startTaping();
    game.pointMouseAtDecoration(7);
    EXPECT_EQ(engine->PickMouseForInteraction().pid, Pid()); // Otherwise the radius allowance isn't what's tested.
    Vis_PIDAndDepth object = engine->PickMouseForTargeting();
    EXPECT_EQ(object.pid, Pid(OBJECT_Decoration, 7));
    EXPECT_GT(object.depth, engine->config->gameplay.MouseInteractionDepth.value());
    game.pressAndReleaseButton(BUTTON_LEFT);
    game.tick(3);
    EXPECT_EQ(foodTape.delta(), 2);
    EXPECT_CONTAINS(statusTape, "You find 2 food");
}

GAME_TEST(Prs, Pr2615b) {
    // Shift-clicking a friendly actor fires the quick spell at it on both sides of the interaction depth, as in
    // vanilla - a friendliness check in the click handler is not allowed to drop the far click.
    for (int depth : {300, 1200}) {
        test.prepareForNextTest();
        engine->config->debug.NoActors.setValue(true);
        engine->config->debug.AllMagic.setValue(true);
        game.startNewGame();
        test.startTaping();
        prepareForBattleTest();
        engine->config->debug.NoActors.setValue(false);

        auto hpTape = actorTapes.hp(0);
        Actor *peasant = game.spawnMonster(pParty->pos + Vec3f(0, depth, 0), MONSTER_PEASANT_DWARF_FEMALE_A_A,
                                           SPAWN_DUMMY | SPAWN_FRIENDLY);
        EXPECT_EQ(peasant->GetActorsRelation(0), HOSTILITY_FRIENDLY); // Otherwise it's the hostile path that's tested.
        pParty->pCharacters[0].uQuickSpell = SPELL_FIRE_FIRE_BOLT;
        game.pointMouseAtActor(0);
        EXPECT_EQ(engine->PickMouseForInteraction().pid == Pid(), depth > engine->config->gameplay.MouseInteractionDepth.value());

        game.pressKey(PlatformKey::KEY_SHIFT);
        game.pressAndReleaseButton(BUTTON_LEFT);
        game.tick(2);
        game.releaseKey(PlatformKey::KEY_SHIFT);
        game.tick(30);
        test.stopTaping();

        EXPECT_LT(hpTape.delta(), 0); // The quick spell fired and hit.
    }
}

GAME_TEST(Prs, Pr2615c) {
    // In vanilla telekinesis doesn't work on decorations at all - its targeting pick skips their billboards, so
    // the decoration branch in castSpell is dead code there, with a garbled line that crashes once reached. OE
    // deliberately lets the pick see decorations, and this is the OE behavior being tested: telekinesis pulls an
    // interactive decoration from beyond click reach.
    auto foodTape = tapes.food();
    auto statusTape = tapes.statusBar();
    game.startNewGame();
    engine->config->debug.AllMagic.setValue(true);
    const LevelDecoration &campfire = pLevelDecorations[7]; // The campfire on the beach.
    ASSERT_EQ(pDecorationTable->decoration(campfire.uDecorationDescID)->hint, "campfire");
    ASSERT_EQ(campfire.uEventID, 0); // The eventless interactive kind - Pr2615d covers the evented kind.
    game.teleportTo(MAP_EMERALD_ISLAND, campfire.vPosition - Vec3f(1000, 0, 0), 0); // Twice the click reach away.
    test.startTaping();
    game.castSpell(0, SPELL_EARTH_TELEKINESIS);
    game.tick(2);
    game.pointMouseAtDecoration(7);
    game.pressAndReleaseButton(BUTTON_LEFT);
    game.tick(3);
    EXPECT_EQ(foodTape.delta(), 2);
    EXPECT_CONTAINS(statusTape, "You find 2 food");
}

GAME_TEST(Prs, Pr2615d) {
    // Telekinesis on a decoration that carries a map event - the campfire in Pr2615c is the interactive kind, this
    // Harmondale fruit tree is the evented kind, and its event hands the player an apple. Fruit trees bear nothing
    // in autumn and winter, and a new game starts on the 1st of January, so the calendar is pushed into summer first.
    game.startNewGame();
    engine->config->debug.AllMagic.setValue(true);
    game.teleportTo(MAP_HARMONDALE, Vec3f(-12192, 9000, 0), 0); // Decorations belong to the loaded map.
    const LevelDecoration &tree = pLevelDecorations[559];
    ASSERT_EQ(pDecorationTable->decoration(tree.uDecorationDescID)->hint, "tree");
    ASSERT_NE(tree.uEventID, 0); // The point of this test - Pr2615c covers the eventless interactive kind.
    game.teleportTo(MAP_HARMONDALE, tree.vPosition - Vec3f(1000, 0, 0), 0); // Twice the click reach away.
    pParty->GetPlayingTime() += Duration::fromDays(150);
    game.tick(2);
    ASSERT_EQ(pParty->pPickedItem.itemId, ITEM_NULL);
    game.castSpell(0, SPELL_EARTH_TELEKINESIS);
    game.tick(2);
    game.pointMouseAtDecoration(559);
    game.pressAndReleaseButton(BUTTON_LEFT);
    game.tick(3);
    EXPECT_EQ(pParty->pPickedItem.itemId, ITEM_RED_APPLE); // The tree handed over an apple.
}

GAME_TEST(Prs, Pr2626) {
    // GetSector used to stop looking after 5 candidate floor faces, so where more floors than that stack up it
    // could miss the one the party is standing on. This spot in Colony Zod has 16 of them, and the truncated
    // search answered sector 9 instead of 23.
    game.startNewGame();
    game.teleportTo(MAP_COLONY_ZOD, Vec3f(-1849, 6726, 934), 0);
    game.tick(2);
    EXPECT_EQ(pIndoor->GetSector(-1849, 6726.5f, 934), 23);
}

GAME_TEST(Issues, Pr2635) {
    // Loading a save and opening the save name editor take a real double click on a slot, and are not
    // triggered by two ordinary clicks on it.
    game.startNewGame();
    game.tick(2);

    // Save something to load back.
    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(2);
    game.doubleClickGuiButton("SaveMenu_Slot0");
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_A);
    game.tick(2);
    game.pressGuiButton("SaveMenu_Save");
    game.tick(10);
    ASSERT_TRUE(ufs->exists("saves/save000.mm7"));

    // Open the load menu and check a row really is selected before anything is clicked.
    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_LoadGame");
    game.tick(3);
    ASSERT_EQ(current_screen_type, SCREEN_LOADGAME);
    ASSERT_TRUE(saveLoadMenu()->hasSelectedSlot()); // A row is selected before the player clicks anything...
    ASSERT_EQ(saveLoadMenu()->selectedSlot().fileName, "save000.mm7"); // ...and it's the one Slot0 points at.

    // Clicking the already selected row does nothing, however many times it's clicked slowly.
    game.pressGuiButton("LoadMenu_Slot0");
    game.tick(2);
    EXPECT_EQ(current_screen_type, SCREEN_LOADGAME);
    game.pressGuiButton("LoadMenu_Slot0");
    game.tick(2);
    EXPECT_EQ(current_screen_type, SCREEN_LOADGAME);

    // A double click on it loads.
    game.doubleClickGuiButton("LoadMenu_Slot0");
    game.tick(2);
    game.skipLoadingScreen();
    game.tick(2);
    EXPECT_EQ(current_screen_type, SCREEN_GAME);

    // Same rule in the save menu - two slow clicks don't open the name input, a double click does.
    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(2);
    game.pressGuiButton("SaveMenu_Slot0");
    game.tick(2);
    game.pressGuiButton("SaveMenu_Slot0");
    game.tick(2);
    EXPECT_NE(saveLoadMenu()->keyboard_input_status, WINDOW_INPUT_IN_PROGRESS);

    game.doubleClickGuiButton("SaveMenu_Slot0");
    game.tick(2);
    EXPECT_EQ(saveLoadMenu()->keyboard_input_status, WINDOW_INPUT_IN_PROGRESS);
}

GAME_TEST(Issues, Issue2636) {
    // Attribute descriptions in Localization were dangling string_views into unquote() temporaries - crashed
    // at startup on FreeBSD and drew garbage in the character screen stats tooltips elsewhere.
    auto textTape = tapes.allGUIWindowsText();
    game.startNewGame();
    test.startTaping();
    game.tick(2);
    game.goToInventory(0);
    game.pressAndReleaseKey(PlatformKey::KEY_C); // Switch to the stats tab.
    game.tick(2);
    EXPECT_EQ(current_screen_type, SCREEN_CHARACTERS);
    game.pressButton(BUTTON_RIGHT, 100, 60); // Right-click hold over the Might row shows its tooltip.
    game.tick(2);
    game.releaseButton(BUTTON_RIGHT);
    game.tick(1);
    EXPECT_CONTAINS(textTape.flatten(), "Might is the statistic that represents a character's overall strength, "
                                        "and the ability to put that strength where it counts.  Characters with a "
                                        "high might statistic do more damage in combat.");
}

// 2700

GAME_TEST(Prs, Pr2723) {
    // Clicking the top or the bottom of a party portrait did nothing. The portrait is an oval with semi-axes 32
    // and 41, and the click test used the circle of radius 32 inscribed in it.
    game.startNewGame();

    pParty->setActiveCharacterIndex(0);
    ASSERT_EQ(pParty->pCharacters[1].timeToRecovery, 0_ticks); // Portrait clicks do nothing while recovering.

    game.pressAndReleaseButton(BUTTON_LEFT, 177, 460); // 36 pixels below the center of the second portrait.
    game.tick();

    EXPECT_EQ(pParty->activeCharacterIndex(), 1);
}

GAME_TEST(Issues, Issue2754) {
    // Clicking a paralyzed friendly peasant made the active character attack it, and the hit turned the peasant
    // hostile. Past the click reach the same click attacked too. The hostile peasant is the control, clicking
    // that one still has to attack.
    for (auto [friendly, depth] : {std::pair(false, 300), std::pair(true, 300), std::pair(true, 1200)}) {
        test.prepareForNextTest();
        engine->config->debug.NoActors.setValue(true);
        game.startNewGame();
        test.startTaping();
        prepareForBattleTest();
        engine->config->debug.NoActors.setValue(false);

        auto hpTape = actorTapes.hp(0);
        auto aggressorTape = actorTapes.custom(0, [](const Actor &a) { return a.ActorEnemy(); });
        auto msgTape = tapes.uiMessages();
        Actor *peasant = game.spawnMonster(pParty->pos + Vec3f(0, depth, 0), MONSTER_PEASANT_DWARF_FEMALE_A_A,
                                           friendly ? SPAWN_DUMMY | SPAWN_FRIENDLY : SPAWN_DUMMY);
        peasant->buffs[ACTOR_BUFF_PARALYZED].Apply(pParty->GetPlayingTime() + Duration::fromDays(1), MASTERY_GRANDMASTER, 0, 0, 0);
        ASSERT_EQ(peasant->GetActorsRelation(nullptr) == HOSTILITY_FRIENDLY, friendly);
        game.pointMouseAtActor(0);
        ASSERT_EQ(engine->PickMouseForInteraction().pid == Pid(), depth > engine->config->gameplay.MouseInteractionDepth.value());

        for (int i = 0; i < 30; i++) {
            game.pressAndReleaseButton(BUTTON_LEFT);
            game.tick(5);
        }
        test.stopTaping();

        if (friendly) {
            EXPECT_MISSES(msgTape.flatten(), UIMSG_Attack); // A swing can miss, the queued message cannot.
            EXPECT_EQ(hpTape.delta(), 0);
            EXPECT_EQ(aggressorTape, tape(false));
        } else {
            EXPECT_CONTAINS(msgTape.flatten(), UIMSG_Attack);
            EXPECT_LT(hpTape.delta(), 0);
        }
    }
}

GAME_TEST(Issues, Issue2759) {
    // Clicking "Learn Skills" in a shop that teaches no skills crashed. Such a shop shouldn't offer the option at all.
    auto houseTape = tapes.house();
    auto textTape = tapes.allGUIWindowsText();
    game.startNewGame();
    test.startTaping();
    game.teleportTo(MAP_TATALIA, Vec3f(19174, 15056, 3040), 0); // In front of the door of Vander's Blades & Bows, facing it.
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_SPACE);
    game.tick(2);
    EXPECT_CONTAINS(houseTape, HOUSE_WEAPON_SHOP_TATALIA_1);
    EXPECT_CONTAINS(textTape.flatten(), "Display Inventory"); // We've seen the shop menu.
    EXPECT_MISSES(textTape.flatten(), "Learn Skills"); // But there was no "Learn Skills" option.
}

GAME_TEST(Issues, Issue2760) {
    // The east wall of an elevator shaft in Stone City was missing while the car was below the top, and the party
    // could walk through it.
    auto elevatorTape = tapes.doorState(10);
    auto xTape = tapes.custom([] { return pParty->pos.x; });
    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    game.teleportTo(MAP_STONE_CITY, Vec3f(-3100, 2176, -64), 270); // On the elevator car, facing the button that sends it down.
    test.startTaping();
    game.pointMouseAtFace(3459);
    game.pressAndReleaseButton(BUTTON_LEFT);
    game.tick(80);
    ASSERT_EQ(elevatorTape, tape(DOOR_OPEN, DOOR_CLOSING, DOOR_CLOSED)); // Make sure the car is down before strafing.
    game.pressKey(PlatformKey::KEY_LEFTBRACKET); // Strafe east, into the wall.
    game.tick(20);
    game.releaseKey(PlatformKey::KEY_LEFTBRACKET);
    game.tick();
    EXPECT_NEAR(xTape.max() + pParty->radius, -2976, 1); // Stopped by the shaft's east wall.
}

GAME_TEST(Issues, Issue2771) {
    // Immolation cast by a map event crashed the game on the next regeneration tick. The buff got caster -1, and the
    // damage sprite built a character pid out of it.
    auto hpTape = actorTapes.hp(0);
    auto casterTape = tapes.custom([] {
        AccessibleVector<Pid> result;
        for (const SpriteObject &sprite : pSpriteObjects)
            if (sprite.uObjectDescID != 0 && sprite.uSpellID == SPELL_FIRE_IMMOLATION)
                result.push_back(sprite.spell_caster_pid);
        return result;
    });
    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    game.teleportTo(MAP_LAND_OF_THE_GIANTS, Vec3f(3796, 6224, 1216), 0); // 300 west of the pedestal, facing it.
    game.goToGame(); // A new party arriving here gets a dialogue with Archibald Ironfist, and a Blaster in hand.
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_2); // Puts the Blaster into the second character's pack.
    game.tick();
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_2); // Makes the second character active.
    game.pointMouseAtDecoration(380);
    game.pressAndReleaseButton(BUTTON_LEFT);
    game.tick(3);
    ASSERT_TRUE(pParty->ImmolationActive());

    game.spawnMonster(pParty->pos + Vec3f(0, 200, 0), MONSTER_TITAN_A, SPAWN_DUMMY | SPAWN_FRIENDLY);
    test.startTaping();
    game.tick(100);
    EXPECT_LT(hpTape.delta(), 0);
    EXPECT_EQ(casterTape.flatten().unique(), tape(Pid::character(1))); // The active character.
}

GAME_TEST(Issues, Issue2776) {
    // After a party death, characters wearing regeneration gear came back with full HP and SP, regenerated for every
    // 5 minutes of the week that the death skips.
    auto deathsTape = tapes.deaths();
    auto hpsTape = charTapes.hps();
    auto mpsTape = charTapes.mps();
    game.startNewGame();
    for (Character &character : pParty->pCharacters) {
        character.inventory.equip(ITEM_SLOT_BOOTS, Item(ITEM_ARTIFACT_HERMES_SANDALS));
        character.mana = character.GetMaxMana() / 2;
    }
    test.startTaping();
    game.tick(); // The frame the party dies in isn't drawn, so the tapes need a frame from before it.
    for (int i = 0; i < 3; i++)
        pParty->pCharacters[i].receiveDamage(10000, DAMAGE_PHYSICAL);
    pParty->pCharacters[3].SetCondition(CONDITION_UNCONSCIOUS, false);
    game.tick();
    EXPECT_EQ(deathsTape.delta(), +1);
    EXPECT_EQ(hpsTape.back(), tape(1, 1, 1, 1));
    EXPECT_EQ(mpsTape.back(), tape(0, 0, 0, 18)); // Dying zeroes SP, only the unconscious sorcerer keeps it.
}

GAME_TEST(Issues, Issue2784a) {
    // Acid Burst impacts were silent.
    auto soundsTape = tapes.sounds();
    engine->config->debug.NoActors.setValue(true);
    engine->config->debug.AllMagic.setValue(true);
    game.startNewGame();
    test.startTaping();
    prepareForBattleTest();
    engine->config->debug.NoActors.setValue(false);
    game.spawnMonster(pParty->pos + Vec3f(0, 800, 0), MONSTER_TITAN_A, SPAWN_DUMMY);
    game.castQuickSpell(0, SPELL_WATER_ACID_BURST);
    game.tick(30);
    EXPECT_CONTAINS(soundNames(soundsTape), "20implosion03");
}

GAME_TEST(Issues, Issue2784b) {
    // Elf banks greeted with the Evil Bank lines.
    auto houseTape = tapes.house();
    auto soundsTape = tapes.sounds();
    game.startNewGame();
    game.teleportTo(MAP_TULAREAN_FOREST, Vec3f(-11514, -10816, 1344), 0); // In front of Nature's Stockpile.
    test.startTaping();
    game.pressAndReleaseKey(PlatformKey::KEY_SPACE);
    game.tick();
    EXPECT_EQ(houseTape.back(), HOUSE_BANK_TULAREAN_FOREST);
    EXPECT_EQ(soundNames(soundsTape).count("Elf Bank 01"), 1);
}

GAME_TEST(Issues, Issue2784c) {
    // The Balanced Axe should keep the dwarf smith's greeting after it moves out of room 82.
    auto houseTape = tapes.house();
    auto soundsTape = tapes.sounds();
    game.startNewGame();
    game.teleportTo(MAP_STONE_CITY, Vec3f(-425, -1461, 0), 225); // In front of The Balanced Axe.
    test.startTaping();
    game.pressAndReleaseKey(PlatformKey::KEY_SPACE);
    game.tick();
    EXPECT_EQ(houseTape.back(), HOUSE_WEAPON_SHOP_STONE_CITY);
    EXPECT_EQ(soundNames(soundsTape).count("Dwarf Weapon Shop 01"), 1);
}

GAME_TEST(Issues, Issue2784d) {
    // Natural Magic greeted with the dwarf smith's lines.
    auto houseTape = tapes.house();
    auto soundsTape = tapes.sounds();
    game.startNewGame();
    game.teleportTo(MAP_TULAREAN_FOREST, Vec3f(-12992, -6906, 1344), 90); // In front of Natural Magic.
    test.startTaping();
    game.pressAndReleaseKey(PlatformKey::KEY_SPACE);
    game.tick();
    EXPECT_EQ(houseTape.back(), HOUSE_MAGIC_SHOP_TULAREAN_FOREST);
    EXPECT_EQ(soundNames(soundsTape).count("Elf Magic Shop 01"), 1);
}

GAME_TEST(Prs, Pr2772a) {
    // A door in Fort Riverstride can't be opened from behind, clicking its back says "Nothing here".
    auto doorState = [] { return std::ranges::find(pIndoor->doors, 3u, &BLVDoor::doorId)->state; };
    Pid doorBack(OBJECT_Face, 618);

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    game.teleportTo(MAP_FORT_RIVERSTRIDE, Vec3f(-440, 1660, -453), 270);
    game.tick();

    ASSERT_EQ(pIndoor->faces[618].eventId, 32);
    ASSERT_EQ(doorState(), DOOR_CLOSED);
    EXPECT_TRUE(pIndoor->faces[618].Clickable());

    Pointi doorPos(160, 220);
    game.moveMouse(doorPos);
    game.tick();
    ASSERT_EQ(engine->PickMouseForTargeting().pid, doorBack);

    game.pressAndReleaseButton(BUTTON_LEFT, doorPos);
    game.tick(50);
    EXPECT_EQ(doorState(), DOOR_OPEN);
}

GAME_TEST(Prs, Pr2772b) {
    // Clicking a face in The Lincoln fires an event that only its pressure plates should fire.
    game.startNewGame();
    game.teleportTo(MAP_LINCOLN, Vec3f(524, 1463, 225), 0);
    game.tick();

    const BLVFace &face = pIndoor->faces[571];
    ASSERT_EQ(face.eventId, 28);
    ASSERT_TRUE(std::ranges::any_of(pIndoor->faces, [](const BLVFace &f) {
        return f.eventId == 28 && (f.attributes & FACE_PRESSURE_PLATE);
    }));
    EXPECT_FALSE(face.Clickable());
}
