#include <unordered_set>

#include "Testing/Game/GameTest.h"

#include "Arcomage/Arcomage.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIProgressBar.h"

#include "Engine/Tables/ItemTable.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/SaveLoad.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Party.h"
#include "Engine/AssetsManager.h"
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

// 100

GAME_TEST(Issues, Issue123) {
    // Party falls when flying
    test.playTraceFromTestData("issue_123.mm7", "issue_123.json");
    // check party is still in the air
    EXPECT_GT(pParty->pos.z, 512);
}

GAME_TEST(Issues, Issue125) {
    // check that fireballs hurt party
    auto healthTape = tapes.totalHp();
    test.playTraceFromTestData("issue_125.mm7", "issue_125.json");
    EXPECT_LT(healthTape.delta(), 0);
}

GAME_TEST(Issues, Issue159) {
    // Exception when entering Tidewater Caverns
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_159.mm7", "issue_159.json");
    EXPECT_EQ(mapTape, tape("out13.odm", "d17.blv", "out13.odm"));
}

GAME_TEST(Issues, Issue163) {
    // Testing that pressing the Load Game button doesn't crash even if the 'saves' folder doesn't exist.
    std::string savesDir = makeDataPath("saves");
    std::string savesDirMoved;

    MM_AT_SCOPE_EXIT({
        if (!savesDirMoved.empty()) {
            std::error_code ec;
            std::filesystem::rename(savesDirMoved, savesDir, ec); // Using std::error_code here, so can't throw.
        }
    });

    if (std::filesystem::exists(savesDir)) {
        savesDirMoved = savesDir + "_moved_for_testing";
        ASSERT_FALSE(std::filesystem::exists(savesDirMoved)); // Throws on failure.
        std::filesystem::rename(savesDir, savesDirMoved);
    }

    game.pressGuiButton("MainMenu_LoadGame"); // Shouldn't crash.
    game.tick(10);
    for (bool used : pSavegameList->pSavegameUsedSlots)
        EXPECT_FALSE(used); // All slots unused.

    game.pressGuiButton("LoadMenu_Load");
    game.tick(10);
    EXPECT_EQ(current_screen_type, SCREEN_LOADGAME);
    EXPECT_FALSE(pGameLoadingUI_ProgressBar->IsActive()); // Load button shouldn't do anything.
}

GAME_TEST(Issues, Issue198) {
    // Check that items can't end up out of bounds of character's inventory.
    test.playTraceFromTestData("issue_198.mm7", "issue_198.json");

    auto forEachInventoryItem = [](auto &&callback) {
        for (const Character &character : pParty->pCharacters) {
            for (int inventorySlot = 0; inventorySlot < Character::INVENTORY_SLOT_COUNT; inventorySlot++) {
                int itemIndex = character.pInventoryMatrix[inventorySlot];
                if (itemIndex <= 0)
                    continue; // Empty or non-primary cell.

                int x = inventorySlot % Character::INVENTORY_SLOTS_WIDTH;
                int y = inventorySlot / Character::INVENTORY_SLOTS_WIDTH;

                callback(character.pInventoryItemList[itemIndex - 1], x, y);
            }
        }
    };

    // Preload item images in the main thread first.
    game.runGameRoutine([&] {
        forEachInventoryItem([](const ItemGen &item, int /*x*/, int /*y*/) {
            // Calling width() forces the texture to be created.
            assets->getImage_ColorKey(pItemTable->pItems[item.uItemID].iconName)->width();
        });
    });

    // Then can safely check everything.
    forEachInventoryItem([](const ItemGen &item, int x, int y) {
        GraphicsImage *image = assets->getImage_ColorKey(pItemTable->pItems[item.uItemID].iconName);
        int width = GetSizeInInventorySlots(image->width());
        int height = GetSizeInInventorySlots(image->height());

        EXPECT_LE(x + width, Character::INVENTORY_SLOTS_WIDTH);
        EXPECT_LE(y + height, Character::INVENTORY_SLOTS_HEIGHT);
    });
}

// 200

GAME_TEST(Issues, Issue201) {
    // Unhandled EVENT_ShowMovie in Event Processor
    auto healthTape = tapes.totalHp();
    auto mapTape = tapes.map();
    auto daysTape = tapes.custom([] { return pParty->GetPlayingTime().GetDays(); });
    test.playTraceFromTestData("issue_201.mm7", "issue_201.json");
    EXPECT_GT(healthTape.delta(), 0); // Party should heal.
    EXPECT_EQ(mapTape, tape("out01.odm", "out02.odm")); // Emerald isle to Harmondale.
    EXPECT_EQ(daysTape.delta(), 7); // Time should advance by a week.
}

GAME_TEST(Issues, Issue202) {
    // Judge doesn't move to house and stays with the party.
    auto hirelingsTape = tapes.custom([] { return pParty->CountHirelings(); });
    auto alignmentTape = tapes.custom([] { return pParty->alignment; });
    test.playTraceFromTestData("issue_202.mm7", "issue_202.json");
    EXPECT_EQ(hirelingsTape.delta(), -1); // Judge shouldn't be with party anymore.
    EXPECT_EQ(alignmentTape, tape(PartyAlignment_Neutral, PartyAlignment_Evil)); // Party should turn evil.
}

GAME_TEST(Issues, Issue203) {
    // Judge's "I lost it" shouldn't crash.
    test.playTraceFromTestData("issue_203.mm7", "issue_203.json");
}

GAME_TEST(Issues, Issue211) {
    // Crash during accidental ok double click
    test.playTraceFromTestData("issue_211.mm7", "issue_211.json");
}

GAME_TEST(Issues, Issue223) {
    // Fire and air resistance not resetting between games
    auto fireTape = charTapes.resistances(CHARACTER_ATTRIBUTE_RESIST_FIRE);
    auto airTape = charTapes.resistances(CHARACTER_ATTRIBUTE_RESIST_AIR);
    test.playTraceFromTestData("issue_223.mm7", "issue_223.json");
    // expect normal resistances after restart 55-00-00-00.
    EXPECT_EQ(fireTape.frontBack(), tape({280, 262, 390, 241}, {5, 0, 0, 0}));
    EXPECT_EQ(airTape.frontBack(), tape({389, 385, 385, 381}, {5, 0, 0, 0}));
}

GAME_TEST(Issues, Issue238) {
    // Party vertical flight speed doesnt use frame pacing
    test.playTraceFromTestData("issue_238.mm7", "issue_238.json");
    EXPECT_LT(pParty->pos.z, 2500);
}

GAME_TEST(Issues, Issue248) {
    // Crash in NPC dialog.
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_248.mm7", "issue_248.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_NPC_DIALOGUE, SCREEN_GAME));
}

