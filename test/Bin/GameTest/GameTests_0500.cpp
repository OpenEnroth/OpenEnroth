#include <unordered_set>
#include <string>

#include "Testing/Game/GameTest.h"

#include "Arcomage/Arcomage.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Engine/Tables/ItemTable.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Actor.h"
#include "Engine/SaveLoad.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "Utility/DataPath.h"
#include "Utility/ScopeGuard.h"

static std::initializer_list<CharacterBuff> allPotionBuffs() {
    static constexpr std::initializer_list<CharacterBuff> result = {
        CHARACTER_BUFF_RESIST_AIR,
        CHARACTER_BUFF_BLESS,
        CHARACTER_BUFF_RESIST_BODY,
        CHARACTER_BUFF_RESIST_EARTH,
        CHARACTER_BUFF_RESIST_FIRE,
        CHARACTER_BUFF_HASTE,
        CHARACTER_BUFF_HEROISM,
        CHARACTER_BUFF_RESIST_MIND,
        CHARACTER_BUFF_PRESERVATION,
        CHARACTER_BUFF_SHIELD,
        CHARACTER_BUFF_STONESKIN,
        CHARACTER_BUFF_ACCURACY,
        CHARACTER_BUFF_ENDURANCE,
        CHARACTER_BUFF_INTELLIGENCE,
        CHARACTER_BUFF_LUCK,
        CHARACTER_BUFF_STRENGTH,
        CHARACTER_BUFF_PERSONALITY,
        CHARACTER_BUFF_SPEED,
        CHARACTER_BUFF_RESIST_WATER,
        CHARACTER_BUFF_WATER_WALK
    };
    return result;
}

// 500

GAME_TEST(Issues, Issue502) {
    // Check that script face animation and voice indexes right characters.
    auto expressionTape = charTapes.expression(3);
    test.playTraceFromTestData("issue_502.mm7", "issue_502.json");
    EXPECT_TRUE(expressionTape.contains(CHARACTER_EXPRESSION_NO));
    EXPECT_EQ(pParty->activeCharacterIndex(), 4);
}

GAME_TEST(Issues, Issue503) {
    // Check that town portal book actually pauses game.
    auto hpTape = charTapes.hps();
    auto noDamageTape = tapes.config(engine->config->debug.NoDamage);
    auto screenTape = tapes.screen();
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_503.mm7", "issue_503.json");
    EXPECT_EQ(hpTape, tape({1147, 699, 350, 242})); // Game was paused, the party wasn't shot at, no HP change.
    EXPECT_EQ(noDamageTape, tape(false)); // HP change was actually possible.
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_BOOKS, SCREEN_GAME)); // TP book was opened.
    EXPECT_EQ(mapTape, tape(MAP_DRAGON_CAVES, MAP_CASTLE_HARMONDALE)); // And party was teleported to Harmondale.
}

GAME_TEST(Issues, Issue504) {
    // Going to prison doesn't recharge hirelings.
    auto yearsTape = tapes.custom([] { return pParty->GetPlayingTime().toYears(); });
    auto heroismTape = tapes.custom([] { return pParty->pPartyBuffs[PARTY_BUFF_HEROISM].Active(); });
    auto castsTape = tapes.custom([] { return pParty->pHirelings[0].bHasUsedTheAbility; });
    test.playTraceFromTestData("issue_504.mm7", "issue_504.json");
    EXPECT_EQ(yearsTape.delta(), +1); // A year spent in prison.
    EXPECT_EQ(heroismTape, tape(false, true, false, true)); // Two casts, before & after prison.
    EXPECT_EQ(castsTape, tape(0, 1, 0, 1)); // Two casts, staying in prison recharges casts.
}

GAME_TEST(Issues, Issue506) {
    // Check that scroll use does not assert.
    auto itemsTape = tapes.totalItemCount();
    auto flyTape = tapes.custom([] { return pParty->pPartyBuffs[PARTY_BUFF_FLY].Active(); });
    test.playTraceFromTestData("issue_506.mm7", "issue_506.json");
    EXPECT_EQ(itemsTape.delta(), -1); // Scroll used up.
    EXPECT_EQ(flyTape, tape(false, true)); // Fly was cast.
}

GAME_TEST(Issues, Issue518) {
    // Armageddon yeets the actors way too far into the sky & actors take stops when falling down.
    auto armageddonTape = tapes.custom([] { return pParty->pCharacters[0].uNumArmageddonCasts; });
    test.playTraceFromTestData("issue_518.mm7", "issue_518.json");
    EXPECT_EQ(armageddonTape, tape(2, 3)); // +1 armageddon cast.

    for (auto &actor : pActors) {
        EXPECT_LT(actor.pos.z, 3500);
    }
}

GAME_TEST(Issues, Issue520) {
    // Party should take fall damage
    auto healthTape = tapes.totalHp();
    test.playTraceFromTestData("issue_520.mm7", "issue_520.json");
    EXPECT_LT(healthTape.delta(), 0);
}

