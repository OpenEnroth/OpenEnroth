#pragma once

#include <vector>
#include <functional>
#include <tuple>

#include "LegacyImages.h"

class Blob;
class BSPModel;
struct IndoorLocation;
struct OutdoorLocation;

struct IndoorLocation_MM7 {
    BLVHeader_MM7 header;
    std::vector<Vec3s> vertices;
    std::vector<BLVFace_MM7> faces;
    std::vector<int16_t> faceData;
    std::vector<std::array<char, 10>> faceTextures;
    std::vector<BLVFaceExtra_MM7> faceExtras;
    std::vector<std::array<char, 10>> faceExtraTextures;
    std::vector<BLVSector_MM7> sectors;
    std::vector<uint16_t> sectorData;
    std::vector<uint16_t> sectorLightData;
    uint32_t doorCount;
    std::vector<LevelDecoration_MM7> decorations;
    std::vector<std::array<char, 32>> decorationNames;
    std::vector<BLVLight_MM7> lights;
    std::vector<BSPNode_MM7> bspNodes;
    std::vector<SpawnPoint_MM7> spawnPoints;
    std::vector<BLVMapOutline_MM7> mapOutlines;
};

void Deserialize(const IndoorLocation_MM7 &src, IndoorLocation *dst);
void Deserialize(const Blob &src, IndoorLocation_MM7 *dst, std::function<void()> progress);


struct IndoorSave_MM7 {
    LocationHeader_MM7 header;
    std::array<char, 875> visibleOutlines;
    std::vector<uint32_t> faceAttributes;
    std::vector<uint16_t> decorationFlags;
    std::vector<Actor_MM7> actors;
    std::vector<SpriteObject_MM7> spriteObjects;
    std::vector<Chest_MM7> chests;
    std::vector<BLVDoor_MM7> doors;
    std::vector<int16_t> doorsData;
    MapEventVariables_MM7 eventVariables;
    LocationTime_MM7 locationTime;
};

void Serialize(const IndoorLocation &src, IndoorSave_MM7 *dst);
void Deserialize(const IndoorSave_MM7 &src, IndoorLocation *dst);
void Serialize(const IndoorSave_MM7 &src, Blob *dst);
void Deserialize(const Blob &src, IndoorSave_MM7 *dst, const IndoorLocation_MM7 &ctx, std::function<void()> progress);


struct BSPModelExtras_MM7 {
    std::vector<Vec3i> vertices;
    std::vector<ODMFace_MM7> faces;
    std::vector<uint16_t> faceOrdering;
    std::vector<BSPNode_MM7> bspNodes;
    std::vector<std::array<char, 10>> faceTextures;
};

void Deserialize(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst);


struct OutdoorLocation_MM7 {
    std::array<char, 32> name;
    std::array<char, 32> fileName;
    std::array<char, 32> desciption;
    std::array<char, 32> skyTexture;
    std::array<char, 32> groundTileset;
    std::array<OutdoorLocationTileType_MM7, 4> tileTypes;
    std::array<uint8_t, 128 * 128> heightMap;
    std::array<uint8_t, 128 * 128> tileMap;
    std::array<uint8_t, 128 * 128> attributeMap;
    uint32_t normalCount;
    std::array<uint32_t , 128 * 128 * 2> someOtherMap;
    std::array<uint16_t, 128 * 128 * 2> normalMap;
    std::vector<Vec3f> normals;
    std::vector<BSPModelData_MM7> models;
    std::vector<BSPModelExtras_MM7> modelExtras;
    std::vector<LevelDecoration_MM7> decorations;
    std::vector<std::array<char, 32>> decorationNames;
    std::vector<uint16_t> decorationPidList;
    std::array<uint32_t, 128 * 128> decorationMap;
    std::vector<SpawnPoint_MM7> spawnPoints;
};

void Deserialize(const OutdoorLocation_MM7 &src, OutdoorLocation *dst);
void Deserialize(const Blob &src, OutdoorLocation_MM7 *dst, std::function<void()> progress);

struct OutdoorSave_MM7 {
    LocationHeader_MM7 header;
    std::array<std::array<uint8_t, 11>, 88> fullyRevealedCells;
    std::array<std::array<uint8_t, 11>, 88> partiallyRevealedCells;
    std::vector<uint32_t> faceAttributes;
    std::vector<uint16_t> decorationFlags;
    std::vector<Actor_MM7> actors;
    std::vector<SpriteObject_MM7> spriteObjects;
    std::vector<Chest_MM7> chests;
    MapEventVariables_MM7 eventVariables;
    LocationTime_MM7 locationTime;
};

