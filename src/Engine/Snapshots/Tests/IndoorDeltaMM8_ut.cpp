#include <array>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Objects/Actor.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/BlobInputStream.h"

// MM8's `.dlv` indoor delta. Structural differences from
// `IndoorDelta_MM7`: `actors` uses the wider `Actor_MM8` (same widening as `OutdoorDelta_MM8`'s), and there's a
// trailing fixed-size 100-element `MapNote_MM8` block with no MM7 equivalent - identical in kind and position to
// `OutdoorDelta_MM8`'s. Verified byte-for-byte against all 47 MM8 `.dlv` dungeons (zero leftover bytes
// on every one). This builds a minimal synthetic fixture with that same layout (one face, one decoration, one
// actor, no doors/sprite objects/chests) to exercise `deserialize`/`reconstruct` without requiring real game data.
namespace {

Blob makeIndoorDeltaMM8Blob(int16_t actorHp, uint16_t actorResFire, MonsterId monsterInfoId) {
    Blob blob;
    BlobOutputStream out(&blob);

    LocationHeader_MM7 header = {};
    serialize(header, &out);

    std::array<char, 875> visibleOutlines = {};
    serialize(visibleOutlines, &out);

    std::vector<uint32_t> faceAttributes(1, 0); // presized by ctx to 1 face.
    serialize(faceAttributes, &out, tags::unsized);

    std::vector<uint16_t> decorationFlags(1, 0); // presized by ctx to 1 decoration.
    serialize(decorationFlags, &out, tags::unsized);

    Actor_MM8 actor = {};
    actor.hp = actorHp;
    actor.monsterInfo.hp = static_cast<uint32_t>(actorHp);
    actor.monsterInfo.resFire = actorResFire; // MM8-only 2-byte-wide field - verifies it round-trips at its offset.
    actor.monsterInfo.id = std::to_underlying(monsterInfoId);
    actor.pos = Vec3s(100, 200, 50);
    std::vector<Actor_MM8> actors = {actor};
    serialize(actors, &out);

    std::vector<SpriteObject_MM7> spriteObjects; // empty
    serialize(spriteObjects, &out);

    std::vector<Chest_MM7> chests; // empty
    serialize(chests, &out);

    std::vector<BLVDoor_MM7> doors; // empty, presized by ctx to 0 doors.
    serialize(doors, &out, tags::unsized);

    std::vector<int16_t> doorsData; // empty, presized by ctx's doorsDataSizeBytes=0.
    serialize(doorsData, &out, tags::unsized);

    PersistentVariables_MM7 eventVariables = {};
    serialize(eventVariables, &out);

    int64_t lastVisitTime = 0;
    serialize(lastVisitTime, &out);
    MapWeather_MM7 weather = {};
    serialize(weather, &out);

    std::array<MapNote_MM8, 100> mapNotes = {};
    mapNotes[0].x = 32767; // Sentinel value seen in MM8 `.dlv` files - just needs to round-trip.
    mapNotes[0].y = 32767;
    serialize(mapNotes, &out);

    out.close();
    return blob;
}

IndoorLocation_MM8 makeContext() {
    IndoorLocation_MM8 location = {};

    BLVFace_MM7 face = {};
    location.faces = {face};

    LevelDecoration_MM7 decoration = {};
    location.decorations = {decoration};

    location.doorCount = 0;
    location.header.doorsDataSizeBytes = 0;

    return location;
}

} // namespace

UNIT_TEST(IndoorDeltaMM8, DeserializeConsumesEntireStreamWithNoLeftoverBytes) {
    Blob blob = makeIndoorDeltaMM8Blob(30, 15, MONSTER_TROLL_A);
    IndoorLocation_MM8 ctx = makeContext();

    IndoorDelta_MM8 delta;
    EXPECT_NO_THROW(deserialize(blob, &delta, tags::context(ctx)));

    ASSERT_EQ(delta.actors.size(), 1u);
    EXPECT_EQ(delta.actors[0].hp, 30);
    EXPECT_EQ(delta.actors[0].monsterInfo.resFire, 15u);
    EXPECT_EQ(delta.actors[0].monsterInfo.id, std::to_underlying(MONSTER_TROLL_A));
    EXPECT_TRUE(delta.spriteObjects.empty());
    EXPECT_TRUE(delta.chests.empty());
    EXPECT_TRUE(delta.doors.empty());
    EXPECT_EQ(delta.mapNotes[0].x, 32767);
    EXPECT_EQ(delta.mapNotes[0].y, 32767);
}

UNIT_TEST(IndoorDeltaMM8, ReconstructsActorAndNarrowsMM8OnlyWideResistance) {
    Blob blob = makeIndoorDeltaMM8Blob(30, 15, MONSTER_TROLL_A);
    IndoorLocation_MM8 ctx = makeContext();

    IndoorDelta_MM8 delta;
    deserialize(blob, &delta, tags::context(ctx));
    ASSERT_EQ(delta.actors.size(), 1u);

    Actor actor;
    reconstruct(delta.actors[0], &actor);

    EXPECT_EQ(actor.hp, 30);
    EXPECT_EQ(actor.monsterInfo.id, MONSTER_TROLL_A);
    // MM8's resistance fields are 2 bytes wide (`MonsterInfo_MM8::resFire`), but the runtime `MonsterInfo` struct
    // is still `uint8_t` - same as what MM7 data always provided - so this must narrow, not overflow/truncate
    // unexpectedly, for values that fit in a byte.
    EXPECT_EQ(actor.monsterInfo.resFire, 15);
    EXPECT_EQ(actor.pos.x, 100);
    EXPECT_EQ(actor.pos.y, 200);
    EXPECT_EQ(actor.pos.z, 50);
}

// MM8 maps used to be written with `IndoorDelta_MM7`'s layout. The writer's output must round-trip through
// the reader with no leftover bytes.
UNIT_TEST(IndoorDeltaMM8, SerializeRoundTripsThroughDeserializeWithNoLeftoverBytes) {
    IndoorLocation_MM8 ctx = makeContext();

    IndoorDelta_MM8 written = {};
    written.faceAttributes = {0};
    written.decorationFlags = {0};
    Actor_MM8 actor = {};
    actor.hp = 42;
    actor.monsterInfo.id = std::to_underlying(MONSTER_TROLL_A);
    written.actors = {actor};

    Blob blob;
    BlobOutputStream out(&blob);
    serialize(written, &out);
    out.close();

    IndoorDelta_MM8 read;
    EXPECT_NO_THROW(deserialize(blob, &read, tags::context(ctx)));

    ASSERT_EQ(read.actors.size(), 1u);
    EXPECT_EQ(read.actors[0].hp, 42);
    EXPECT_EQ(read.actors[0].monsterInfo.id, std::to_underlying(MONSTER_TROLL_A));
}
