#include "Testing/Unit/UnitTest.h"

#include "Engine/GameType.h"
#include "Engine/Objects/Character.h"

// MM8 Dragons wear no equipment. Every paperdoll equip attempt runs through `CanEquip_RaceAndAlignmentCheck`.
UNIT_TEST(Character, DragonCanEquipNothingUnderMm8) {
    setGameType(GAME_TYPE_MM8);

    Character dragon;
    dragon.classType = CLASS_MM8_DRAGON;
    ASSERT_EQ(dragon.GetRace(), RACE_MM8_DRAGON);

    EXPECT_FALSE(dragon.CanEquip_RaceAndAlignmentCheck(ITEM_LEATHER_ARMOR));
    EXPECT_FALSE(dragon.CanEquip_RaceAndAlignmentCheck(ITEM_BRASS_RING));

    setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
}

UNIT_TEST(Character, NonDragonCanStillEquipUnderMm8) {
    setGameType(GAME_TYPE_MM8);

    Character knight;
    knight.classType = CLASS_MM8_KNIGHT;
    ASSERT_NE(knight.GetRace(), RACE_MM8_DRAGON);

    EXPECT_TRUE(knight.CanEquip_RaceAndAlignmentCheck(ITEM_LEATHER_ARMOR));

    setGameType(GAME_TYPE_MM7); // Restore the default so other tests aren't affected.
}
