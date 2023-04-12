#pragma once

#include <vector>
#include <string>

#include <glm.hpp>

#include "Engine/Tables/TileFrameTable.h"

#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/SpawnPoint.h"
#include "Media/Audio/AudioPlayer.h"

#define DAY_ATTRIB_FOG 1

#pragma pack(push, 1)
struct ODMHeader {
    uint32_t uVersion;
    char pMagic[4];
    uint32_t uCompressedSize;
    uint32_t uDecompressedSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct OutdoorLocationTileType {
    Tileset tileset;
    uint16_t uTileID;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DMap {
    uint8_t field0;
    uint8_t field1;
};
#pragma pack(pop)

/*   79 */
#pragma pack(push, 1)
struct OutdoorLocationTerrain {
    //----- (0047C794) --------------------------------------------------------
    inline OutdoorLocationTerrain() {
        this->field_10 = 0;
        this->field_12 = 0;
    }

    void _47C7A9();
    void Release();
    void FillDMap(int X, int Y, int W, int Z);
    int _47CB57(unsigned char *pixels_8bit, int a2, int num_pixels);
    bool ZeroLandscape();


    std::array<uint8_t, 128 * 128> pHeightmap{};
    std::array<uint8_t, 128 * 128> pTilemap{};
    std::array<uint8_t, 128 * 128> pAttributemap{};
    std::array<DMap, 128 * 128> pDmap{};
    int16_t field_10 = 0;
    int16_t field_12 = 0;
    int16_t field_14 = 0;
    int16_t field_16 = 0;
    int field_18 = 0;
    int field_1C = 0;
};
#pragma pack(pop)

struct OutdoorLocation {
    OutdoorLocation();
    void subconstuctor();
    // int New_SKY_NIGHT_ID;
    void ExecDraw(unsigned int bRedraw);
    void PrepareActorsDrawList();
    void CreateDebugLocation();
    void Release();
    bool Load(const std::string &filename, int days_played,
              int respawn_interval_days, bool *outdoors_was_respawned);
    int getTileIdByTileMapId(signed int a2);

    /**
     * @offset 0x47ED83
     */
    int getTileMapIdByGrid(int gridX, int gridY);

    /**
     * @offset 0x47EDB3
     */
    int getTileAttribByGrid(int gridX, int gridY);
    int DoGetHeightOnTerrain(signed int sX, signed int sZ);

    /**
     * @offset 0x47EE49
     */
    SoundID getSoundIdByGrid(int X_pos, int Y_pos, bool isRunning);
    int UpdateDiscoveredArea(int a2, int a3, int unused);
    bool IsMapCellFullyRevealed(signed int a2, signed int a3);
    bool IsMapCellPartiallyRevealed(signed int a2, signed int a3);
    bool _47F0E2();
    bool PrepareDecorations();
    void ArrangeSpriteObjects();
    bool InitalizeActors(int a1);
    bool LoadRoadTileset();
    bool LoadTileGroupIds();
    double GetFogDensityByTime();

    /**
     * @offset 0x488EB1
     */
    int getTileAttribByPos(int sX, int sY);

    /**
     * @offset 0x488EEF
     */
    TileDesc *getTileDescByPos(int sX, int sZ);

    /**
     * @offset 0x47ED08
     */
    TileDesc *getTileDescByGrid(int uX, int uZ);
    int GetHeightOnTerrain(int sX, int sZ);
    bool Initialize(const std::string &filename, int days_played,
                    int respawn_interval_days,
                    bool * outdoors_was_respawned);
    // bool Release2();
    bool GetTravelDestination(signed int sPartyX, signed int sPartyZ,
                              char *pOut, signed int a5);
    void MessWithLUN();
    void UpdateSunlightVectors();
    void UpdateFog();
    int getNumFoodRequiredToRestInCurrentPos(const Vec3i &pos);
    void SetFog();
    void Draw();

    double GetPolygonMaxZ(struct RenderVertexSoft *pVertex, unsigned int unumverts);
    double GetPolygonMinZ(struct RenderVertexSoft *pVertices, unsigned int unumverts);

    static void LoadActualSkyFrame();

    std::string level_filename;
    std::string location_filename;
    std::string location_file_description;
    std::string sky_texture_filename;
    std::string ground_tileset;
    std::array<OutdoorLocationTileType, 4> pTileTypes;  // [3]  road tileset
    struct OutdoorLocationTerrain pTerrain;
    std::array<uint16_t, 128 * 128> pCmap; // Unused
    BSPModelList pBModels;
    std::vector<uint16_t> pFaceIDLIST;
    std::array<uint32_t, 128 * 128> pOMAP;
    Texture *sky_texture = nullptr;        // signed int sSky_TextureID;
    Texture *main_tile_texture;  // signed int sMainTile_BitmapID;
    int16_t field_F0;
    int16_t field_F2;
    int field_F4;
    char field_F8[968];
    std::vector<SpawnPoint> pSpawnPoints;
    struct DDM_DLV_Header ddm;
    LocationTime_stru1 loc_time;
    unsigned char
        uFullyRevealedCellOnMap[88][11];  // 968         the inner array is 11
                                          // bytes long, because every bit is
                                          // used for a separate cell, so in the
                                          // end it's 11 * 8 bits = 88 values
    unsigned char uPartiallyRevealedCellOnMap[88][11];  // [968]
    int field_CB8;
    int max_terrain_dimming_level;
    int field_CC0;
    unsigned int pSpriteIDs_LUN[8];
    unsigned int uSpriteID_LUNFULL;
    int field_CE8;
    unsigned int uSpriteID_LUN3_4_cp;
    int field_CF0;
    unsigned int uSpriteID_LUN1_2_cp;
    int field_CF8;
    unsigned int uSpriteID_LUN1_4_cp;
    int field_D00;
    uint16_t uSpriteID_LUN_SUN;
    int16_t field_D06;
    int field_D08;
    int field_D0C;
    int field_D10;
    int field_D14;
    int field_D24;
    int field_D28;
    int field_D2C;
    glm::vec3 vSunlight;
    unsigned int *field_D3C;
    int field_D40;
    int field_D44;
    int field_D48;
    int field_D4C;
    float field_D50;
    int field_D54;
    int field_D58;
    int field_D5C;
    int field_D60;
    int field_D64;
    char field_D68[111900];
    float fFogDensity;
    int uLastSunlightUpdateMinute;

    Logger *log = nullptr;
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renderer = nullptr;
    LightmapBuilder *lightmap_builder = nullptr;
};

extern OutdoorLocation *pOutdoor;

void ODM_UpdateUserInputAndOther();
int ODM_GetFloorLevel(const Vec3i &pos, int unused, bool *pOnWater,
                      int *bmodel_pid, int bWaterWalk);
int GetCeilingHeight(int Party_X, signed int Party_Y, int Party_ZHeight,
                     int *pFaceID);
void ODM_GetTerrainNormalAt(int pos_x, int pos_y, Vec3i *out);
void UpdateActors_ODM();
void ODM_ProcessPartyActions();
char Is_out15odm_underwater();
void SetUnderwaterFog();
void sub_487DA9();
void ODM_LoadAndInitialize(const std::string &pLevelFilename,
                           struct ODMRenderParams *thisa);
unsigned int GetLevelFogColor();
int sub_47C3D7_get_fog_specular(int unused, int a2, float a3);
unsigned int WorldPosToGridCellX(int);
unsigned int WorldPosToGridCellY(int);
int GridCellToWorldPosX(int);
int GridCellToWorldPosY(int);
void sub_481ED9_MessWithODMRenderParams();
bool IsTerrainSlopeTooHigh(int pos_x, int pos_y);
int GetTerrainHeightsAroundParty2(int x, int y, bool *pIsOnWater, int bFloatAboveWater);
