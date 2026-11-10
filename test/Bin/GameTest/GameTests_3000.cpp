#include <string>
#include <tuple>

#include "Testing/Game/GameTest.h"

#include "Engine/MapEnums.h"
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
    for (auto [actorId, sex, minPortrait, maxPortrait] : {std::tuple(18, SEX_MALE, 2, 100), std::tuple(1, SEX_FEMALE, 201, 250)}) {
        SCOPED_TRACE(fmt::format("actorId={}", actorId));
        test.prepareForNextTest();
        auto screenTape = tapes.screen();
        auto speakerTape = tapes.custom([] { return houseNpcs.empty() ? -1 : speakingNpcId; });
        auto texturesTape = tapes.hudTextures();
        auto portraitTape = tapes.custom([] { return houseNpcs.empty() ? std::string() : houseNpcs[0].icon->name(); });
        game.startNewGame();
        test.startTaping();
        Actor &peasant = pActors[actorId];
        MonsterType monsterType = monsterTypeForMonsterId(peasant.monsterInfo.id);
        ASSERT_EQ(raceForMonsterType(monsterType), RACE_HUMAN);
        ASSERT_EQ(sexForMonsterType(monsterType), sex);
        ASSERT_GE(peasant.npcId, 5000); // Ids from 5000 up are NPCs generated on map load.

        game.teleportTo(MAP_EMERALD_ISLAND, peasant.pos - Vec3f(300, 0, 0), 0);
        game.pointMouseAtActor(actorId);
        game.pressAndReleaseButton(BUTTON_LEFT);
        game.tick();

        EXPECT_EQ(screenTape, tape(SCREEN_GAME, SCREEN_NPC_DIALOGUE));
        EXPECT_EQ(speakerTape, tape(-1, static_cast<int>(peasant.npcId)));
        ASSERT_EQ(portraitTape.size(), 2);
        EXPECT_CONTAINS(texturesTape.back(), portraitTape.back());
        int portrait = fromString<int>(portraitTape.back().substr(3)); // Portrait textures are named "npc" plus the portrait id.
        EXPECT_GE(portrait, minPortrait);
        EXPECT_LE(portrait, maxPortrait);
    }
}
