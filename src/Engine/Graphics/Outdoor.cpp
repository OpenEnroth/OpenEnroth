#include "Engine/Graphics/Outdoor.h"

#include <algorithm>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Collisions.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIRest.h"
#include "GUI/UI/UITransition.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"
#include "Library/Logger/Logger.h"

#include "Utility/Memory/FreeDeleter.h"
#include "Utility/Math/TrigLut.h"
#include "Utility/Math/FixPoint.h"
#include "Utility/Exception.h"

// TODO(pskelton): make this neater
static DecalBuilder *decal_builder = EngineIocContainer::ResolveDecalBuilder();

MapStartPoint uLevel_StartingPointType;

OutdoorLocation *pOutdoor = new OutdoorLocation;
ODMRenderParams *pODMRenderParams;

SkyBillboardStruct SkyBillboard;  // skybox planes
std::array<struct Polygon, 2000 + 18000> array_77EC08;

struct FogProbabilityTableEntry {
    unsigned char small_fog_chance;
    unsigned char average_fog_chance;
    unsigned char dense_fog_chance;
    unsigned char unused;
};

static constexpr IndexedArray<FogProbabilityTableEntry, MAP_EMERALD_ISLAND, MAP_SHOALS> fog_probability_table = {
    {MAP_EMERALD_ISLAND,        {20, 10, 5, 0}},
    {MAP_HARMONDALE,            {20, 10, 5, 0}},
    {MAP_ERATHIA,               {20, 10, 5, 0}},
    {MAP_TULAREAN_FOREST,       {20, 10, 5, 0}},
    {MAP_DEYJA,                 {20, 10, 5, 0}},
    {MAP_BRACADA_DESERT,        {10, 5, 0, 0}},
    {MAP_CELESTE,               {0, 0, 0, 0}},
    {MAP_PIT,                   {0, 0, 0, 0}},
    {MAP_EVENMORN_ISLAND,       {20, 30, 50, 0}},
    {MAP_MOUNT_NIGHON,          {30, 20, 10, 0}},
    {MAP_BARROW_DOWNS,          {10, 5, 0, 0}},
    {MAP_LAND_OF_THE_GIANTS,    {20, 10, 5, 0}},
    {MAP_TATALIA,               {20, 10, 5, 0}},
    {MAP_AVLEE,                 {20, 10, 5, 0}},
    {MAP_SHOALS,                {0, 100, 0, 0}}
};

// for future sky textures?
static constexpr std::array<int, 9> skyTexturesIds1 = {{3, 3, 3, 3, 3, 3, 3, 3, 3}};
static constexpr std::array<int, 7> skyTexturesIds2 = {{3, 3, 3, 3, 3, 3, 3}};

//----- (0047A59E) --------------------------------------------------------
void OutdoorLocation::ExecDraw(unsigned int bRedraw) {
    /*pCamera3D->debug_flags = 0;
    if (viewparams->draw_d3d_outlines)
        pCamera3D->debug_flags |= ODM_RENDER_DRAW_D3D_OUTLINES;*/

    // if (bRedraw || true /*render->pRenderD3D*/) {
        // pODMRenderParams->RotationToInts();
        // sub_481ED9_MessWithODMRenderParams();

        // inlined

        //----- (00481ED9) --------------------------------------------------------
        // void sub_481ED9_MessWithODMRenderParams() {
            pODMRenderParams->uNumPolygons = 0;
            // pODMRenderParams->uNumEdges = 0;
            // pODMRenderParams->uNumSpans = 0;
            // pODMRenderParams->uNumSurfs = 0;
            pODMRenderParams->uNumBillboards = 0;
            // pODMRenderParams->field_44 = 0;
        //}

    //}

    pODMRenderParams->uMapGridCellX = WorldPosToGridCellX(pParty->pos.x);
    pODMRenderParams->uMapGridCellY = WorldPosToGridCellY(pParty->pos.y);

    assert(pODMRenderParams->uMapGridCellX <= 127 && pODMRenderParams->uMapGridCellY <= 127);

    // if (bRedraw) {
        // sub_487DA9(); // wipes poly array feild 108 doesnt do anything
    //}

    if (pParty->uCurrentMinute != pOutdoor->uLastSunlightUpdateMinute)
        pOutdoor->UpdateSunlightVectors();

    pOutdoor->UpdateFog();
    // pCamera3D->sr_Reset_list_0037C();

    // if (render->pRenderD3D) // d3d - redraw always
    {
        SkyBillboard.CalcSkyFrustumVec(1, 0, 0, 0, 1, 0);  // sky box frustum
        render->DrawOutdoorSky();
        render->DrawOutdoorTerrain();
        render->DrawOutdoorBuildings();

        // render->DrawBezierTerrain();
    }

    // TODO(pskelton): consider order of drawing / lighting
    pMobileLightsStack->uNumLightsActive = 0;
    pStationaryLightsStack->uNumLightsActive = 0;
    engine->StackPartyTorchLight();

    // engine->PrepareBloodsplats(); // not used?
    if (bRedraw)
        UpdateDiscoveredArea(WorldPosToGridCellX(pParty->pos.x),
                             WorldPosToGridCellY(pParty->pos.y),
                             1);
    engine->SetForceRedraw(false);

    uNumDecorationsDrawnThisFrame = 0;
    uNumSpritesDrawnThisFrame = 0;
    uNumBillboardsToDraw = 0;

    PrepareActorsDrawList();

    if (!pODMRenderParams->bDoNotRenderDecorations)
        render->PrepareDecorationsRenderList_ODM();

    render->DrawSpriteObjects();
    render->TransformBillboardsAndSetPalettesODM();

    // temp hack to show snow every third day in winter
    switch (pParty->uCurrentMonth) {
        case 11:
        case 0:
        case 1:
            pWeather->bRenderSnow = (pParty->uCurrentDayOfMonth % 3) == 0;
            break;
        default:
            pWeather->bRenderSnow = false;
            break;
    }
}

//----- (00441CFF) --------------------------------------------------------
void OutdoorLocation::Draw() {
    bool redrawWorld = true;
    if (!(pParty->uFlags & PARTY_FLAGS_1_ForceRedraw) && !engine->IsForceRedraw())
        redrawWorld = false;
    pOutdoor->ExecDraw(redrawWorld);

    engine->DrawParticles();
    // pWeather->Draw();// если раскомментировать скорость снега быстрее
    trail_particle_generator.UpdateParticles();
}

//----- (00488E23) --------------------------------------------------------
double OutdoorLocation::GetFogDensityByTime() {
    if (pParty->uCurrentHour < 5) {  // ночь
        pWeather->bNight = true;
        return 60.0 * 0.016666668;
    } else if (pParty->uCurrentHour >= 5 && pParty->uCurrentHour < 6) {  // рассвет
        pWeather->bNight = false;
        return (60.0 - (double)(60 * pParty->uCurrentHour +
                                pParty->uCurrentMinute - 300)) *
               0.016666668;
    } else if (pParty->uCurrentHour >= 6 && pParty->uCurrentHour < 20) {  // день
        pWeather->bNight = false;
        return 0.0;
    } else if (pParty->uCurrentHour >= 20 &&
               pParty->uCurrentHour < 21) {  // сумерки
        pWeather->bNight = false;
        return ((double)(pParty->uCurrentHour - 20) * 60.0 +
                (double)(signed int)pParty->uCurrentMinute) *
               0.016666668;
    } else {  // ночь
        pWeather->bNight = true;
        return 60.0 * 0.016666668;
    }
}

int OutdoorLocation::getTileAttribByPos(int sX, int sY) {
    int gridY = WorldPosToGridCellY(sY);
    int gridX = WorldPosToGridCellX(sX);

    return getTileAttribByGrid(gridX, gridY);
}

TileDesc *OutdoorLocation::getTileDescByPos(int sX, int sY) {
    int gridY = WorldPosToGridCellY(sY);
    int gridX = WorldPosToGridCellX(sX);

    return this->getTileDescByGrid(gridX, gridY);
}

//----- (00488F2E) --------------------------------------------------------
int OutdoorLocation::GetHeightOnTerrain(int sX, int sZ) {
/* Функция предоставляет возможность перемещать камеру таким образом, чтобы она
имитировала ходьбу по ландшафту. То есть нам надо менять высоту камеры
(координату Y) в зависимости от того, в каком месте ландшафта мы находимся. Для
этого мы сначала должны определить по координатам X и Z камеры квадрат ландшафта
в котором мы находимся. Все это делает функция Terrain::getHeight; в своих
параметрах она получает координаты X и Z камеры и возвращает высоту,
на которой должна быть расположена камера, чтобы она оказалась над ландшафтом.*/
    int result;  // eax@5

    if (sX < 0 || sX > 127 || sZ < 0 || sZ > 127)
        result = 0;
    else
        result = DoGetHeightOnTerrain(sX, sZ);
    return result;
}

//----- (00488F5C) --------------------------------------------------------
bool OutdoorLocation::Initialize(const std::string &filename, int days_played,
                                 int respawn_interval_days,
                                 bool *outdoors_was_respawned) {
    decal_builder->Reset(0);

    if (!filename.empty()) {
        Release();

        // pSprites_LOD->DeleteSomeOtherSprites();
        // pSpriteFrameTable->ResetLoadedFlags();

        Load(filename, days_played, respawn_interval_days, outdoors_was_respawned);

        ::day_attrib = this->loc_time.day_attrib;
        ::day_fogrange_1 = this->loc_time.day_fogrange_1;
        ::day_fogrange_2 = this->loc_time.day_fogrange_2;
        if (Is_out15odm_underwater())
            SetUnderwaterFog();

        return true;
    }

    return false;
}

static constexpr IndexedArray<std::array<MAP_TYPE, 4>, MAP_EMERALD_ISLAND, MAP_SHOALS> foot_travel_destinations = {
    // from                      north                south                east                 west
    {MAP_EMERALD_ISLAND,        {MAP_INVALID,         MAP_INVALID,         MAP_INVALID,         MAP_INVALID}},
    {MAP_HARMONDALE,            {MAP_TULAREAN_FOREST, MAP_BARROW_DOWNS,    MAP_TULAREAN_FOREST, MAP_ERATHIA}},
    {MAP_ERATHIA,               {MAP_DEYJA,           MAP_BRACADA_DESERT,  MAP_HARMONDALE,      MAP_TATALIA}},
    {MAP_TULAREAN_FOREST,       {MAP_AVLEE,           MAP_HARMONDALE,      MAP_INVALID,         MAP_DEYJA}},
    {MAP_DEYJA,                 {MAP_TULAREAN_FOREST, MAP_ERATHIA,         MAP_TULAREAN_FOREST, MAP_ERATHIA}},
    {MAP_BRACADA_DESERT,        {MAP_ERATHIA,         MAP_INVALID,         MAP_BARROW_DOWNS,    MAP_INVALID}},
    {MAP_CELESTE,               {MAP_INVALID,         MAP_INVALID,         MAP_INVALID,         MAP_INVALID}},
    {MAP_PIT,                   {MAP_INVALID,         MAP_INVALID,         MAP_INVALID,         MAP_INVALID}},
    {MAP_EVENMORN_ISLAND,       {MAP_INVALID,         MAP_INVALID,         MAP_INVALID,         MAP_INVALID}},
    {MAP_MOUNT_NIGHON,          {MAP_INVALID,         MAP_INVALID,         MAP_INVALID,         MAP_INVALID}},
    {MAP_BARROW_DOWNS,          {MAP_HARMONDALE,      MAP_BRACADA_DESERT,  MAP_HARMONDALE,      MAP_BRACADA_DESERT}},
    {MAP_LAND_OF_THE_GIANTS,    {MAP_INVALID,         MAP_INVALID,         MAP_INVALID,         MAP_INVALID}},
    {MAP_TATALIA,               {MAP_INVALID,         MAP_INVALID,         MAP_ERATHIA,         MAP_INVALID}},
    {MAP_AVLEE,                 {MAP_INVALID,         MAP_TULAREAN_FOREST, MAP_TULAREAN_FOREST, MAP_INVALID}},
    {MAP_SHOALS,                {MAP_INVALID,         MAP_INVALID,         MAP_INVALID,         MAP_INVALID}}
};

static constexpr IndexedArray<std::array<int, 4>, MAP_EMERALD_ISLAND, MAP_SHOALS> foot_travel_times = {
    // from                  north south east west
    {MAP_EMERALD_ISLAND,        {0, 0, 0, 0}},
    {MAP_HARMONDALE,            {5, 5, 7, 5}},
    {MAP_ERATHIA,               {5, 5, 5, 5}},
    {MAP_TULAREAN_FOREST,       {5, 5, 0, 5}},
    {MAP_DEYJA,                 {7, 5, 5, 4}},
    {MAP_BRACADA_DESERT,        {5, 0, 5, 0}},
    {MAP_CELESTE,               {0, 0, 0, 0}},
    {MAP_PIT,                   {0, 0, 0, 0}},
    {MAP_EVENMORN_ISLAND,       {0, 0, 0, 0}},
    {MAP_MOUNT_NIGHON,          {0, 0, 0, 0}},
    {MAP_BARROW_DOWNS,          {5, 7, 7, 5}},
    {MAP_LAND_OF_THE_GIANTS,    {0, 0, 0, 0}},
    {MAP_TATALIA,               {0, 0, 5, 0}},
    {MAP_AVLEE,                 {0, 7, 5, 0}},
    {MAP_SHOALS,                {0, 0, 0, 0}},
};

static constexpr IndexedArray<std::array<MapStartPoint, 4>, MAP_EMERALD_ISLAND, MAP_SHOALS> foot_travel_arrival_points = {
    // from                      north                south                east                 west
    {MAP_EMERALD_ISLAND,        {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party}},
    {MAP_HARMONDALE,            {MapStartPoint_South, MapStartPoint_North, MapStartPoint_South, MapStartPoint_East}},
    {MAP_ERATHIA,               {MapStartPoint_South, MapStartPoint_North, MapStartPoint_West, MapStartPoint_East}},
    {MAP_TULAREAN_FOREST,       {MapStartPoint_East, MapStartPoint_North, MapStartPoint_Party, MapStartPoint_East}},
    {MAP_DEYJA,                 {MapStartPoint_West, MapStartPoint_North, MapStartPoint_West, MapStartPoint_North}},
    {MAP_BRACADA_DESERT,        {MapStartPoint_South, MapStartPoint_Party, MapStartPoint_West, MapStartPoint_Party}},
    {MAP_CELESTE,               {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party}},
    {MAP_PIT,                   {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party}},
    {MAP_EVENMORN_ISLAND,       {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party}},
    {MAP_MOUNT_NIGHON,          {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party}},
    {MAP_BARROW_DOWNS,          {MapStartPoint_South, MapStartPoint_East, MapStartPoint_South, MapStartPoint_East}},
    {MAP_LAND_OF_THE_GIANTS,    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party}},
    {MAP_TATALIA,               {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_West, MapStartPoint_Party}},
    {MAP_AVLEE,                 {MapStartPoint_Party, MapStartPoint_North, MapStartPoint_North, MapStartPoint_Party}},
    {MAP_SHOALS,                {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party}},
};