GAME_TEST(Issues, Issue268_939) {
    // Crash in ODM_GetFloorLevel.
    test.playTraceFromTestData("issue_268.mm7", "issue_268.json");

    // #939: Quick reference doesn't match vanilla.
    // hp
    EXPECT_EQ(pParty->pCharacters[0].GetHealth(), 71);
    EXPECT_EQ(pParty->pCharacters[1].GetHealth(), 80);
    EXPECT_EQ(pParty->pCharacters[2].GetHealth(), 154);
    EXPECT_EQ(pParty->pCharacters[3].GetHealth(), 169);
    // sp
    EXPECT_EQ(pParty->pCharacters[0].GetMana(), 0);
    EXPECT_EQ(pParty->pCharacters[1].GetMana(), 0);
    EXPECT_EQ(pParty->pCharacters[2].GetMana(), 55);
    EXPECT_EQ(pParty->pCharacters[3].GetMana(), 19);
    // ac
    EXPECT_EQ(pParty->pCharacters[0].GetActualAC(), 126);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAC(), 77);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAC(), 82);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAC(), 66);
    // attack
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(false), 30);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAttack(false), 37);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAttack(false), 29);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAttack(false), 9);
    // dmg
    EXPECT_EQ(pParty->pCharacters[0].GetMeleeDamageString(), "35 - 41");
    EXPECT_EQ(pParty->pCharacters[1].GetMeleeDamageString(), "39 - 55");
    EXPECT_EQ(pParty->pCharacters[2].GetMeleeDamageString(), "35 - 39");
    EXPECT_EQ(pParty->pCharacters[3].GetMeleeDamageString(), "Wand");
    // shoot
    EXPECT_EQ(pParty->pCharacters[0].GetRangedAttack(), 18);
    EXPECT_EQ(pParty->pCharacters[1].GetRangedAttack(), 23);
    EXPECT_EQ(pParty->pCharacters[2].GetRangedAttack(), 21);
    EXPECT_EQ(pParty->pCharacters[3].GetRangedAttack(), 17);
    // dmg
    EXPECT_EQ(pParty->pCharacters[0].GetRangedDamageString(), "9 - 14");
    EXPECT_EQ(pParty->pCharacters[1].GetRangedDamageString(), "11 - 16");
    EXPECT_EQ(pParty->pCharacters[2].GetRangedDamageString(), "11 - 16");
    EXPECT_EQ(pParty->pCharacters[3].GetRangedDamageString(), "Wand");
    // skills
    auto checkSkills = [](std::initializer_list<std::pair<int, int>> numSkillPairs) {
        for (auto pair : numSkillPairs) {
            int pSkillsCount = 0;
            for (CharacterSkillType j : allVisibleSkills()) {
                if (pParty->pCharacters[pair.first].pActiveSkills[j]) {
                    ++pSkillsCount;
                }
            }
            EXPECT_EQ(pSkillsCount, pair.second);
        }
    };
    // NB vanilla gets wrong count for characters - doesnt count learning
    checkSkills({ {0, 13}, {1, 11}, {2, 20}, {3, 11} });
}

GAME_TEST(Issues, Issue271) {
    // Party shouldn't yell when landing from flight.
    auto expressionTape = charTapes.expression(1);
    auto landingTape = tapes.custom([] { return !!(pParty->uFlags & PARTY_FLAGS_1_LANDING); });
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("issue_271.mm7", "issue_271.json");
    EXPECT_FALSE(expressionTape.contains(CHARACTER_EXPRESSION_FEAR));
    EXPECT_EQ(landingTape, tape(false, true));
    EXPECT_LT(zTape.delta(), -1000);
}

GAME_TEST(Issues, Issue272a) {
    // Controls menu bugs - resetting controls doesn't work.
    auto rightTape = tapes.config(engine->config->keybindings.Right);
    test.playTraceFromTestData("issue_272a.mm7", "issue_272a.json");
    EXPECT_EQ(rightTape, tape(PlatformKey::KEY_RIGHT, PlatformKey::KEY_H, PlatformKey::KEY_RIGHT)); // Pressing 'default' resets keys.
}

GAME_TEST(Issues, Issue272b) {
    // Check you cant leave menu with conflicting keys.
    test.playTraceFromTestData("issue_272b.mm7", "issue_272b.json");
    EXPECT_EQ(current_screen_type, SCREEN_KEYBOARD_OPTIONS);
    EXPECT_EQ(engine->_statusBar->get(), "Please resolve all key conflicts!");
}

GAME_TEST(Issues, Issue289a) {
    // Collisions climbing walls in dragon cave.
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("issue_289a.mm7", "issue_289a.json");
    // Shouldnt climb too high
    EXPECT_LT(zTape.max(), 350);
}

GAME_TEST(Issues, Issue289b) {
    // Collisions clipping through surfaces.
    float rx = 12032.0f, ry = 3720.0f;
    float rr = 12224.0f, rt = 3912.0f;
    auto distTape = tapes.custom([&] {
        // test party distance to square of pillar
        float dx = std::max({ rx - pParty->pos.x, 0.0f, pParty->pos.x - rr });
        float dy = std::max({ ry - pParty->pos.y, 0.0f, pParty->pos.y - rt });
        return (dx*dx + dy*dy);
    });
    test.playTraceFromTestData("issue_289b.mm7", "issue_289b.json");
    EXPECT_GT(distTape.min(), pParty->radius * pParty->radius);
}

GAME_TEST(Issues, Issue290) {
    // Town Hall bugs.
    auto fineTape = tapes.custom([] { return pParty->GetFine(); });
    auto goldTape = tapes.gold();
    test.playTraceFromTestData("issue_290.mm7", "issue_290.json");
    EXPECT_EQ(fineTape.delta(), -1000);
    EXPECT_EQ(goldTape.delta(), -1000);
}

GAME_TEST(Issues, Issue292a) {
    // Collisions - slip through signs
    auto yTape = tapes.custom([] { return pParty->pos.y; });
    test.playTraceFromTestData("issue_292a.mm7", "issue_292a.json");
    // Shouldnt pass through sign
    EXPECT_LT(yTape.max(), 4855.0f);
}

GAME_TEST(Issues, Issue292b) {
    // Collisions - becoming stuck
    float rx = -3028.0f, ry = 5815.0f;
    float rr = -2894.0f, rt = 6090.0f;
    auto distTape = tapes.custom([&] {
        // test party distance to square of altar
        float dx = std::max({ rx - pParty->pos.x, 0.0f, pParty->pos.x - rr });
        float dy = std::max({ ry - pParty->pos.y, 0.0f, pParty->pos.y - rt });
        return (dx * dx + dy * dy);
        });
    test.playTraceFromTestData("issue_292b.mm7", "issue_292b.json");
    EXPECT_GT(distTape.min(), pParty->radius * pParty->radius);
}

GAME_TEST(Issues, Issue293a) {
    // Test that barrels in castle Harmondale work and can be triggered only once, and that trash piles work,
    // give an item once, but give disease indefinitely.
    auto totalItemsTape = tapes.totalItemCount();
    auto conditionsTape = charTapes.conditions();
    test.playTraceFromTestData("issue_293a.mm7", "issue_293a.json", [] {
        EXPECT_EQ(pParty->pCharacters[0].uMight, 30);
        EXPECT_EQ(pParty->pCharacters[0].uIntelligence, 5);
        EXPECT_EQ(pParty->pCharacters[0].uPersonality, 5);
        EXPECT_EQ(pParty->pCharacters[0].uEndurance, 13);
        EXPECT_EQ(pParty->pCharacters[0].uSpeed, 14);
        EXPECT_EQ(pParty->pCharacters[0].uAccuracy, 13);
        EXPECT_EQ(pParty->pCharacters[0].uLuck, 7);
    });

    EXPECT_EQ(totalItemsTape.delta(), +1);
    EXPECT_EQ(conditionsTape.frontBack(), tape({CONDITION_GOOD, CONDITION_GOOD, CONDITION_GOOD, CONDITION_GOOD},
                                               {CONDITION_DISEASE_WEAK, CONDITION_DISEASE_WEAK, CONDITION_DISEASE_WEAK, CONDITION_DISEASE_WEAK}));
    EXPECT_EQ(pParty->pCharacters[0].uMight, 30);
    EXPECT_EQ(pParty->pCharacters[0].uIntelligence, 7); // +2
    EXPECT_EQ(pParty->pCharacters[0].uPersonality, 5);
    EXPECT_EQ(pParty->pCharacters[0].uEndurance, 13);
    EXPECT_EQ(pParty->pCharacters[0].uSpeed, 14);
    EXPECT_EQ(pParty->pCharacters[0].uAccuracy, 15); // +2
    EXPECT_EQ(pParty->pCharacters[0].uLuck, 7);
}

