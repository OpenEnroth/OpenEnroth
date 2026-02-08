#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

#include "EntitySnapshots.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Hash.h"

/**
 * @file
 *
 * Snapshots in this header are representations of game binary files, one struct per single file.
 *
 * Struct fields are laid out in the order in which they are laid out in binary files.
 */

class Blob;
class BSPModel;
struct IndoorLocation;
struct OutdoorLocation;
class OutdoorTerrain;
struct SaveGame;
struct SaveGameLite;
struct SpriteFrameTable;

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

void reconstruct(const IndoorLocation_MM7 &src, IndoorLocation *dst);
void deserialize(InputStream &src, IndoorLocation_MM7 *dst);


struct IndoorDelta_MM7 {
    LocationHeader_MM7 header;
    std::array<char, 875> visibleOutlines;
    std::vector<uint32_t> faceAttributes;
    std::vector<uint16_t> decorationFlags;
    std::vector<Actor_MM7> actors;
    std::vector<SpriteObject_MM7> spriteObjects;
    std::vector<Chest_MM7> chests;
    std::vector<BLVDoor_MM7> doors;
    std::vector<int16_t> doorsData;
    PersistentVariables_MM7 eventVariables;
    LocationTime_MM7 locationTime;
};

void snapshot(const IndoorLocation &src, IndoorDelta_MM7 *dst);
void reconstruct(const IndoorDelta_MM7 &src, IndoorLocation *dst);
void serialize(const IndoorDelta_MM7 &src, OutputStream *dst);
void deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx);


struct BSPModelExtras_MM7 {
    std::vector<Vec3i> vertices;
    std::vector<ODMFace_MM7> faces;
    std::vector<uint16_t> faceOrdering; // Seems to be filled either with zeros or garbage, not used in OE.
    std::vector<BSPNode_MM7> bspNodes;
    std::vector<std::array<char, 10>> faceTextures;
};

void reconstruct(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst);


struct OutdoorLocation_MM7 {
    std::array<char, 32> name;
    std::array<char, 32> fileName;
    std::array<char, 32> desciption;
    std::array<char, 32> skyTexture;
    std::array<char, 32> groundTilesetUnused;
    std::array<OutdoorTileType_MM7, 4> tileTypes;
    std::array<uint8_t, 128 * 128> heightMap;
    std::array<uint8_t, 128 * 128> tileMap;
    std::array<uint8_t, 128 * 128> attributeMap;
    uint32_t normalCount; // Number of elements in `normals`.
    std::array<uint32_t , 128 * 128 * 2> someOtherMap; // Not used in OE, not even sure what this is.
    std::array<uint16_t, 128 * 128 * 2> normalMap; // Indices into `normals`, unused as we recalculate normals on load.
    std::vector<Vec3f> normals;
    std::vector<BSPModelData_MM7> models;
    std::vector<BSPModelExtras_MM7> modelExtras;
    std::vector<LevelDecoration_MM7> decorations;
    std::vector<std::array<char, 32>> decorationNames;
    std::vector<uint16_t> decorationPidList;
    std::array<uint32_t, 128 * 128> decorationMap;
    std::vector<SpawnPoint_MM7> spawnPoints;
};

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst);
void reconstruct(const OutdoorLocation_MM7 &src, OutdoorLocation *dst);
void deserialize(InputStream &src, OutdoorLocation_MM7 *dst);

struct OutdoorDelta_MM7 {
    LocationHeader_MM7 header;
    std::array<std::array<uint8_t, 11>, 88> fullyRevealedCells;
    std::array<std::array<uint8_t, 11>, 88> partiallyRevealedCells;
    std::vector<uint32_t> faceAttributes;
    std::vector<uint16_t> decorationFlags;
    std::vector<Actor_MM7> actors;
    std::vector<SpriteObject_MM7> spriteObjects;
    std::vector<Chest_MM7> chests;
    PersistentVariables_MM7 eventVariables;
    LocationTime_MM7 locationTime;
};

void snapshot(const OutdoorLocation &src, OutdoorDelta_MM7 *dst);
void reconstruct(const OutdoorDelta_MM7 &src, OutdoorLocation *dst);
void serialize(const OutdoorDelta_MM7 &src, OutputStream *dst);
void deserialize(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx);


struct SaveGame_MM7 {
    SaveGameHeader_MM7 header; // In header.bin.
    Party_MM7 party; // In party.bin.
    Timer_MM7 eventTimer; // In clock.bin.
    ActiveOverlayList_MM7 overlays; // In overlay.bin.
    std::array<NPCData_MM7, 501> npcData; // In npcdata.bin.
    std::array<uint16_t, 51> npcGroups; // In npcgroup.bin.
    std::unordered_map<std::string, Blob> mapDeltas; // Map deltas by name (e.g. "out01.ddm", "d29.dlv").
    std::unordered_map<std::pair<int, int>, Blob> lloydImages; // Lloyd's Beacon images as PCX blobs, by {playerIndex, beaconIndex}.
    Blob thumbnail; // In image.pcx - save thumbnail.
};

void snapshot(const SaveGame &src, SaveGame_MM7 *dst);
void reconstruct(const SaveGame_MM7 &src, SaveGame *dst);
void serialize(const SaveGame_MM7 &src, Blob *dst);
void deserialize(const Blob &src, SaveGame_MM7 *dst);


struct SaveGameLite_MM7 {
    SaveGameHeader_MM7 header;
    Blob thumbnail;
};

void reconstruct(const SaveGameLite_MM7 &src, SaveGameLite *dst);
void deserialize(const Blob &src, SaveGameLite_MM7 *dst);


struct SpriteFrameTable_MM7 {
    uint32_t frameCount;
    uint32_t eframeCount;
    std::vector<SpriteFrame_MM7> frames;
    std::vector<uint16_t> eframes;
};

void reconstruct(const SpriteFrameTable_MM7 &src, SpriteFrameTable *dst);
void deserialize(InputStream &src, SpriteFrameTable_MM7 *dst);