//----- (0048902E) --------------------------------------------------------
bool OutdoorLocation::GetTravelDestination(int sPartyX, int sPartyZ, std::string *pOut) {
    signed int direction;       // esi@7
    MAP_TYPE destinationMap;  // eax@23

    std::string str = this->level_filename;
    str = str.substr(str.find_first_of("0123456789"));
    MAP_TYPE mapNumberAsInt = static_cast<MAP_TYPE>(atoi(str.c_str()));

    // TODO(captainurist): pit & celeste fall into the range below. Also, the logic here is retarded.
    if (this->level_filename.length() != 9 || mapNumberAsInt < MAP_EMERALD_ISLAND || mapNumberAsInt > MAP_SHOALS)
        return false;

    if (sPartyX < -22528)  // граница карты
        direction = 4;
    else if (sPartyX > 22528)
        direction = 3;
    else if (sPartyZ < -22528)
        direction = 2;
    else if (sPartyZ > 22528)
        direction = 1;
    else
        return false;

    if (mapNumberAsInt == MAP_AVLEE && direction == 4) {  // to Shoals
        bool wholePartyUnderwaterSuitEquipped = true;
        for (Character &player : pParty->pCharacters) {
            if (!player.hasUnderwaterSuitEquipped()) {
                wholePartyUnderwaterSuitEquipped = false;
                break;
            }
        }

        if (wholePartyUnderwaterSuitEquipped) {
            uDefaultTravelTime_ByFoot = 1;
            *pOut = "out15.odm";  // Shoals
            uLevel_StartingPointType = MapStartPoint_East;
            pParty->uFlags &= 0xFD7Bu; // ~0x0284
            return true;
        }
    } else if (mapNumberAsInt == MAP_SHOALS && direction == 3) {  // from Shoals
        uDefaultTravelTime_ByFoot = 1;
        *pOut = "out14.odm";  // Avlee
        uLevel_StartingPointType = MapStartPoint_West;
        pParty->uFlags &= 0xFD7Bu; // ~0x0284
        return true;
    }
    destinationMap = foot_travel_destinations[mapNumberAsInt][direction - 1];
    if (destinationMap == MAP_INVALID)
        return false;

    assert(destinationMap <= MAP_SHOALS);

    uDefaultTravelTime_ByFoot = foot_travel_times[mapNumberAsInt][direction - 1];
    uLevel_StartingPointType = foot_travel_arrival_points[mapNumberAsInt][direction - 1];
    *pOut = pMapStats->pInfos[destinationMap].pFilename;
    return true;
}

//----- (0048917E) --------------------------------------------------------
void OutdoorLocation::MessWithLUN() {
    this->pSpriteIDs_LUN[0] = -1;
    this->pSpriteIDs_LUN[1] = 0;
    this->pSpriteIDs_LUN[2] = pSpriteFrameTable->FastFindSprite("LUN1-4");
    this->pSpriteIDs_LUN[3] = 0;
    this->pSpriteIDs_LUN[4] = pSpriteFrameTable->FastFindSprite("LUN1-2");
    this->pSpriteIDs_LUN[5] = 0;
    this->pSpriteIDs_LUN[6] = pSpriteFrameTable->FastFindSprite("LUN3-4");
    this->pSpriteIDs_LUN[7] = 0;
    this->uSpriteID_LUNFULL = pSpriteFrameTable->FastFindSprite("LUNFULL");
    this->uSpriteID_LUN1_2_cp = pSpriteFrameTable->FastFindSprite("LUN1-2");
    this->uSpriteID_LUN1_4_cp = pSpriteFrameTable->FastFindSprite("LUN1-4");
    this->uSpriteID_LUN3_4_cp = pSpriteFrameTable->FastFindSprite("LUN3-4");
    this->field_D60 = -1;
    this->field_CF0 = 4;
    this->field_CF8 = 4;
    this->field_D00 = 4;
    this->field_CE8 = 0;
    this->field_D3C = this->pSpriteIDs_LUN;
    this->field_D40 = 0;
    this->field_D44 = 0;
    this->field_D48 = 0;
    this->field_D4C = 131072;
    this->field_D5C = 0;
    this->field_D64 = 0;
    this->field_D28 = -1;
    this->field_D08 = 0;
    this->field_D0C = 0;
    this->field_D10 = 0;
    this->field_D24 = 0;
    this->field_D2C = 0;
    this->uSpriteID_LUN_SUN = pSpriteFrameTable->FastFindSprite("LUN-SUN");
    this->field_D14 = -131072;
    for (uint i = 0; i < 8; i++)
        pSpriteFrameTable->InitializeSprite(this->pSpriteIDs_LUN[i]);  // v2 += 2;
    pSpriteFrameTable->InitializeSprite(this->uSpriteID_LUN_SUN);
}

//----- (004892E6) --------------------------------------------------------
void OutdoorLocation::UpdateSunlightVectors() {
    unsigned int minutes;  // edi@3
    double v8;        // st7@4

    if (pParty->uCurrentHour >= 5 && pParty->uCurrentHour < 21) {
        minutes = pParty->uCurrentMinute + 60 * (pParty->uCurrentHour - 5);

        this->vSunlight.x = cos((minutes * pi) / 960.0);
        this->vSunlight.y = 0;
        this->vSunlight.z = sin((minutes * pi) / 960.0);

        if (minutes >= 480)
            v8 = 960 - minutes;
        else
            v8 = minutes;
        this->max_terrain_dimming_level = (int)(20.0 - v8 / 480.0 * 20.0);
        this->uLastSunlightUpdateMinute = pParty->uCurrentMinute;
    }
}

//----- (004893C1) --------------------------------------------------------
void OutdoorLocation::UpdateFog() {
    fFogDensity = GetFogDensityByTime();
}

int OutdoorLocation::getNumFoodRequiredToRestInCurrentPos(const Vec3i &pos) {
    bool is_on_water = false;
    int bmodel_standing_on_pid = 0;
    ODM_GetFloorLevel(pos, pParty->defaultHeight, &is_on_water, &bmodel_standing_on_pid, 0);
    if (pParty->isAirborne() || bmodel_standing_on_pid || is_on_water) {
        return 2;
    }

    switch (getTileDescByPos(pos.x, pos.y)->tileset) {
        case Tileset_Grass:
            return 1;
        case Tileset_Snow:
            return 3;
        case Tileset_Desert:
            return 5;
        case Tileset_CooledLava:
        case Tileset_Dirt:
            return 4;
        case Tileset_Water:
            return 3;
        default:
            return 2;
    }
}

//----- (00489487) --------------------------------------------------------
void OutdoorLocation::SetFog() {
    pOutdoor->level_filename = pCurrentMapName;

    MAP_TYPE map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id == MAP_INVALID || map_id == MAP_CELESTE ||
        map_id == MAP_PIT || map_id > MAP_SHOALS)
        return;

    uint chance = vrng->random(100);

    if (chance < fog_probability_table[map_id].small_fog_chance) {
        ::day_fogrange_1 = 4096;
        ::day_fogrange_2 = 8192;
        ::day_attrib |= DAY_ATTRIB_FOG;
    } else if (chance <
               fog_probability_table[map_id].small_fog_chance +
                   fog_probability_table[map_id].average_fog_chance) {
        ::day_fogrange_2 = 4096;
        ::day_fogrange_1 = 0;
        ::day_attrib |= DAY_ATTRIB_FOG;
    } else if (fog_probability_table[map_id].dense_fog_chance &&
               chance <
                   fog_probability_table[map_id].small_fog_chance +
                       fog_probability_table[map_id].average_fog_chance +
                       fog_probability_table[map_id].dense_fog_chance) {
        ::day_fogrange_2 = 2048;
        ::day_fogrange_1 = 0;
        ::day_attrib |= DAY_ATTRIB_FOG;
    } else {
        ::day_attrib &= ~DAY_ATTRIB_FOG;
    }

    if (Is_out15odm_underwater()) SetUnderwaterFog();
    pOutdoor->loc_time.day_fogrange_1 = ::day_fogrange_1;
    pOutdoor->loc_time.day_fogrange_2 = ::day_fogrange_2;
    pOutdoor->loc_time.day_attrib = ::day_attrib;
}

//----- (0047C7A9) --------------------------------------------------------
void OutdoorLocationTerrain::_47C7A9() {
    this->field_10 = 0;
    this->field_12 = 0;
    this->field_16 = 0;
    this->field_14 = 0;
    this->field_1C = 0;
    this->field_18 = 0;
}

//----- (0047C7C2) --------------------------------------------------------
void OutdoorLocationTerrain::Release() {  // очистить локацию
    _47C7A9();
}

//----- (0047C80A) --------------------------------------------------------
void OutdoorLocationTerrain::FillDMap(int X, int Y, int W, int Z) {
    double v6;                    // st7@1
    double v7;                    // st7@2
    double v8;                    // st7@2
    int result;                   // eax@3
    int v10;                      // eax@4
    int v11;                      // ecx@5
    int v12;                      // ecx@6
    int v13;                      // edi@7
    int v14;                      // edx@9
                                  //  int v15; // eax@15
    uint8_t *pMapHeight;  // ebx@15
    char *v17;                      // eax@15
    int v18;                      // ecx@15
    int v19;                      // esi@15
    int v20;                      // edi@15
    int v21;                      // edx@15
    int v22;                      // ecx@15
    char *v23;                      // ebx@15
    int v24;                      // ecx@15
    int v25;                      // ST28_4@15
    double v26;                   // st7@15
    double v27;                   // st6@15
    double v28;                   // st5@15
    double v29;                   // st7@15
    double v30;                   // st7@16
    double v31;                   // st7@17
    int v32;                      // eax@21
    double v33;                   // st7@21
    double v34;                   // st6@21
    double v35;                   // st5@21
    double v36;                   // st7@21
    double v37;                   // st7@22
    double v38;                   // st7@23
    int v39;                      // [sp+14h] [bp-34h]@8
    int v40;                      // [sp+18h] [bp-30h]@15
    int v41;                      // [sp+1Ch] [bp-2Ch]@15
    int v42;                      // [sp+20h] [bp-28h]@15
    int v44;                      // [sp+28h] [bp-20h]@21
    float v45;                    // [sp+2Ch] [bp-1Ch]@1
    float v46;                    // [sp+30h] [bp-18h]@1
    float v47;                    // [sp+34h] [bp-14h]@1
    // int v48; // [sp+38h] [bp-10h]@7
    int v49;    // [sp+3Ch] [bp-Ch]@10
    int v50;    // [sp+40h] [bp-8h]@9
    float v51;  // [sp+44h] [bp-4h]@15
    float v52;  // [sp+44h] [bp-4h]@21
    float v53;  // [sp+50h] [bp+8h]@15
    float v54;  // [sp+50h] [bp+8h]@21
                //  int v55; // [sp+54h] [bp+Ch]@15
    float v56;  // [sp+54h] [bp+Ch]@15
    float v57;  // [sp+54h] [bp+Ch]@21

    v46 = -64.0;
    v47 = -64.0;
    v45 = 64.0;
    v6 = sqrt(12288.0);
    if (v6 != 0.0) {
        v7 = 1.0 / v6;
        v45 = 64.0 * v7;
        v8 = v7 * -64.0;
        v46 = v8;
        v47 = v8;
    }
    result = Y;
    if (Y > Z) {
        v10 = Z ^ Y;
        Z ^= Y ^ Z;
        result = Z ^ v10;
    }
    v11 = X;
    if (X > W) {
        v12 = W ^ X;
        W ^= X ^ W;
        v11 = W ^ v12;
    }
    // v48 = result - 1;
    if (result - 1 <= Z) {
        v39 = v11 - 1;
        for (v13 = result - 1; v13 <= Z; v13++) {
            v50 = v39;
            if (v39 <= W) {
                result = (v39 - 63) << 9;
                v49 = (v39 - 63) << 9;
                for (v14 = v39; v14 <= W; v14++) {
                    if (v13 >= 0 && result >= -32256 && v13 <= 127 &&
                        result <= 32768) {
                        // v15 = pOutLocTerrain->field_10;
                        // v55 = pOutLocTerrain->field_10;
                        pMapHeight = this->pHeightmap.data();
                        v17 = (char *)(&pMapHeight[v13 * this->field_10] + v14);
                        v18 = -v13;
                        v19 = (64 - v13) << 9;
                        v20 = 32 * *(char *)v17;
                        v21 = 32 * *(char *)(v17 + 1);

                        v22 = (v18 + 63) << 9;
                        v41 = v22;
                        v23 = (char *)(&pMapHeight[this->field_10 * (v13 + 1)] +
                                    v14);
                        v24 = v22 - v19;
                        v40 = 32 * *(char *)v23;
                        v42 = 32 * *(char *)(v23 + 1);

                        v25 = v49 - 512 - v49;
                        v26 = (double)-((v20 - v21) * v24);
                        v51 = v26;
                        v27 = (double)-(v25 * (v42 - v21));
                        v53 = v27;
                        v28 = (double)(v25 * v24);
                        v56 = v28;
                        v29 = sqrt(v28 * v28 + v27 * v27 + v26 * v26);
                        if (v29 != 0.0) {
                            v30 = 1.0 / v29;
                            v51 = v51 * v30;
                            v53 = v53 * v30;
                            v56 = v30 * v56;
                        }
                        v31 = (v56 * v47 + v53 * v46 + v51 * v45) * 31.0;
                        if (v31 < 0.0) v31 = 0.0;
                        if (v31 > 31.0) v31 = 31.0;
                        v44 = 2 * (v14 + v13 * this->field_10);
                        // pOutLocTerrain = pOutLocTerrain2;
                        *((char *)this->pDmap.data() + v44 + 1) = (int64_t)v31;

                        v32 = v49 - (v49 - 512);
                        v33 = (double)-((v42 - v40) * (v19 - v41));
                        v52 = v33;
                        v34 = (double)-(v32 * (v20 - v40));
                        v54 = v34;
                        v35 = (double)(v32 * (v19 - v41));
                        v57 = v35;
                        v36 = sqrt(v35 * v35 + v34 * v34 + v33 * v33);
                        if (v36 != 0.0) {
                            v37 = 1.0 / v36;
                            v52 = v52 * v37;
                            v54 = v54 * v37;
                            v57 = v37 * v57;
                        }
                        v38 = (v57 * v47 + v54 * v46 + v52 * v45) * 31.0;
                        if (v38 < 0.0) v38 = 0.0;
                        if (v38 > 31.0) v38 = 31.0;
                        // v13 = v48;
                        *((char *)this->pDmap.data() + v44) = (int64_t)v38;
                        // v14 = v50;
                        result = v49;
                    }
                    // ++v14;
                    result += 512;
                    // v50 = v14;
                    v49 = result;
                }
            }
            // ++v13;
            // v48 = v13;
        }
        // while ( v13 <= Z );
    }
}

//----- (0047CB57) --------------------------------------------------------
int OutdoorLocationTerrain::_47CB57(unsigned char *pixels_8bit, int a2,
                                    int num_pixels) {
    int result;  // eax@2
                        //  uint16_t *v5; // edx@3
                        //  double v6; // st7@3
                        //  int v8; // eax@3
                        //  int v9; // eax@4
                        //  int v10; // eax@5
                        //  double v11; // st6@7
                        //  signed int v12; // edi@7
                        //  int v13; // esi@9
                        //  char *v14; // esi@10
                        //  signed int v15; // ecx@10
                        //  char v16[256]; // [sp+4h] [bp-124h]@9
                        //  uint16_t *v17; // [sp+104h] [bp-24h]@3
                        //  float v22; // [sp+118h] [bp-10h]@3
                        //  float v23; // [sp+11Ch] [bp-Ch]@3
                        //  int i; // [sp+120h] [bp-8h]@3
                        //  unsigned int v25; // [sp+124h] [bp-4h]@5
                        //  signed int a2a; // [sp+134h] [bp+Ch]@3
                        //  unsigned int a2b; // [sp+134h] [bp+Ch]@7
                        //  float a3a; // [sp+138h] [bp+10h]@7
                        //  int a3b; // [sp+138h] [bp+10h]@9

    int num_r_bits = 5;
    int num_g_bits = 6;
    int num_b_bits = 5;

    int r_mask = 0xF800;
    int g_mask = 0x7E0;
    int b_mask = 0x1F;

    // if ( render->pRenderD3D )
    result = 0;
    /*else
    {
      __debugbreak();
      v5 = PaletteManager::Get_Dark_or_Red_LUT(a2, 0, 1);
      v6 = 0.0;
      v22 = 0.0;
      v8 = 0;
      v17 = v5;
      v23 = 0.0;
      a2a = 0;
      for ( i = 0; i < num_pixels; ++i )
      {
        v9 = *(char *)(v8 + pixels_8bit);
        if ( v9 )
        {
          v10 = v5[v9];
          v6 = v6 + (double)((signed int)(r_mask & v10) >> (num_b_bits +
    num_g_bits));
          ++a2a;
          v25 = b_mask & v10;
          v22 = (double)((signed int)(g_mask & v10) >> num_b_bits) + v22;
          v23 = (double)(signed int)(b_mask & v10) + v23;
        }
        v8 = i + 1;
      }
      v11 = 1.0 / (double)a2a;
      a3a = v11;
      v25 = (int64_t)(a3a * v22);
      i = (int64_t)(a3a * v23);
      v12 = 0;
      a2b = num_b_bits + num_g_bits;
      while ( 1 )
      {
        v13 = v17[v12];
        a3b = abs((int64_t)(int64_t)(v11 * v6) - ((signed int)(r_mask &
    v17[v12]) >> a2b)); BYTE3(a3b) = abs((signed)v25 - ((signed int)(g_mask &
    v13) >> num_b_bits)) + a3b; v16[v12++] = abs((signed)i - (signed)(b_mask &
    v13)) + BYTE3(a3b); if ( v12 >= 256 ) break;
      }
      result = 0;
      v14 = (char *)&pPaletteManager->field_D1600[42][23][116];
      v15 = 0;
      do
      {
        if ( (uint8_t)v16[v15] < (signed int)v14 )
        {
          v14 = (char *)(uint8_t)v16[v15];
          result = v15;
        }
        ++v15;
      }
      while ( v15 < 256 );
    }*/
    return result;
}

