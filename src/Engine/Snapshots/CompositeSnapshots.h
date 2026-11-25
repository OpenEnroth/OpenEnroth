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

// MM8's `.blv` indoor snapshot, verified byte-for-byte against all 47 MM8 `.blv` dungeons
// (zero leftover bytes on every one). The only structural differences from `IndoorLocation_MM7` are the two
// element types with MM8-only fields appended - `BLVSector_MM8` (an extra `eaxEnvironment` field) and
// `BLVLight_MM8` (an extra `lightId` field) - every other field has an identical binary layout, so this reuses
// the MM7 element types directly.
struct IndoorLocation_MM8 {
    BLVHeader_MM7 header;
    std::vector<Vec3s> vertices;
    std::vector<BLVFace_MM7> faces;
    std::vector<int16_t> faceData;
    std::vector<std::array<char, 10>> faceTextures;
    std::vector<BLVFaceExtra_MM7> faceExtras;
    std::vector<std::array<char, 10>> faceExtraTextures;
    std::vector<BLVSector_MM8> sectors;
    std::vector<uint16_t> sectorData;
    std::vector<uint16_t> sectorLightData;
    uint32_t doorCount;
    std::vector<LevelDecoration_MM7> decorations;
    std::vector<std::array<char, 32>> decorationNames;
    std::vector<BLVLight_MM8> lights;
    std::vector<BSPNode_MM7> bspNodes;
    std::vector<SpawnPoint_MM7> spawnPoints;
    std::vector<BLVMapOutline_MM7> mapOutlines;
};

void reconstruct(const IndoorLocation_MM8 &src, IndoorLocation *dst);
void deserialize(InputStream &src, IndoorLocation_MM8 *dst);


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
    int64_t lastVisitTime;
    MapWeather_MM7 weather; // Not used, indoor maps have no weather.
};

void snapshot(const IndoorLocation &src, IndoorDelta_MM7 *dst);
void reconstruct(const IndoorDelta_MM7 &src, IndoorLocation *dst);
void serialize(const IndoorDelta_MM7 &src, OutputStream *dst);
void deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx);

// MM8's indoor delta (`.dlv`). Verified byte-for-byte
// against all 47 MM8 `.dlv` dungeons (zero leftover bytes on every one): same structure as
// `IndoorDelta_MM7` (`header`/`visibleOutlines`/`faceAttributes`/`decorationFlags`/`doors`/`doorsData`/
// `eventVariables`/`lastVisitTime`+`weather` all byte-identical), except `actors` uses the wider `Actor_MM8` (same widening
// as `OutdoorDelta_MM8`), and there's a trailing fixed-size 100-element `MapNote_MM8` block with no MM7
// equivalent, positioned right after `lastVisitTime`+`weather` - identical in kind and position to `OutdoorDelta_MM8`'s.
struct IndoorDelta_MM8 {
    LocationHeader_MM7 header;
    std::array<char, 875> visibleOutlines;
    std::vector<uint32_t> faceAttributes;
    std::vector<uint16_t> decorationFlags;
    std::vector<Actor_MM8> actors;
    std::vector<SpriteObject_MM7> spriteObjects;
    std::vector<Chest_MM7> chests;
    std::vector<BLVDoor_MM7> doors;
    std::vector<int16_t> doorsData;
    PersistentVariables_MM7 eventVariables;
    int64_t lastVisitTime;
    MapWeather_MM7 weather;
    std::array<MapNote_MM8, 100> mapNotes; // MM8-only, not reconstructed into runtime state, see `MapNote_MM8`.
};

void snapshot(const IndoorLocation &src, IndoorDelta_MM8 *dst);
void reconstruct(const IndoorDelta_MM8 &src, IndoorLocation *dst);
void serialize(const IndoorDelta_MM8 &src, OutputStream *dst);
void deserialize(InputStream &src, IndoorDelta_MM8 *dst, ContextTag<IndoorLocation_MM8> ctx);