GAME_TEST(Issues, Issue521) {
    // 500 endurance leads to asserts in Character::SetRecoveryTime
    auto enduranceTape = charTapes.stat(0, CHARACTER_ATTRIBUTE_ENDURANCE);
    auto hpsTape = charTapes.hps();
    auto activeCharTape = tapes.activeCharacterIndex();
    test.playTraceFromTestData("issue_521.mm7", "issue_521.json");
    EXPECT_EQ(enduranceTape, tape(500)); // First char is beefy.
    EXPECT_LT(hpsTape.delta().max(), 0); // All chars took damage.
    EXPECT_EQ(activeCharTape, tape(1)); // First char didn't flinch.
}

GAME_TEST(Issues, Issue527) {
    // Check Cure Disease spell works
    auto diseaseTape = tapes.custom([] { return pParty->pCharacters[0].conditions.Has(CONDITION_DISEASE_WEAK); });
    test.playTraceFromTestData("issue_527.mm7", "issue_527.json");
    EXPECT_EQ(diseaseTape, tape(true, false)); // Disease healed!
}

GAME_TEST(Issues, Issue540) {
    // Check that Mass Distortion and Charm without target does not assert
    test.playTraceFromTestData("issue_540.mm7", "issue_540.json");
}

GAME_TEST(Issues, Issue558) {
    // Check that performing alchemy does not cause out of bound access
    test.playTraceFromTestData("issue_558.mm7", "issue_558.json");
    EXPECT_EQ(pParty->pPickedItem.uItemID, ITEM_POTION_CURE_WOUNDS);
}

GAME_TEST(Issues, Issue563) {
    // Check that drinking some buff potions does not cause assert
    test.playTraceFromTestData("issue_563.mm7", "issue_563.json");
}

GAME_TEST(Issues, Issue571) {
    // Check that item potion cannot be wastefully applied to unrelated item
    auto itemsTape = tapes.totalItemCount();
    test.playTraceFromTestData("issue_571.mm7", "issue_571.json");
    EXPECT_EQ(itemsTape.delta(), 0);
    EXPECT_NE(pParty->pPickedItem.uItemID, ITEM_NULL);
}

GAME_TEST(Issues, Issue573) {
    // Make Recharge Item effect non-decreasing
    auto chargeTape = tapes.custom([] {  return pParty->pCharacters[1].pInventoryItemList[33].uNumCharges; });
    auto manaTape = tapes.custom([] { return pParty->pCharacters[0].mana; });
    auto itemsTape = tapes.totalItemCount();
    test.playTraceFromTestData("issue_573.mm7", "issue_573.json");
    EXPECT_EQ(chargeTape.size(), 1); // Make sure we dont lose any charges
    EXPECT_EQ(chargeTape.back(), 21);
    EXPECT_LT(manaTape.back(), manaTape.front()); // Make sure spell was cast
    EXPECT_EQ(itemsTape.delta(), -1); // And that potion was used
}

GAME_TEST(Issues, Issue574) {
    // Check that applying recharge item potion produces correct number of charges
    auto itemsTape = tapes.totalItemCount();
    test.playTraceFromTestData("issue_574.mm7", "issue_574.json");
    EXPECT_EQ(itemsTape.delta(), -1); // Minus potion.
    EXPECT_EQ(pParty->pPickedItem.uMaxCharges, pParty->pPickedItem.uNumCharges);
}

GAME_TEST(Issues, Issue578) {
    // Check that rest & heal work after waiting
    auto healthTape = tapes.totalHp();
    test.playTraceFromTestData("issue_578.mm7", "issue_578.json");
    EXPECT_EQ(healthTape, tape(350, 419));
}

GAME_TEST(Issues, Issue598) {
    // Assert when accessing character inventory from the shop screen
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_598.mm7", "issue_598.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE, SCREEN_SHOP_INVENTORY, SCREEN_HOUSE, SCREEN_SHOP_INVENTORY));
}

// 600

GAME_TEST(Issues, Issue601) {
    // Check that Master Healer NPC skill work and does not assert
    auto conditionsTape = charTapes.conditions();
    auto hpTape = charTapes.hps();
    test.playTraceFromTestData("issue_601.mm7", "issue_601.json");
    EXPECT_EQ(conditionsTape.frontBack(), tape({CONDITION_SLEEP, CONDITION_CURSED, CONDITION_FEAR, CONDITION_DEAD},
                                               {CONDITION_GOOD, CONDITION_GOOD, CONDITION_GOOD, CONDITION_GOOD}));
    EXPECT_EQ(hpTape.frontBack(), tape({66, 128, 86, 70}, {126, 190, 96, 80}));
}

GAME_TEST(Issues, Issue608) {
    // Check that using Gate Master ability does not deplete mana of character
    auto manaTape = charTapes.mp(0);
    test.playTraceFromTestData("issue_608.mm7", "issue_608.json");
    EXPECT_EQ(manaTape.delta(), 0);
}

GAME_TEST(Issues, Issue611) {
    // Heal and reanimate dont work
    test.playTraceFromTestData("issue_611.mm7", "issue_611.json");
    // expect chars to be healed and zombies
    EXPECT_EQ(pParty->pCharacters[0].health, 45);
    EXPECT_EQ(pParty->pCharacters[1].health, 39);
    EXPECT_EQ(pParty->pCharacters[2].conditions.Has(CONDITION_ZOMBIE), true);
    EXPECT_EQ(pParty->pCharacters[3].conditions.Has(CONDITION_ZOMBIE), true);
}

