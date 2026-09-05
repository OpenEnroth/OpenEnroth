#pragma once

#include <array>
#include <vector>
#include <string>

#include "Engine/SpawnPoint.h"
#include "Engine/MapEnums.h"
#include "Engine/PartyPlacement.h"
#include "Engine/LocationInfo.h"

#include "Core/Time/Time.h"

#include "Library/Color/Color.h"

#include "BSPModel.h"
#include "MapWeather.h"
#include "LocationFunctions.h"
#include "OutdoorTerrain.h"

struct DecalBuilder;
struct SpellFxRenderer;
struct TileData;
struct RenderVertexSoft;
struct ODMRenderParams;

struct DMap {
    uint8_t field0 = 0;
    uint8_t field1 = 0;
};

struct OutdoorLocation {
    OutdoorLocation();
    ~OutdoorLocation();
    // int New_SKY_NIGHT_ID;
    void ExecDraw(unsigned int bRedraw);
    void PrepareActorsDrawList();
    void CreateDebugLocation();
    void Release();
    void Load(std::string_view filename, int days_played, int respawn_interval_days, bool *outdoors_was_respawned);

    int UpdateDiscoveredArea(Vec2i gridPos);
    bool IsMapCellFullyRevealed(signed int a2, signed int a3);
    bool IsMapCellPartiallyRevealed(signed int a2, signed int a3);
    bool PrepareDecorations();
    bool InitalizeActors(MapId a1);
    double GetFogDensityByTime();

    bool Initialize(std::string_view filename, int days_played,
                    int respawn_interval_days,
                    bool * outdoors_was_respawned);
    // bool Release2();

    /**
     * @offset 0x48902E
     *
     * @param partyX                    Party x position, out of bounds when foot travel is possible.
     * @param partyY                    Party y position.
     * @return                          Neighbouring map the party walks into, and the start point it arrives at.
     *                                  Map is MAP_INVALID when there is nowhere to go.
     */
    // TODO(captainurist): also sets uDefaultTravelTime_ByFoot and clears party flags as a side effect, and the travel
    //                     dialog calls it every frame just to draw the map name. Split the lookup from the commit.
    MapDestination getTravelDestination(int partyX, int partyY);
    void UpdateSunlightVectors();
    void UpdateFog();
    int getNumFoodRequiredToRestInCurrentPos(const Vec3f &pos);
    void SetFog();
    void SetUnderwaterFog();
    void Draw();

    double GetPolygonMaxZ(RenderVertexSoft *pVertex, unsigned int unumverts);
    double GetPolygonMinZ(RenderVertexSoft *pVertices, unsigned int unumverts);

    static void LoadActualSkyFrame();

    BLVFace &face(Pid pid) {
        assert(pid.type() == OBJECT_Face);
        return pBModels[pid.id() >> 6].faces[pid.id() & 0x3F];
    }

    BSPModel &model(Pid pid) {
        assert(pid.type() == OBJECT_Face);
        return pBModels[pid.id() >> 6];
    }

    std::string sky_texture_filename;
    OutdoorTerrain pTerrain;
    std::vector<BSPModel> pBModels;
    std::vector<Pid> pFaceIDLIST;
    std::array<uint32_t, 128 * 128> pOMAP;
    GraphicsImage *sky_texture = nullptr;        // signed int sSky_TextureID;
    std::vector<SpawnPoint> pSpawnPoints;
    LocationInfo ddm;
    Time lastVisitTime;
    MapWeather weather;
    std::array<std::array<uint8_t, 11>, 88> uFullyRevealedCellOnMap;
                                          // 968         the inner array is 11
                                          // bytes long, because every bit is
                                          // used for a separate cell, so in the
                                          // end it's 11 * 8 bits = 88 values
    std::array<std::array<uint8_t, 11>, 88> uPartiallyRevealedCellOnMap;  // [968]
    int max_terrain_dimming_level = 0;
    Vec3f vSunlight;
    float fFogDensity = 0;
    int uLastSunlightUpdateMinute = 0;

    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
};

extern OutdoorLocation *pOutdoor;

void ODM_UpdateUserInputAndOther();
float ODM_GetFloorLevel(const Vec3f &pos, bool *pOnWater, int *faceId);
int GetCeilingHeight(int Party_X, signed int Party_Y, int Party_ZHeight,
                     int *pFaceID);
void UpdateActors_ODM();
void ODM_ProcessPartyActions();

/**
 * @offset 0x4610AA
 */
void loadAndPrepareODM(MapId mapid, bool bLoading);
Color GetLevelFogColor();

void sub_481ED9_MessWithODMRenderParams();
