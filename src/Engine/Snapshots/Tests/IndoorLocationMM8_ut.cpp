#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/Graphics/Indoor.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/BlobInputStream.h"

// MM8's `.blv` indoor snapshot has two extra fields vs MM7's - `BLVSector_MM8::eaxEnvironment` (right
// after `flags`) and `BLVLight_MM8::lightId` (appended at the end) - but is otherwise byte-identical (all 47
// MM8 `.blv` dungeons parse with zero leftover bytes). This builds a minimal synthetic
// fixture with that same layout (one sector, one light, everything else empty) to exercise
// `deserialize`/`reconstruct` without requiring game data.
namespace {

Blob makeIndoorLocationMM8Blob(int32_t eaxEnvironment, int32_t lightId) {
    Blob blob;
    BlobOutputStream out(&blob);

    BLVHeader_MM7 header = {};
    header.faceDataSizeBytes = 0;
    header.sectorDataSizeBytes = 0;
    header.sectorLightDataSizeBytes = 0;
    header.doorsDataSizeBytes = 0;
    serialize(header, &out);

    std::vector<Vec3s> vertices; // empty
    serialize(vertices, &out);

    std::vector<BLVFace_MM7> faces; // empty
    serialize(faces, &out);
    // faceData (presized by faceDataSizeBytes=0) & faceTextures (presized by faces.size()=0): nothing to write.

    std::vector<BLVFaceExtra_MM7> faceExtras; // empty
    serialize(faceExtras, &out);
    // faceExtraTextures (presized by faceExtras.size()=0): nothing to write.

    BLVSector_MM8 sector = {};
    sector.flags = 42;
    sector.eaxEnvironment = eaxEnvironment;
    sector.minAmbientLightLevel = 7;
    sector.firstBspNode = -1;
    std::vector<BLVSector_MM8> sectors = {sector};
    serialize(sectors, &out);
    // sectorData & sectorLightData (presized by header fields, both 0): nothing to write.

    uint32_t doorCount = 0;
    serialize(doorCount, &out);

    std::vector<LevelDecoration_MM7> decorations; // empty
    serialize(decorations, &out);
    // decorationNames (presized by decorations.size()=0): nothing to write.

    BLVLight_MM8 light = {};
    light.uBrightness = 99;
    light.lightId = lightId;
    std::vector<BLVLight_MM8> lights = {light};
    serialize(lights, &out);

    std::vector<BSPNode_MM7> bspNodes; // empty
    serialize(bspNodes, &out);

    std::vector<SpawnPoint_MM7> spawnPoints; // empty
    serialize(spawnPoints, &out);

    std::vector<BLVMapOutline_MM7> mapOutlines; // empty
    serialize(mapOutlines, &out);

    out.close();
    return blob;
}

} // namespace

UNIT_TEST(IndoorLocationMM8, DeserializeConsumesEntireStreamWithNoLeftoverBytes) {
    Blob blob = makeIndoorLocationMM8Blob(0xDEADBEEF, 0x0CAFE);

    IndoorLocation_MM8 location;
    EXPECT_NO_THROW(deserialize(blob, &location));

    ASSERT_EQ(location.sectors.size(), 1u);
    EXPECT_EQ(location.sectors[0].flags, 42);
    EXPECT_EQ(location.sectors[0].eaxEnvironment, static_cast<int32_t>(0xDEADBEEF));

    ASSERT_EQ(location.lights.size(), 1u);
    EXPECT_EQ(location.lights[0].uBrightness, 99);
    EXPECT_EQ(location.lights[0].lightId, 0x0CAFE);

    EXPECT_TRUE(location.vertices.empty());
    EXPECT_TRUE(location.faces.empty());
    EXPECT_EQ(location.doorCount, 0u);
}

UNIT_TEST(IndoorLocationMM8, ReconstructsSectorAndLightFromMM8OnlyFields) {
    Blob blob = makeIndoorLocationMM8Blob(0xDEADBEEF, 0x0CAFE);

    IndoorLocation_MM8 location;
    deserialize(blob, &location);

    IndoorLocation indoor;
    reconstruct(location, &indoor);

    // `eaxEnvironment`/`lightId` are MM8-only, unused in OE - what matters is that the surrounding fields
    // (flags, minAmbientLightLevel, firstBspNode, uBrightness) still reconstruct correctly despite them.
    ASSERT_EQ(indoor.sectors.size(), 1u);
    EXPECT_EQ(indoor.sectors[0].flags, SectorFlags(42));
    EXPECT_EQ(indoor.sectors[0].minAmbientLightLevel, 7);
    EXPECT_EQ(indoor.sectors[0].firstBspNode, -1);

    ASSERT_EQ(indoor.lights.size(), 1u);
    EXPECT_EQ(indoor.lights[0].uBrightness, 99);
}
