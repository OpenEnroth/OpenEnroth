#include <memory>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/RenderEntities.h"
#include "Engine/stru314.h"

#include "Library/Color/Color.h"
#include "Library/Geometry/Vec.h"

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
}

UNIT_TEST(DecalBuilder, DecalRingEvictsOldest) {
    // DecalsCount used to double as the ring cursor and was reset to zero on reaching Decals.size(), which dropped
    // every decal in the level at once instead of dropping just the oldest one.
    auto builder = std::make_unique<DecalBuilder>();  // Megabytes large, don't put it on the stack.
    const int ringSize = builder->Decals.size();

    stru314 floorPlane;
    floorPlane.Normal = Vec3f(0, 0, 1);
    floorPlane.dist = 0;
    floorPlane.computeBasis();

    // Wide enough that none of the decals built below is clipped away.
    RenderVertexSoft faceVerts[4];
    faceVerts[0].vWorldPosition = Vec3f(-100, -100, 0);
    faceVerts[1].vWorldPosition = Vec3f(1200, -100, 0);
    faceVerts[2].vWorldPosition = Vec3f(1200, 100, 0);
    faceVerts[3].vWorldPosition = Vec3f(-100, 100, 0);

    // Splats march along the x axis so that DecalXPos tells which one ended up in a slot.
    auto addDecalAt = [&] (int x) {
        Bloodsplat splat;
        splat.pos = Vec3f(x, 0, 10);
        splat.radius = 20;
        EXPECT_TRUE(builder->Build_Decal_Geometry(0, LocationIndoors, &splat, splat.radius, Color(), splat.pos.z,
                                                  &floorPlane, 4, faceVerts, 0));
    };

    addDecalAt(0);
    EXPECT_EQ(std::pair(builder->DecalsCount, builder->decalsCursor), (std::pair<size_t, size_t>(1, 1)));
    EXPECT_EQ(builder->Decals[0].DecalXPos, 0);

    for (int x = 1; x < ringSize; x++)
        addDecalAt(x);
    EXPECT_EQ(std::pair(builder->DecalsCount, builder->decalsCursor), (std::pair<size_t, size_t>(ringSize, 0)));

    addDecalAt(ringSize);
    EXPECT_EQ(std::pair(builder->DecalsCount, builder->decalsCursor), (std::pair<size_t, size_t>(ringSize, 1)));
    EXPECT_EQ(builder->Decals[0].DecalXPos, ringSize);  // Oldest was evicted.
    EXPECT_EQ(builder->Decals[1].DecalXPos, 1);  // Everything else is still there.
}
