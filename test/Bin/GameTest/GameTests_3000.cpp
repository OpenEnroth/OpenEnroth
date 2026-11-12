#include <set>
#include <string>
#include <utility>

#include "Testing/Game/GameTest.h"

#include "Engine/MapEnums.h"
#include "Engine/Party.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/MonsterEnumFunctions.h"

#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIHouses.h"

#include "Library/Serialization/Serialization.h"

#include "GameTestCommon.h"

// 3000

GAME_TEST(Issues, Issue3002) {
    // Human peasants with a generated name and profession had elf portraits.
    test.prepareForNextTest(10, RANDOM_ENGINE_MERSENNE_TWISTER); // Short frames keep wandering peasants under the cursor.
    auto speakerTape = tapes.custom([] { return houseNpcs.empty() ? -1 : speakingNpcId; });
    auto texturesTape = tapes.hudTextures();
    auto portraitTape = tapes.custom([] { return houseNpcs.empty() ? std::string() : houseNpcs[0].icon->name(); });
    game.startNewGame();
    test.startTaping();
    pParty->_questBits.set(QBIT_EMERALD_ISLAND_MARGARET_TOUR_OFF); // Margaret's tour hints fire where the party lands.

    AccessibleVector<int> expectedSpeakers = {-1};
    std::set<Sex> sexes;
    for (const Actor &peasant : pActors) {
        if (peasant.npcId < 5000)
            continue; // Ids from 5000 up are the NPCs rolled on map load.
        SCOPED_TRACE(fmt::format("actorId={}", peasant.id));
        MonsterType monsterType = monsterTypeForMonsterId(peasant.monsterInfo.id);
        ASSERT_EQ(raceForMonsterType(monsterType), RACE_HUMAN);
        Sex sex = sexForMonsterType(monsterType);
        auto [minPortrait, maxPortrait] = sex == SEX_MALE ? std::pair(2, 100) : std::pair(201, 250);

        game.teleportTo(MAP_EMERALD_ISLAND, peasant.pos - Vec3f(300, 0, 0), 0);
        game.pointMouseAtActor(peasant.id);
        game.pressAndReleaseButton(BUTTON_LEFT);
        game.tick();
        EXPECT_CONTAINS(texturesTape.back(), portraitTape.back());
        int portrait = fromString<int>(portraitTape.back().substr(3)); // Portrait textures are named "npc" plus the portrait id.
        EXPECT_GE(portrait, minPortrait);
        EXPECT_LE(portrait, maxPortrait);

        game.pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
        game.tick();
        expectedSpeakers.push_back(peasant.npcId);
        expectedSpeakers.push_back(-1);
        sexes.insert(sex);
    }

    EXPECT_EQ(speakerTape, expectedSpeakers);
    EXPECT_EQ(sexes, std::set({SEX_MALE, SEX_FEMALE}));
}

GAME_TEST(Issues, Issue3009) {
    // A Divine Restoration potion used on another character's portrait cured the character holding it instead.
    game.startNewGame();
    for (int i = 0; i < 3; i++)
        pParty->pCharacters[i].SetCondition(CONDITION_WEAK, 0);

    auto weakTape = charTapes.haveConditions(CONDITION_WEAK);
    auto activeTape = tapes.activeCharacterIndex();
    auto heldItemTape = tapes.custom([] { return pParty->pPickedItem.itemId; });
    test.startTaping();
    game.pressAndReleaseKey(PlatformKey::KEY_DIGIT_3);
    game.tick();
    pParty->setHoldingItem(Item(ITEM_POTION_DIVINE_RESTORATION));
    game.pressGuiButton("Game_Character1", BUTTON_RIGHT); // Use the potion on the first character's portrait.
    game.tick();

    EXPECT_EQ(weakTape, tape({true, true, true, false}, {false, true, true, false})); // Only the first character is cured.
    EXPECT_EQ(activeTape, tape(2, 0)); // The third character used the potion and went into recovery.
    EXPECT_EQ(heldItemTape, tape(ITEM_NULL)); // The potion is gone in the frame it was picked up in.
}