GAME_TEST(Issues, Issue613a) {
    // Check that maximum food cooked by NPC is 14. "Prepare feast" option.
    auto foodTape = tapes.food();
    test.playTraceFromTestData("issue_613a.mm7", "issue_613a.json");
    EXPECT_EQ(foodTape, tape(13, 14));
}

GAME_TEST(Issues, Issue613b) {
    // Check that maximum food cooked by NPC is 14. "Prepare meal" option.
    auto foodTape = tapes.food();
    test.playTraceFromTestData("issue_613b.mm7", "issue_613b.json");
    EXPECT_EQ(foodTape, tape(13, 14));
}

GAME_TEST(Issues, Issue615a) {
    // Ensure that clicking between active portraits changes active character.
    auto activeCharTape = tapes.activeCharacterIndex();
    test.playTraceFromTestData("issue_615a.mm7", "issue_615a.json");
    EXPECT_EQ(activeCharTape.frontBack(), tape(1, 3));
}

GAME_TEST(Issues, Issue615b) {
    // Assert when clicking on character portrait when no active character is present.
    auto activeCharTape = tapes.activeCharacterIndex();
    test.playTraceFromTestData("issue_615b.mm7", "issue_615b.json");
    EXPECT_EQ(activeCharTape.frontBack(), tape(1, 4));
}

GAME_TEST(Issues, Issue625) {
    // Every character getting club at the start of the game
    game.startNewGame();
    EXPECT_FALSE(pParty->hasItem(ITEM_CLUB));
}

GAME_TEST(Issues, Issue624) {
    // Test that key repeating work
    test.playTraceFromTestData("issue_624.mm7", "issue_624.json");
    EXPECT_EQ(keyboardInputHandler->GetTextInput(), "");
}

GAME_TEST(Issues, Issue626) {
    // Last loaded save is not remembered
    std::string savesDir = makeDataPath("saves");
    std::string savesDirMoved;

    MM_AT_SCOPE_EXIT({
        std::error_code ec;
        std::filesystem::remove_all(savesDir);
        if (!savesDirMoved.empty()) {
            std::filesystem::rename(savesDirMoved, savesDir, ec); // Using std::error_code here, so can't throw.
        }
    });

    if (std::filesystem::exists(savesDir)) {
        savesDirMoved = savesDir + "_moved_for_testing";
        ASSERT_FALSE(std::filesystem::exists(savesDirMoved)); // Throws on failure.
        std::filesystem::rename(savesDir, savesDirMoved);
    }

    std::filesystem::create_directory(savesDir);

    game.startNewGame();

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(10);
    game.pressGuiButton("SaveMenu_Slot0");
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_0);
    game.tick(2);
    game.pressGuiButton("SaveMenu_Save");
    game.tick(10);

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(10);
    game.pressGuiButton("SaveMenu_Slot1");
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_1);
    game.tick(2);
    game.pressGuiButton("SaveMenu_Save");
    game.tick(10);

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_LoadGame");
    game.tick(10);
    game.pressGuiButton("LoadMenu_Slot1");
    game.tick(2);
    game.pressGuiButton("LoadMenu_Load");
    game.tick(2);
    game.skipLoadingScreen();
    game.tick(2);

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_LoadGame");
    game.tick(10);

    EXPECT_EQ(pSavegameList->selectedSlot, 1);
}

GAME_TEST(Issues, Issue645) {
    // Characters does not enter unconscious state
    auto conditionsTape = charTapes.conditions();
    test.playTraceFromTestData("issue_645.mm7", "issue_645.json");
    EXPECT_EQ(conditionsTape.frontBack(), tape({CONDITION_GOOD, CONDITION_GOOD, CONDITION_GOOD, CONDITION_GOOD},
                                               {CONDITION_UNCONSCIOUS, CONDITION_GOOD, CONDITION_UNCONSCIOUS, CONDITION_UNCONSCIOUS}));
}

GAME_TEST(Issues, Issue651) {
    // array subscript out of range
    test.playTraceFromTestData("issue_651.mm7", "issue_651.json");
    // check for valid pids
    for (auto &obj : pSpriteObjects) {
        ObjectType castertype = obj.spell_caster_pid.type();
        int casterid = obj.spell_caster_pid.id();
        if (castertype == OBJECT_Actor) {
            EXPECT_TRUE(casterid < pActors.size());
        }
    }
}

GAME_TEST(Issues, Issue661) {
    // HP/SP regen from items is too high.
    auto timeTape = tapes.time();
    auto healthTape = charTapes.hp(0);
    auto manaTape = charTapes.mp(0);
    test.playTraceFromTestData("issue_661.mm7", "issue_661.json");

    // Actual # of 5-min ticks hit is 12.
    Duration interval = Duration::fromMinutes(5);
    Duration firstTick = timeTape.front().toDurationSinceSilence().roundedUp(interval);
    Duration lastTick = timeTape.back().toDurationSinceSilence().roundedDown(interval);
    EXPECT_EQ((lastTick - firstTick) / interval + 1, 12);

    // One item that heals hp, three items heal mana.
    EXPECT_EQ(healthTape.delta(), +12);
    EXPECT_EQ(manaTape.delta(), +3 * 12);
}

