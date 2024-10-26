#pragma once

#include <array>
#include <vector>
#include <string>

#include "Engine/Data/TileEnums.h"
#include "Engine/SpawnPoint.h"
#include "Engine/MapEnums.h"

#include "Media/Audio/SoundEnums.h"

#include "Library/Color/Color.h"

#include "BSPModel.h"
#include "LocationInfo.h"
#include "LocationTime.h"
#include "LocationFunctions.h"

struct DecalBuilder;
struct SpellFxRenderer;
struct TileData;
struct RenderVertexSoft;
struct ODMRenderParams;

struct OutdoorLocationTileType {
    Tileset tileset;
    uint16_t uTileID;
};

struct DMap {
    uint8_t field0;
    uint8_t field1;
};

struct OutdoorLocationTerrain {
    void FillDMap(int X, int Y, int W, int Z);
    int _47CB57(unsigned char *pixels_8bit, int a2, int num_pixels);
    bool ZeroLandscape();

    std::array<uint8_t, 128 * 128> pHeightmap{};
    std::array<uint8_t, 128 * 128> pTilemap{};
    std::array<uint8_t, 128 * 128> pAttributemap{};
    std::array<DMap, 128 * 128> pDmap{};
};

struct OutdoorLocation {
    OutdoorLocation();
    // int New_SKY_NIGHT_ID;
    void ExecDraw(unsigned int bRedraw);
    void PrepareActorsDrawList();
    void CreateDebugLocation();
    void Release();
    void Load(std::string_view filename, int days_played, int respawn_interval_days, bool *outdoors_was_respawned);
    int getTileIdByTileMapId(signed int a2);

    /**
     * @offset 0x47ED83
     */
    int getTileMapIdByGrid(int gridX, int gridY);

    /**
     * @offset 0x47EDB3
     */
    TILE_DESC_FLAGS getTileAttribByGrid(int gridX, int gridY);
    int DoGetHeightOnTerrain(signed int sX, signed int sZ);

    /**
     * @offset 0x47EE49
     */
    SoundId getSoundIdByGrid(int X_pos, int Y_pos, bool isRunning);
    int UpdateDiscoveredArea(int a2, int a3, int unused);
    bool IsMapCellFullyRevealed(signed int a2, signed int a3);
    bool IsMapCellPartiallyRevealed(signed int a2, signed int a3);
    bool PrepareDecorations();
    void ArrangeSpriteObjects();
    bool InitalizeActors(MapId a1);
    void LoadBaseTileIds();
    double GetFogDensityByTime();

    /**
     * @offset 0x488EB1
     */
    TILE_DESC_FLAGS getTileAttribByPos(int sX, int sY);

    /**
     * @offset 0x488EEF
     */
    TileData *getTileDescByPos(int sX, int sZ);

    /**
     * @offset 0x47ED08
     */
    TileData *getTileDescByGrid(int uX, int uZ);
    int GetHeightOnTerrain(int sX, int sZ);
    bool Initialize(std::string_view filename, int days_played,
                    int respawn_interval_days,
                    bool * outdoors_was_respawned);
    // bool Release2();

    /**
     * @offset 0x48902E
     */
    MapId getTravelDestination(int partyX, int partyY);
    void UpdateSunlightVectors();
    void UpdateFog();
    int getNumFoodRequiredToRestInCurrentPos(const Vec3f &pos);
    void SetFog();
    void Draw();

    double GetPolygonMaxZ(RenderVertexSoft *pVertex, unsigned int unumverts);
    double GetPolygonMinZ(RenderVertexSoft *pVertices, unsigned int unumverts);

    static void LoadActualSkyFrame();

    ODMFace &face(Pid pid) {
        assert(pid.type() == OBJECT_Face);
        return pBModels[pid.id() >> 6].pFaces[pid.id() & 0x3F];
    }

    BSPModel &model(Pid pid) {
        assert(pid.type() == OBJECT_Face);
        return pBModels[pid.id() >> 6];
    }

    std::string level_filename;
    std::string location_filename;
    std::string location_file_description;
    std::string sky_texture_filename;
    std::array<OutdoorLocationTileType, 4> pTileTypes;  // [3]  road tileset
    OutdoorLocationTerrain pTerrain;
    std::vector<BSPModel> pBModels;
    std::vector<Pid> pFaceIDLIST;
    std::array<uint32_t, 128 * 128> pOMAP;
    GraphicsImage *sky_texture = nullptr;        // signed int sSky_TextureID;
    std::vector<SpawnPoint> pSpawnPoints;
    LocationInfo ddm;
    LocationTime loc_time;
    std::array<std::array<uint8_t, 11>, 88> uFullyRevealedCellOnMap;
                                          // 968         the inner array is 11
                                          // bytes long, because every bit is
                                          // used for a separate cell, so in the
                                          // end it's 11 * 8 bits = 88 values
    std::array<std::array<uint8_t, 11>, 88> uPartiallyRevealedCellOnMap;  // [968]
    int max_terrain_dimming_level;
    Vec3f vSunlight;
    float fFogDensity;
    int uLastSunlightUpdateMinute;

    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
};

extern OutdoorLocation *pOutdoor;

void ODM_UpdateUserInputAndOther();
float ODM_GetFloorLevel(const Vec3f &pos, int unused, bool *pOnWater,
                      int *faceId, int bWaterWalk);
int GetCeilingHeight(int Party_X, signed int Party_Y, int Party_ZHeight,
                     int *pFaceID);
void ODM_GetTerrainNormalAt(float pos_x, float pos_y, Vec3f *out);
void UpdateActors_ODM();
void ODM_ProcessPartyActions();
void SetUnderwaterFog();
void sub_487DA9();

/**
 * @offset 0x4610AA
 */
void loadAndPrepareODM(MapId mapid, bool bLoading, ODMRenderParams *a2);
Color GetLevelFogColor();
int sub_47C3D7_get_fog_specular(int unused, int a2, float a3);
unsigned int WorldPosToGridCellX(int);
unsigned int WorldPosToGridCellY(int);
int GridCellToWorldPosX(int);
int GridCellToWorldPosY(int);
void sub_481ED9_MessWithODMRenderParams();
bool IsTerrainSlopeTooHigh(int pos_x, int pos_y);
int GetTerrainHeightsAroundParty2(int x, int y, bool *pIsOnWater, int bFloatAboveWater);
void TeleportToStartingPoint(MapStartPoint point);  // idb

extern MapStartPoint uLevel_StartingPointType;
