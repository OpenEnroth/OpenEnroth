#include <array>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Objects/Actor.h"
#include "Utility/Exception.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/BlobInputStream.h"

// MM8's `.ddm` outdoor delta save-state. Structural
// differences from `OutdoorDelta_MM7`: `actors` uses the wider `Actor_MM8` (972 bytes vs MM7's 836, its
// `monsterInfo` has 2-byte-wide resistances instead of MM7's 1-byte and a wider spell buffs array), and there's
// a trailing fixed-size 100-element `MapNote_MM8` block with no MM7 equivalent. Verified byte-for-byte against
// MM8's `out01.ddm` (232068 bytes, fully consumed, 0 leftover, 62 actors, 20 chests). This builds a
// minimal synthetic fixture with that same layout (one face, one decoration, one actor, no sprite objects/chests)
// to exercise `deserialize`/`reconstruct` without requiring game data.
namespace {

// `includeTrailer` models the difference between the `games.lod` template `.ddm` (no trailing dword past
// `mapNotes`) and one written by the original game's save (one trailing dword) - see `mapNotesTrailer`'s
// comment in CompositeSnapshots.h. `respawnCount`/`lastRespawnDay` are set to match: save-written files
// always have both non-zero, the template always has both zero.
Blob makeOutdoorDeltaMM8Blob(int16_t actorHp, uint16_t actorResFire, MonsterId monsterInfoId, bool includeTrailer = false) {
    Blob blob;
    BlobOutputStream out(&blob);

    LocationHeader_MM7 header = {};
    if (includeTrailer) {
        header.info.respawnCount = 1;
        header.info.lastRespawnDay = 1;
    }
    serialize(header, &out);

    std::array<std::array<uint8_t, 11>, 88> fullyRevealedCells = {};
    serialize(fullyRevealedCells, &out);

    std::array<std::array<uint8_t, 11>, 88> partiallyRevealedCells = {};
    serialize(partiallyRevealedCells, &out);

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

    PersistentVariables_MM7 eventVariables = {};
    serialize(eventVariables, &out);

    int64_t lastVisitTime = 0;
    serialize(lastVisitTime, &out);
    MapWeather_MM7 weather = {};
    serialize(weather, &out);

    std::array<MapNote_MM8, 100> mapNotes = {};
    mapNotes[0].x = 32767; // Sentinel value seen in the real out01.ddm - just needs to round-trip.
    mapNotes[0].y = 32767;
    serialize(mapNotes, &out);

    if (includeTrailer) {
        uint32_t mapNotesTrailer = 0; // Every real save examined reads this back as zero, see the struct comment.
        serialize(mapNotesTrailer, &out);
    }

    out.close();
    return blob;
}

OutdoorLocation_MM8 makeContext() {
    OutdoorLocation_MM8 location = {};

    BSPModelData_MM7 model = {};
    model.numFaces = 1;
    location.models = {model};

    LevelDecoration_MM7 decoration = {};
    location.decorations = {decoration};

    return location;
}

} // namespace

UNIT_TEST(OutdoorDeltaMM8, DeserializeConsumesEntireStreamWithNoLeftoverBytes) {
    Blob blob = makeOutdoorDeltaMM8Blob(30, 15, MONSTER_TROLL_A);
    OutdoorLocation_MM8 ctx = makeContext();

    OutdoorDelta_MM8 delta;
    EXPECT_NO_THROW(deserialize(blob, &delta, tags::context(ctx)));

    ASSERT_EQ(delta.actors.size(), 1u);
    EXPECT_EQ(delta.actors[0].hp, 30);
    EXPECT_EQ(delta.actors[0].monsterInfo.resFire, 15u);
    EXPECT_EQ(delta.actors[0].monsterInfo.id, std::to_underlying(MONSTER_TROLL_A));
    EXPECT_TRUE(delta.spriteObjects.empty());
    EXPECT_TRUE(delta.chests.empty());
    EXPECT_EQ(delta.mapNotes[0].x, 32767);
    EXPECT_EQ(delta.mapNotes[0].y, 32767);
}

UNIT_TEST(OutdoorDeltaMM8, ReconstructsActorAndNarrowsMM8OnlyWideResistance) {
    Blob blob = makeOutdoorDeltaMM8Blob(30, 15, MONSTER_TROLL_A);
    OutdoorLocation_MM8 ctx = makeContext();

    OutdoorDelta_MM8 delta;
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

// A save-written `.ddm` (`header.info.lastRespawnDay != 0`) carries one more trailing `uint32_t`
// past `mapNotes` that the `games.lod` template above doesn't. This
// should decode with zero leftover bytes too, not throw and fall back to a fresh respawn.
UNIT_TEST(OutdoorDeltaMM8, DeserializeConsumesEntireStreamWhenSaveWriterTrailerIsPresent) {
    Blob blob = makeOutdoorDeltaMM8Blob(30, 15, MONSTER_TROLL_A, /* includeTrailer */ true);
    OutdoorLocation_MM8 ctx = makeContext();

    OutdoorDelta_MM8 delta;
    EXPECT_NO_THROW(deserialize(blob, &delta, tags::context(ctx)));

    EXPECT_EQ(delta.header.info.lastRespawnDay, 1);
    ASSERT_EQ(delta.actors.size(), 1u);
    EXPECT_EQ(delta.actors[0].hp, 30);
    EXPECT_EQ(delta.mapNotesTrailer, 0u);
}

// The conditional trailer read must not turn into "swallow whatever's left" - unmodeled
// data past the one known trailing dword should still be reported, not silently discarded.
UNIT_TEST(OutdoorDeltaMM8, DeserializeStillThrowsOnDataPastTheKnownTrailer) {
    Blob blob = makeOutdoorDeltaMM8Blob(30, 15, MONSTER_TROLL_A, /* includeTrailer */ true);
    Blob junk = Blob::copy("\0\0\0\0", 4); // Extra unmodeled bytes past `mapNotesTrailer`.
    Blob blobWithJunk = Blob::concat(blob, junk);
    OutdoorLocation_MM8 ctx = makeContext();

    OutdoorDelta_MM8 delta;
    EXPECT_THROW(deserialize(blobWithJunk, &delta, tags::context(ctx)), Exception);
}

// MM8 maps used to be written with `OutdoorDelta_MM7`'s layout. The writer's output must round-trip,
// including `mapNotesTrailer`.
UNIT_TEST(OutdoorDeltaMM8, SerializeRoundTripsThroughDeserializeWithNoLeftoverBytes) {
    OutdoorLocation_MM8 ctx = makeContext();

    OutdoorDelta_MM8 written = {};
    written.header.info.respawnCount = 1;
    written.header.info.lastRespawnDay = 1;
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

    OutdoorDelta_MM8 read;
    EXPECT_NO_THROW(deserialize(blob, &read, tags::context(ctx)));

    ASSERT_EQ(read.actors.size(), 1u);
    EXPECT_EQ(read.actors[0].hp, 42);
    EXPECT_EQ(read.actors[0].monsterInfo.id, std::to_underlying(MONSTER_TROLL_A));
    EXPECT_EQ(read.mapNotesTrailer, 0u);
}