GAME_TEST(Issues, Issue662) {
    // "of Air magic" should give floor(skill / 2) skill level bonus (like all other such bonuses)
    test.loadGameFromTestData("issue_662.mm7");
    EXPECT_EQ(pParty->pCharacters[3].pActiveSkills[CHARACTER_SKILL_AIR], CombinedSkillValue(6, CHARACTER_SKILL_MASTERY_EXPERT));
    EXPECT_EQ(pParty->pCharacters[3].GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_AIR), 3);
    pParty->pCharacters[3].pActiveSkills[CHARACTER_SKILL_AIR] = CombinedSkillValue(5, CHARACTER_SKILL_MASTERY_EXPERT);
    EXPECT_EQ(pParty->pCharacters[3].GetItemsBonus(CHARACTER_ATTRIBUTE_SKILL_AIR), 2);
}

GAME_TEST(Issues, Issue663) {
    // Cant switch between inactive char inventory in chests
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_663.mm7", "issue_663.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_CHEST, SCREEN_CHEST_INVENTORY));
    // should switch to char 2 inv
    EXPECT_EQ(pParty->activeCharacterIndex(), 2);
    EXPECT_GT(pParty->activeCharacter().timeToRecovery, 0_ticks);
}

GAME_TEST(Issues, Issue664) {
    // Party sliding down shallow slopes outdoors
    test.playTraceFromTestData("issue_664.mm7", "issue_664.json");
    // shouldnt move
    EXPECT_EQ(pParty->pos.x, 7323);
    EXPECT_EQ(pParty->pos.y, 10375);
    EXPECT_EQ(pParty->pos.z, 309);
}

GAME_TEST(Issues, Issue674) {
    // Check that map timers are working
    auto healthTape = charTapes.hp(0);
    test.playTraceFromTestData("issue_674.mm7", "issue_674.json");
    EXPECT_EQ(healthTape.delta(), +5);
}

GAME_TEST(Issues, Issue675) {
    // generateItem used to generate invalid enchantments outside of the [0, 24] range in some cases.
    // Also, generateItem used to assert.
    std::initializer_list<ItemTreasureLevel> levels = {
        ITEM_TREASURE_LEVEL_1, ITEM_TREASURE_LEVEL_2, ITEM_TREASURE_LEVEL_3,
        ITEM_TREASURE_LEVEL_4, ITEM_TREASURE_LEVEL_5, ITEM_TREASURE_LEVEL_6
    };

    std::unordered_set<CharacterAttributeType> generatedEnchantments;

    ItemGen item;
    for (int i = 0; i < 300; i++) {
        for (ItemTreasureLevel level : levels) {
            pItemTable->generateItem(level, RANDOM_ITEM_ANY, &item);
            if (isPotion(item.uItemID)) {
                EXPECT_GE(item.potionPower, 1);
                EXPECT_FALSE(item.attributeEnchantment);
            } else {
                EXPECT_EQ(item.potionPower, 0);
                if (item.attributeEnchantment) {
                    EXPECT_GE(*item.attributeEnchantment, CHARACTER_ATTRIBUTE_FIRST_ENCHANTABLE);
                    EXPECT_LE(*item.attributeEnchantment, CHARACTER_ATTRIBUTE_LAST_ENCHANTABLE);
                    generatedEnchantments.insert(*item.attributeEnchantment);
                }
            }
        }
    }

    EXPECT_EQ(generatedEnchantments.size(), 24); // All possible enchantments can be generated.
}

GAME_TEST(Issues, Issue676) {
    // Jump spell doesn't work
    test.playTraceFromTestData("issue_676.mm7", "issue_676.json");
    EXPECT_EQ(pParty->pos.toInt(), Vec3i(12041, 11766, 908));
}

GAME_TEST(Issues, Issue677) {
    // Haste doesn't impose weakness after it ends
    auto hasteTape = tapes.custom([] { return pParty->pPartyBuffs[PARTY_BUFF_HASTE].Active(); });
    auto conditionsTape = charTapes.conditions();
    test.playTraceFromTestData("issue_677.mm7", "issue_677.json");
    EXPECT_EQ(hasteTape, tape(true, false));
    EXPECT_EQ(conditionsTape, tape({CONDITION_GOOD, CONDITION_CURSED, CONDITION_GOOD, CONDITION_GOOD},
                                   {CONDITION_WEAK, CONDITION_WEAK, CONDITION_WEAK, CONDITION_WEAK}));
}

GAME_TEST(Issues, Issue680) {
    // Chest items duplicate sometimes
    auto chestItemsCount = tapes.custom([] { return std::count_if(vChests[4].igChestItems.cbegin(), vChests[4].igChestItems.cend(), [&](ItemGen item) { return item.uItemID != ITEM_NULL; }); });
    test.playTraceFromTestData("issue_680.mm7", "issue_680.json");
    // Make sure we havent gained any duplicates
    EXPECT_EQ(chestItemsCount.front(), chestItemsCount.back());
    // And that items were added and removed
    EXPECT_GE(chestItemsCount.size(), 2);
}