GAME_TEST(Issues, Issue293b) {
    // Test that table food in castle Harmondale is pickable only once and gives apples.
    auto foodTape = tapes.food();
    auto totalItemsTape = tapes.totalItemCount();
    auto hasAppleTape = tapes.hasItem(ITEM_RED_APPLE);
    test.playTraceFromTestData("issue_293b.mm7", "issue_293b.json");
    EXPECT_EQ(foodTape, tape(7));
    EXPECT_EQ(totalItemsTape, tape(18, 19));
    EXPECT_EQ(hasAppleTape, tape(false, true)); // Got an apple!
}

GAME_TEST(Issues, Issue293c) {
    // Test that cauldrons work, and work only once. The cauldron tested is in the Barrow Downs.
    test.playTraceFromTestData("issue_293c.mm7", "issue_293c.json", [] {
        EXPECT_EQ(pParty->pCharacters[0].sResAirBase, 230); // An interesting save we have here.
        EXPECT_EQ(pParty->pCharacters[1].sResAirBase, 50);
        EXPECT_EQ(pParty->pCharacters[2].sResAirBase, 24);
        EXPECT_EQ(pParty->pCharacters[3].sResAirBase, 18);
    });

    EXPECT_EQ(pParty->pCharacters[0].sResAirBase, 230);
    EXPECT_EQ(pParty->pCharacters[1].sResAirBase, 52); // +2
    EXPECT_EQ(pParty->pCharacters[2].sResAirBase, 24);
    EXPECT_EQ(pParty->pCharacters[3].sResAirBase, 18);
}

GAME_TEST(Issues, Issue294) {
    // Testing that party auto-casting shrapnel successfully targets rats & kills them, gaining experience.
    auto experienceTape = tapes.totalExperience();
    test.playTraceFromTestData("issue_294.mm7", "issue_294.json");
    // EXPECT_GT(experienceTape.delta(), 0); // Expect the giant rat to be dead after four shrapnel casts from character #4.
    // TODO(captainurist): ^passes now, but for the wrong reason - the rat decided to move after recent patches
}

// 300

GAME_TEST(Prs, Pr314_742) {
    // Check that character creating menu works.
    // Trace pretty much presses all the buttons and opens all the popups possible.
    test.playTraceFromTestData("pr_314.mm7", "pr_314.json");

    for (int i = 0; i < 4; i++)
        EXPECT_EQ(pParty->pCharacters[i].uLuck, 20);

    EXPECT_EQ(pParty->pCharacters[0].classType, CLASS_MONK);
    EXPECT_EQ(pParty->pCharacters[1].classType, CLASS_THIEF);
    EXPECT_EQ(pParty->pCharacters[2].classType, CLASS_RANGER);
    EXPECT_EQ(pParty->pCharacters[3].classType, CLASS_CLERIC);
    EXPECT_EQ(pParty->pCharacters[0].GetRace(), RACE_ELF);
    EXPECT_EQ(pParty->pCharacters[1].GetRace(), RACE_ELF);
    EXPECT_EQ(pParty->pCharacters[2].GetRace(), RACE_GOBLIN);
    EXPECT_EQ(pParty->pCharacters[3].GetRace(), RACE_ELF);

    // #742: Check that party qbits are set even if we press Clear when creating a party.
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_RED_POTION_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_SEASHELL_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_LONGBOW_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_PLATE_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_LUTE_ACTIVE));
    EXPECT_TRUE(pParty->_questBits.test(QBIT_EMERALD_ISLAND_HAT_ACTIVE));
}

GAME_TEST(Issues, Issue315) {
    test.loadGameFromTestData("issue_315.mm7");
    game.startNewGame(); // This shouldn't crash.
}

GAME_TEST(Issues, Issue331_679) {
    // Assert when traveling by horse caused by out of bound access to pObjectList->pObjects.
    auto goldTape = tapes.gold();
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_331.mm7", "issue_331.json");
    EXPECT_EQ(mapTape, tape("out04.odm", "out02.odm", "out04.odm")); // We did travel.

    // #679: Loading autosave after travelling by stables / boat results in gold loss.
    EXPECT_EQ(goldTape.delta(), 0);
    EXPECT_LT(goldTape.min(), goldTape.front()); // We did spend money.
}

GAME_TEST(Prs, Pr347) {
    // Testing that shops work.
    auto itemsTape = tapes.totalItemCount();
    auto goldTape = tapes.gold();
    test.playTraceFromTestData("pr_347.mm7", "pr_347.json");
    EXPECT_GT(itemsTape.delta(), 0); // Bought smth.
    EXPECT_LT(goldTape.delta(), 0); // Spent on items.
}

GAME_TEST(Issues, Issue355) {
    // EVENT_CastSpell damage to characters (fire bolts in temple of the moon for example) doesnt match GOG.
    // GOG: 6-2. OpenEnroth: 9-5.
    auto healthTape = charTapes.hps();
    test.playTraceFromTestData("issue_355.mm7", "issue_355.json");
    auto damageRange = healthTape.reversed().adjacentDeltas().flattened().filtered([] (int damage) { return damage > 0; }).minMax();
    // 2d3+0 with a non-random engine can't roll 2 or 6, so all values should be in [3, 5].
    EXPECT_EQ(damageRange, tape(3, 5));
}

GAME_TEST(Issues, Issue388) {
    // Testing that Arcomage works.
    // Trace enters tavern, plays arcomage, plays a couple of cards then exits and leaves tavern.
    int oldfpslimit = pArcomageGame->_targetFPS;
    pArcomageGame->_targetFPS = 500;

    auto arcomageTape = tapes.custom([] { return !!pArcomageGame->bGameInProgress; });
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_388.mm7", "issue_388.json");
    EXPECT_EQ(arcomageTape, tape(false, true, false)); // We've played arcomage.
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE, SCREEN_GAME)); // And returned to game screen.
    EXPECT_EQ(pArcomageGame->GameOver, 1); // With arcomage exit flag.

    pArcomageGame->_targetFPS = oldfpslimit;
}

GAME_TEST(Issues, Issue395) {
    // Check that learning skill works as intended.
    auto expTape = charTapes.experiences();
    auto learningTape = charTapes.skillLevels(CHARACTER_SKILL_LEARNING);
    test.playTraceFromTestData("issue_395.mm7", "issue_395.json");
    EXPECT_EQ(expTape.frontBack(), tape({100, 100, 100, 100}, {214, 228, 237, 258}));
    EXPECT_EQ(learningTape, tape({0, 4, 6, 10}));
}