//----- (0047CCE2) --------------------------------------------------------
bool OutdoorLocationTerrain::ZeroLandscape() {
    this->pHeightmap.fill(0);
    this->pTilemap.fill(90);
    this->pAttributemap.fill(0);
    this->pDmap.fill({0, 0});
    this->field_12 = 128;
    this->field_10 = 128;
    this->field_16 = 7;
    this->field_14 = 7;
    this->field_1C = 127;
    this->field_18 = 127;
    return true;
}

//----- (0047CDE2) --------------------------------------------------------
void OutdoorLocation::CreateDebugLocation() {
    this->level_filename = "blank";
    this->location_filename = "i6.odm";
    this->location_file_description = "MM6 Outdoor v1.00";

    this->pTileTypes[0].tileset = Tileset_Grass;
    this->pTileTypes[1].tileset = Tileset_Water;
    this->pTileTypes[2].tileset = Tileset_Badlands;
    this->pTileTypes[3].tileset = Tileset_RoadGrassCobble;
    this->LoadTileGroupIds();
    this->LoadRoadTileset();
    this->pSpawnPoints.clear();
    this->pTerrain.ZeroLandscape();
    this->pTerrain.FillDMap(0, 0, 128, 128);

    this->pOMAP.fill(0);
    this->pFaceIDLIST.clear();
    this->sky_texture_filename = "plansky1";
    this->sky_texture = assets->getBitmap(this->sky_texture_filename);
}

//----- (0047CF9C) --------------------------------------------------------
void OutdoorLocation::Release() {
    this->level_filename = "blank";
    this->location_filename = "default.odm";
    this->location_file_description = "MM6 Outdoor v1.00";
    this->sky_texture_filename = "sky043";

    pBModels.clear();
    pSpawnPoints.clear();
    pTerrain.Release();
    pFaceIDLIST.clear();
    pTerrainNormals.clear();

    // free shader data for outdoor location
    render->ReleaseTerrain();
}

void OutdoorLocation::Load(const std::string &filename, int days_played, int respawn_interval_days, bool *outdoors_was_respawned) {
    //if (engine->IsUnderwater()) {
    //    pPaletteManager->pPalette_tintColor[0] = 0x10;
    //    pPaletteManager->pPalette_tintColor[1] = 0xC2;
    //    pPaletteManager->pPalette_tintColor[2] = 0x99;
    //    pPaletteManager->SetMistColor(37, 143, 92);
    //} else {
    //    pPaletteManager->pPalette_tintColor[0] = 0;
    //    pPaletteManager->pPalette_tintColor[1] = 0;
    //    pPaletteManager->pPalette_tintColor[2] = 0;
    //    if (pPaletteManager->pPalette_mistColor[0] != 128 ||
    //        pPaletteManager->pPalette_mistColor[1] != 128 ||
    //        pPaletteManager->pPalette_mistColor[2] != 128) {
    //        pPaletteManager->SetMistColor(128, 128, 128);
    //        //pPaletteManager->RecalculateAll();
    //    }
    //}

    if (!pGames_LOD->exists(filename))
        Error("Unable to find %s in Games.LOD", filename.c_str());

    std::string minimap_filename = filename.substr(0, filename.length() - 4);
    viewparams->location_minimap = assets->getImage_Solid(minimap_filename);

    std::string odm_filename = std::string(filename);
    odm_filename.replace(odm_filename.length() - 4, 4, ".odm");

    OutdoorLocation_MM7 location;
    deserialize(pGames_LOD->read(odm_filename), &location);
    reconstruct(location, this);

    // ****************.ddm file*********************//

    std::string ddm_filename = filename;
    ddm_filename = ddm_filename.replace(ddm_filename.length() - 4, 4, ".ddm");

    bool respawnInitial = false; // Perform initial location respawn?
    bool respawnTimed = false; // Perform timed location respawn?
    OutdoorDelta_MM7 delta;
    if (Blob blob = pSave_LOD->LoadCompressed(ddm_filename)) {
        try {
            deserialize(blob, &delta, tags::context(location));

            size_t totalFaces = 0;
            for (BSPModel &model : pBModels)
                totalFaces += model.pFaces.size();

            // Level was changed externally and we have a save there? Don't crash, just respawn.
            if (delta.header.totalFacesCount && delta.header.bmodelCount && delta.header.decorationCount &&
                (delta.header.totalFacesCount != totalFaces || delta.header.bmodelCount != pBModels.size() || delta.header.decorationCount != pLevelDecorations.size()))
                respawnInitial = true;

            // Entering the level for the 1st time?
            if (delta.header.info.lastRespawnDay == 0)
                respawnInitial = true;

            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)
                respawn_interval_days = 0x1BAF800;

            if (!respawnInitial && days_played - delta.header.info.lastRespawnDay >= respawn_interval_days)
                respawnTimed = true;
        } catch (const Exception &e) {
            logger->error("Failed to load '{}', respawning location: {}", ddm_filename, e.what());
            respawnInitial = true;
        }
    }

    assert(respawnInitial + respawnTimed <= 1);

    if (respawnInitial) {
        deserialize(pGames_LOD->read(ddm_filename), &delta, tags::context(location));
        *outdoors_was_respawned = true;
    } else if (respawnTimed) {
        auto header = delta.header;
        auto fullyRevealedCells = delta.fullyRevealedCells;
        auto partiallyRevealedCells = delta.partiallyRevealedCells;
        deserialize(pGames_LOD->read(ddm_filename), &delta, tags::context(location));
        delta.header = header;
        delta.fullyRevealedCells = fullyRevealedCells;
        delta.partiallyRevealedCells = partiallyRevealedCells;
        *outdoors_was_respawned = true;
    } else {
        *outdoors_was_respawned = false;
    }

    reconstruct(delta, this);

    if (respawnTimed || respawnInitial)
        ddm.lastRespawnDay = days_played;
    if (respawnTimed)
        ddm.respawnCount++;

    pTileTable->InitializeTileset(Tileset_Dirt);
    pTileTable->InitializeTileset(Tileset_Snow);
    pTileTable->InitializeTileset(pTileTypes[0].tileset);
    pTileTable->InitializeTileset(pTileTypes[1].tileset);
    pTileTable->InitializeTileset(pTileTypes[2].tileset);
    pTileTable->InitializeTileset(pTileTypes[3].tileset);

    // LABEL_150:
    if (pWeather->bRenderSnow) {  // Ritor1: it's include for snow
        loc_time.sky_texture_name = "sky19";
    } else if (loc_time.last_visit) {
        if (loc_time.last_visit.GetDays() % 28 != pParty->uCurrentDayOfMonth) {
            int sky_to_use;
            if (vrng->random(100) >= 20)
                sky_to_use = skyTexturesIds1[vrng->random(9)];
            else
                sky_to_use = skyTexturesIds2[vrng->random(7)];
            loc_time.sky_texture_name = fmt::format("plansky{}", sky_to_use);
        }
    } else {
        loc_time.sky_texture_name = "plansky3";
    }

    this->sky_texture = assets->getBitmap(loc_time.sky_texture_name);
}

int OutdoorLocation::getTileIdByTileMapId(int mapId) {
    int result;  // eax@2
    int v3;             // eax@3

    if (mapId >= 90) {
        v3 = (mapId - 90) / 36;
        if (v3 && v3 != 1 && v3 != 2) {
            if (v3 == Tileset_CooledLava)
                result = this->pTileTypes[3].uTileID;
            else
                result = mapId;
        } else {
            result = this->pTileTypes[v3].uTileID;
        }
    } else {
        result = 0;
    }
    return result;
}

TileDesc *OutdoorLocation::getTileDescByGrid(int sX, int sY) {
    int v3;  // esi@5
             //  unsigned int result; // eax@9

    if (sX < 0 || sX > 127 || sY < 0 || sY > 127)
        return 0;

    v3 = this->pTerrain.pTilemap[sY * 128 + sX];
    if (v3 < 198) {  // < Tileset_3
        if (v3 >= 90)
            v3 = v3 + this->pTileTypes[(v3 - 90) / 36].uTileID -
                 36 * ((v3 - 90) / 36) - 90;
    } else {
      v3 = v3 + this->pTileTypes[3].uTileID - 198;
    }

    if (engine->config->graphics.SeasonsChange.value()) {
        switch (pParty->uCurrentMonth) {
            case 11:
            case 0:
            case 1:            // winter
                if (v3 >= 90) {  // Tileset_Grass begins at TileID = 90
                    if (v3 <= 95)  // some grastyl entries
                        v3 = 348;
                    else if (v3 <= 113)  // rest of grastyl & all grdrt*
                        v3 = 348 + (v3 - 96);
                }
                /*switch (v3)
                {
                case 102: v3 = 354; break;  // grdrtNE -> SNdrtne
                case 104: v3 = 356; break;  // grdrtNW -> SNdrtnw
                case 108: v3 = 360; break;  // grdrtN  -> SNdrtn
                }*/
                break;

            case 2:
            case 3:
            case 4:  // spring
            case 8:
            case 9:
            case 10:  // autumn
                if (v3 >= 90 &&
                    v3 <= 113)  // just convert all Tileset_Grass to dirt
                    v3 = 1;
                break;

            case 5:
            case 6:
            case 7:  // summer
                // all tiles are green grass by default
                break;

            default:
                assert(pParty->uCurrentMonth >= 0 &&
                       pParty->uCurrentMonth < 12);
        }
    }

    return &pTileTable->tiles[v3];
}

int OutdoorLocation::getTileMapIdByGrid(signed int gridX, signed int gridY) {
    if (gridX < 0 || gridX > 127 || gridY < 0 || gridY > 127)
        return 0;

    return this->pTerrain.pTilemap[128 * gridY + gridX];
}

int OutdoorLocation::getTileAttribByGrid(int gridX, int gridY) {
    if (gridX < 0 || gridX > 127 || gridY < 0 || gridY > 127)
        return 0;

    int v3 = this->pTerrain.pTilemap[gridY * 128 + gridX];
    if (v3 >= 90)
        v3 = v3 + this->pTileTypes[(v3 - 90) / 36].uTileID - 36 * ((v3 - 90) / 36) - 90;
    return pTileTable->tiles[v3].uAttributes;
}

//----- (0047EE16) --------------------------------------------------------
int OutdoorLocation::DoGetHeightOnTerrain(signed int sX, signed int sZ) {
    if (sX < 0 || sX > 127 || sZ < 0 || sZ > 127)
        return 0;

    return 32 * pTerrain.pHeightmap[sZ * 128 + sX];
}

SoundID OutdoorLocation::getSoundIdByGrid(int X_pos, int Y_pos, bool isRunning) {
    if (!getTileIdByTileMapId(getTileMapIdByGrid(X_pos, Y_pos))) {
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt;
    }

    switch (getTileDescByGrid(X_pos, Y_pos)->tileset) {
        case Tileset_Grass:
            return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
        case Tileset_Snow:
            return isRunning ? SOUND_RunSnow : SOUND_WalkSnow;
        case Tileset_Desert:
            return isRunning ? SOUND_RunDesert : SOUND_WalkDesert;
        case Tileset_CooledLava:
            return isRunning ? SOUND_RunCooledLava : SOUND_WalkCooledLava;
        case Tileset_Dirt:
            // Water sounds were used
            return isRunning ? SOUND_RunDirt : SOUND_WalkDirt;
        case Tileset_Water:
            // Dirt sounds were used
            return isRunning ? SOUND_RunWater : SOUND_WalkWater;
        case Tileset_Badlands:
            return isRunning ? SOUND_RunBadlands : SOUND_WalkBadlands;
        case Tileset_Swamp:
            return isRunning ? SOUND_RunSwamp : SOUND_WalkSwamp;
        case Tileset_Tropical:
            // TODO(Nik-RE-dev): is that correct?
            return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
        case Tileset_RoadGrassCobble:
        case Tileset_RoadGrassDirt:
        case Tileset_RoadSnowCobble:
        case Tileset_RoadSnowDirt:
        case Tileset_RoadSandCobble:
        case Tileset_RoadSandDirt:
        case Tileset_RoadVolcanoCobble:
        case Tileset_RoadVolcanoDirt:
        case Tileset_RoadCrackedCobble:
        case Tileset_RoadCrackedDirt:
        case Tileset_RoadSwampCobble:
        case Tileset_RoadSwampDir:
        case Tileset_RoadTropicalCobble:
        case Tileset_RoadTropicalDirt:
            return isRunning ? SOUND_RunRoad : SOUND_WalkRoad;
        case Tileset_City:
        case Tileset_RoadCityStone:
            // TODO(Nik-RE-dev): is that correct?
        default:
            return isRunning ? SOUND_RunGround : SOUND_WalkGround;
    }
}

//----- (0047EF60) --------------------------------------------------------
int OutdoorLocation::UpdateDiscoveredArea(int X_grid_pos, int Y_grid_poa, int unused) {
    for (int i = -10; i < 10; i++) {
        int currYpos = Y_grid_poa + i - 20;
        for (int j = -10; j < 10; j++) {
            int currXpos = X_grid_pos + j - 20;
            int distanceSquared = i * i + j * j;
            if (distanceSquared <= 100 && currYpos >= 0 && currYpos <= 87 && currXpos >= 0 && currXpos <= 87) {
                unsigned char v13 = 1 << (7 - currXpos % 8);
                this->uPartiallyRevealedCellOnMap[currYpos][currXpos / 8] |= v13;
                if (distanceSquared <= 49)
                    this->uFullyRevealedCellOnMap[currYpos][currXpos / 8] |= v13;
            }
        }
    }
    return 1;
}

//----- (0047F04C) --------------------------------------------------------
bool OutdoorLocation::IsMapCellFullyRevealed(int x_pos, int y_pos) {
    if (x_pos < 0 || x_pos >= 88 || y_pos < 0 || y_pos >= 88)
        return false;
    else
        return (uFullyRevealedCellOnMap[y_pos][x_pos / 8] & (1 << (7 - (x_pos) % 8))) != 0;
}

//----- (0047F097) --------------------------------------------------------
bool OutdoorLocation::IsMapCellPartiallyRevealed(int x_pos, int y_pos) {
    if (x_pos < 0 || x_pos >= 88 || y_pos < 0 || y_pos >= 88)
        return false;
    else
        return (uPartiallyRevealedCellOnMap[y_pos][x_pos / 8] & (1 << (7 - (x_pos) % 8))) != 0;
}