GAME_TEST(Issues, Issue681) {
    // Collisions: You can jump through roof in Tidewater Caverns
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("issue_681.mm7", "issue_681.json");
    EXPECT_LT(zTape.max(), 990);
}

GAME_TEST(Issues, Issue689) {
    // Testing that clicking on load game scroll is not crashing the game then there's small amount of saves present.
    std::string savesDir = makeDataPath("saves");
    std::string savesDirMoved;

    MM_AT_SCOPE_EXIT({
        std::error_code ec;
        std::filesystem::remove_all(savesDir);
        if (!savesDirMoved.empty()) {
            std::filesystem::rename(savesDirMoved, savesDir, ec); // Using std::error_code here, so can't throw.
        }
    });

    if (std::filesystem::exists(savesDir)) {
        savesDirMoved = savesDir + "_moved_for_testing";
        ASSERT_FALSE(std::filesystem::exists(savesDirMoved)); // Throws on failure.
        std::filesystem::rename(savesDir, savesDirMoved);
    }

    std::filesystem::create_directory(savesDir);

    game.startNewGame();

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_SaveGame");
    game.tick(10);
    game.pressGuiButton("SaveMenu_Slot0");
    game.tick(2);
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_0);
    game.tick(2);
    game.pressGuiButton("SaveMenu_Save");
    game.tick(10);

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_LoadGame");
    game.tick(10);
    game.pressGuiButton("LoadMenu_Scroll"); // Sould not crash
    game.tick(2);
    game.pressGuiButton("LoadMenu_Slot0");
    game.tick(2);
    game.pressGuiButton("LoadMenu_Load");
    game.tick(2);
    game.skipLoadingScreen();
    game.tick(2);

    game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
    game.tick(2);
    game.pressGuiButton("GameMenu_Quit");
    game.tick(2);
    game.pressGuiButton("GameMenu_Quit");
    game.tick(10);
    game.pressGuiButton("MainMenu_LoadGame"); // Should not crash because of last loaded save
    game.tick(10);
    game.pressGuiButton("LoadMenu_Scroll"); // Sould not crash
}

GAME_TEST(Issues, Issue691) {
    // Test that hitting escape when in transition window does not crash
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_691.mm7", "issue_691.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_CHANGE_LOCATION, SCREEN_GAME));
}

// 700

GAME_TEST(Issues, Issue700) {
    // Test that event check for killed monsters work
    auto goldTape = tapes.gold();
    test.playTraceFromTestData("issue_700.mm7", "issue_700.json");
    EXPECT_EQ(goldTape, tape(21541));
}

GAME_TEST(Issues, Issue720) {
    // Test that quest book is opening fine
    test.playTraceFromTestData("issue_720.mm7", "issue_720.json");
    EXPECT_EQ(current_screen_type, SCREEN_BOOKS);
    EXPECT_EQ(pGUIWindow_CurrentMenu->eWindowType, WINDOW_QuestBook);
}

GAME_TEST(Issues, Issue724) {
    // Test that item potion can be applied to equipped items.
    auto hardenedTape = tapes.custom([] { return !!(pParty->pCharacters[3].GetItem(ITEM_SLOT_MAIN_HAND)->uAttributes & ITEM_HARDENED); });
    test.playTraceFromTestData("issue_724.mm7", "issue_724.json");
    EXPECT_EQ(hardenedTape, tape(false, true));
}

GAME_TEST(Issues, Issue728_746) {
    // Mousing over facets with nonexisting events shouldn't crash the game.
    // Clicking faces with invalid events should show message "nothing here" #746
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_728.mm7", "issue_728.json");
    // Hover, click and space
    EXPECT_EQ(statusTape, tape("", "Nothing here", "", "Nothing here", ""));
}

GAME_TEST(Issues, Issue730) {
    // Thrown items are throwing a party of their own
    test.playTraceFromTestData("issue_730.mm7", "issue_730.json");
    for (size_t i = 0; i < pSpriteObjects.size(); ++i) {
        EXPECT_EQ(pSpriteObjects[i].vVelocity.toInt(), Vec3i(0, 0, 0));
    }
}

GAME_TEST(Issues, Issue735a) {
    // Trace-only test: battle with ~60 monsters in a dungeon.
    test.playTraceFromTestData("issue_735a.mm7", "issue_735a.json");
}

GAME_TEST(Issues, Issue735b) {
    // Trace-only test: battle with over 30 monsters in the open.
    test.playTraceFromTestData("issue_735b.mm7", "issue_735b.json");
}

GAME_TEST(Issues, Issue735c) {
    // Trace-only test: entering the dragon cave on Emerald Isle, hugging the walls and shooting fireballs.
    // Checking location names explicitly so that we'll notice if party misses cave entrance after retracing.
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_735c.mm7", "issue_735c.json");
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_DRAGONS_LAIR)); // Emerald Isle -> Dragon's cave.
}

GAME_TEST(Issues, Issue735d) {
    // Trace-only test: turn-based battle with ~60 monsters in a dungeon, casting poison cloud.
    auto turnBasedTape = tapes.turnBasedMode();
    test.playTraceFromTestData("issue_735d.mm7", "issue_735d.json");
    EXPECT_EQ(turnBasedTape, tape(false, true, false));
}