// 400

GAME_TEST(Issues, Issue402) {
    // Attacking while wearing wetsuits shouldn't assert.
    auto checkCharactersWearWetsuits = [] {
        for (int i = 0; i < 4; i++)
            EXPECT_TRUE(pParty->pCharacters[i].wearsItemAnywhere(ITEM_QUEST_WETSUIT));
    };

    test.playTraceFromTestData("issue_402.mm7", "issue_402.json", [&] {
        checkCharactersWearWetsuits();
    });
    checkCharactersWearWetsuits();
}

GAME_TEST(Issues, Issue403_970) {
    // Entering Lincoln shouldn't crash.
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_403.mm7", "issue_403.json");
    EXPECT_EQ(mapTape, tape("out15.odm", "d23.blv")); // Shoals -> Lincoln.

    // #970: Armor Class is wrong.
    EXPECT_EQ(pParty->pCharacters[0].GetActualAC(), 10);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAC(), 5);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAC(), 10);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAC(), 7);
}

GAME_TEST(Issues, Issue405) {
    // FPS affects effective recovery time.
    auto runTrace = [&] {
        test.loadGameFromTestData("issue_405.mm7");
        // TODO(captainurist): Drop this if once we fix #1174. Right now the 1st char is selected on load, and pressing
        //                     the portrait again opens up character screen.
        if (pParty->activeCharacterIndex() != 1) {
            game.pressGuiButton("Game_Character1");
            game.tick(1);
        }
        game.pressGuiButton("Game_CastSpell");
        game.tick(1);
        game.pressGuiButton("SpellBook_Spell7"); // 7 is immolation.
        game.tick(1);
        game.pressGuiButton("SpellBook_Spell7"); // Confirm.
        game.tick(1);
    };
    engine->config->debug.AllMagic.setValue(true);

    // 100ms/frame
    test.restart(100, RANDOM_ENGINE_SEQUENTIAL);
    runTrace();
    game.tick(10);
    EXPECT_TRUE(pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Active());
    int firstRemainingRecovery = pParty->pCharacters[0].timeToRecovery;

    // 10ms/frame
    test.restart(10, RANDOM_ENGINE_SEQUENTIAL);
    runTrace();
    game.tick(100);
    EXPECT_TRUE(pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Active());
    int secondRemainingRecovery = pParty->pCharacters[0].timeToRecovery;

    EXPECT_EQ(firstRemainingRecovery, secondRemainingRecovery);
}

GAME_TEST(Issues, Issue408_939_970_996) {
    // Testing that the gameover loop works.
    // Trace enters throne room - resurecta - final task and exits gameover loop.
    auto screenTape = tapes.screen();
    auto mapTape = tapes.map();
    auto certTape = tapes.custom([] { return assets->winnerCert; });
    test.playTraceFromTestData("issue_408.mm7", "issue_408.json");
    // we should return to game screen
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE, SCREEN_GAMEOVER_WINDOW, SCREEN_GAME));
    // windowlist size should be 1
    EXPECT_EQ(lWindowList.size(), 1);
    // should have saved a winner cert tex
    EXPECT_GT(certTape.size(), 1);
    // we should be teleported to harmondale
    EXPECT_EQ(mapTape, tape("d30.blv", "out02.odm"));

    // #970: Armor Class is wrong.
    // #939: Quick reference doesnt match vanilla.
    // #996: Wrong attack damage when dual wielding blaster and offhand weapon.
    // hp
    EXPECT_EQ(pParty->pCharacters[0].GetHealth(), 1240);
    EXPECT_EQ(pParty->pCharacters[1].GetHealth(), 397);
    EXPECT_EQ(pParty->pCharacters[2].GetHealth(), 307);
    EXPECT_EQ(pParty->pCharacters[3].GetHealth(), 285);
    // sp
    EXPECT_EQ(pParty->pCharacters[0].GetMana(), 0);
    EXPECT_EQ(pParty->pCharacters[1].GetMana(), 77);
    EXPECT_EQ(pParty->pCharacters[2].GetMana(), 57);
    EXPECT_EQ(pParty->pCharacters[3].GetMana(), 543);
    // ac
    EXPECT_EQ(pParty->pCharacters[0].GetActualAC(), 137);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAC(), 128);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAC(), 87);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAC(), 92);
    // attack
    EXPECT_EQ(pParty->pCharacters[0].GetActualAttack(false), 75);
    EXPECT_EQ(pParty->pCharacters[1].GetActualAttack(false), 100);
    EXPECT_EQ(pParty->pCharacters[2].GetActualAttack(false), 96);
    EXPECT_EQ(pParty->pCharacters[3].GetActualAttack(false), 97);
    // dmg
    EXPECT_EQ(pParty->pCharacters[0].GetMeleeDamageString(), "32 - 61");
    EXPECT_EQ(pParty->pCharacters[1].GetMeleeDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[2].GetMeleeDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[3].GetMeleeDamageString(), "17 - 37");
    // shoot
    EXPECT_EQ(pParty->pCharacters[0].GetRangedAttack(), 63);
    EXPECT_EQ(pParty->pCharacters[1].GetRangedAttack(), 100);
    EXPECT_EQ(pParty->pCharacters[2].GetRangedAttack(), 96);
    EXPECT_EQ(pParty->pCharacters[3].GetRangedAttack(), 97);
    // dmg
    EXPECT_EQ(pParty->pCharacters[0].GetRangedDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[1].GetRangedDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[2].GetRangedDamageString(), "17 - 37");
    EXPECT_EQ(pParty->pCharacters[3].GetRangedDamageString(), "17 - 37");
    // skills
    auto checkSkills = [](std::initializer_list<std::pair<int, int>> numSkillPairs) {
        for (auto pair : numSkillPairs) {
            int pSkillsCount = 0;
            for (CharacterSkillType j : allVisibleSkills()) {
                if (pParty->pCharacters[pair.first].pActiveSkills[j]) {
                    ++pSkillsCount;
                }
            }
            EXPECT_EQ(pSkillsCount , pair.second);
        }
    };
    // NB vanilla gets wrong count for character index 2 (13) - doesnt count learning
    checkSkills({ {0, 10}, {1, 11}, {2, 14}, {3, 9} });
}

GAME_TEST(Issues, Issue417a) {
    // Testing that portal nodes looping doesnt assert.
    test.playTraceFromTestData("issue_417a.mm7", "issue_417a.json");
}

GAME_TEST(Issues, Issue417b) {
    // Testing that portal nodes looping doesnt assert.
    test.playTraceFromTestData("issue_417b.mm7", "issue_417b.json");
}

static void check427Buffs(const char *ctx, std::initializer_list<int> players, bool hasBuff) {
    for (int character : players) {
        for (CharacterBuff buff : {CHARACTER_BUFF_BLESS, CHARACTER_BUFF_PRESERVATION, CHARACTER_BUFF_HAMMERHANDS, CHARACTER_BUFF_PAIN_REFLECTION}) {
            EXPECT_EQ(pParty->pCharacters[character].pCharacterBuffs[buff].Active(), hasBuff)
                << "(with ctx=" << ctx << ", character=" << character << ", buff=" << std::to_underlying(buff) << ")";
        }
    }
}

