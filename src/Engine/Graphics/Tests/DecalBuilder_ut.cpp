#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Graphics/DecalBuilder.h"

UNIT_TEST(BloodsplatContainer, AddBloodsplatSaturates) {
    // uNumBloodsplats used to wrap modulo 64 while every consumer read it as a count, so exactly 64 splats in one
    // frame reported zero, and all of them were dropped.
    auto fill = [] (int count) {
        BloodsplatContainer container;
        int accepted = 0;
        for (int i = 0; i < count; i++)
            accepted += container.AddBloodsplat(Vec3f(i, 0, 0), 10, Color()) ? 1 : 0;
        return std::pair(container.uNumBloodsplats, accepted);
    };

    EXPECT_EQ(fill(63), std::pair(63u, 63));
    EXPECT_EQ(fill(64), std::pair(64u, 64));
    EXPECT_EQ(fill(65), std::pair(64u, 64));
    EXPECT_EQ(fill(128), std::pair(64u, 64));
}
