#include <string_view>

#include "Testing/Game/GameTest.h"

#include "Engine/Objects/MonsterEnums.h"

// Forward declaration. `ParseMissleAttackType` is defined in Monsters.cpp without
// being exposed through Monsters.h, but it has external linkage, so we can pull
// it in for the unit test.
MonsterProjectile ParseMissleAttackType(std::string_view missle_attack_str);

GAME_TEST(Monsters, ParseMissleAttackTypeFlamingArrow) {
    // Regression test for issue #2144.
    // monsters.txt stores the flaming-arrow projectile column as the 6-char abbreviation
    // "FireAr" (this is the actual cell value for e.g. Elite Archer and Bowman). The parser
    // used to compare against "ARROWF", which never matched, so fire-arrow archers silently
    // fell back to MONSTER_PROJECTILE_NONE.
    EXPECT_EQ(ParseMissleAttackType("FireAr"), MONSTER_PROJECTILE_FLAMING_ARROW);
    // The comparison is case-insensitive, matching the original engine's behavior.
    EXPECT_EQ(ParseMissleAttackType("firear"), MONSTER_PROJECTILE_FLAMING_ARROW);
    EXPECT_EQ(ParseMissleAttackType("FIREAR"), MONSTER_PROJECTILE_FLAMING_ARROW);

    // Spot-check the other archer cell so we don't regress the happy path.
    EXPECT_EQ(ParseMissleAttackType("Arrow"), MONSTER_PROJECTILE_ARROW);
    EXPECT_EQ(ParseMissleAttackType("ARROW"), MONSTER_PROJECTILE_ARROW);

    // And a few elemental bolts to make sure we didn't break anything else.
    EXPECT_EQ(ParseMissleAttackType("Fire"), MONSTER_PROJECTILE_FIRE_BOLT);
    EXPECT_EQ(ParseMissleAttackType("Water"), MONSTER_PROJECTILE_WATER_BOLT);
    EXPECT_EQ(ParseMissleAttackType("Ener"), MONSTER_PROJECTILE_ENERGY_BOLT);

    // Unknown / empty inputs should fall back to NONE.
    EXPECT_EQ(ParseMissleAttackType(""), MONSTER_PROJECTILE_NONE);
    EXPECT_EQ(ParseMissleAttackType("ARROWF"), MONSTER_PROJECTILE_NONE);
    EXPECT_EQ(ParseMissleAttackType("nonsense"), MONSTER_PROJECTILE_NONE);
}