GAME_TEST(Issues, Issue427a) {
    // Test that some of the buff spells that start to affect whole party starting from certain mastery work correctly.
    // In this test mastery is not enough for the whole party buff.
    test.playTraceFromTestData("issue_427a.mm7", "issue_427a.json");

    // Check that spell targeting works correctly - 1st char is getting the buffs.
    check427Buffs("a", {0}, true);
    check427Buffs("a", {1, 2, 3}, false);
}

GAME_TEST(Issues, Issue427b_528) {
    // Test that some of the buff spells that start to affect whole party starting from certain mastery work correctly.
    // In this test mastery is enough for the whole party.
    auto manaTape = charTapes.mp(2);
    test.playTraceFromTestData("issue_427b.mm7", "issue_427b.json");

    // Check that all character have buffs.
    check427Buffs("b", {0, 1, 2, 3}, true);

    // #528: Check that spells that target single character or entire party depending on mastery drain mana.
    EXPECT_EQ(manaTape.delta(), -60);
}

GAME_TEST(Issues, Issue442) {
    // Test that regular UI is blocked on spell cast.
    auto blessTape = tapes.custom([] { return pParty->pCharacters[1].pCharacterBuffs[CHARACTER_BUFF_BLESS].Active(); });
    test.playTraceFromTestData("issue_442.mm7", "issue_442.json");
    EXPECT_EQ(blessTape, tape(false, true));
}

GAME_TEST(Prs, Pr469) {
    // Assert when using Quick Spell button when spell is not set.
    test.playTraceFromTestData("pr_469.mm7", "pr_469.json", [] {
        for (int i = 0; i < 4; i++)
            EXPECT_EQ(pParty->pCharacters[i].uQuickSpell, SPELL_NONE);
    });
}

GAME_TEST(Issues, Issue488) {
    // Test that Mass Distortion spell works.
    auto actorHpTape = actorTapes.hp(24);
    test.playTraceFromTestData("issue_488.mm7", "issue_488.json");
    EXPECT_EQ(actorHpTape, tape(3, 2));
}

GAME_TEST(Issues, Issue489) {
    // Test that AOE version of Shrinking Ray spell works.
    auto chibisTape = actorTapes.countByBuff(ACTOR_BUFF_SHRINK);
    test.playTraceFromTestData("issue_489.mm7", "issue_489.json");
    EXPECT_EQ(chibisTape, tape(0, 15));
}

GAME_TEST(Issues, Issue490) {
    // Check that Poison Spray sprites are moving and doing damage.
    auto experienceTape = charTapes.experience(0);
    test.playTraceFromTestData("issue_490.mm7", "issue_490.json");
    EXPECT_EQ(experienceTape, tape(279, 285));
}

GAME_TEST(Issues, Issue491) {
    // Check that opening and closing Lloyd book does not cause Segmentation Fault.
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_491.mm7", "issue_491.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_SPELL_BOOK, SCREEN_GAME, SCREEN_BOOKS, SCREEN_GAME));
}

GAME_TEST(Issues, Issue492) {
    // Check that spells that target all visible actors work.
    auto experienceTape = charTapes.experiences();
    test.playTraceFromTestData("issue_492.mm7", "issue_492.json");
    EXPECT_EQ(experienceTape.frontBack(), tape({279, 311, 266, 260}, {287, 319, 274, 268}));
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
    EXPECT_EQ(mapTape, tape("mdt12.blv", "d29.blv")); // And party was teleported to Harmondale.
}

GAME_TEST(Issues, Issue504) {
    // Going to prison doesn't recharge hirelings.
    auto yearsTape = tapes.custom([] { return pParty->GetPlayingTime().GetYears(); });
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
    auto healthTape = tapes.totalHp();
    auto activeCharTape = tapes.custom([] { return pParty->activeCharacterIndex(); });
    test.playTraceFromTestData("issue_521.mm7", "issue_521.json");
    EXPECT_LT(healthTape.delta(), 0); // Party took fall damage.
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

GAME_TEST(Issues, Issue615) {
    // test 1 - ensure that clicking between active portraits changes active character.
    test.playTraceFromTestData("issue_615a.mm7", "issue_615a.json", []() { EXPECT_EQ(pParty->activeCharacterIndex(), 1); });
    EXPECT_EQ(pParty->activeCharacterIndex(), 3);
    // Assert when clicking on character portrait when no active character is present
    test.playTraceFromTestData("issue_615b.mm7", "issue_615b.json", []() { EXPECT_EQ(pParty->activeCharacterIndex(), 1); });
    EXPECT_EQ(pParty->activeCharacterIndex(), 4);
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
    auto healthTape = charTapes.hp(0);
    auto manaTape = charTapes.mp(0);
    test.playTraceFromTestData("issue_661.mm7", "issue_661.json");
    // two hour wait period is 24 blocks of 5 mins
    // one item that heals hp, three items heal mana
    EXPECT_EQ(healthTape.delta(), +24);
    EXPECT_EQ(manaTape.delta(), +3 * 24);
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
    EXPECT_GT(pParty->activeCharacter().timeToRecovery, 0);
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
    EXPECT_EQ(pParty->pos.toInt(), Vec3i(12042, 11779, 912));
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
    auto hardenedTape = tapes.custom([] { return !!(pParty->pCharacters[3].GetNthEquippedIndexItem(ITEM_SLOT_MAIN_HAND)->uAttributes & ITEM_HARDENED); });
    test.playTraceFromTestData("issue_724.mm7", "issue_724.json");
    EXPECT_EQ(hardenedTape, tape(false, true));
}

GAME_TEST(Issues, Issue728) {
    // Mousing over facets with nonexisting events shouldn't crash the game.
    test.playTraceFromTestData("issue_728.mm7", "issue_728.json");
}

GAME_TEST(Issues, Issue730) {
    // Thrown items are throwing a party of their own
    test.playTraceFromTestData("issue_730.mm7", "issue_730.json");
    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
        EXPECT_EQ(pSpriteObjects[i].vVelocity, Vec3i(0, 0, 0));
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
    EXPECT_EQ(mapTape, tape("out01.odm", "d28.blv")); // Emerald Isle -> Dragon's cave.
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
            EXPECT_GT(pParty->pCharacters[0].pCharacterBuffs[buff].GetExpireTime(), pParty->GetPlayingTime() + GameTime::FromHours(10));
    });

    EXPECT_GT(timeTape.delta(), GameTime::FromHours(8)); // Check that we did rest.
    EXPECT_LT(timeTape.delta(), GameTime::FromHours(10)); // Check that we didn't wait out the buff expire times.
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
    EXPECT_EQ(58, player0.GetAttackRecoveryTime(false)); // CHARACTER_BUFF_HASTE.
    // EXPECT_EQ(59, player0.GetAttackRecoveryTime(true)); // Can't call this b/c no bow.
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
    EXPECT_EQ(pParty->pCharacters[0].uIntelligenceBonus, 25);
}

GAME_TEST(Issues, Issue815) {
    // Test that subtract variable for character bits work
    test.playTraceFromTestData("issue_815.mm7", "issue_815.json");
    EXPECT_EQ(pParty->pCharacters[0].uIntelligenceBonus, 25);
}

GAME_TEST(Issues, Issue816) {
    // Test that encountering trigger event instruction does not assert
    test.playTraceFromTestData("issue_816.mm7", "issue_816.json"); // Should not assert
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
    // Death Blossom + ice blast crash
    auto deathsTape = actorTapes.countByState(AIState::Dead);
    test.playTraceFromTestData("issue_832.mm7", "issue_832.json");
    EXPECT_EQ(deathsTape.frontBack(), tape(0, 3));
}