GAME_TEST(Issues, Issue741) {
    // Game crashing when walking into a wall in Temple of the moon
    test.playTraceFromTestData("issue_741.mm7", "issue_741.json");
    EXPECT_NE(pBLVRenderParams->uPartySectorID, 0);
}

GAME_TEST(Issues, Issue742) {
    // No starting Quests in Questbook
    game.startNewGame();
    // check all starting quests
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_RED_POTION_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_SEASHELL_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_LONGBOW_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_PLATE_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_LUTE_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_HAT_ACTIVE));
}

GAME_TEST(Issues, Issue755) {
    // Resurrection doesn't crash. Crashes were actually quite random because the code was reading pointer parts as ints.
    auto actor2Tape = actorTapes.aiState(2);
    auto actor37Tape = actorTapes.aiState(37);
    test.playTraceFromTestData("issue_755.mm7", "issue_755.json");
    EXPECT_TRUE(actor2Tape.contains(Dead));
    EXPECT_TRUE(actor2Tape.contains(Resurrected));
    EXPECT_TRUE(actor37Tape.contains(Dead));
    EXPECT_TRUE(actor37Tape.contains(Resurrected));
}

GAME_TEST(Issues, Issue760) {
    // Check that mixing potions when character inventory is full does not discard empty bottle
    auto itemsTape = tapes.totalItemCount();
    test.playTraceFromTestData("issue_760.mm7", "issue_760.json");
    EXPECT_EQ(itemsTape.delta(), 0);
    EXPECT_EQ(pParty->pPickedItem.uItemID, ITEM_POTION_BOTTLE);
}

GAME_TEST(Issues, Issue774) {
    // Background stunned actors do idle motions
    test.playTraceFromTestData("issue_774.mm7", "issue_774.json");
    for (auto &act : pActors) {
        if (!(act.attributes & ACTOR_FULL_AI_STATE))
            EXPECT_TRUE(act.aiState == Stunned || act.aiState == Dead);
    }
}

GAME_TEST(Issues, Issue779) {
    // Test that you can't load game from save menu
    test.playTraceFromTestData("issue_779.mm7", "issue_779.json");
    EXPECT_EQ(current_screen_type, SCREEN_SAVEGAME);
}

void check783784Buffs(bool haveBuffs) {
    for (CharacterBuff buff : allPotionBuffs())
        EXPECT_EQ(pParty->pCharacters[0].pCharacterBuffs[buff].Active(), haveBuffs) << "buff=" << static_cast<int>(buff);
}

GAME_TEST(Issues, Issue783) {
    // Check that all character buffs expire after rest.
    auto timeTape = tapes.time();
    test.playTraceFromTestData("issue_783.mm7", "issue_783.json", [&] {
        check783784Buffs(true); // Should have all buffs at start.

        // And all buffs should expire way in the future.
        for (CharacterBuff buff : allPotionBuffs())
            EXPECT_GT(pParty->pCharacters[0].pCharacterBuffs[buff].GetExpireTime(), pParty->GetPlayingTime() + Duration::fromHours(10));
    });

    EXPECT_GT(timeTape.delta(), Duration::fromHours(8)); // Check that we did rest.
    EXPECT_LT(timeTape.delta(), Duration::fromHours(10)); // Check that we didn't wait out the buff expire times.
    check783784Buffs(false); // Check that the buffs still expired.
}

GAME_TEST(Issues, Issue784) {
    // Check that buff potions actually work.
    test.playTraceFromTestData("issue_784.mm7", "issue_784.json", [] {
        check783784Buffs(false);
    });
    check783784Buffs(true);

    // Check that buffs have effect.
    // Potions were at power 75, that's 75*3=225 points for attribute bonuses.
    const Character &player0 = pParty->pCharacters[0];

    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_AIR));
    EXPECT_EQ(5, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_ATTACK)); // CHARACTER_BUFF_BLESS.
    EXPECT_EQ(5, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RANGED_ATTACK)); // CHARACTER_BUFF_BLESS.
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_BODY));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_EARTH));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_FIRE));
    EXPECT_EQ(58_ticks, player0.GetAttackRecoveryTime(false)); // CHARACTER_BUFF_HASTE.
    // EXPECT_EQ(59_ticks, player0.GetAttackRecoveryTime(true)); // Can't call this b/c no bow.
    EXPECT_EQ(5, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS)); // CHARACTER_BUFF_HEROISM.
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_MIND));
    // No check for CHARACTER_BUFF_PRESERVATION.
    // No check for CHARACTER_BUFF_SHIELD.
    EXPECT_EQ(5, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_AC_BONUS)); // CHARACTER_BUFF_STONESKIN.
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_ACCURACY));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_ENDURANCE));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_INTELLIGENCE));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_LUCK));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_MIGHT));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_PERSONALITY));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_SPEED));
    EXPECT_EQ(225, player0.GetMagicalBonus(CHARACTER_ATTRIBUTE_RESIST_WATER));
    // No check for CHARACTER_BUFF_WATER_WALK
}

