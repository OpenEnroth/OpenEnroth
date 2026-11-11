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
