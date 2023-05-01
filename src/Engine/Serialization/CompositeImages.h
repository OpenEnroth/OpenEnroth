#pragma once

#include <vector>
#include <functional>

#include "LegacyImages.h"

class Blob;
struct IndoorLocation;

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
    std::vector<uint16_t> doorsData;
    MapEventVariables_MM7 eventVariables;
    LocationTime_MM7 locationTime;
};

void Serialize(const IndoorLocation &src, IndoorSave_MM7 *dst);
void Deserialize(const IndoorSave_MM7 &src, IndoorLocation *dst);
void Serialize(const IndoorSave_MM7 &src, Blob *dst);
void Deserialize(const Blob &src, IndoorSave_MM7 *dst, size_t doorCount, size_t doorDataCount, std::function<void()> progress);

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