//----- (0047F138) --------------------------------------------------------
bool OutdoorLocation::PrepareDecorations() {
    int v1 = 0;
    int v8 = 0;
    if (pCurrentMapName == "out09.odm") {
        v8 = 1;
    }

    decorationsWithSound.clear();
    for (uint i = 0; i < pLevelDecorations.size(); ++i) {
        LevelDecoration *decor = &pLevelDecorations[i];

        pDecorationList->InitializeDecorationSprite(decor->uDecorationDescID);
        const DecorationDesc *decoration = pDecorationList->GetDecoration(decor->uDecorationDescID);

        if (decoration->uSoundID) {
            decorationsWithSound.push_back(i);
        }

        if (v8 && decor->uCog == 20)
            decor->uFlags |= LEVEL_DECORATION_OBELISK_CHEST;
        if (!decor->uEventID) {
            if (decor->IsInteractive()) {
                if (v1 < 124) {
                    decor->eventVarId = v1;
                    if (!engine->_persistentVariables.decorVars[v1++])
                        decor->uFlags |= LEVEL_DECORATION_INVISIBLE;
                }
            }
        }
    }

    pGameLoadingUI_ProgressBar->Progress();
    return true;
}

void OutdoorLocation::ArrangeSpriteObjects() {
    if (!pSpriteObjects.empty()) {
        for (int i = 0; i < (signed int)pSpriteObjects.size(); ++i) {
            if (pSpriteObjects[i].uObjectDescID) {
                if (!(pSpriteObjects[i].uAttributes & SPRITE_DROPPED_BY_PLAYER) && !pSpriteObjects[i].IsUnpickable()) {
                    bool bOnWater = false;
                    pSpriteObjects[i].vPosition.z =
                        GetTerrainHeightsAroundParty2(
                            pSpriteObjects[i].vPosition.x,
                            pSpriteObjects[i].vPosition.y, &bOnWater, 0);
                }
                if (pSpriteObjects[i].containing_item.uItemID != ITEM_NULL) {
                    if (pSpriteObjects[i].containing_item.uItemID != ITEM_POTION_BOTTLE &&
                        pItemTable->pItems[pSpriteObjects[i].containing_item.uItemID].uEquipType == EQUIP_POTION &&
                        !pSpriteObjects[i].containing_item.uEnchantmentType)
                        pSpriteObjects[i].containing_item.uEnchantmentType = grng->random(15) + 5;
                    pItemTable->SetSpecialBonus(&pSpriteObjects[i].containing_item);
                }
            }
        }
    }
    pGameLoadingUI_ProgressBar->Progress();
}

//----- (0047F2D3) --------------------------------------------------------
bool OutdoorLocation::InitalizeActors(MAP_TYPE a1) {
    bool alert_status;  // [sp+348h] [bp-8h]@1
                       //  int v9; // [sp+34Ch] [bp-4h]@1

    alert_status = false;
    for (int i = 0; i < pActors.size(); ++i) {
        if (!(pActors[i].attributes & ACTOR_UNKNOW7)) {
            if (!alert_status) {
                pActors[i].currentActionTime = 0;
                pActors[i].currentActionLength = 0;
                if (pActors[i].attributes & ACTOR_UNKNOW11)
                    pActors[i].aiState = AIState::Disabled;
                if (pActors[i].aiState != AIState::Removed &&
                    pActors[i].aiState != AIState::Disabled &&
                    (pActors[i].currentHP == 0 ||
                     pActors[i].monsterInfo.uHP == 0))
                    pActors[i].aiState = AIState::Dead;
                pActors[i].speed.x = 0;
                pActors[i].speed.y = 0;
                pActors[i].speed.z = 0;
                pActors[i].UpdateAnimation();
                pActors[i].monsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Friendly;
                pActors[i].PrepareSprites(0);
            } else {
                pActors[i].aiState = AIState::Disabled;
                pActors[i].attributes |= ACTOR_UNKNOW11;
            }
        } else if (a1 == MAP_INVALID) {
            pActors[i].aiState = AIState::Disabled;
            pActors[i].attributes |= ACTOR_UNKNOW11;
        } else if (alert_status) {
            pActors[i].currentActionTime = 0;
            pActors[i].currentActionLength = 0;
            if (pActors[i].attributes & ACTOR_UNKNOW11)
                pActors[i].aiState = AIState::Disabled;
            if (pActors[i].aiState != AIState::Removed &&
                pActors[i].aiState != AIState::Disabled &&
                (pActors[i].currentHP == 0 ||
                 pActors[i].monsterInfo.uHP == 0))
                pActors[i].aiState = AIState::Dead;
            pActors[i].speed.x = 0;
            pActors[i].speed.y = 0;
            pActors[i].speed.z = 0;
            pActors[i].UpdateAnimation();
            pActors[i].monsterInfo.uHostilityType =
                MonsterInfo::Hostility_Friendly;
            pActors[i].PrepareSprites(0);
        } else {
            pActors[i].aiState = AIState::Disabled;
            pActors[i].attributes |= ACTOR_UNKNOW11;
            alert_status = GetAlertStatus();
        }
    }

    pGameLoadingUI_ProgressBar->Progress();
    // no use for this
    //  Actor thisa;
    //  thisa.pMonsterInfo.uID = 45;
    //  thisa.PrepareSprites(0);
    return 1;
}

//----- (0047F3EA) --------------------------------------------------------
bool OutdoorLocation::LoadRoadTileset() {
    pTileTypes[3].uTileID =
        pTileTable->GetTileForTerrainType(pTileTypes[3].tileset, 1);
    pTileTable->InitializeTileset(pTileTypes[3].tileset);
    return 1;
}

//----- (0047F420) --------------------------------------------------------
bool OutdoorLocation::LoadTileGroupIds() {
    for (uint i = 0; i < 3; ++i)
        pTileTypes[i].uTileID =
            pTileTable->GetTileForTerrainType(pTileTypes[i].tileset, 1);

    return true;
}

// TODO: move to actors?
//  combined with IndoorLocation::PrepareActorRenderList_BLV() (0043FDED) ----
//----- (0047B42C) --------------------------------------------------------
void OutdoorLocation::PrepareActorsDrawList() {
    unsigned int Angle_To_Cam;   // eax@11
    signed int Cur_Action_Time;    // eax@16
    SpriteFrame *frame;  // eax@24
    int Sprite_Octant;           // [sp+24h] [bp-3Ch]@11

    for (int i = 0; i < pActors.size(); ++i) {
        pActors[i].attributes &= ~ACTOR_VISIBLE;
        if (pActors[i].aiState == Removed || pActors[i].aiState == Disabled) {
            continue;
        }

        if (uNumBillboardsToDraw >= 500) return;

        // view culling
        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            bool onlist = false;
            for (uint j = 0; j < pBspRenderer->uNumVisibleNotEmptySectors; j++) {
                if (pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[j] == pActors[i].sectorId) {
                    onlist = true;
                    break;
                }
            }
            if (!onlist) continue;
        } else {
            if (!IsCylinderInFrustum(pActors[i].pos.toFloat(), pActors[i].radius)) continue;
        }

        int z = pActors[i].pos.z;
        int x = pActors[i].pos.x;
        int y = pActors[i].pos.y;

        Angle_To_Cam = TrigLUT.atan2(pActors[i].pos.x - pCamera3D->vCameraPos.x, pActors[i].pos.y - pCamera3D->vCameraPos.y);

        Sprite_Octant = ((signed int)(TrigLUT.uIntegerPi +
                                      ((signed int)TrigLUT.uIntegerPi >> 3) + pActors[i].yawAngle -
                                      Angle_To_Cam) >> 8) & 7;

        Cur_Action_Time = pActors[i].currentActionTime;
        if (pParty->bTurnBasedModeOn) {
            if (pActors[i].currentActionAnimation == ANIM_Walking)
                Cur_Action_Time = 32 * i + pMiscTimer->uTotalTimeElapsed;
        } else {
            if (pActors[i].currentActionAnimation == ANIM_Walking)
                Cur_Action_Time = 32 * i + pEventTimer->uTotalTimeElapsed;
        }

        if (pActors[i].buffs[ACTOR_BUFF_STONED].Active() ||
            pActors[i].buffs[ACTOR_BUFF_PARALYZED].Active())
            Cur_Action_Time = 0;

        int v49 = 0;
        float v4 = 0.0f;
        if (pActors[i].aiState == Summoned) {
            if (PID_TYPE(pActors[i].summonerId) != OBJECT_Actor ||
                pActors[PID_ID(pActors[i].summonerId)]
                .monsterInfo.uSpecialAbilityDamageDiceSides != 1) {
                z += floorf(pActors[i].height * 0.5f + 0.5f);
            } else {
                v49 = 1;
                spell_fx_renderer->_4A7F74(pActors[i].pos.x, pActors[i].pos.y, z);
                v4 = (1.0 - (double)pActors[i].currentActionTime /
                            (double)pActors[i].currentActionLength) *
                     (double)(2 * pActors[i].height);
                z -= floorf(v4 + 0.5f);
                if (z > pActors[i].pos.z) z = pActors[i].pos.z;
            }
        }


        if (pActors[i].aiState == Summoned && !v49)
            frame = pSpriteFrameTable->GetFrame(uSpriteID_Spell11, Cur_Action_Time);
        else if (pActors[i].aiState == Resurrected)
            frame = pSpriteFrameTable->GetFrameBy_x(
                pActors[i].spriteIds[pActors[i].currentActionAnimation], Cur_Action_Time);
        else
            frame = pSpriteFrameTable->GetFrame(
                pActors[i].spriteIds[pActors[i].currentActionAnimation], Cur_Action_Time);

        // no sprite frame to draw
        if (frame->icon_name == "null") continue;
        if (frame->hw_sprites[Sprite_Octant]->texture->height() == 0 || frame->hw_sprites[Sprite_Octant]->texture->width() == 0)
            __debugbreak();

        int flags = 0;
        // v16 = (int *)frame->uFlags;
        if (frame->uFlags & 2) flags = 2;
        if (frame->uFlags & 0x40000) flags |= 0x40;
        if (frame->uFlags & 0x20000) flags |= 0x80;
        if ((256 << Sprite_Octant) & frame->uFlags) flags |= 4;
        if (frame->uGlowRadius) {
            pMobileLightsStack->AddLight(Vec3f(x, y, z), pActors[i].sectorId, frame->uGlowRadius, colorTable.White,
                                         _4E94D3_light_type);
        }

        int view_x = 0, view_y = 0, view_z = 0;
        bool visible = pCamera3D->ViewClip(x, y, z, &view_x, &view_y, &view_z);

        if (visible) {
            if (2 * std::abs(view_x) >= std::abs(view_y)) {
                int projected_x = 0;
                int projected_y = 0;
                pCamera3D->Project(view_x, view_y, view_z, &projected_x, &projected_y);

                float proj_scale = frame->scale * (pCamera3D->ViewPlaneDist_X) / (view_x);
                int screen_space_half_width = static_cast<int>(proj_scale * frame->hw_sprites[Sprite_Octant]->uWidth / 2.0f);
                int screen_space_height = static_cast<int>(proj_scale * frame->hw_sprites[Sprite_Octant]->uHeight);

                if (projected_x + screen_space_half_width >= (signed int)pViewport->uViewportTL_X &&
                    projected_x - screen_space_half_width <= (signed int)pViewport->uViewportBR_X) {
                    if (projected_y >= pViewport->uViewportTL_Y && (projected_y - screen_space_height) <= pViewport->uViewportBR_Y) { // test
                        ++uNumBillboardsToDraw;
                        ++uNumSpritesDrawnThisFrame;

                        pActors[i].attributes |= ACTOR_VISIBLE;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].hwsprite = frame->hw_sprites[Sprite_Octant];
                        pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID = pActors[i].sectorId;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].uPaletteIndex = frame->GetPaletteIndex();

                        pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x = proj_scale;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = proj_scale;

                        if (pActors[i].buffs[ACTOR_BUFF_SHRINK].Active() &&
                            pActors[i].buffs[ACTOR_BUFF_SHRINK].power > 0) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1]
                                .screenspace_projection_factor_y = 1.0f / pActors[i].buffs[ACTOR_BUFF_SHRINK].power *
                                                                   pBillboardRenderList[uNumBillboardsToDraw - 1]
                                .screenspace_projection_factor_y;
                        } else if (pActors[i].buffs[ACTOR_BUFF_MASS_DISTORTION].Active()) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y =
                                spell_fx_renderer->_4A806F_get_mass_distortion_value(&pActors[i]) *
                                pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y;
                        }

                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_x = projected_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_y = projected_y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_z = view_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_x = x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_y = y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_z = z;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].dimming_level = 0;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].object_pid = PID(OBJECT_Actor, i);
                        pBillboardRenderList[uNumBillboardsToDraw - 1].field_14_actor_id = i;

                        pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = flags | 0x200;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = frame;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor =
                            pMonsterList->pMonsters[pActors[i].monsterInfo.uID - 1].sTintColor;  // *((int *)&v35[v36] - 36);
                        if (pActors[i].buffs[ACTOR_BUFF_STONED].Active()) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E =
                                flags | 0x100;
                        }
                    }
                }
            }
        }
    }
}

int ODM_GetFloorLevel(const Vec3i &pos, int unused, bool *pIsOnWater,
                      int *faceId, int bWaterWalk) {
    std::array<int, 20> current_Face_id{};                   // dword_721110
    std::array<int, 20> current_BModel_id{};                 // dword_721160
    std::array<int, 20> odm_floor_level{};                   // idb
    current_BModel_id[0] = -1;
    current_Face_id[0] = -1;
    odm_floor_level[0] = GetTerrainHeightsAroundParty2(pos.x, pos.y, pIsOnWater, bWaterWalk);

    int surface_count = 1;

    for (BSPModel &model : pOutdoor->pBModels) {
        if (!model.pBoundingBox.containsXY(pos.x, pos.y))
            continue;

        if (model.pFaces.empty())
            continue;

        for (ODMFace &face : model.pFaces) {
            if (face.Ethereal())
                continue;

            if (face.uNumVertices == 0)
                continue;

            if (face.uPolygonType != POLYGON_Floor && face.uPolygonType != POLYGON_InBetweenFloorAndWall)
                continue;

            if (!face.pBoundingBox.containsXY(pos.x, pos.y))
                continue;

            int slack = engine->config->gameplay.FloorChecksEps.value();
            if (!face.Contains(pos, model.index, slack, FACE_XY_PLANE))
                continue;

            int floor_level;
            if (face.uPolygonType == POLYGON_Floor) {
                floor_level = model.pVertices[face.pVertexIDs[0]].z;
            } else {
                floor_level = face.zCalc.calculate(pos.x, pos.y);
            }
            odm_floor_level[surface_count] = floor_level;
            current_BModel_id[surface_count] = model.index;
            current_Face_id[surface_count] = face.index;
            surface_count++;

            if (surface_count >= 20)
                break;
        }
    }

    if (surface_count == 1) {
        *faceId = 0;
        return odm_floor_level[0]; // No bmodels, just the terrain.
    }

    int current_floor_level = odm_floor_level[0];
    int current_idx = 0;
    for (uint i = 1; i < surface_count; ++i) {
        if (current_floor_level <= pos.z + 5) {
            if (odm_floor_level[i] >= current_floor_level && odm_floor_level[i] <= pos.z + 5) {
                current_floor_level = odm_floor_level[i];
                current_idx = i;
            }
        } else if (odm_floor_level[i] < current_floor_level) {
            current_floor_level = odm_floor_level[i];
            current_idx = i;
        }
    }
    if (!current_idx)
        *faceId = 0;
    else
        *faceId = current_Face_id[current_idx] | (current_BModel_id[current_idx] << 6);

    if (current_idx) {
        *pIsOnWater = false;
        if (pOutdoor->pBModels[current_BModel_id[current_idx]].pFaces[current_Face_id[current_idx]].Fluid())
            *pIsOnWater = true;
    }

    return std::max(odm_floor_level[0], odm_floor_level[current_idx]);
}

