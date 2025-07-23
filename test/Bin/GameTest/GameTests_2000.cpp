#include "Testing/Game/GameTest.h"

#include "Engine/Engine.h"
#include "Engine/MapEnums.h"
#include "Engine/Party.h"
#include "Engine/Graphics/Outdoor.h"
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

GAME_TEST(Issues, Issue2104) {
    // Enemies always hit with ranged attacks.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);
    auto hpTape = charTapes.hp(0);
    auto spritesTape = tapes.sprites();

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();

    // Move party in front of the bridge.
    pParty->pos = Vec3f(12552, 2000, 1);

    // Make sure only the 1st char is alive.
    for (int i = 1; i < 4; i++)
        pParty->pCharacters[i].SetCondDeadWithBlockCheck(false);

    // And make sure he won't die from all the shooting.
    Character &char0 = pParty->pCharacters[0];
    char0.sLevelModifier = 100;
    char0.health = pParty->pCharacters[0].GetMaxHealth();

    // And make sure he has some armor.
    char0.setSkillValue(CHARACTER_SKILL_LEATHER, CombinedSkillValue(1, CHARACTER_SKILL_MASTERY_NOVICE));
    char0.inventory.equip(ITEM_SLOT_ARMOUR, Item(ITEM_LEATHER_ARMOR));

    // Spawn an archer & wait.
    engine->config->debug.NoActors.setValue(false);
    game.spawnMonster(pParty->pos + Vec3f(0, 1500, 0), MONSTER_ELF_ARCHER_A);
    game.tick(300);

    int arrowCount = spritesTape.count([](auto sprites) { return sprites.contains(SPRITE_ARROW_PROJECTILE); });
    int hitCount = hpTape.size() - 1;

    ASSERT_GT(hitCount, 0); // Should have hit some.
    ASSERT_GT(arrowCount, hitCount); // And missed some.
}

GAME_TEST(Issues, Issue2109) {
    // Shield spell effect being applied from multiple sources.
    // What we had in this test before the fix was that the damage received was reduced 2^6 times, so was always zero.
    test.prepareForNextTest(100, RANDOM_ENGINE_MERSENNE_TWISTER);
    auto hpTape = charTapes.hp(0);

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();

    // Move party in front of the bridge.
    pParty->pos = Vec3f(12552, 2000, 1);

    // Wizard's eye is handy for debugging this test.
    Time tomorrow = pParty->GetPlayingTime() + Duration::fromDays(1);
    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(tomorrow, CHARACTER_SKILL_MASTERY_GRANDMASTER, 30, 0, 0);

    // Make sure only the 1st char is alive.
    for (int i = 1; i < 4; i++)
        pParty->pCharacters[i].SetCondDeadWithBlockCheck(false);

    // Prepare char0 properly.
    Character &char0 = pParty->pCharacters[0];
    char0.sLevelModifier = 100;
    char0.health = pParty->pCharacters[0].GetMaxHealth();
    char0._stats[ATTRIBUTE_LUCK] = 0; // We don't want luck rolls that decrease damage dealt.

    // Apply shield from potions & spells.
    char0.pCharacterBuffs[CHARACTER_BUFF_SHIELD].Apply(tomorrow, CHARACTER_SKILL_MASTERY_GRANDMASTER, 30, 0, 0);
    pParty->pPartyBuffs[PARTY_BUFF_SHIELD].Apply(tomorrow, CHARACTER_SKILL_MASTERY_GRANDMASTER, 30, 0, 0);

    // Equip all shield-giving gear.
    Item ring1(ITEM_ANGELS_RING);
    ring1.specialEnchantment = ITEM_ENCHANTMENT_OF_SHIELDING;
    char0.inventory.equip(ITEM_SLOT_RING1, ring1);

    Item ring2(ITEM_ANGELS_RING);
    ring2.specialEnchantment = ITEM_ENCHANTMENT_OF_STORM;
    char0.inventory.equip(ITEM_SLOT_RING2, ring2);

    char0.setSkillValue(CHARACTER_SKILL_PLATE, CombinedSkillValue(10, CHARACTER_SKILL_MASTERY_EXPERT)); // We don't want plate's damage reduction at Master.
    char0.inventory.equip(ITEM_SLOT_ARMOUR, Item(ITEM_ARTIFACT_GOVERNORS_ARMOR));

    char0.setSkillValue(CHARACTER_SKILL_SHIELD, CombinedSkillValue(10, CHARACTER_SKILL_MASTERY_GRANDMASTER)); // But we want GM shield for shielding.
    char0.inventory.equip(ITEM_SLOT_OFF_HAND, Item(ITEM_BRONZE_SHIELD));

    // At this point we have 6 sources of shielding. Spawn archers & let them shoot.
    engine->config->debug.NoActors.setValue(false);
    for (int i = 0; i < 4; i++) {
        game.tick(7);
        Actor *archer = game.spawnMonster(pParty->pos + Vec3f(0, 1500, 0), MONSTER_ELF_ARCHER_A);
        archer->monsterInfo.level = 200; // Make the archers miss less.
    }
    game.tick(150);

    ASSERT_GE(hpTape.size(), 2); // Should have received some damage.
    auto damageRange = hpTape.reverse().adjacentDeltas().minMax();
    EXPECT_GE(damageRange[0], 3); // Elf archer's damage is 4d2+2 (so 6-10), after shield it's 3-5.
    EXPECT_LE(damageRange[1], 5);
}

GAME_TEST(Issues, Issue2118) {
    auto potionsTape = tapes.totalItemCount(ITEM_TYPE_POTION);

    engine->config->debug.NoActors.setValue(true);
    game.startNewGame();
    test.startTaping();
    game.teleportTo(MAP_MERCENARY_GUILD, Vec3f(-1160, 3340, -127), 270);

    for (int i = 0; i < 50; i++) {
        game.pressAndReleaseButton(BUTTON_LEFT, 200, 200);
        game.tick();
        engine->_persistentVariables.mapVars[4] = 0; // This one is increased on each click, so we cheat.
    }

    EXPECT_LT(potionsTape.front(), potionsTape.back()); // Got some potions.

    for (const Character &character : pParty->pCharacters)
        for (InventoryConstEntry item : character.inventory.entries())
            if (item->isPotion() && item->itemId != ITEM_POTION_BOTTLE)
                EXPECT_GT(item->potionPower, 0); // Potions were properly generated.
}