GAME_TEST(Issues, Issue833) {
    // Test that quick spell castable on Shift and no crash with Shift+Click when quick spell is not set
    auto mana0Tape = charTapes.mp(0);
    auto mana1Tape = charTapes.mp(1);
    test.playTraceFromTestData("issue_833.mm7", "issue_833.json");
    EXPECT_EQ(mana0Tape.delta(), -2);
    EXPECT_EQ(mana1Tape.delta(), 0);
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
    EXPECT_NE(buf.Get(0)->pName, "Dummy");
}

GAME_TEST(Issues, Issue878) {
    // Test that numpad number keys are working
    auto bankTape = tapes.custom([] { return pParty->uNumGoldInBank; });
    test.playTraceFromTestData("issue_878.mm7", "issue_878.json");
    EXPECT_EQ(bankTape, tape(0, 123));
}

GAME_TEST(Issues, Issue895) {
    // Test that entering magic guild does not shift date
    auto timeTape = tapes.time();
    test.playTraceFromTestData("issue_895.mm7", "issue_895.json");
    EXPECT_LT(timeTape.delta(), GameTime::FromMinutes(5));
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

// 1000

GAME_TEST(Issues, Issues1004) {
    // Collisions: Can walk right through the bridge on Emerald Isle
    auto xTape = tapes.custom([] { return pParty->pos.x; });
    test.playTraceFromTestData("issue_1004.mm7", "issue_1004.json");
    EXPECT_LT(xTape.max(), 12552 + 1);
}

GAME_TEST(Prs, Pr1005) {
    // Testing collisions - stairs should work. In this test case the party is walking onto a wooden paving in Tatalia.
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("pr_1005.mm7", "pr_1005.json");
    EXPECT_EQ(zTape.frontBack(), tape(154, 193)); // Paving is at z=192, party z should be this value +1.
}

GAME_TEST(Issues, Issue1020) {
    // Test finishing the scavenger hunt quest. The game should not crash when there is no dialogue options.
    test.playTraceFromTestData("issue_1020.mm7", "issue_1020.json"); // Should not assert
}

GAME_TEST(Issues, Issue1034) {
    // Crash when casting telekinesis outdoors.
    auto houseTape = tapes.house();
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_1034.mm7", "issue_1034.json");
    EXPECT_TRUE(statusTape.contains("Select Target")); // Telekinesis message.
    EXPECT_EQ(houseTape, tape(HOUSE_INVALID, HOUSE_WEAPON_SHOP_EMERALD_ISLAND)); // We have entered into the shop.
}

GAME_TEST(Issues, Issue1036) {
    // Test that elemental magic guilds teach Learning skill and self magic guilds teach Meditation skill.
    test.playTraceFromTestData("issue_1036.mm7", "issue_1036.json");
    EXPECT_TRUE(pParty->pCharacters[2].pActiveSkills[CHARACTER_SKILL_LEARNING]);
    EXPECT_TRUE(pParty->pCharacters[2].pActiveSkills[CHARACTER_SKILL_MEDITATION]);
}

GAME_TEST(Issues, Issue1038) {
    // Crash while fighting Eyes in Nighon Tunnels
    auto conditionsTape = charTapes.conditions();
    test.playTraceFromTestData("issue_1038.mm7", "issue_1038.json");
    EXPECT_EQ(conditionsTape.frontBack(), tape({CONDITION_GOOD, CONDITION_INSANE, CONDITION_GOOD, CONDITION_INSANE},
                                               {CONDITION_SLEEP, CONDITION_INSANE, CONDITION_UNCONSCIOUS, CONDITION_UNCONSCIOUS}));
}

GAME_TEST(Issues, Issue1040) {
    // Crash when talking to 4-th dark advisor
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_1040.mm7", "issue_1040.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_HOUSE, SCREEN_GAME));
}

GAME_TEST(Issues, Issue1051) {
    // Collision code asserts when fighting Magogs in Nighon Tunnels.
    // Note that the bug only reproduces on high fps, the trace is shot at 15ms per frame.
    auto frameTimeTape = tapes.config(engine->config->debug.TraceFrameTimeMs);
    test.playTraceFromTestData("issue_1051.mm7", "issue_1051.json");
    EXPECT_EQ(frameTimeTape, tape(15)); // Don't redo this at different FPS, the problem won't reproduce.
}

GAME_TEST(Issues, Issue1068) {
    // Kills assert if characters don't have learning skill, but party has an npc that gives learning boost.
    auto expTape = charTapes.experiences();
    test.playTraceFromTestData("issue_1068.mm7", "issue_1068.json");
    EXPECT_EQ(expTape.frontBack(), tape({158039, 156727, 157646, 157417}, {158518, 157206, 158125, 157896}));
}

GAME_TEST(Issues, Issue1093) {
    // Town Portal on master can be cast near enemies
    auto screenTape = tapes.screen();
    auto manaTape = charTapes.mp(3);
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_1093.mm7", "issue_1093.json");
    EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_SPELL_BOOK, SCREEN_GAME));
    EXPECT_EQ(manaTape, tape(355)); // Character's mana didn't change.
    EXPECT_TRUE(statusTape.contains("Cast Town Portal"));
    EXPECT_TRUE(statusTape.contains("Spell failed"));
}

// 1100

GAME_TEST(Issues, Issue1115) {
    // Entering Arena on level 21 should not crash the game
    auto mapTape = tapes.map();
    auto dialogueTape = tapes.dialogueType();
    auto levelTape = charTapes.levels();
    test.playTraceFromTestData("issue_1115.mm7", "issue_1115.json");
    EXPECT_EQ(mapTape, tape("out02.odm", "d05.blv")); // Harmondale -> Arena.
    EXPECT_TRUE(dialogueTape.contains(DIALOGUE_ARENA_SELECT_CHAMPION));
    EXPECT_EQ(levelTape, tape({21, 21, 21, 21}));
}

GAME_TEST(Issues, Issue1155) {
    // Crash when pressing [Game Options] while talking to NPCs
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_1155.mm7", "issue_1155.json");
    EXPECT_FALSE(screenTape.contains(SCREEN_SPELL_BOOK));
    EXPECT_FALSE(screenTape.contains(SCREEN_REST));
    EXPECT_FALSE(screenTape.contains(SCREEN_QUICK_REFERENCE));
    EXPECT_FALSE(screenTape.contains(SCREEN_OPTIONS));
    EXPECT_FALSE(screenTape.contains(SCREEN_BOOKS));
    EXPECT_TRUE(screenTape.contains(SCREEN_CHARACTERS));
    EXPECT_TRUE(screenTape.contains(SCREEN_BRANCHLESS_NPC_DIALOG));
}