// not sure if right- or left-handed coordinate space assumed, so this could be
// normal of inverse normal
// for a right-handed system, that would be an inverse normal
//----- (0046DCC8) --------------------------------------------------------
void ODM_GetTerrainNormalAt(int pos_x, int pos_y, Vec3i *out) {
    uint grid_x = WorldPosToGridCellX(pos_x);
    uint grid_y = WorldPosToGridCellY(pos_y);

    int grid_pos_x1 = GridCellToWorldPosX(grid_x);
    int grid_pos_x2 = GridCellToWorldPosX(grid_x + 1);
    int grid_pos_y1 = GridCellToWorldPosY(grid_y);
    int grid_pos_y2 = GridCellToWorldPosY(grid_y + 1);

    int x1y1_z = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_y);
    int x2y1_z = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_y);
    int x2y2_z = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_y + 1);
    int x1y2_z = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_y + 1);

    Vec3f side1, side2;

    //float side1_dx, side1_dz, side1_dy, side2_dx, side2_dz, side2_dy;

    int dx = std::abs(pos_x - grid_pos_x1);
    int dy = std::abs(grid_pos_y1 - pos_y);
    if (dy >= dx) {
        side2 = Vec3f(grid_pos_x2 - grid_pos_x1, 0.0f, x2y2_z - x1y2_z);
        side1 = Vec3f(0.0f, grid_pos_y1 - grid_pos_y2, x1y1_z - x1y2_z);
        /*       |\
           side1 |  \
                 |____\
                 side 2      */
    } else {
        side2 = Vec3f(grid_pos_x1 - grid_pos_x2, 0.0f, x1y1_z - x2y1_z);
        side1 = Vec3f(0.0f, grid_pos_y2 - grid_pos_y1, x2y2_z - x2y1_z);
        /*   side 2
             _____
             \    |
               \  | side 1
                 \|       */
    }

    Vec3f n = cross(side2, side1);
    float mag = n.length();
    if (fabsf(mag) < 1e-6f) {
        *out = Vec3i(0, 0, 65536);
    } else {
        *out = (n / mag).toFixpoint();
    }
}
//----- (0046BE0A) --------------------------------------------------------
void ODM_UpdateUserInputAndOther() {
    bool v0;        // eax@5
    std::string pOut;  // [sp+8h] [bp-20h]@5

    UpdateObjects();
    ODM_ProcessPartyActions();
    if (pParty->pos.x < -22528 || pParty->pos.x > 22528 ||
        pParty->pos.y < -22528 || pParty->pos.y > 22528) {
        pOutdoor->level_filename = pCurrentMapName;
        v0 = pOutdoor->GetTravelDestination(pParty->pos.x, pParty->pos.y, &pOut);
        if (!engine->IsUnderwater() && (pParty->isAirborne() || (pParty->uFlags & (PARTY_FLAGS_1_STANDING_ON_WATER | PARTY_FLAGS_1_WATER_DAMAGE)) ||
                             pParty->uFlags & PARTY_FLAGS_1_BURNING || pParty->bFlying) ||
            !v0) {
            if (pParty->pos.x < -22528) pParty->pos.x = -22528;
            if (pParty->pos.x > 22528) pParty->pos.x = 22528;
            if (pParty->pos.y < -22528) pParty->pos.y = -22528;
            if (pParty->pos.y > 22528) pParty->pos.y = 22528;
        } else {
            pDialogueWindow = new GUIWindow_Travel();  // TravelUI_Load();
        }
    }
    UpdateActors_ODM();
    checkDecorationEvents();
}
//----- (0041F54A) --------------------------------------------------------
void OutdoorLocation::LoadActualSkyFrame() {
    if (rest_ui_sky_frame_current) {
        rest_ui_sky_frame_current->Release();
        rest_ui_sky_frame_current = nullptr;
    }
    if (rest_ui_hourglass_frame_current) {
        rest_ui_hourglass_frame_current->Release();
        rest_ui_hourglass_frame_current = nullptr;
    }

    rest_ui_sky_frame_current = assets->getImage_ColorKey(
        fmt::format("TERRA{:03}", pParty->uCurrentMinute / 6 + 10 * pParty->uCurrentHour));
}

OutdoorLocation::OutdoorLocation() {
    this->log = EngineIocContainer::ResolveLogger();
    this->decal_builder = EngineIocContainer::ResolveDecalBuilder();
    this->spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();

    this->sky_texture = nullptr;

    subconstuctor();
    uLastSunlightUpdateMinute = 0;
}

void OutdoorLocation::subconstuctor() {
    // OutdoorLocationTerrain::OutdoorLocationTerrain(&this->pTerrain);
    field_F0 = 0;
    field_F4 = 0x40000000u;
    // DLVHeader::DLVHeader(&v1->ddm);
}