struct BSPModelExtras_MM7 {
    std::vector<Vec3i> vertices;
    std::vector<ODMFace_MM7> faces;
    std::vector<uint16_t> facesOrdering; // Seems to be filled either with zeros or garbage, not used in OE.
    std::vector<BSPNode_MM7> bspNodes;
    std::vector<std::array<char, 10>> faceTextures;
};

void reconstruct(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst,
                 std::vector<Vec3f> *vertices, std::vector<BLVFace> *faces);


struct OutdoorLocation_MM7 {
    std::array<char, 32> name; // Always "blank" in MM7 data.
    std::array<char, 32> fileName; // Always "default.odm" in MM7 data.
    std::array<char, 32> description; // Always "MM6 Outdoor v1.00" or "MM6 Outdoor v7.00" in MM7 data.
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

// MM8's `.odm` outdoor snapshot at the same 128x128 grid size as MM7, verified byte-for-byte against
// MM8's `out01.odm` (311492+2459768=2771260 bytes, fully consumed). The only structural difference from
// `OutdoorLocation_MM7` is one extra `attributes` field inserted between `tileTypes` and `heightMap` - every
// other field (including all the BSP model / decoration / spawn point sub-structures) has an identical binary
// layout, so this reuses the MM7 element types directly.
struct OutdoorLocation_MM8 {
    std::array<char, 32> name;
    std::array<char, 32> fileName;
    std::array<char, 32> description;
    std::array<char, 32> skyTexture;
    std::array<char, 32> groundTilesetUnused;
    std::array<OutdoorTileType_MM7, 4> tileTypes;
    uint32_t attributes; // MM8-only; meaning unknown, unused in OE.
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

void reconstruct(const OutdoorLocation_MM8 &src, OutdoorTerrain *dst);
void reconstruct(const OutdoorLocation_MM8 &src, OutdoorLocation *dst);
void deserialize(InputStream &src, OutdoorLocation_MM8 *dst);

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
    int64_t lastVisitTime;
    MapWeather_MM7 weather;
};

void snapshot(const OutdoorLocation &src, OutdoorDelta_MM7 *dst);
void reconstruct(const OutdoorDelta_MM7 &src, OutdoorLocation *dst);
void serialize(const OutdoorDelta_MM7 &src, OutputStream *dst);
void deserialize(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx);

// MM8's outdoor delta (`.ddm`) save-state. Verified
// byte-for-byte against MM8's `out01.ddm` (232068 bytes, fully consumed, 0 leftover): same structure as
// `OutdoorDelta_MM7` (`fullyRevealedCells`/`partiallyRevealedCells` at the same 88x11 size, `faceAttributes`/
// `decorationFlags` presized the same way, `eventVariables`/`lastVisitTime`+`weather` byte-identical), except `actors` uses
// the wider `Actor_MM8` (972 bytes vs MM7's 836, see `Actor_MM8`), and there's a trailing fixed-size 100-element
// `MapNote_MM8` block (see there) with no MM7 equivalent, positioned right after `lastVisitTime`+`weather`.
//
// A `.ddm` written by the original game's save carries one more trailing `uint32_t` past `mapNotes` that the
// `games.lod` template lacks. It has read as zero in every save examined. Meaning unknown. Read only if the
// stream has data left.
struct OutdoorDelta_MM8 {
    LocationHeader_MM7 header;
    std::array<std::array<uint8_t, 11>, 88> fullyRevealedCells;
    std::array<std::array<uint8_t, 11>, 88> partiallyRevealedCells;
    std::vector<uint32_t> faceAttributes;
    std::vector<uint16_t> decorationFlags;
    std::vector<Actor_MM8> actors;
    std::vector<SpriteObject_MM7> spriteObjects;
    std::vector<Chest_MM7> chests;
    PersistentVariables_MM7 eventVariables;
    int64_t lastVisitTime;
    MapWeather_MM7 weather;
    std::array<MapNote_MM8, 100> mapNotes; // MM8-only, not reconstructed into runtime state, see `MapNote_MM8`.
    uint32_t mapNotesTrailer = 0; // MM8-only, save-writer-only (see comment above), not reconstructed.
};

void snapshot(const OutdoorLocation &src, OutdoorDelta_MM8 *dst);
void reconstruct(const OutdoorDelta_MM8 &src, OutdoorLocation *dst);
void serialize(const OutdoorDelta_MM8 &src, OutputStream *dst);
void deserialize(InputStream &src, OutdoorDelta_MM8 *dst, ContextTag<OutdoorLocation_MM8> ctx);


// One Adventurer's Inn bench member (`PartyMM8Roster`), as written to the MM8-only
// `mm8party.bin` save entry. `Character_MM7` keeps the original save layout and has no
// `Character::rosterId` field, so the id is stored alongside it here.
struct MM8PartyExtraMember_MM7 {
    int32_t rosterId;
    Character_MM7 character;
};
static_assert(sizeof(MM8PartyExtraMember_MM7) == sizeof(int32_t) + sizeof(Character_MM7));
MM_DECLARE_MEMCOPY_SERIALIZABLE(MM8PartyExtraMember_MM7)

// MM8-only save extra, carrying the party state `Party_MM7`'s fixed MM7 layout has
// no room for - `Party::pCharacters`' 5th active slot and the Adventurer's Inn bench
// (`PartyMM8Roster`). Written as its own optional LOD
// entry (`mm8party.bin`, see `serialize`/`deserialize(const Blob &, SaveGame_MM7 *)`) rather than
// folded into `Party_MM7`, so that saves without the entry keep loading:
// a missing entry means "no 5th character, no bench".
struct MM8PartyExtra_MM7 {
    // `Party::pCharacters[i].rosterId` for i in [0, 5) - slot 0 (the champion) is always 0. Stored
    // for every slot, not just the active ones past `Party_MM7::characters`' first 4, so a reader
    // never has to guess which of the 5 came from which source.
    std::array<int32_t, 5> rosterIds;
    // `Party::pCharacters[4]` - `Party_MM7::characters` only has room for the first 4.
    Character_MM7 fifthCharacter;
    // `PartyMM8Roster::members()`, in enumeration order.
    std::vector<MM8PartyExtraMember_MM7> innMembers;
};

void snapshot(const Party &src, MM8PartyExtra_MM7 *dst);
void reconstruct(const MM8PartyExtra_MM7 &src, Party *dst);
void serialize(const MM8PartyExtra_MM7 &src, OutputStream *dst);
void deserialize(InputStream &src, MM8PartyExtra_MM7 *dst);


struct SaveGame_MM7 {
    SaveGameHeader_MM7 header; // In header.bin.
    Party_MM7 party; // In party.bin.
    Timer_MM7 eventTimer; // In clock.bin.
    ActiveOverlayList_MM7 overlays; // In overlay.bin.
    // 551 entries, the size of an MM8 save's npcdata.bin. Indices 528-550 are filler rows in npcdata.txt.
    // MM7's is 501, so an MM7 npcdata.bin fails to deserialize with a size error.
    std::array<NPCData_MM7, 551> npcData; // In npcdata.bin.
    std::array<uint16_t, 51> npcGroups; // In npcgroup.bin.
    std::unordered_map<std::string, Blob> mapDeltas; // Map deltas by name (e.g. "out01.ddm", "d29.dlv").
    std::unordered_map<std::pair<int, int>, Blob> lloydImages; // Lloyd's Beacon images as PCX blobs, by {playerIndex, beaconIndex}.
    Blob thumbnail; // In image.pcx - save thumbnail.
    MM8PartyExtra_MM7 mm8PartyExtra; // In mm8party.bin, MM8-only, optional - see there.
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