GAME_TEST(Issues, Issue1164) {
    // CHARACTER_EXPRESSION_NO animation ending abruptly - should show the character moving his/her head to the left,
    // then to the right.
    auto expressionTape = tapes.custom([] { return std::pair(pParty->pCharacters[0].expression, pEventTimer->Time()); });
    auto frameTimeTape = tapes.config(engine->config->debug.TraceFrameTimeMs);
    test.playTraceFromTestData("issue_1164.mm7", "issue_1164.json");
    EXPECT_EQ(frameTimeTape, tape(15)); // Don't redo at other frame rates.

    auto isNo = [] (const auto &pair) { return pair.first == CHARACTER_EXPRESSION_NO; };
    auto begin = std::find_if(expressionTape.begin(), expressionTape.end(), isNo);
    auto end = std::find_if_not(begin, expressionTape.end(), isNo);
    ASSERT_NE(end, expressionTape.end());

    // CHARACTER_EXPRESSION_NO should take 144 ticks, minus one frame. This one frame is an implementation artifact,
    // shouldn't really be there, but for now we test it the way it actually works.
    auto ticks = end->second - begin->second;
    int frameTicks = (128 * 15 + 999) / 1000;
    EXPECT_GE(ticks, 144 - frameTicks);
}

GAME_TEST(Issues, Issue1191) {
    auto foodTape = tapes.food();
    test.playTraceFromTestData("issue_1191.mm7", "issue_1191.json");

    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_FIRE).level(), 10);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_AIR).level(), 4);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_WATER).level(), 4);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_EARTH).level(), 4);

    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_SPIRIT).level(), 4);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_MIND).level(), 9); // 4, +3 dragon, +2 Ruler's ring
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_BODY).level(), 4);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_DARK).level(), 0);
    EXPECT_EQ(pParty->pCharacters[0].getActualSkillValue(CHARACTER_SKILL_LIGHT).level(), 0);

    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_FIRE).level(), 4);
    EXPECT_LE(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_AIR).level(), 3); // She has no skill. 0 or 3 skill level is fine
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_WATER).level(), 4);
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_EARTH).level(), 18); // 10, +3 dragon, +5 ring

    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_SPIRIT).level(), 13);
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_MIND).level(), 4);
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_BODY).level(), 18); // 10, +3 dragon, +5 ring
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_DARK).level(), 0);
    EXPECT_EQ(pParty->pCharacters[2].getActualSkillValue(CHARACTER_SKILL_LIGHT).level(), 0);

    // TODO(captainurist): Uncomment when food issues (1226) resolved
    // EXPECT_EQ(foodTape.delta(), -3);
    // EXPECT_EQ(pParty->GetFood(), 7);
}

GAME_TEST(Issues, Issue1196) {
    // Assert fails in Character::playEmotion when character looks down
    auto expr = tapes.custom([] { return pParty->activeCharacter().expression; });
    test.playTraceFromTestData("issue_1196.mm7", "issue_1196.json");
    EXPECT_FALSE(expr.contains(CHARACTER_EXPRESSION_32));
    EXPECT_TRUE(expr.contains(CHARACTER_EXPRESSION_LOOK_UP));
    EXPECT_TRUE(expr.contains(CHARACTER_EXPRESSION_LOOK_DOWN));
}

GAME_TEST(Issues, Issue1197) {
    // Assert on party death
    auto loc = tapes.map();
    auto deaths = tapes.deaths();
    test.playTraceFromTestData("issue_1197.mm7", "issue_1197.json");
    EXPECT_TRUE(loc.contains("out01.odm")); // make it back to emerald
    EXPECT_EQ(deaths.delta(), 1);
}

// 1200

GAME_TEST(Issues, Issue1251a) {
    // Part A - test that wand damage matches vanilla damage should be in range (d6 per skill) 8-48 for novice 8 fireball wand
    auto dragonhealth = tapes.custom([] { return pActors[0].currentHP; });
    test.playTraceFromTestData("issue_1251a.mm7", "issue_1251a.json");

    auto damageRange = dragonhealth.reversed().adjacentDeltas().minMax();
    EXPECT_GE(damageRange.front(), 8);
    EXPECT_LE(damageRange.back(), 48);
}

GAME_TEST(Issues, Issue1251b) {
    // Make sure charm wand doesn't assert
    auto charmedActors = actorTapes.countByBuff(ACTOR_BUFF_CHARM);
    test.playTraceFromTestData("issue_1251b.mm7", "issue_1251b.json");
    EXPECT_EQ(charmedActors.delta(), 3);
}

GAME_TEST(Issues, Issue1255) {
    // Cant buy green wand
    auto wandTape = tapes.hasItem(ITEM_FAIRY_WAND_OF_LASHING);
    test.playTraceFromTestData("issue_1255.mm7", "issue_1255.json");
    EXPECT_EQ(wandTape, tape(false, true));
}

GAME_TEST(Issues, Issue1273) {
    // Assert when clicking on shop video area
    auto dialogueTape = tapes.dialogueType();
    test.playTraceFromTestData("issue_1273.mm7", "issue_1273.json");
    EXPECT_EQ(dialogueTape, tape(DIALOGUE_NULL, DIALOGUE_MAIN));
}

GAME_TEST(Issues, Issue1277) {
    // Crash when press enter on character skills tab
    test.playTraceFromTestData("issue_1277.mm7", "issue_1277.json");
    EXPECT_EQ(current_screen_type, SCREEN_CHARACTERS);
}

GAME_TEST(Issues, Issue1281) {
    // Assert when drinking from THE WELL in Eofol.
    auto acTape = charTapes.ac(0);
    test.playTraceFromTestData("issue_1281.mm7", "issue_1281.json");
    EXPECT_EQ(acTape.delta(), -50); // We've hit the -50 AC branch in the script that used to trigger the assertion.
}

GAME_TEST(Issues, Issue1282) {
    // Picking up an item asserts.
    auto itemTape = tapes.hasItem(ITEM_LEATHER_ARMOR);
    auto totalObjectsTape = tapes.mapItemCount();
    test.playTraceFromTestData("issue_1282.mm7", "issue_1282.json");
    EXPECT_EQ(itemTape, tape(false, true));
    EXPECT_EQ(totalObjectsTape.delta(), -1);
}

// 1300

GAME_TEST(Issues, Issue1315) {
    // Dying in turn-based mode asserts.
    auto deathsTape = tapes.deaths();
    auto mapTape = tapes.map();
    auto stateTape = tapes.custom([] { return std::tuple(pParty->bTurnBasedModeOn, uGameState); });
    test.playTraceFromTestData("issue_1315.mm7", "issue_1315.json");
    EXPECT_EQ(deathsTape.delta(), +1);
    EXPECT_EQ(mapTape, tape("out12.odm", "out02.odm")); // Land of the Giants -> Harmondale.
    EXPECT_EQ(stateTape, tape(std::tuple(false, GAME_STATE_PLAYING),
                              std::tuple(true, GAME_STATE_PLAYING),
                              std::tuple(false, GAME_STATE_PARTY_DIED), // Instant switch from turn-based & alive into realtime & dead,
                              std::tuple(false, GAME_STATE_PLAYING)));  // meaning that the party died in turn-based mode.
}

GAME_TEST(Prs, Pr1325) {
    // Trolls drop vials of troll blood.
    auto vialsTape = tapes.mapItemCount(ITEM_REAGENT_VIAL_OF_TROLL_BLOOD);
    auto deadTape = actorTapes.countByState(AIState::Dead);
    test.playTraceFromTestData("pr_1325.mm7", "pr_1325.json");
    EXPECT_EQ(vialsTape.delta(), +6);
    EXPECT_EQ(deadTape.delta(), +84); // Too much armageddon...
}

