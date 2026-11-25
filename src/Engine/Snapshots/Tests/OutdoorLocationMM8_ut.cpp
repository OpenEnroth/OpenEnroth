#include <algorithm>
#include <cmath>
#include <limits>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/Graphics/OutdoorTerrain.h"
#include "Engine/Tables/TileTable.h"
#include "Library/Geometry/Point.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Streams/BlobInputStream.h"

// MM8's `.odm` outdoor snapshot has one extra `attributes` field (right after `tileTypes`, before
// `heightMap`) compared to `OutdoorLocation_MM7`, but is otherwise byte-identical (verified against MM8's
// `out01.odm`: 311492+2459768=2771260 bytes, all consumed). This builds a minimal synthetic fixture with that same
// layout (all vector counts zero, so no game data/textures are needed) to exercise `deserialize`/`reconstruct`.
namespace {

// `heightMapOverride`, when non-null, replaces the uniform `heightValue` fill. The slope-boundary test
// needs a non-flat tile to exercise `heightByPos`'s slope branch.
Blob makeOutdoorLocationMM8Blob(uint8_t heightValue, int16_t tileId,
                                 const std::array<uint8_t, 128 * 128> *heightMapOverride = nullptr) {
    Blob blob;
    BlobOutputStream out(&blob);

    std::array<char, 32> name = {};
    std::ranges::copy(std::string_view("blank"), name.begin());
    serialize(name, &out);

    std::array<char, 32> fileName = {};
    std::ranges::copy(std::string_view("default.odm"), fileName.begin());
    serialize(fileName, &out);

    std::array<char, 32> description = {};
    serialize(description, &out);

    std::array<char, 32> skyTexture = {};
    std::ranges::copy(std::string_view("sky"), skyTexture.begin());
    serialize(skyTexture, &out);

    std::array<char, 32> groundTilesetUnused = {};
    serialize(groundTilesetUnused, &out);

    std::array<OutdoorTileType_MM7, 4> tileTypes = {};
    for (OutdoorTileType_MM7 &tileType : tileTypes) {
        tileType.tileset = TILESET_MM7_GRASS;
        tileType.tileId = 0;
    }
    serialize(tileTypes, &out);

    uint32_t attributes = 0xDEADBEEF; // MM8-only field, unused in OE - just needs to round-trip the byte offset.
    serialize(attributes, &out);

    std::array<uint8_t, 128 * 128> heightMap = {};
    if (heightMapOverride) {
        heightMap = *heightMapOverride;
    } else {
        heightMap.fill(heightValue);
    }
    serialize(heightMap, &out);

    std::array<uint8_t, 128 * 128> tileMap = {};
    tileMap.fill(static_cast<uint8_t>(tileId));
    serialize(tileMap, &out);

    std::array<uint8_t, 128 * 128> attributeMap = {};
    serialize(attributeMap, &out);

    uint32_t normalCount = 0;
    serialize(normalCount, &out);

    std::array<uint32_t, 128 * 128 * 2> someOtherMap = {};
    serialize(someOtherMap, &out);

    std::array<uint16_t, 128 * 128 * 2> normalMap = {};
    serialize(normalMap, &out);

    // normals (presized by normalCount=0), models, decorations, decorationPidList: all empty vectors.
    uint32_t zero = 0;
    serialize(zero, &out); // models count
    serialize(zero, &out); // decorations count
    serialize(zero, &out); // decorationPidList count

    std::array<uint32_t, 128 * 128> decorationMap = {};
    serialize(decorationMap, &out);

    serialize(zero, &out); // spawnPoints count

    out.close();
    return blob;
}

} // namespace

UNIT_TEST(OutdoorLocationMM8, DeserializeConsumesEntireStreamWithNoLeftoverBytes) {
    Blob blob = makeOutdoorLocationMM8Blob(5, 0);

    OutdoorLocation_MM8 location;
    EXPECT_NO_THROW(deserialize(blob, &location));

    EXPECT_EQ(std::string_view(location.name.data()), "blank");
    EXPECT_EQ(std::string_view(location.fileName.data()), "default.odm");
    EXPECT_EQ(location.attributes, 0xDEADBEEF);
    EXPECT_EQ(location.heightMap[0], 5);
    EXPECT_EQ(location.tileMap[0], 0);
    EXPECT_TRUE(location.models.empty());
    EXPECT_TRUE(location.decorations.empty());
    EXPECT_TRUE(location.spawnPoints.empty());
}

UNIT_TEST(OutdoorLocationMM8, ReconstructsTerrainGridDimsAndSpotCheckedHeightTile) {
    Blob blob = makeOutdoorLocationMM8Blob(5, 0);

    OutdoorLocation_MM8 location;
    deserialize(blob, &location);

    TileTable tileTable;
    tileTable.addTile(TileData{.textureName = "grass00", .tileset = TILESET_GRASS, .variant = TILE_VARIANT_BASE1});
    TileTable *previousTileTable = pTileTable;
    pTileTable = &tileTable;

    OutdoorTerrain terrain;
    reconstruct(location, &terrain);

    pTileTable = previousTileTable;

    // Spot-checked height/tile value: heightMap was filled with 5 (-> 5*32 world units), tileMap with tile id 0.
    EXPECT_FLOAT_EQ(terrain.heightByGrid(Pointi(0, 0)), 5 * 32);
    EXPECT_EQ(terrain.tileIdByGrid(Pointi(0, 0)), 0);

    // Grid dims: MM8 uses the same 128x128 grid as MM7 - in bounds vs just out of bounds.
    EXPECT_FLOAT_EQ(terrain.heightByGrid(Pointi(127, 127)), 5 * 32);
    EXPECT_FLOAT_EQ(terrain.heightByGrid(Pointi(128, 128)), 0);
}

// `OutdoorTerrain::heightByPos` used to assert on a slope tile for a non-finite position. No finite
// position produces an out-of-range `lpos`/`rpos` for the tile `worldToGrid` maps it into. This builds a
// heightmap that's sloped almost everywhere, so a NaN position is overwhelmingly likely to hit a slope
// tile whatever grid cell its cast lands on.
UNIT_TEST(OutdoorLocationMM8, HeightByPosOnNonFinitePositionDoesNotCrash) {
    std::array<uint8_t, 128 * 128> heightMap = {};
    for (int y = 0; y < 128; y++)
        for (int x = 0; x < 128; x++)
            heightMap[y * 128 + x] = static_cast<uint8_t>((x * 3 + y * 5) % 251);

    Blob blob = makeOutdoorLocationMM8Blob(0, 0, &heightMap);

    OutdoorLocation_MM8 location;
    deserialize(blob, &location);

    TileTable tileTable;
    tileTable.addTile(TileData{.textureName = "grass00", .tileset = TILESET_GRASS, .variant = TILE_VARIANT_BASE1});
    TileTable *previousTileTable = pTileTable;
    pTileTable = &tileTable;

    OutdoorTerrain terrain;
    reconstruct(location, &terrain);

    pTileTable = previousTileTable;

    float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT_TRUE(std::isfinite(terrain.heightByPos(Vec3f(nan, 100, 0))));
    EXPECT_TRUE(std::isfinite(terrain.heightByPos(Vec3f(100, nan, 0))));
    EXPECT_TRUE(std::isfinite(terrain.heightByPos(Vec3f(nan, nan, 0))));

    float inf = std::numeric_limits<float>::infinity();
    EXPECT_TRUE(std::isfinite(terrain.heightByPos(Vec3f(inf, 100, 0))));
    EXPECT_TRUE(std::isfinite(terrain.heightByPos(Vec3f(100, -inf, 0))));
}