// TODO(pskelton): Magic numbers
// TODO(pskelton): check pointer maths for updating fly and waterwalk overlays
// TODO(pskelton): Split function up
// TODO(pskelton): Pass party as param
//----- (00473893) --------------------------------------------------------
void ODM_ProcessPartyActions() {
    bool waterWalkActive = false;
    pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
    if (pParty->WaterWalkActive()) {
        waterWalkActive = true;
        engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].overlayID + 119] |= 1;
        if (!pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff &&
            pParty->pCharacters[pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].caster - 1].mana <= 0)
            waterWalkActive = false;
    }

    int floorFaceId = 0;
    bool partyIsOnWater = false;

    int floorZ = ODM_GetFloorLevel(pParty->pos, pParty->height,
                                   &partyIsOnWater, &floorFaceId, waterWalkActive);
    int partyNotOnModel = floorFaceId == 0;
    int currentGroundLevel = floorZ + 1;

    bool partyHasFeatherFall = pParty->FeatherFallActive() || pParty->wearsItemAnywhere(ITEM_ARTIFACT_LADYS_ESCORT)
                                    || pParty->uFlags & (PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);
    if (partyHasFeatherFall)
        pParty->uFallStartZ = floorZ;
    else
        floorZ = pParty->uFallStartZ;

    // face id of any model ceiling face above party
    int ceilingFaceID = 0;
    // height of any model ceiling above party
    int ceilingHeight = -1;

    if (pParty->bFlying)
        ceilingHeight = GetCeilingHeight(pParty->pos.x, pParty->pos.y, pParty->pos.z + pParty->height, &ceilingFaceID);

    // is the party in the air - NB not accurate when on slopes of models
    bool partyNotTouchingFloor = false;
    if (pParty->pos.z <= currentGroundLevel) {  // landing from flight
        ceilingHeight = -1;
        pParty->bFlying = false;
        pParty->uFlags &= ~(PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);
    } else {
        partyNotTouchingFloor = true;
    }

    bool partyCloseToGround = pParty->pos.z - currentGroundLevel <= 32;

    // check if we should be flying
    if (!engine->IsUnderwater() && !pParty->FlyActive()) {
        pParty->bFlying = false;
        pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
    }

     // is party standing on any trigger faces
    int triggerID{ 0 };
    if (!partyNotTouchingFloor) {
        if (pParty->floor_face_id != floorFaceId && floorFaceId) {
            int BModel_id = floorFaceId >> 6;
            if (BModel_id < pOutdoor->pBModels.size()) {
                int face_id = floorFaceId & 0x3F;
                if (pOutdoor->pBModels[BModel_id].pFaces[face_id].uAttributes & FACE_PRESSURE_PLATE) {
                    triggerID = pOutdoor->pBModels[BModel_id].pFaces[face_id].sCogTriggeredID;
                }
            }
        }
        pParty->floor_face_id = floorFaceId;
    }

    // set params before input
    int partyInputYSpeed = 0;
    int partyInputXSpeed = 0;
    int partyInputZSpeed = pParty->speed.z;
    if (pParty->bFlying) {
        partyInputZSpeed = 0;
    }
    int partyOldFlightZ = pParty->sPartySavedFlightZ;

    bool partyAtHighSlope = IsTerrainSlopeTooHigh(pParty->pos.x, pParty->pos.y);
    bool partyIsRunning = false;
    bool partyIsWalking = false;
    bool noFlightBob = false;

    int partyViewNewYaw = pParty->_viewYaw;
    int partyViewNewPitch = pParty->_viewPitch;

    bool flyDown{ false };

    int64_t dturn = ((int64_t) pEventTimer->dt_fixpoint * pParty->_yawRotationSpeed * TrigLUT.uIntegerPi / 180) >> 16;
    while (pPartyActionQueue->uNumActions) {
        switch (pPartyActionQueue->Next()) {
            case PARTY_FlyUp:
            {
                if (!pParty->FlyActive() && !engine->IsUnderwater()) break;

                pParty->bFlying = false;
                if (engine->IsUnderwater() ||
                    pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff ||
                    (pParty->pCharacters[pParty->pPartyBuffs[PARTY_BUFF_FLY].caster - 1].mana > 0 || engine->config->debug.AllMagic.value())) {
                    if (pParty->sPartySavedFlightZ < engine->config->gameplay.MaxFlightHeight.value() || partyNotTouchingFloor) {
                        pParty->bFlying = true;
                        pParty->speed.z = 0;
                        noFlightBob = true;
                        pParty->uFlags &= ~(PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);
                        if (pParty->sPartySavedFlightZ < engine->config->gameplay.MaxFlightHeight.value()) {
                            partyInputZSpeed = pParty->walkSpeed * 4;
                            partyOldFlightZ = pParty->pos.z;
                        }
                    }
                }
            } break;

            case PARTY_FlyDown:
                // Fly down behaviour is now handled below with landing
                flyDown = true;
                break;

            case PARTY_TurnLeft:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    partyViewNewYaw += engine->config->settings.TurnSpeed.value();  // discrete turn
                else
                    partyViewNewYaw += dturn * fTurnSpeedMultiplier;  // time-based smooth turn

                partyViewNewYaw &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_TurnRight:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    partyViewNewYaw -= engine->config->settings.TurnSpeed.value();
                else
                    partyViewNewYaw -= dturn * fTurnSpeedMultiplier;

                partyViewNewYaw &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_FastTurnLeft:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    partyViewNewYaw += engine->config->settings.TurnSpeed.value();
                else
                    partyViewNewYaw += 2.0f * fTurnSpeedMultiplier * dturn;

                partyViewNewYaw &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_FastTurnRight:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    partyViewNewYaw -= engine->config->settings.TurnSpeed.value();
                else
                    partyViewNewYaw -= 2.0f * fTurnSpeedMultiplier * dturn;

                partyViewNewYaw &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_StrafeLeft:
            {
                float sin_y = sinf(2 * pi_double * partyViewNewYaw / 2048.0);
                int dx = sin_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                partyInputXSpeed -= 3 * dx / 4;

                float cos_y = cosf(2 * pi_double * partyViewNewYaw / 2048.0);
                int dy = cos_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                partyInputYSpeed += 3 * dy / 4;

                partyIsWalking = true;
            } break;

            case PARTY_StrafeRight:
            {
                float sin_y = sinf(2 * pi_double * partyViewNewYaw / 2048.0);
                int dx = sin_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                partyInputXSpeed += 3 * dx / 4;

                float cos_y = cosf(2 * pi_double * partyViewNewYaw / 2048.0);
                int dy = cos_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                partyInputYSpeed -= 3 * dy / 4;

                partyIsWalking = true;
            } break;

            case PARTY_WalkForward:
            {
                float sin_y = sinf(2 * pi_double * partyViewNewYaw / 2048.0),
                      cos_y = cosf(2 * pi_double * partyViewNewYaw / 2048.0);

                int dx = cos_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                int dy = sin_y * pParty->walkSpeed * fWalkSpeedMultiplier;

                if (engine->config->debug.TurboSpeed.value()) {
                    partyInputXSpeed += dx * 12;
                    partyInputYSpeed += dy * 12;
                } else {
                    partyInputXSpeed += dx;
                    partyInputYSpeed += dy;
                }

                partyIsWalking = true;
            } break;

            case PARTY_RunForward:
            {
                float sin_y = sinf(2 * pi_double * partyViewNewYaw / 2048.0);
                float cos_y = cosf(2 * pi_double * partyViewNewYaw / 2048.0);

                int dx = cos_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                int dy = sin_y * pParty->walkSpeed * fWalkSpeedMultiplier;

                if (pParty->bFlying) {
                    if (engine->config->debug.TurboSpeed.value()) {
                        partyInputXSpeed += dx * 24;
                        partyInputYSpeed += dy * 24;
                    } else {
                        partyInputXSpeed += 4 * dx;
                        partyInputYSpeed += 4 * dy;
                    }
                } else if (partyAtHighSlope && !floorFaceId) {
                    partyInputXSpeed += dx;
                    partyInputYSpeed += dy;
                    partyIsWalking = true;
                } else {
                    if (engine->config->debug.TurboSpeed.value()) {
                        partyInputXSpeed += dx * 12;
                        partyInputYSpeed += dy * 12;
                    } else {
                        partyInputXSpeed += 2 * dx;
                        partyInputYSpeed += 2 * dy;
                    }

                    partyIsRunning = true;
                }
            } break;

            case PARTY_WalkBackward: {
                float sin_y = sinf(2 * pi_double * partyViewNewYaw / 2048.0);
                float cos_y = cosf(2 * pi_double * partyViewNewYaw / 2048.0);

                int dx = cos_y * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                partyInputXSpeed -= dx;

                int dy = sin_y * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                partyInputYSpeed -= dy;
                partyIsWalking = true;
            } break;

            case PARTY_RunBackward:
            {
                float sin_y = sinf(2 * pi_double * partyViewNewYaw / 2048.0);
                float cos_y = cosf(2 * pi_double * partyViewNewYaw / 2048.0);

                int dx = cos_y * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                int dy = sin_y * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;

                if (pParty->bFlying) {
                    partyInputXSpeed -= 4 * dx;
                    partyInputYSpeed -= 4 * dy;
                } else {
                    partyInputXSpeed -= dx;
                    partyInputYSpeed -= dy;
                    partyIsWalking = true;
                }
            } break;

            case PARTY_CenterView:
                partyViewNewPitch = 0;
                break;

            case PARTY_LookUp:
                partyViewNewPitch += engine->config->settings.VerticalTurnSpeed.value();
                if (partyViewNewPitch > 128) partyViewNewPitch = 128;
                if (pParty->hasActiveCharacter()) {
                    pParty->activeCharacter().playReaction(SPEECH_LOOK_UP);
                }
                break;

            case PARTY_LookDown:
                partyViewNewPitch -= engine->config->settings.VerticalTurnSpeed.value();
                if (partyViewNewPitch < -128) partyViewNewPitch = -128;
                if (pParty->hasActiveCharacter()) {
                    pParty->activeCharacter().playReaction(SPEECH_LOOK_DOWN);
                }
                break;

            case PARTY_Jump:
                if ((!partyAtHighSlope || floorFaceId) &&
                    // to avoid jump hesitancy when moving downhill
                    (!partyNotTouchingFloor || (partyCloseToGround && partyInputZSpeed <= 0)) &&
                    pParty->jump_strength &&
                    !(pParty->uFlags & PARTY_FLAGS_1_WATER_DAMAGE) &&
                    !(pParty->uFlags & PARTY_FLAGS_1_BURNING)) {
                    partyNotTouchingFloor = true;
                    partyInputZSpeed += pParty->jump_strength * 96;
                    // boost party upwards slightly so we dont "land" straight away
                    pParty->pos.z += 1;
                }
                break;

            case PARTY_Land:
                if (pParty->bFlying) {
                    pParty->uFlags |= PARTY_FLAGS_1_LANDING;
                }
                pPartyActionQueue->uNumActions = 0;
                break;

            default:
                assert(false);
        }
    }

    // Behaviour divergence from vanilla - now treat landing process as flying down for consistency
    if (pParty->uFlags & PARTY_FLAGS_1_LANDING || flyDown) {
        if (pParty->FlyActive() || engine->IsUnderwater()) {
            pParty->bFlying = false;
            if (engine->IsUnderwater() ||
                pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff ||
                (pParty->pCharacters[pParty->pPartyBuffs[PARTY_BUFF_FLY].caster - 1].mana > 0 || engine->config->debug.AllMagic.value())) {
                partyOldFlightZ = pParty->pos.z;
                partyInputZSpeed = -pParty->walkSpeed * 4;
                pParty->bFlying = true;
                noFlightBob = true;
                if (flyDown)
                    pParty->uFlags &= ~(PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);
            }
        }
    }

    if (partyInputXSpeed * partyInputXSpeed + partyInputYSpeed * partyInputYSpeed < 400 && !partyAtHighSlope) {
        partyInputYSpeed = 0;
        partyInputXSpeed = 0;
    }

    // set party look angles
    pParty->_viewYaw = partyViewNewYaw;
    pParty->_viewPitch = partyViewNewPitch;

    int partyNewX = pParty->pos.x;
    int partyNewY = pParty->pos.y;
    int partyNewZ = pParty->pos.z;

    //-------------------------------------------
    if (pParty->bFlying) {
        // bob up and down in the air
        if (noFlightBob) {
            partyNewZ = partyOldFlightZ;
        } else {
            partyNewZ = partyOldFlightZ + 4 * TrigLUT.cos(platform->tickCount());
        }

        if (pParty->FlyActive())
            engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].overlayID + 119] &= 0xFE;
        pParty->uFallStartZ = partyNewZ;
    } else if (partyNewZ < currentGroundLevel) {
        if (partyIsOnWater && partyInputZSpeed)
            SpriteObject::createSplashObject({partyNewX, partyNewY, currentGroundLevel});
        partyInputZSpeed = 0;
        partyNewZ = currentGroundLevel;
        pParty->uFallStartZ = currentGroundLevel;
        partyOldFlightZ = partyNewZ;
        if (pParty->FlyActive())
            engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].overlayID + 119] |= 1;
    } else {
        partyOldFlightZ = partyNewZ;
        if (pParty->FlyActive())
            engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].overlayID + 119] |= 1;
    }
    //------------------------------------------

    // has a modifiction been made to party position to push it away from a slope
    bool partySlopeMod{ false };
    if (partyNotTouchingFloor && !pParty->bFlying) {  // add gravity
        partyInputZSpeed += (-(pEventTimer->uTimeElapsed * GetGravityStrength()) << 1);
    } else if (!partyNotTouchingFloor) {
        if (!floorFaceId) {
            // rolling down the hill
            // how it's done: you get a little bit pushed in the air along
            // terrain normal, getting in the air and falling to the gravity,
            // gradually sliding downwards. nice trick
            partyNewZ = currentGroundLevel;
            if (partyAtHighSlope) {
                Vec3i v98;
                ODM_GetTerrainNormalAt(partyNewX, partyNewY, &v98);
                int v35 = partyInputZSpeed + (8 * -(pEventTimer->uTimeElapsed * GetGravityStrength()));
                int dot = std::abs(partyInputXSpeed * v98.x + partyInputYSpeed * v98.y + v35 * v98.z) >> 16;
                partyInputXSpeed += fixpoint_mul(dot, v98.x);
                partyInputYSpeed += fixpoint_mul(dot, v98.y);
                partyInputZSpeed = v35 + fixpoint_mul(dot, v98.z);
                partySlopeMod = true;
            }
        }
    }

    if (partyNotTouchingFloor) {
        if (!engine->IsUnderwater() && partyInputZSpeed <= 0) {
            if (partyInputZSpeed < -500 && !pParty->bFlying &&
                pParty->pos.z - currentGroundLevel > 1000 &&
                !pParty->FeatherFallActive() &&
                !(pParty->uFlags & (PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING))) {  // falling scream
                for (int i = 0; i < 4; ++i) {
                    if (!pParty->pCharacters[i].HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) &&
                        !pParty->pCharacters[i].WearsItem(ITEM_ARTIFACT_HERMES_SANDALS, ITEM_SLOT_BOOTS) &&
                        pParty->pCharacters[i].CanAct())
                        pParty->pCharacters[i].playReaction(SPEECH_FALLING);
                }
            }
        }
    } else {
      pParty->uFallStartZ = partyNewZ;
    }

    // has the party collided with a outdoor model
    bool partyHasHitModel{ false };

    // --(Collisions)-------------------------------------------------------------------
    collision_state.ignored_face_id = -1;
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = pParty->radius;
    collision_state.radius_hi = pParty->radius / 2.0f;
    collision_state.check_hi = true;
    // make 100 attempts to satisfy collisions
    for (uint i = 0; i < 100; i++) {
        collision_state.position_hi.x = partyNewX;
        collision_state.position_hi.y = partyNewY;
        collision_state.position_hi.z = partyNewZ + (pParty->height - 32) + 1;

        collision_state.position_lo.x = partyNewX;
        collision_state.position_lo.y = partyNewY;
        collision_state.position_lo.z = partyNewZ + collision_state.radius_lo + 1;

        collision_state.velocity.x = partyInputXSpeed;
        collision_state.velocity.y = partyInputYSpeed;
        collision_state.velocity.z = partyInputZSpeed;

        collision_state.uSectorID = 0;

        int frame_movement_dt = 0;
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
            frame_movement_dt = 13312;
        if (collision_state.PrepareAndCheckIfStationary(frame_movement_dt))
            break;

        CollideOutdoorWithModels(true);
        CollideOutdoorWithDecorations(WorldPosToGridCellX(pParty->pos.x), WorldPosToGridCellY(pParty->pos.y));
        _46ED8A_collide_against_sprite_objects(Pid::character(0));

        for (size_t actor_id = 0; actor_id < pActors.size(); ++actor_id)
            CollideWithActor(actor_id, 0);

        int new_pos_low_y = 0;
        int new_pos_low_x = 0;
        int new_pos_low_z = 0;
        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            new_pos_low_x = collision_state.new_position_lo.x;
            new_pos_low_y = collision_state.new_position_lo.y;
            new_pos_low_z = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
        } else {
            new_pos_low_x = partyNewX + collision_state.adjusted_move_distance * collision_state.direction.x;
            new_pos_low_y = partyNewY + collision_state.adjusted_move_distance * collision_state.direction.y;
            new_pos_low_z = partyNewZ + collision_state.adjusted_move_distance * collision_state.direction.z;
        }

        int allnewfloor = ODM_GetFloorLevel(Vec3i(new_pos_low_x, new_pos_low_y, new_pos_low_z), pParty->height, &partyIsOnWater, &floorFaceId, 0);
        int party_y_pid;
        int x_advance_floor = ODM_GetFloorLevel(Vec3i(new_pos_low_x, partyNewY, new_pos_low_z), pParty->height, &partyIsOnWater, &party_y_pid, 0);
        int party_x_pid;
        int y_advance_floor = ODM_GetFloorLevel(Vec3i(partyNewX, new_pos_low_y, new_pos_low_z), pParty->height, &partyIsOnWater, &party_x_pid, 0);
        bool terr_slope_advance_x = IsTerrainSlopeTooHigh(new_pos_low_x, partyNewY);
        bool terr_slope_advance_y = IsTerrainSlopeTooHigh(partyNewX, new_pos_low_y);

        partyNotOnModel = false;
        if (!party_y_pid && !party_x_pid && !floorFaceId) partyNotOnModel = true;

        int move_in_y = 1;
        int move_in_x = 1;
        if (engine->IsUnderwater() || !partyNotOnModel) {
            partyNewX = new_pos_low_x;
            partyNewY = new_pos_low_y;
        } else {
            if (terr_slope_advance_x && x_advance_floor > partyNewZ) move_in_x = 0;
            if (terr_slope_advance_y && y_advance_floor > partyNewZ) move_in_y = 0;

            if (move_in_x) {
                partyNewX = new_pos_low_x;
                if (move_in_y) partyNewY = new_pos_low_y;
            } else if (move_in_y) {
                partyNewY = new_pos_low_y;
            } else {
                if (IsTerrainSlopeTooHigh(new_pos_low_x, new_pos_low_y) && allnewfloor <= partyNewZ) {
                    // move down the hill is allowed
                    partyNewX = new_pos_low_x;
                    partyNewY = new_pos_low_y;
                }
            }
        }

        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            if (!partyNotOnModel) {
                partyNewX = collision_state.new_position_lo.x;
                partyNewY = collision_state.new_position_lo.y;
            }
            partyNewZ = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
            break;
        }

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        partyNewX = new_pos_low_x;
        partyNewY = new_pos_low_y;
        partyNewZ = new_pos_low_z;
        int collisionPID = collision_state.pid;

        if (PID_TYPE(collision_state.pid) == OBJECT_Actor) {
            if (pParty->Invisible())
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
        }

        if (PID_TYPE(collision_state.pid) == OBJECT_Decoration) {
            int atanDecoration = TrigLUT.atan2(
                new_pos_low_x - pLevelDecorations[(signed int)collision_state.pid >> 3].vPosition.x,
                new_pos_low_y - pLevelDecorations[(signed int)collision_state.pid >> 3].vPosition.y);
            partyInputXSpeed = TrigLUT.cos(atanDecoration) * integer_sqrt(partyInputXSpeed * partyInputXSpeed + partyInputYSpeed * partyInputYSpeed);
            partyInputYSpeed = TrigLUT.sin(atanDecoration) * integer_sqrt(partyInputXSpeed * partyInputXSpeed + partyInputYSpeed * partyInputYSpeed);
        }

        if (PID_TYPE(collision_state.pid) == OBJECT_Face) {
            partyHasHitModel = true;
            BSPModel *pModel = &pOutdoor->pBModels[(signed int)collision_state.pid >> 9];
            ODMFace *pODMFace = &pModel->pFaces[((signed int)collision_state.pid >> 3) & 0x3F];
            int bSmallZDelta = (pODMFace->pBoundingBox.z2 - pODMFace->pBoundingBox.z1) <= 32;
            bool bFaceSlopeTooSteep = pODMFace->facePlane.normal.z < 0.70767211914f; // Was 46378 fixpoint

            if (engine->IsUnderwater())
                bFaceSlopeTooSteep = false;

            if (partyInputXSpeed * partyInputXSpeed + partyInputYSpeed * partyInputYSpeed < 400) {
                partyInputXSpeed = 0;
                partyInputYSpeed = 0;
            }

            if (pParty->floor_face_id != PID_ID(collisionPID) && pODMFace->Pressure_Plate()) {
                pParty->floor_face_id = PID_ID(collisionPID);
                triggerID = pODMFace->sCogTriggeredID;  // this one triggers tour events / traps
            }

            // TODO(pskelton): these should probably be if else for polygon types
            if (pODMFace->uPolygonType == POLYGON_Floor) {
                pParty->bFlying = false;
                pParty->uFlags &= ~(PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);
                if (partyInputZSpeed < 0) partyInputZSpeed = 0;
                partyNewZ = pModel->pVertices[pODMFace->pVertexIDs[0]].z + 1;
            }

            if (!bSmallZDelta && (pODMFace->uPolygonType != POLYGON_InBetweenFloorAndWall || bFaceSlopeTooSteep)) {  // упёрся в столб
                partySlopeMod = true;

                // push party away from the surface
                int dot = std::abs(partyInputYSpeed * pODMFace->facePlane.normal.y +
                           partyInputZSpeed * pODMFace->facePlane.normal.z +
                           partyInputXSpeed * pODMFace->facePlane.normal.x);
                if ((collision_state.speed / 8) > dot)
                    dot = collision_state.speed / 8;
                partyInputXSpeed += dot * pODMFace->facePlane.normal.x;
                partyInputYSpeed += dot * pODMFace->facePlane.normal.y;
                int v54 = 0;
                if (!bFaceSlopeTooSteep)
                    v54 = dot * pODMFace->facePlane.normal.z;
                partyInputZSpeed += v54;
                int v55 = collision_state.radius_lo - pODMFace->facePlane.signedDistanceTo(Vec3f(new_pos_low_x, new_pos_low_y, new_pos_low_z));
                if (v55 > 0) {
                    partyNewX = new_pos_low_x + pODMFace->facePlane.normal.x * v55;
                    partyNewY = new_pos_low_y + pODMFace->facePlane.normal.y * v55;
                    if (!bFaceSlopeTooSteep)
                        partyNewZ = new_pos_low_z + pODMFace->facePlane.normal.z * v55;
                }
            }

            if (pODMFace->uPolygonType == POLYGON_InBetweenFloorAndWall) {
                pParty->bFlying = false;
                pParty->uFlags &= ~(PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);

                // this pushes party slightly up away from the surface so you can climb it
                float dot = std::abs(partyInputYSpeed * pODMFace->facePlane.normal.y +
                           partyInputZSpeed * pODMFace->facePlane.normal.z +
                           partyInputXSpeed * pODMFace->facePlane.normal.x);
                if ((collision_state.speed / 8) > dot)
                    dot = collision_state.speed / 8;
                partyInputZSpeed += dot * pODMFace->facePlane.normal.z;
                partyInputXSpeed += dot * pODMFace->facePlane.normal.x;
                partyInputYSpeed += dot * pODMFace->facePlane.normal.y;

                partySlopeMod = true;
            }
        }

        // ~0.9x reduce party speed and try again
        partyInputXSpeed = fixpoint_mul(58500, partyInputXSpeed);
        partyInputYSpeed = fixpoint_mul(58500, partyInputYSpeed);
        partyInputZSpeed = fixpoint_mul(58500, partyInputZSpeed);
    }

    if (!partyNotTouchingFloor || partyCloseToGround)
        pParty->setAirborne(false);
    else
        pParty->setAirborne(true);

    Vec3i partyOldPosition = pParty->pos;
    int partyCurrentXGrid = WorldPosToGridCellX(pParty->pos.x);
    int partyCurrentYGrid = WorldPosToGridCellY(pParty->pos.y);
    int partyNewXGrid = WorldPosToGridCellX(partyNewX);
    int partyNewYGrid = WorldPosToGridCellY(partyNewY);

    // this gets if tile is not water
    bool partyCurrentOnLand = !(pOutdoor->getTileAttribByGrid(partyCurrentXGrid, partyCurrentYGrid) & TILE_DESC_WATER);
    bool partyNewXOnLand = !(pOutdoor->getTileAttribByGrid(partyNewXGrid, partyCurrentYGrid) & TILE_DESC_WATER);
    bool partyNewYOnLand = !(pOutdoor->getTileAttribByGrid(partyCurrentXGrid, partyNewYGrid) & TILE_DESC_WATER);

    // -(update party co-ords)---------------------------------------
    bool notWater{ false };
    if (partyNewXGrid == partyCurrentXGrid && partyNewYGrid == partyCurrentYGrid && partyCurrentOnLand/*partyNewXOnLand && partyNewYOnLand*/)
        notWater = true;

    if (!partyNotOnModel)
        notWater = true;

    if (notWater) {
        pParty->pos.x = partyNewX;
        pParty->pos.y = partyNewY;

        if (partySlopeMod) {
            pParty->speed.z = partyNewZ - pParty->pos.z;
        } else {
            pParty->speed.z = partyInputZSpeed;
        }

        pParty->pos.z = partyNewZ;
        pParty->sPartySavedFlightZ = partyOldFlightZ;

        pParty->uFlags &= ~(PARTY_FLAGS_1_BURNING | PARTY_FLAGS_1_WATER_DAMAGE);
    } else {
        // we are on/approaching water tile
        bool waterMoveX;
        bool waterMoveY;

        if (pParty->bFlying || !partyCloseToGround || waterWalkActive || !partyCurrentOnLand)
            waterMoveX = 1;
        else
            waterMoveX = partyNewXOnLand != 0;

        bool partyDrowningFlag = false;

        if (!pParty->bFlying && partyCloseToGround && !waterWalkActive) {
            if (partyCurrentOnLand) {
                waterMoveY = partyNewYOnLand != 0;
            } else {
                partyDrowningFlag = true;
                waterMoveY = true;
            }
        } else {
            waterMoveY = true;
        }

        if (waterMoveX) pParty->pos.x = partyNewX;
        if (waterMoveY) pParty->pos.y = partyNewY;

        if (waterMoveY || waterMoveX) {
            if (waterWalkActive) {
                pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
                engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].overlayID + 119] |= 1;
                if (!partyNewXOnLand || !partyNewYOnLand) {
                    if (!pParty->bFlying) {
                        pParty->uFlags |= PARTY_FLAGS_1_STANDING_ON_WATER;
                        engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].overlayID + 119] &= 0xFFFE;
                    }
                }
            }
        }

        pParty->pos.z = partyNewZ;
        pParty->speed.z = partyInputZSpeed;
        pParty->sPartySavedFlightZ = partyOldFlightZ;

        pParty->uFlags &= ~(PARTY_FLAGS_1_BURNING | PARTY_FLAGS_1_WATER_DAMAGE);

        if (partyDrowningFlag) {
            bool onWater = false;
            int pTerrainHeight = GetTerrainHeightsAroundParty2(pParty->pos.x, pParty->pos.y, &onWater, 1);
            if (pParty->pos.z <= pTerrainHeight + 1) {
                pParty->uFlags |= PARTY_FLAGS_1_WATER_DAMAGE;
            }
        }
    }

    // height restriction
    if (pParty->pos.z > 8160) {
        pParty->uFallStartZ = 8160;
        pParty->pos.z = 8160;
    }

    // new ground level
    int newFloorLevel = ODM_GetFloorLevel(Vec3i(partyNewX, partyNewY, partyNewZ), pParty->height,
                                          &partyIsOnWater, &floorFaceId, waterWalkActive);
    int newGroundLevel = newFloorLevel + 1;

    // Falling damage
    if (!triggerID ||
        (eventProcessor(triggerID, Pid(), 1), pParty->pos.x == partyNewX) &&
        pParty->pos.y == partyNewY && pParty->pos.z == partyNewZ) {
        if (((pParty->pos.z <= newGroundLevel || partyHasHitModel) && partyInputZSpeed < 0)) {
            pParty->speed.z = 0;
            if (!partyHasHitModel)
                pParty->pos.z = newGroundLevel;
            if (pParty->uFallStartZ - partyNewZ > 512 && !partyHasFeatherFall &&
                (partyNewZ <= newGroundLevel || partyHasHitModel) &&
                !engine->IsUnderwater()) {
                if (pParty->uFlags & (PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING)) {
                    pParty->uFlags &= ~(PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);
                } else {
                    pParty->giveFallDamage(pParty->uFallStartZ - pParty->pos.z);
                }
            }
            pParty->uFallStartZ = partyNewZ;
        }
        if (ceilingFaceID && pParty->pos.z < ceilingHeight && (pParty->height + pParty->pos.z) >= ceilingHeight) {
            pParty->pos.z = ceilingHeight - pParty->height - 1;
            pParty->sPartySavedFlightZ = pParty->pos.z;
        }
    }

    if (partySlopeMod)
        pParty->uFallStartZ = partyNewZ;

    // walking / running sounds ------------------------
    if (engine->config->settings.WalkSound.value()) {
        bool canStartNewSound = !pAudioPlayer->isWalkingSoundPlays();

        // Start sound processing only when actual movement is performed to avoid stopping sounds on high FPS
        if (pEventTimer->uTimeElapsed) {
            // TODO(Nik-RE-dev): use calculated velocity of party and walk/run flags instead of delta
            int walkDelta = integer_sqrt((partyOldPosition - pParty->pos).lengthSqr());

            if (walkDelta < 2) {
                // mute the walking sound when stopping
                pAudioPlayer->stopWalkingSounds();
            } else {
                // Delta limits for running/walking has been changed. Previously:
                // - for run limit was >= 16
                // - for walk limit was >= 8
                // - stop sound if delta < 8
                if (!partyNotTouchingFloor || partyCloseToGround) {
                    int modelId = pParty->floor_face_id >> 6;
                    int faceId = pParty->floor_face_id & 0x3F;
                    bool isModelWalk = !partyNotOnModel && pOutdoor->pBModels[modelId].pFaces[faceId].Visible();
                    SoundID sound = SOUND_Invalid;
                    if (partyIsRunning) {
                        if (walkDelta >= 4 ) {
                            if (isModelWalk) {
                                sound = SOUND_RunWood;
                            } else {
                                // Old comment: 56 is ground run
                                sound = pOutdoor->getSoundIdByGrid(WorldPosToGridCellX(partyOldPosition.x), WorldPosToGridCellY(partyOldPosition.y), true);
                            }
                        }
                    } else if (partyIsWalking) {
                        if (walkDelta >= 2) {
                            if (isModelWalk) {
                                sound = SOUND_RunWood;
                            } else {
                                sound = pOutdoor->getSoundIdByGrid(WorldPosToGridCellX(partyOldPosition.x), WorldPosToGridCellY(partyOldPosition.y), false);
                            }
                        }
                    }

                    if (sound != pParty->currentWalkingSound) {
                        pAudioPlayer->stopWalkingSounds();
                        canStartNewSound = true;
                    }
                    if (sound != SOUND_Invalid && canStartNewSound) {
                        pParty->currentWalkingSound = sound;
                        pAudioPlayer->playWalkSound(sound);
                    }
                } else {
                    pAudioPlayer->stopWalkingSounds();
                }
            }
        }
    }
}