GAME_TEST(Issues, Issue790) {
    // Test that pressing New Game button in game menu works
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_790.mm7", "issue_790.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_MENU, SCREEN_PARTY_CREATION));
}

GAME_TEST(Issues, Issue792) {
    // Test that event timers do not fire in-between game loading process
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_792.mm7", "issue_792.json"); // Should not assert
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_MENU, SCREEN_GAME, SCREEN_PARTY_CREATION, SCREEN_GAME, SCREEN_MENU, SCREEN_LOADGAME, SCREEN_GAME));
}

GAME_TEST(Issues, Issue797) {
    // Jump spell not working - party should move and not take falling damage
    auto healthTape = tapes.totalHp();
    test.playTraceFromTestData("issue_797.mm7", "issue_797.json");
    EXPECT_EQ(healthTape.delta(), 0);
}

// 800

GAME_TEST(Issues, Issue808) {
    // Test that cycling characters with TAB do not assert when all characters are recovering
    test.playTraceFromTestData("issue_808.mm7", "issue_808.json"); // Should not assert
}

GAME_TEST(Issues, Issue814) {
    // Test that compare variable for autonotes do not assert
    test.playTraceFromTestData("issue_814.mm7", "issue_814.json"); // Should not assert
    EXPECT_EQ(pParty->pCharacters[0]._statBonuses[CHARACTER_ATTRIBUTE_INTELLIGENCE], 25);
}

GAME_TEST(Issues, Issue815) {
    // Test that subtract variable for character bits work
    test.playTraceFromTestData("issue_815.mm7", "issue_815.json");
    EXPECT_EQ(pParty->pCharacters[0]._statBonuses[CHARACTER_ATTRIBUTE_INTELLIGENCE], 25);
}

GAME_TEST(Issues, Issue816) {
    // Test that encountering trigger event instruction does not assert
    test.playTraceFromTestData("issue_816.mm7", "issue_816.json"); // Should not assert
}

GAME_TEST(Issues, Issue820a) {
    // Cannot interact with fruit trees if party is too close.
    auto statusTape = tapes.statusBar();
    auto foodTape = tapes.food();
    test.playTraceFromTestData("issue_820A.mm7", "issue_820A.json");
    EXPECT_TRUE(statusTape.contains("Fruit Tree"));
    EXPECT_GT(foodTape.back(), foodTape.front());
}

GAME_TEST(Issues, Issue820b) {
    // Cannot attack trees in Tularean forest.
    auto statusTape = tapes.statusBar();
    auto treeHealthTape = actorTapes.hp(80);
    test.playTraceFromTestData("issue_820B.mm7", "issue_820B.json");
    EXPECT_TRUE(statusTape.contains(fmt::format("Zoltan hits Tree for {} damage", -treeHealthTape.delta())));
    EXPECT_LT(treeHealthTape.delta(), 0);
}

GAME_TEST(Issues, Issue830) {
    // Mouseover hints for UI elements not showing
    game.startNewGame();
    game.tick(1);
    engine->_statusBar->clearEvent();
    // Portrait: Name and conditions of the character
    game.moveMouse(65, 424);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Zoltan the Knight: Good");
    //HP / SP Bar(either one) : Display current and max HP and SP both
    game.moveMouse(102, 426);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "45 / 45 Hit Points    0 / 0 Spell Points");
    // Minimap : Display time, day of the week and full date
    game.moveMouse(517, 111);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "9:00am Monday 1 January 1168");
    //Zoom in / out minimap buttons : Display description of the button
    game.moveMouse(523, 140);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Zoom In");
    game.moveMouse(577, 140);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Zoom Out");
    // Food : Display total amount of food(bit redundant, but it is there)
    game.moveMouse(520, 329);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "You have 7 food");
    // Gold : Display amount of gold on party and in bank
    game.moveMouse(575, 327);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "You have 200 total gold, 0 in the Bank");
    // Books : Description of each book(journal, autonotes etc)
    game.moveMouse(513, 387);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Current Quests");
    game.moveMouse(540, 382);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Auto Notes");
    game.moveMouse(556, 381);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Maps");
    game.moveMouse(586, 396);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Calendar");
    game.moveMouse(611, 400);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "History");
    // Buttons : Description of the 4 buttons in the corner(cast spell, rest, quick ref, game options)
    game.moveMouse(494, 461);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Cast Spell");
    game.moveMouse(541, 460);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Rest");
    game.moveMouse(585, 461);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Quick Reference");
    game.moveMouse(621, 460);
    game.tick(1);
    EXPECT_EQ(engine->_statusBar->get(), "Game Options");
}

GAME_TEST(Issues, Issue832) {
    // Death Blossom + ice blast crash.
    auto spritesTape = tapes.sprites();
    auto deathsTape = actorTapes.countByState(AIState::Dead);
    test.playTraceFromTestData("issue_832.mm7", "issue_832.json");

    // Check that there was a frame with both ice blast and death blossom sprites in the air. The crash was due to
    // spell targeting code stumbling when encountering non-monster sprites in the sprite list.
    EXPECT_TRUE(spritesTape.contains([] (const AccessibleVector<SpriteId> &sprites) {
        return sprites.containsAll(SPRITE_SPELL_EARTH_DEATH_BLOSSOM_FALLOUT, SPRITE_SPELL_WATER_ICE_BLAST);
    }));

    // Peasants were harmed during recording of this trace.
    EXPECT_EQ(deathsTape.frontBack(), tape(0, 2));
}