GAME_TEST(Issues, Issue1331) {
    // "of David" enchanted bows should do double damage against Titans.
    auto hpsTape = actorTapes.hps({31, 33});
    auto deadTape = actorTapes.indicesByState(AIState::Dead);
    test.playTraceFromTestData("issue_1331.mm7", "issue_1331.json");
    EXPECT_EQ(deadTape.frontBack(), tape(std::initializer_list<int>{}, {31, 33})); // Check that titans are dead.

    // Damage as stated in the character sheet is 41-45. Crossbow is 4d2+7. Because of how non-random engine works,
    // 4d2+7 will always roll 13, and thus the 41-45 range is effectively compressed into 43-43.
    //
    // With the "of David" enchantment, the 4d2+7 part of the damage is doubled, so max damage is now 43+13=56.
    //
    // Min damage is so low because titans have physical resistance. And then we also have to multiply the damage by
    // two because the character shoots two arrows at a time.
    EXPECT_EQ(pParty->pCharacters[2].GetBowItem()->special_enchantment, ITEM_ENCHANTMENT_TITAN_SLAYING);
    EXPECT_EQ(pParty->pCharacters[2].GetRangedDamageString(), "41 - 45");
    auto damageRange = hpsTape.reversed().adjacentDeltas().flattened().filtered([] (int damage) { return damage > 0; }).minMax();
    // 2 -> 3 change here can happen. This just means that the Titans' physical resistance was never "lucky enough" to roll the damage down to 1 two times in a row.
    EXPECT_EQ(damageRange, tape(/*1 * 2*/ 3, (43 + 13) * 2));
}

GAME_TEST(Issues, Issue1338) {
    // Casting telepathy on an actor and then killing it results in the actor not dropping any gold.
    auto deadTape = actorTapes.indicesByState(AIState::Dead);
    auto statusTape = tapes.statusBar();
    auto goldTape = tapes.gold();
    auto peasantGoldTape = tapes.custom([] { return pActors[18].items[3].goldAmount; });
    test.playTraceFromTestData("issue_1338.mm7", "issue_1338.json");
    EXPECT_EQ(deadTape, tape(std::initializer_list<int>{}, {18}, std::initializer_list<int>{})); // Alive -> Dead -> corpse picked up.
    EXPECT_GT(peasantGoldTape.max(), 0); // Peasant should have had gold generated.
    EXPECT_EQ(goldTape.delta(), peasantGoldTape.max());
    EXPECT_TRUE(statusTape.contains(fmt::format("{} gold", peasantGoldTape.max()))); // Telepathy status message.
    EXPECT_TRUE(statusTape.contains(fmt::format("You found {} gold!", peasantGoldTape.max()))); // Corpse pickup message.
}

GAME_TEST(Issues, Issue1340) {
    // Gold piles in chests are generated with 0 gold.
    auto goldTape = tapes.gold();
    auto mapTape = tapes.map();
    auto statusTape = tapes.statusBar();
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_1340.mm7", "issue_1340.json");
    EXPECT_EQ(mapTape, tape("out01.odm", "d29.blv")); // Emerald Isle -> Castle Harmondale. Map change is important because
                                                      // we want to trigger map respawn on first visit.
    EXPECT_TRUE(screenTape.contains(SCREEN_CHEST));
    EXPECT_GT(goldTape.delta(), 0); // Party should have picked some gold from the chest.
    EXPECT_FALSE(statusTape.contains("You found 0 gold!")); // No piles of 0 size.
    for (int gold : goldTape.adjacentDeltas())
        EXPECT_TRUE(statusTape.contains(fmt::format("You found {} gold!", gold)));
}

GAME_TEST(Issues, Issue1341) {
    // Can't steal gold from peasants.
    auto goldTape = tapes.gold();
    auto peasantGoldTape = tapes.custom([] { return pActors[6].items[3].goldAmount; });
    auto statusTape = tapes.statusBar();
    auto deadTape = actorTapes.countByState(AIState::Dead);
    test.playTraceFromTestData("issue_1341.mm7", "issue_1341.json");
    EXPECT_GT(goldTape.delta(), 0); // We did steal some gold.
    EXPECT_EQ(peasantGoldTape.max(), goldTape.delta()); // And we did steal it from this peasant.
    EXPECT_TRUE(statusTape.contains("Roderick failed to steal anything!")); // We have tried many times.
    EXPECT_TRUE(statusTape.contains(fmt::format("Roderick stole {} gold!", peasantGoldTape.max()))); // And succeeded.
    EXPECT_EQ(deadTape, tape(0)); // No one died in the process.
}

GAME_TEST(Issues, Issue1342) {
    // Gold piles are generated with 0 gold.
    auto goldTape = tapes.gold();
    auto pilesTape = tapes.mapItemCount(ITEM_GOLD_SMALL);
    auto statusTape = tapes.statusBar();
    auto mapTape = tapes.map();
    test.playTraceFromTestData("issue_1342.mm7", "issue_1342.json");
    EXPECT_EQ(mapTape, tape("out01.odm", "d28.blv")); // Emerald Isle -> Dragon Cave. Map change is important here
                                                      // because we need to trigger map respawn on first visit.
    EXPECT_GT(goldTape.delta(), 0); // We picked up some gold.
    EXPECT_EQ(pilesTape, tape(0, 10, 9, 8, 7)); // Minus three small gold piles.
    EXPECT_FALSE(statusTape.contains("You found 0 gold!")); // No piles of 0 size.
    for (int gold : goldTape.adjacentDeltas())
        EXPECT_TRUE(statusTape.contains(fmt::format("You found {} gold!", gold)));
}

GAME_TEST(Issues, Issue1364) {
    // Saving in Arena should display an appropriate status message.
    auto mapTape = tapes.map();
    auto statusTape = tapes.statusBar();
    auto screenTape = tapes.screen();
    test.playTraceFromTestData("issue_1364.mm7", "issue_1364.json");
    EXPECT_EQ(mapTape, tape("out02.odm", "d05.blv")); // Harmondale -> Arena.
    EXPECT_TRUE(statusTape.contains("No saving in the Arena")); // Clicking the save button didn't work.
    EXPECT_TRUE(screenTape.contains(SCREEN_HOUSE)); // We have visited the stables.
    EXPECT_TRUE(screenTape.contains(SCREEN_MENU)); // Opened the game menu while in the Arena.
    EXPECT_FALSE(screenTape.contains(SCREEN_SAVEGAME)); // But save menu didn't open on click.
}

GAME_TEST(Issues, Issue1368) {
    // maybeWakeSoloSurvivor() error
    auto canActTape = tapes.custom([] { return pParty->canActCount(); });
    auto sleepTape = tapes.custom([] { return pParty->pCharacters[0].conditions.Has(CONDITION_SLEEP); });
    test.playTraceFromTestData("issue_1368.mm7", "issue_1368.json");
    // No one can act - try waking
    EXPECT_EQ(canActTape.min(), 0);
    // Shouldve been asleep
    EXPECT_TRUE(sleepTape.contains(true));
    // But awake at the end
    EXPECT_EQ(sleepTape.back(), false);
}

GAME_TEST(Issues, Issue1371) {
    // Collisions - Party struggles to climb stairs to quarter deck
    auto zTape = tapes.custom([] { return pParty->pos.z; });
    test.playTraceFromTestData("issue_1371.mm7", "issue_1371.json");
    EXPECT_GT(zTape.max(), 448);
}