int GetCeilingHeight(int Party_X, signed int Party_Y, int Party_ZHeight, int *pFaceID) {
    std::array<int, 20> face_indices{};
    std::array<int, 20> model_indices{};
    std::array<int, 20> ceiling_height_level{};
    model_indices[0] = -1;
    face_indices[0] = -1;
    ceiling_height_level[0] = 10000;  // no ceiling

    int ceiling_count = 1;

    for (BSPModel &model : pOutdoor->pBModels) {
        if (!model.pBoundingBox.containsXY(Party_X, Party_Y))
            continue;

        for (ODMFace &face : model.pFaces) {
            if (face.Ethereal())
                continue;

            if (face.uPolygonType != POLYGON_Ceiling && face.uPolygonType != POLYGON_InBetweenCeilingAndWall)
                continue;

            if (!face.pBoundingBox.containsXY(Party_X, Party_Y))
                continue;

            int slack = engine->config->gameplay.FloorChecksEps.value();
            if (!face.Contains(Vec3i(Party_X, Party_Y, 0), model.index, slack, FACE_XY_PLANE))
                continue;

            if (ceiling_count >= 20)
                break;

            int height_level;
            if (face.uPolygonType == POLYGON_Ceiling)
                height_level = model.pVertices[face.pVertexIDs[0]].z;
            else
                height_level = face.zCalc.calculate(Party_X, Party_Y);

            ceiling_height_level[ceiling_count] = height_level;
            model_indices[ceiling_count] = model.index;
            face_indices[ceiling_count] = face.index;

            ++ceiling_count;
        }
    }

    if (!ceiling_count) {
        *pFaceID = 0;
        return ceiling_height_level[0];
    }

    int result_idx = 0;
    for (int i = 0; i < ceiling_count; ++i) {
        if (ceiling_height_level[i] == ceiling_height_level[0])
            result_idx = i;
        else if (ceiling_height_level[i] < ceiling_height_level[0] && ceiling_height_level[0] > Party_ZHeight + 15)
            result_idx = i;
        else if (ceiling_height_level[i] > ceiling_height_level[0] && ceiling_height_level[i] <= Party_ZHeight + 15)
            result_idx = i;
    }

    if (result_idx != 0) {
        *pFaceID = face_indices[result_idx] | (model_indices[result_idx] << 6);
        return ceiling_height_level[result_idx];
    } else {
        *pFaceID = 0;
        return ceiling_height_level[result_idx];
    }
}

//----- (00464839) --------------------------------------------------------
char Is_out15odm_underwater() {
    return (pCurrentMapName == "out15.odm");
}

//----- (00464851) --------------------------------------------------------
void SetUnderwaterFog() {
    day_fogrange_1 = 50;
    day_fogrange_2 = 5000;
}

//----- (00487DA9) --------------------------------------------------------
void sub_487DA9() {
    // for (int i = 0; i < 20000; ++i) array_77EC08[i].field_108 = 0;
}

//----- (004706C6) --------------------------------------------------------
void UpdateActors_ODM() {
    if (engine->config->debug.NoActors.value())
        return;  // uNumActors = 0;

    for (unsigned int Actor_ITR = 0; Actor_ITR < pActors.size(); ++Actor_ITR) {
        if (pActors[Actor_ITR].aiState == Removed || pActors[Actor_ITR].aiState == Disabled ||
            pActors[Actor_ITR].aiState == Summoned || !pActors[Actor_ITR].moveSpeed)
                continue;

        bool Water_Walk = MonsterStats::BelongsToSupertype(pActors[Actor_ITR].monsterInfo.uID, MONSTER_SUPERTYPE_WATER_ELEMENTAL);

        pActors[Actor_ITR].sectorId = 0;

        bool uIsFlying = pActors[Actor_ITR].monsterInfo.uFlying;
        if (!pActors[Actor_ITR].CanAct())
            uIsFlying = 0;

        bool Slope_High = IsTerrainSlopeTooHigh(pActors[Actor_ITR].pos.x, pActors[Actor_ITR].pos.y);
        int Model_On_PID = 0;
        bool uIsOnWater = false;
        int Floor_Level = ODM_GetFloorLevel(pActors[Actor_ITR].pos, pActors[Actor_ITR].height, &uIsOnWater, &Model_On_PID, Water_Walk);
        bool Actor_On_Terrain = Model_On_PID == 0;

        bool uIsAboveFloor = (pActors[Actor_ITR].pos.z > (Floor_Level + 1));

        // make bloodsplat when the ground is hit
        if (!pActors[Actor_ITR].donebloodsplat) {
            if (pActors[Actor_ITR].aiState == Dead || pActors[Actor_ITR].aiState == Dying) {
                if (pActors[Actor_ITR].pos.z < Floor_Level + 30) { // 30 to provide small error / rounding factor
                    if (pMonsterStats->pInfos[pActors[Actor_ITR].monsterInfo.uID].bBloodSplatOnDeath) {
                        if (engine->config->graphics.BloodSplats.value()) {
                            float splatRadius = pActors[Actor_ITR].radius * engine->config->graphics.BloodSplatsMultiplier.value();
                            decal_builder->AddBloodsplat(Vec3f(pActors[Actor_ITR].pos.x, pActors[Actor_ITR].pos.y, Floor_Level + 30), colorTable.Red, splatRadius);
                        }
                        pActors[Actor_ITR].donebloodsplat = true;
                    }
                }
            }
        }

        if (pActors[Actor_ITR].aiState == Dead && uIsOnWater && !uIsAboveFloor) {
            pActors[Actor_ITR].aiState = Removed;
            continue;
        }

        // MOVEMENT
        if (pActors[Actor_ITR].currentActionAnimation == ANIM_Walking) {
            int Actor_Speed = pActors[Actor_ITR].moveSpeed;
            if (pActors[Actor_ITR].buffs[ACTOR_BUFF_SLOWED].Active())
                Actor_Speed = Actor_Speed * 0.5;
            if (pActors[Actor_ITR].aiState == Fleeing || pActors[Actor_ITR].aiState == Pursuing)
                Actor_Speed *= 2;
            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_WAIT) {
                Actor_Speed *= debug_turn_based_monster_movespeed_mul;
            }
            if (Actor_Speed > 1000)
                Actor_Speed = 1000;

            pActors[Actor_ITR].speed.x = TrigLUT.cos(pActors[Actor_ITR].yawAngle) * Actor_Speed;
            pActors[Actor_ITR].speed.y = TrigLUT.sin(pActors[Actor_ITR].yawAngle) * Actor_Speed;
            if (uIsFlying) {
                pActors[Actor_ITR].speed.z = TrigLUT.sin(pActors[Actor_ITR].pitchAngle) * Actor_Speed;
            }
        } else {
            pActors[Actor_ITR].speed.x = fixpoint_mul(55000, pActors[Actor_ITR].speed.x);
            pActors[Actor_ITR].speed.y = fixpoint_mul(55000, pActors[Actor_ITR].speed.y);
            if (uIsFlying)
                pActors[Actor_ITR].speed.z = fixpoint_mul(55000, pActors[Actor_ITR].speed.z);
        }

        // BELOW FLOOR - POP UPWARDS
        if (pActors[Actor_ITR].pos.z < Floor_Level) {
            pActors[Actor_ITR].pos.z = Floor_Level;
            pActors[Actor_ITR].speed.z = uIsFlying != 0 ? 0x14 : 0;
        }
        // GRAVITY
        if (!uIsAboveFloor || uIsFlying) {
            if (Slope_High && !uIsAboveFloor && Actor_On_Terrain) {
                Vec3i Terrain_Norm;
                pActors[Actor_ITR].pos.z = Floor_Level;
                ODM_GetTerrainNormalAt(pActors[Actor_ITR].pos.x, pActors[Actor_ITR].pos.y, &Terrain_Norm);
                uint16_t Gravity = GetGravityStrength();

                pActors[Actor_ITR].speed.z += -16 * pEventTimer->uTimeElapsed * Gravity;
                int v73 = std::abs(Terrain_Norm.x * pActors[Actor_ITR].speed.x +
                              Terrain_Norm.z * pActors[Actor_ITR].speed.z +
                              Terrain_Norm.y * pActors[Actor_ITR].speed.y) >> 15;

                pActors[Actor_ITR].speed.x += fixpoint_mul(v73, Terrain_Norm.x);
                pActors[Actor_ITR].speed.y += fixpoint_mul(v73, Terrain_Norm.y);
                pActors[Actor_ITR].yawAngle -= 32;
                // pActors[Actor_ITR].vVelocity.z += fixpoint_mul(v73, Terrain_Norm.z);
            }
        } else {
            pActors[Actor_ITR].speed.z -= pEventTimer->uTimeElapsed * GetGravityStrength();
        }

        // ARMAGEDDON PANIC
        if (pParty->armageddon_timer != 0 && pActors[Actor_ITR].CanAct() && pParty->armageddonForceCount > 0) {
            pActors[Actor_ITR].speed.x += grng->random(100) - 50;
            pActors[Actor_ITR].speed.y += grng->random(100) - 50;
            pActors[Actor_ITR].speed.z += grng->random(100) - 20;
            pActors[Actor_ITR].aiState = Stunned;
            pActors[Actor_ITR].yawAngle += grng->random(32) - 16;
            pActors[Actor_ITR].UpdateAnimation();
        }

        // TODO(pskelton): this cancels out the above - is this intended
        // MOVING TOO SLOW
        if (pActors[Actor_ITR].speed.xy().lengthSqr() < 400 && Slope_High == 0) {
            pActors[Actor_ITR].speed.y = 0;
            pActors[Actor_ITR].speed.x = 0;
        }

        // COLLISIONS
        ProcessActorCollisionsODM(pActors[Actor_ITR], uIsFlying);

        // WATER TILE CHECKING
        if (!Water_Walk) {
            // tile on (1) tile heading (2)
            bool tile1IsLand, tile2IsLand;
            tile1IsLand = !(pOutdoor->getTileAttribByPos(pActors[Actor_ITR].pos.x, pActors[Actor_ITR].pos.y) & TILE_DESC_WATER);
            tile2IsLand = !(pOutdoor->getTileAttribByPos(pActors[Actor_ITR].pos.x + pActors[Actor_ITR].speed.x,
                                                         pActors[Actor_ITR].pos.y + pActors[Actor_ITR].speed.y) & TILE_DESC_WATER);
            if (!uIsFlying && tile1IsLand && !tile2IsLand) {
                // approaching water - turn away
                if (pActors[Actor_ITR].CanAct()) {
                    pActors[Actor_ITR].yawAngle -= 32;
                    pActors[Actor_ITR].currentActionTime = 0;
                    pActors[Actor_ITR].currentActionLength = 128;
                    pActors[Actor_ITR].aiState = Fleeing;
                }
            }
            if (!uIsFlying && !tile1IsLand && !uIsAboveFloor && Actor_On_Terrain) {
                // on water and shouldnt be
                bool tileTestLand = false;  // reset land found
                int Grid_X = WorldPosToGridCellX(pActors[Actor_ITR].pos.x);
                int Grid_Z = WorldPosToGridCellY(pActors[Actor_ITR].pos.y);
                for (int i = Grid_X - 1; i <= Grid_X + 1; i++) {
                    // scan surrounding cells for land
                    for (int j = Grid_Z - 1; j <= Grid_Z + 1; j++) {
                        tileTestLand = !(pOutdoor->getTileAttribByGrid(i, j) & TILE_DESC_WATER);
                        if (tileTestLand) {  // found land
                            int target_x = GridCellToWorldPosX(i);
                            int target_y = GridCellToWorldPosY(j);
                            if (pActors[Actor_ITR].CanAct()) {  // head to land
                                pActors[Actor_ITR].yawAngle = TrigLUT.atan2(target_x - pActors[Actor_ITR].pos.x,
                                                                             target_y - pActors[Actor_ITR].pos.y);
                                pActors[Actor_ITR].currentActionTime = 0;
                                pActors[Actor_ITR].currentActionLength = 128;
                                pActors[Actor_ITR].aiState = Fleeing;
                                break;
                            }
                        }
                    }
                    if (tileTestLand) {  // break out nested loop
                        break;
                    }
                }
                if (!tileTestLand) {
                    // no land found so drowning damage
                    // pActors[Actor_ITR].sCurrentHP -= 1;
                    // logger->Warning("DROWNING");
                }
            }
        }
    }
}