GAME_TEST(Issues, Issue833a) {
    // Test that quick spell is castable on Shift+click.
    auto manaTape = charTapes.mp(0);
    auto quickSpellTape = charTapes.quickSpell(0);
    auto spritesTape = tapes.sprites();
    test.playTraceFromTestData("issue_833a.mm7", "issue_833a.json");
    EXPECT_EQ(manaTape.delta(), -2);
    EXPECT_EQ(quickSpellTape, tape(SPELL_FIRE_FIRE_BOLT));
    EXPECT_TRUE(spritesTape.flattened().contains(SPRITE_SPELL_FIRE_FIRE_BOLT_IMPACT)); // Fire bolt was cast by 1st character.
}

GAME_TEST(Issues, Issue833b) {
    // Test that shift+clicking when no quick spell is set doesn't assert.
    auto manaTape = charTapes.mp(0);
    auto quickSpellTape = charTapes.quickSpell(0);
    auto actorsHpTape = actorTapes.totalHp();
    auto soundsTape = tapes.sounds();
    test.playTraceFromTestData("issue_833b.mm7", "issue_833b.json");
    EXPECT_EQ(manaTape.delta(), 0);
    EXPECT_EQ(quickSpellTape, tape(SPELL_NONE));
    EXPECT_EQ(actorsHpTape.size(), 1); // No one was hurt.
    EXPECT_TRUE(soundsTape.flattened().contains(SOUND_error));
}

GAME_TEST(Issues, Issue840) {
    // Test that entering Body Guild in erathia does not crash
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_840.mm7", "issue_840.json"); // Should not crash
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE));
}

GAME_TEST(Issues, Issue844) {
    // Test that entering trainer in Stone City does not assert
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_844.mm7", "issue_844.json"); // Should not assert
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE));
}

GAME_TEST(Issues, Issue867) {
    // Test that temple donations work correctly
    test.playTraceFromTestData("issue_867.mm7", "issue_867.json");
    EXPECT_TRUE(pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Active());
}

GAME_TEST(Issues, Issue868) {
    // Test that resurrecting in evil temples set zombie status.
    auto conditionTape = tapes.custom([] { return pParty->pCharacters[0].GetMajorConditionIdx(); });
    test.playTraceFromTestData("issue_868.mm7", "issue_868.json");
    EXPECT_EQ(conditionTape, tape(CONDITION_DEAD, CONDITION_ZOMBIE));
}

GAME_TEST(Issues, Issue872) {
    // Test that loading game set correct names on unique NPCs.
    test.playTraceFromTestData("issue_872.mm7", "issue_872.json");
    FlatHirelings buf;
    buf.Prepare();
    EXPECT_NE(buf.Get(0)->name, "Dummy");
}

GAME_TEST(Issues, Issue878) {
    // Test that numpad number keys are working
    auto bankTape = tapes.custom([] { return pParty->uNumGoldInBank; });
    test.playTraceFromTestData("issue_878.mm7", "issue_878.json");
    EXPECT_EQ(bankTape, tape(0, 123));
}

GAME_TEST(Issues, Issue880) {
    // Arcomage deck missing from white cliff caves
    auto objectsTape = tapes.custom( [] { return std::ranges::count_if(pSpriteObjects, [](const SpriteObject& obj) {return obj.uObjectDescID != 0; }); });
    test.playTraceFromTestData("issue_880.mm7", "issue_880.json");
    EXPECT_EQ(objectsTape.front(), objectsTape.back());
    EXPECT_EQ(pSpriteObjects[0].containing_item.uItemID, ITEM_QUEST_ARCOMAGE_DECK);
}

GAME_TEST(Issues, Issue895) {
    // Test that entering magic guild does not shift date
    auto timeTape = tapes.time();
    test.playTraceFromTestData("issue_895.mm7", "issue_895.json");
    EXPECT_LT(timeTape.delta(), Duration::fromMinutes(5));
}

// 900

GAME_TEST(Issues, Issue906_773) {
    // Issue with some use of Spellbuff Expired() - check actors cast buffs.
    // #773: AI_SpellAttack using wrong actor buff for bless.
    auto blessTape = actorTapes.hasBuff(2, ACTOR_BUFF_BLESS);
    auto heroismTape = actorTapes.hasBuff(2, ACTOR_BUFF_HEROISM);
    test.playTraceFromTestData("issue_906.mm7", "issue_906.json");
    EXPECT_EQ(blessTape, tape(true, false, true));
    EXPECT_EQ(heroismTape, tape(true, false, true));
}

GAME_TEST(Issues, Issue929) {
    // Test that blaster sells for 1 gold and selling not asserts
    auto goldTape = tapes.gold();
    auto itemsTape = tapes.totalItemCount();
    test.playTraceFromTestData("issue_929.mm7", "issue_929.json");
    EXPECT_EQ(goldTape.delta(), +1);
    EXPECT_EQ(itemsTape.delta(), -1);
}