//----- (0047A384) --------------------------------------------------------
void ODM_LoadAndInitialize(const std::string &pFilename, ODMRenderParams *thisa) {
    MapInfo *map_info;            // edi@4
    // size_t v7;              // eax@19

    // thisa->AllocSoftwareDrawBuffers();
    pWeather->bRenderSnow = false;
    render->ClearZBuffer();
    // thisa = (ODMRenderParams *)1;
    GetAlertStatus(); // Result unused.
    pParty->_delayedReactionTimer = 0;
    MAP_TYPE map_id = pMapStats->GetMapInfo(pCurrentMapName);
    unsigned int respawn_interval = 0;
    if (map_id != MAP_INVALID) {
        map_info = &pMapStats->pInfos[map_id];
        respawn_interval = map_info->uRespawnIntervalDays;
    }
    day_attrib &= ~DAY_ATTRIB_FOG;
    dword_6BE13C_uCurrentlyLoadedLocationID = map_id;
    bool outdoor_was_respawned;
    pOutdoor->Initialize(pFilename, pParty->GetPlayingTime().GetDays() + 1,
        respawn_interval, &outdoor_was_respawned);

    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)) {
        Actor::InitializeActors();
        SpriteObject::InitializeSpriteObjects();
    }
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN;

    if (outdoor_was_respawned && map_id != MAP_INVALID) {
        for (uint i = 0; i < pOutdoor->pSpawnPoints.size(); ++i) {
            SpawnPoint *spawn = &pOutdoor->pSpawnPoints[i];

            if (spawn->uKind == OBJECT_Actor)
                SpawnEncounter(map_info, spawn, 0, 0, 0);
            else
                SpawnRandomTreasure(map_info, spawn);
        }
        RespawnGlobalDecorations();
    }
    pOutdoor->PrepareDecorations();
    pOutdoor->ArrangeSpriteObjects();
    pOutdoor->InitalizeActors(map_id);
    pOutdoor->MessWithLUN();
    pOutdoor->level_filename = pFilename;
    pWeather->Initialize();
    pCamera3D->_viewYaw = pParty->_viewYaw;
    pCamera3D->_viewPitch = pParty->_viewPitch;
    // pODMRenderParams->RotationToInts();
    pOutdoor->UpdateSunlightVectors();

    for (int i = 0; i < 20000; ++i) {
        array_77EC08[i].ptr_38 = &SkyBillboard;

        array_77EC08[i].ptr_48 = nullptr;
    }

    MM7Initialization();
}
// returns 0xXXYYZZ fog color
Color GetLevelFogColor() {
    if (engine->IsUnderwater()) {
        return colorTable.OliveDrab;
    }

    if (day_attrib & DAY_ATTRIB_FOG) {
        if (pWeather->bNight) {  // night-time fog
            if (false) {
                logger->error("decompilation can be inaccurate, please send savegame to Nomad");
                __debugbreak();
            }
            if (pWeather->bNight) {
                return colorTable.DarkGray;
            } else {
                return Color();
            }
        } else {
            int64_t v1 = (int64_t)((1.0 - pOutdoor->fFogDensity) * 200.0 +
                                   pOutdoor->fFogDensity * 31.0);
            int out = v1 & 0xFF;
            return Color(out, out, out);
        }
    }

    return Color();
}

// TODO(pskelton): drop this
// returns 0xZZ000000
// basically how much fog should be applied 255-0 (no fog -> max fog)
int sub_47C3D7_get_fog_specular(int unused, int isSky, float screen_depth) {
    int v7;

    bool isNight = pWeather->bNight;
    if (engine->IsUnderwater()) isNight = false;

    if (pParty->armageddon_timer ||
        !(day_attrib & DAY_ATTRIB_FOG) && !engine->IsUnderwater())
        return 0xFF000000;
    if (isNight) {
        if (screen_depth < (double)day_fogrange_1) {
            v7 = 0;
            if (screen_depth == 0.0) v7 = 216;
            if (isSky) v7 = 248;
            return (255 - v7) << 24;
        } else {
            if (screen_depth > (double)day_fogrange_2) {
                v7 = 216;
                if (screen_depth == 0.0) v7 = 216;
                if (isSky) v7 = 248;
                return (255 - v7) << 24;
            }
            v7 = (int64_t)((screen_depth - (double)day_fogrange_1) /
                                  ((double)day_fogrange_2 -
                                   (double)day_fogrange_1) *
                                  216.0);
        }
    } else {
        if (screen_depth < (double)day_fogrange_1) {
            // no fog
            v7 = 0;
            if (screen_depth == 0.0) v7 = 216;
            if (isSky) v7 = 248;
            return (255 - v7) << 24;
        } else {
            if (screen_depth > (double)day_fogrange_2) {
                // full fog
                v7 = 216;
                if (screen_depth == 0.0) v7 = 216;
                if (isSky) v7 = 248;
                return (255 - v7) << 24;
            } else {
                // linear interpolation
                v7 =
                    floorf(((screen_depth - (double)day_fogrange_1) * 216.0 /
                    ((double)day_fogrange_2 - (double)day_fogrange_1)) +
                        0.5f);
            }
        }
    }
    if (v7 > 216) {
        v7 = 216;
    } else {
        if (screen_depth == 0.0) v7 = 216;
    }
    if (isSky) v7 = 248;
    return (255 - v7) << 24;
}

//----- (0047F44B) --------------------------------------------------------
unsigned int WorldPosToGridCellX(int sWorldPosX) {
    return (sWorldPosX >> 9) + 64;  // sar is in original exe, resulting -880 / 512 = -1
                                    //                               and -880 sar 9 = -2
}

//----- (0047F458) --------------------------------------------------------
unsigned int WorldPosToGridCellY(int sWorldPosY) {
    return 63 - (sWorldPosY >> 9);  // sar is in original exe, resulting -880 / 512 = -1
                                    //                               and -880 sar 9 = -2
}

//----- (0047F469) --------------------------------------------------------
int GridCellToWorldPosX(int a1) { return (a1 - 64) << 9; }

//----- (0047F476) --------------------------------------------------------
int GridCellToWorldPosY(int a1) { return (64 - a1) << 9; }


//----- (004823F4) --------------------------------------------------------
bool IsTerrainSlopeTooHigh(int pos_x, int pos_y) {
    // unsigned int v2; // ebx@1
    // unsigned int v3; // edi@1
    // int v4; // eax@1
    // int v6; // esi@5
    // int v7; // ecx@6
    // int v8; // edx@6
    // int v9; // eax@6
    // int y_min; // esi@10
    // int v11; // [sp+14h] [bp-8h]@1
    // int v12; // [sp+18h] [bp-4h]@1

    // v12 = a1;
    // v11 = a2;
    unsigned int grid_x = WorldPosToGridCellX(pos_x);
    unsigned int grid_z = WorldPosToGridCellY(pos_y);

    int party_grid_x1 = GridCellToWorldPosX(grid_x);
    // dword_76D56C_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(grid_x + 1);
    // dword_76D570_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(grid_x + 1);
    // dword_76D574_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(grid_x);
    int party_grid_z1 = GridCellToWorldPosY(grid_z);
    // dword_76D55C_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosY(grid_z);
    // dword_76D560_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosY(grid_z + 1);
    // dword_76D564_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosY(grid_z + 1);
    int party_x1z1_y = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_z);
    int party_x2z1_y = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_z);
    int party_x2z2_y = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_z + 1);
    int party_x1z2_y = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_z + 1);
    // dword_76D554_terrain_cell_world_pos_around_party_y = v4;
    if (party_x1z1_y == party_x2z1_y && party_x2z1_y == party_x2z2_y &&
        party_x2z2_y == party_x1z2_y)
        return false;

    int dx = std::abs(pos_x - party_grid_x1), dz = std::abs(party_grid_z1 - pos_y);

    int y1, y2, y3;
    if (dz >= dx) {
        y1 = party_x1z2_y;
        y2 = party_x2z2_y;
        y3 = party_x1z1_y;
        /*  lower-left triangle
          y3 | \
             |   \
             |     \
             |______ \
          y1           y2   */
    } else {
        y1 = party_x2z1_y;  // upper-right
        y2 = party_x1z1_y;  //  y2_______ y1
        y3 = party_x2z2_y;  //    \     |
                            /*      \   |
                                      \ |
                                       y3     */
    }

    int y_min = std::min(y1, std::min(y2, y3));  // не верно при подъёме на склон
    int y_max = std::max(y1, std::max(y2, y3));
    return (y_max - y_min) > 512;
}

//----- (0048257A) --------------------------------------------------------
int GetTerrainHeightsAroundParty2(int x, int y, bool *pIsOnWater, int bFloatAboveWater) {
    //  int result; // eax@9
    int v8;          // ebx@11
    int v9;          // eax@11
    int v10;         // ecx@11
    int v13;         // [sp+10h] [bp-8h]@11
    signed int v14;  // [sp+14h] [bp-4h]@3
    int v15;         // [sp+24h] [bp+Ch]@11

    unsigned int grid_x = WorldPosToGridCellX(x);
    unsigned int grid_y = WorldPosToGridCellY(y);

    int grid_x1 = GridCellToWorldPosX(grid_x),
        grid_x2 = GridCellToWorldPosX(grid_x + 1);
    int grid_y1 = GridCellToWorldPosY(grid_y),
        grid_y2 = GridCellToWorldPosY(grid_y + 1);

    int y_x1z1 = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_y),
        y_x2z1 = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_y),
        y_x2z2 = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_y + 1),
        y_x1z2 = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_y + 1);
    // v4 = WorldPosToGridCellX(x);
    // v5 = WorldPosToGridCellY(v12);
    // dword_76D538_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(v4);
    // dword_76D53C_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(v4 + 1);
    // dword_76D540_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(v4 + 1);
    // dword_76D544_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(v4);
    // dword_76D528_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosY(v5);
    // dword_76D52C_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosY(v5);
    // dword_76D530_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosY(v5 + 1);
    // dword_76D534_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosY(v5 + 1);
    // dword_76D518_terrain_cell_world_pos_around_party_y =
    // pOutdoor->DoGetHeightOnTerrain(v4, v5);
    // dword_76D51C_terrain_cell_world_pos_around_party_y =
    // pOutdoor->DoGetHeightOnTerrain(v4 + 1, v5);
    // dword_76D520_terrain_cell_world_pos_around_party_y =
    // pOutdoor->DoGetHeightOnTerrain(v4 + 1, v5 + 1);
    // dword_76D524_terrain_cell_world_pos_around_party_y =
    // pOutdoor->DoGetHeightOnTerrain(v4, v5 + 1);
    *pIsOnWater = false;
    if (pOutdoor->getTileAttribByGrid(grid_x, grid_y) & TILE_DESC_WATER) {
        *pIsOnWater = true;
    }
    v14 = 0;
    if (!bFloatAboveWater && *pIsOnWater) v14 = -60;
    if (y_x1z1 != y_x2z1 || y_x2z1 != y_x2z2 || y_x2z2 != y_x1z2) {
        if (std::abs(grid_y1 - y) >= std::abs(x - grid_x1)) {
            v8 = y_x1z2;
            v9 = y_x2z2;
            v10 = y_x1z1;
            v15 = x - grid_x1;
            v13 = y - grid_y2;
        } else {
            v8 = y_x2z1;
            v9 = y_x1z1;
            v10 = y_x2z2;
            v15 = grid_x2 - x;
            v13 = grid_y1 - y;
        }
        return v14 + v8 + fixpoint_mul(v13, (v10 - v8) * 128) +
               fixpoint_mul(v15, (v9 - v8) * 128);
    } else {
        return y_x1z1;
    }
}

//----- (00436A6D) --------------------------------------------------------
double OutdoorLocation::GetPolygonMinZ(RenderVertexSoft *pVertices, unsigned int unumverts) {
    double result = FLT_MAX;
    for (uint i = 0; i < unumverts; i++) {
        if (pVertices[i].vWorldPosition.z < result) {
            result = pVertices[i].vWorldPosition.z;
        }
    }
    return result;
}

//----- (00436A40) --------------------------------------------------------
double OutdoorLocation::GetPolygonMaxZ(RenderVertexSoft *pVertex, unsigned int unumverts) {
    double result = FLT_MIN;
    for (uint i = 0; i < unumverts; i++) {
        if (pVertex[i].vWorldPosition.z > result)
            result = pVertex[i].vWorldPosition.z;
    }
    return result;
}

void TeleportToStartingPoint(MapStartPoint point) {
    std::string pName = toString(point);

    if (pDecorationList->GetDecorIdByName(pName)) {
        if (!pLevelDecorations.empty()) {
            for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
                if (pLevelDecorations[i].uDecorationDescID == pDecorationList->GetDecorIdByName(pName)) {
                    pParty->pos = pLevelDecorations[i].vPosition;
                    pParty->speed = Vec3i();
                    pParty->uFallStartZ = pParty->pos.z;
                    pParty->_viewYaw = (TrigLUT.uIntegerHalfPi * pLevelDecorations[i].field_1A) / 90;
                    if (pLevelDecorations[i]._yawAngle)
                        pParty->_viewYaw = pLevelDecorations[i]._yawAngle;
                    pParty->_viewPitch = 0;
                }
            }
        }
        if (Start_Party_Teleport_Flag) {
            if (Party_Teleport_X_Pos)
                pParty->pos.x = Party_Teleport_X_Pos;
            if (Party_Teleport_Y_Pos)
                pParty->pos.y = Party_Teleport_Y_Pos;
            if (Party_Teleport_Z_Pos) {
                pParty->pos.z = Party_Teleport_Z_Pos;
                pParty->uFallStartZ = Party_Teleport_Z_Pos;
            }
            if (Party_Teleport_Cam_Yaw != -1)
                pParty->_viewYaw = Party_Teleport_Cam_Yaw;
            if (Party_Teleport_Cam_Pitch)
                pParty->_viewPitch = Party_Teleport_Cam_Pitch;
            if (Party_Teleport_Z_Speed)
                pParty->speed = Vec3i(0, 0, Party_Teleport_Z_Speed);
        }
        Party_Teleport_Cam_Yaw = -1;
        Start_Party_Teleport_Flag = 0;
        Party_Teleport_Z_Speed = 0;
        Party_Teleport_Cam_Pitch = 0;
        Party_Teleport_Z_Pos = 0;
        Party_Teleport_Y_Pos = 0;
        Party_Teleport_X_Pos = 0;
    }
}
