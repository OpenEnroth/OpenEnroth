#include "Engine/Graphics/Outdoor.h"

#include <algorithm>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Events.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Collisions.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Serialization/LegacyImages.h"
#include "Engine/Serialization/Deserializer.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/stru123.h"
#include "Engine/Tables/TileFrameTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/BspRenderer.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIRest.h"
#include "GUI/UI/UITransition.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/Memory/FreeDeleter.h"
#include "Utility/Math/TrigLut.h"
#include "Library/Random/Random.h"

using EngineIoc = Engine_::IocContainer;

// TODO(pskelton): make this neater
static DecalBuilder* decal_builder = EngineIoc::ResolveDecalBuilder();

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
} fog_probability_table[15] = {
    {20, 10, 5, 0},   // MAP_EMERALD_ISLE
    {20, 10, 5, 0},   // MAP_HARMONDALE
    {20, 10, 5, 0},   // MAP_STEADWICK
    {20, 10, 5, 0},   // MAP_PIERPONT
    {20, 10, 5, 0},   // MAP_DEYJA
    {10, 5, 0, 0},    // MAP_BRAKADA_DESERT
    {0, 0, 0, 0},     // MAP_CELESTIA
    {0, 0, 0, 0},     // MAP_THE_PIT
    {20, 30, 50, 0},  // MAP_EVENMORN_ISLE
    {30, 20, 10, 0},  // MAP_MOUNT_NIGHON
    {10, 5, 0, 0},    // MAP_BARROW_DOWNS
    {20, 10, 5, 0},   // MAP_LAND_OF_GIANTS
    {20, 10, 5, 0},   // MAP_TATALIA
    {20, 10, 5, 0},   // MAP_AVLEE
    {0, 100, 0, 0}    // MAP_SHOALS
};

// for future sky textures?
std::array<int, 9> dword_4EC268 = {{3, 3, 3, 3, 3, 3, 3, 3, 3}};
std::array<int, 7> dword_4EC28C = {{3, 3, 3, 3, 3, 3, 3}};
int dword_4EC2A8 = 9;
int dword_4EC2AC = 7;

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

    pODMRenderParams->uMapGridCellX = WorldPosToGridCellX(pParty->vPosition.x);
    pODMRenderParams->uMapGridCellY = WorldPosToGridCellY(pParty->vPosition.y);

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

    engine->PushStationaryLights(-1);
    // engine->PrepareBloodsplats(); // not used?
    if (bRedraw)
        UpdateDiscoveredArea(WorldPosToGridCellX(pParty->vPosition.x),
                             WorldPosToGridCellY(pParty->vPosition.y),
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

//----- (00488EB1) --------------------------------------------------------
int OutdoorLocation::GetTileAttribByPos(int sX, int sY) {
    int gridY = WorldPosToGridCellY(sY);
    int gridX = WorldPosToGridCellX(sX);

    return GetTileAttribByGrid(gridX, gridY);
}

//----- (00488EEF) --------------------------------------------------------
TileDesc *OutdoorLocation::GetTileDescByPos(int sX, int sY) {
    int gridY = WorldPosToGridCellY(sY);
    int gridX = WorldPosToGridCellX(sX);

    return this->GetTileDescByGrid(gridX, gridY);
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
        assets->ReleaseAllImages();  // dummy

        // pSprites_LOD->DeleteSomeOtherSprites();
        // pSpriteFrameTable->ResetLoadedFlags();

        if (!this->Load(filename, days_played, respawn_interval_days, outdoors_was_respawned)) {
            logger->Warning("Couldn't Load Map!");
            CreateDebugLocation();
        }

        ::day_attrib = this->loc_time.day_attrib;
        ::day_fogrange_1 = this->loc_time.day_fogrange_1;
        ::day_fogrange_2 = this->loc_time.day_fogrange_2;
        if (Is_out15odm_underwater()) SetUnderwaterFog();
        _6BE134_odm_main_tile_group = this->pTileTypes[0].tileset;

        return true;
    }

    return false;
}

char foot_travel_destinations[15][4] = {
    // north           south               east              west from
    {MAP_INVALID, MAP_INVALID, MAP_INVALID, MAP_INVALID},  // MAP_EMERALD_ISLE
    {MAP_PIERPONT, MAP_BARROW_DOWNS, MAP_PIERPONT, MAP_STEADWICK},  // MAP_HARMONDALE
    {MAP_DEYJA, MAP_BRAKADA_DESERT, MAP_HARMONDALE, MAP_TATALIA},  // MAP_STEADWICK
    {MAP_AVLEE, MAP_HARMONDALE, MAP_INVALID, MAP_DEYJA},  // MAP_PIERPONT
    {MAP_PIERPONT, MAP_STEADWICK, MAP_PIERPONT, MAP_STEADWICK},  // MAP_DEYJA
    {MAP_STEADWICK, MAP_INVALID, MAP_BARROW_DOWNS, MAP_INVALID},  // MAP_BRAKADA_DESERT
    {MAP_INVALID, MAP_INVALID, MAP_INVALID, MAP_INVALID},  // MAP_CELESTIA
    {MAP_INVALID, MAP_INVALID, MAP_INVALID, MAP_INVALID},  // MAP_THE_PIT
    {MAP_INVALID, MAP_INVALID, MAP_INVALID, MAP_INVALID},  // MAP_EVENMORN_ISLE
    {MAP_INVALID, MAP_INVALID, MAP_INVALID, MAP_INVALID},  // MAP_MOUNT_NIGHON
    {MAP_HARMONDALE, MAP_BRAKADA_DESERT, MAP_HARMONDALE, MAP_BRAKADA_DESERT},  // MAP_BARROW_DOWNS
    {MAP_INVALID, MAP_INVALID, MAP_INVALID, MAP_INVALID},  // MAP_LAND_OF_GIANTS
    {MAP_INVALID, MAP_INVALID, MAP_STEADWICK, MAP_INVALID},  // MAP_TATALIA
    {MAP_INVALID, MAP_PIERPONT, MAP_PIERPONT, MAP_INVALID},  // MAP_AVLEE
    {MAP_INVALID, MAP_INVALID, MAP_INVALID, MAP_INVALID}     // MAP_SHOALS
};
unsigned char foot_travel_times[15][4] = {
    // north south east  west    from
    {0, 0, 0, 0},  // MAP_EMERALD_ISLE
    {5, 5, 7, 5},  // MAP_HARMONDALE
    {5, 5, 5, 5},  // MAP_STEADWICK
    {5, 5, 0, 5},  // MAP_PIERPONT
    {7, 5, 5, 4},  // MAP_DEYJA
    {5, 0, 5, 0},  // MAP_BRAKADA_DESERT
    {0, 0, 0, 0},  // MAP_CELESTIA
    {0, 0, 0, 0},  // MAP_THE_PIT
    {0, 0, 0, 0},  // MAP_EVENMORN_ISLE
    {0, 0, 0, 0},  // MAP_MOUNT_NIGHON
    {5, 7, 7, 5},  // MAP_BARROW_DOWNS
    {0, 0, 0, 0},  // MAP_LAND_OF_GIANTS
    {0, 0, 5, 0},  // MAP_TATALIA
    {0, 7, 5, 0},  // MAP_AVLEE
    {0, 0, 0, 0},  // MAP_SHOALS
};

MapStartPoint foot_travel_arrival_points[15][4] = {
    // north                south                east                 west from
    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party},  // MAP_EMERALD_ISLE
    {MapStartPoint_South, MapStartPoint_North, MapStartPoint_South, MapStartPoint_East},  // MAP_HARMONDALE
    {MapStartPoint_South, MapStartPoint_North, MapStartPoint_West, MapStartPoint_East},  // MAP_STEADWICK
    {MapStartPoint_East, MapStartPoint_North, MapStartPoint_Party, MapStartPoint_East},  // MAP_PIERPONT
    {MapStartPoint_West, MapStartPoint_North, MapStartPoint_West, MapStartPoint_North},  // MAP_DEYJA
    {MapStartPoint_South, MapStartPoint_Party, MapStartPoint_West, MapStartPoint_Party},  // MAP_BRAKADA_DESERT
    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party},  // MAP_CELESTIA
    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party},  // MAP_THE_PIT
    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party},  // MAP_EVENMORN_ISLE
    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party},  // MAP_MOUNT_NIGHON
    {MapStartPoint_South, MapStartPoint_East, MapStartPoint_South, MapStartPoint_East},  // MAP_BARROW_DOWNS
    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party},  // MAP_LAND_OF_GIANTS
    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_West, MapStartPoint_Party},  // MAP_TATALIA
    {MapStartPoint_Party, MapStartPoint_North, MapStartPoint_North, MapStartPoint_Party},  // MAP_AVLEE
    {MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party, MapStartPoint_Party},  // MAP_SHOALS
};

//----- (0048902E) --------------------------------------------------------
bool OutdoorLocation::GetTravelDestination(signed int sPartyX,
                                           signed int sPartyZ, char *pOut,
                                           signed int a5) {
    char *mapNumberAsStr;       // eax@3
    signed int direction;       // esi@7
    signed int destinationMap;  // eax@23

    std::string str = this->level_filename;
    str = str.substr(str.find_first_of("0123456789"));
    int mapNumberAsInt = atoi(str.c_str());

    if (a5 < 10 || this->level_filename.length() != 9 || mapNumberAsInt < 1 ||
        mapNumberAsInt > 15)  // длина  .odm и количество локаций
        return 0;
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
        if (pPlayers[1]->HasUnderwaterSuitEquipped() &&
            pPlayers[2]->HasUnderwaterSuitEquipped() &&
            pPlayers[3]->HasUnderwaterSuitEquipped() &&
            pPlayers[4]->HasUnderwaterSuitEquipped()) {
            uDefaultTravelTime_ByFoot = 1;
            strcpy(pOut, "out15.odm");  // Shoals
            uLevel_StartingPointType = MapStartPoint_East;
            HEXRAYS_LOWORD(pParty->uFlags) &= 0xFD7Bu;
            return true;
        }
    } else if (mapNumberAsInt == MAP_SHOALS && direction == 3) {  // from Shoals
        uDefaultTravelTime_ByFoot = 1;
        strcpy(pOut, "out14.odm");  // Avlee
        uLevel_StartingPointType = MapStartPoint_West;
        HEXRAYS_LOWORD(pParty->uFlags) &= 0xFD7Bu;
        return true;
    }
    destinationMap = foot_travel_destinations[mapNumberAsInt - 1][direction - 1];
    if (destinationMap == MAP_INVALID)
        return false;

    assert(destinationMap <= MAP_SHOALS);

    uDefaultTravelTime_ByFoot = foot_travel_times[mapNumberAsInt - 1][direction - 1];
    uLevel_StartingPointType = foot_travel_arrival_points[mapNumberAsInt - 1][direction - 1];
    sprintf(pOut, "out%02d.odm", destinationMap);  // локация направления
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

int OutdoorLocation::GetNumFoodRequiredToRestInCurrentPos(const Vec3i &pos) {
    bool is_on_water = false;
    int bmodel_standing_on_pid = 0;
    ODM_GetFloorLevel(pos, pParty->uDefaultPartyHeight, &is_on_water, &bmodel_standing_on_pid, 0);
    if (pParty->IsAirborne() || bmodel_standing_on_pid || is_on_water)
        return 2;

    // TODO: we're passing in pos and then using pParty->vPosition, why?
    int v7 = _47ED83(WorldPosToGridCellX(pParty->vPosition.x),
                     WorldPosToGridCellY(pParty->vPosition.y));
    switch (pTileTable->pTiles[GetTileIdByTileMapId(v7)].tileset) {
        case Tileset_Grass:  // на траве
            return 1;
        case Tileset_Snow:  // на снегу
            return 3;
        case Tileset_Desert:  // на песке
            return 5;
        case Tileset_CooledLava:
        case Tileset_Dirt:  // на грязи
            return 4;
        case Tileset_Water:  // on water (на воде)
            return 3;        // еденицы еды
        default:
            return 2;
    }
}

//----- (00489487) --------------------------------------------------------
void OutdoorLocation::SetFog() {
    pOutdoor->level_filename = pCurrentMapName;

    MAP_TYPE map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id == MAP_INVALID || map_id == MAP_CELESTIA ||
        map_id == MAP_THE_PIT || map_id > MAP_SHOALS)
        return;

    uint chance = vrng->Random(100);

    if (chance < fog_probability_table[map_id - 1].small_fog_chance) {
        ::day_fogrange_1 = 4096;
        ::day_fogrange_2 = 8192;
        ::day_attrib |= DAY_ATTRIB_FOG;
    } else if (chance <
               fog_probability_table[map_id - 1].small_fog_chance +
                   fog_probability_table[map_id - 1].average_fog_chance) {
        ::day_fogrange_2 = 4096;
        ::day_fogrange_1 = 0;
        ::day_attrib |= DAY_ATTRIB_FOG;
    } else if (fog_probability_table[map_id - 1].dense_fog_chance &&
               chance <
                   fog_probability_table[map_id - 1].small_fog_chance +
                       fog_probability_table[map_id - 1].average_fog_chance +
                       fog_probability_table[map_id - 1].dense_fog_chance) {
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
    this->sky_texture_filename = pDefaultSkyTexture.data();
    this->sky_texture = assets->GetBitmap(this->sky_texture_filename);

    this->ground_tileset = byte_6BE124_cfg_textures_DefaultGroundTexture.data();
    // this->sMainTile_BitmapID =
    // pBitmaps_LOD->LoadTexture(this->ground_tileset.c_str());
    this->main_tile_texture = assets->GetBitmap(this->ground_tileset);
}

//----- (0047CF9C) --------------------------------------------------------
void OutdoorLocation::Release() {
    this->level_filename = "blank";
    this->location_filename = "default.odm";
    this->location_file_description = "MM6 Outdoor v1.00";
    this->sky_texture_filename = "sky043";
    this->ground_tileset = "hm005";

    pBModels.clear();
    pSpawnPoints.clear();
    pTerrain.Release();
    pFaceIDLIST.clear();
    pTerrainNormals.clear();

    // free shader data for outdoor location
    render->ReleaseTerrain();
}

bool OutdoorLocation::Load(const std::string &filename, int days_played,
                           int respawn_interval_days,
                           bool *outdoors_was_respawned) {
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

    _6807E0_num_decorations_with_sounds_6807B8 = 0;

    static_assert(sizeof(BSPModelData) == 188);

    if (!pGames_LOD->DoesContainerExist(filename)) {
        Error("Unable to find %s in Games.LOD", filename.c_str());
    }

    std::string minimap_filename = filename.substr(0, filename.length() - 4);
    viewparams->location_minimap = assets->GetImage_Solid(minimap_filename);

    std::string odm_filename = std::string(filename);
    odm_filename.replace(odm_filename.length() - 4, 4, ".odm");

    BlobDeserializer stream(pGames_LOD->LoadCompressed(odm_filename));

    stream.ReadSizedString(&this->level_filename, 32);
    stream.ReadSizedString(&this->location_filename, 32);
    stream.ReadSizedString(&this->location_file_description, 32);
    stream.ReadSizedString(&this->sky_texture_filename, 32);
    stream.ReadSizedString(&this->ground_tileset, 32);

    static_assert(sizeof(pTileTypes) == 16, "Wrong type size");
    stream.ReadRaw(&pTileTypes);

    LoadTileGroupIds();
    LoadRoadTileset();
    this->ground_tileset = "grastyl";

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    // *******************Terrain**************************//
    stream.ReadRaw(&pTerrain.pHeightmap);  // карта высот
    stream.ReadRaw(&pTerrain.pTilemap);  // карта тайлов
    stream.ReadRaw(&pTerrain.pAttributemap);  // карта аттрибутов

    pTerrain.FillDMap(0, 0, 128, 128);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    uint32_t uNumTerrainNormals;
    stream.ReadRaw(&uNumTerrainNormals);  // количество нормалей
    stream.ReadRaw(&pTerrainSomeOtherData);
    stream.ReadRaw(&pTerrainNormalIndices);  // индексы нормалей
    stream.ReadSizedVector(&pTerrainNormals, uNumTerrainNormals);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    // ************BModels************************//
    pBModels.Load(&stream);
    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    // ******************Decorations**********************//
    static_assert(sizeof(LevelDecoration) == 32);
    stream.ReadVector(&pLevelDecorations);

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < pLevelDecorations.size(); ++i) {
        std::string name;
        stream.ReadSizedString(&name, 32);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(name);
    }

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    stream.ReadVector(&pFaceIDLIST);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    stream.ReadRaw(&pOMAP);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    stream.ReadLegacyVector<SpawnPoint_MM7>(&pSpawnPoints);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    // ****************.ddm file*********************//

    std::string ddm_filename = filename;
    ddm_filename = ddm_filename.replace(ddm_filename.length() - 4, 4, ".ddm");
    Blob blob = pNew_LOD->LoadCompressed(ddm_filename);

    if (blob) {
        stream.Reset(blob);

        static_assert(sizeof(DDM_DLV_Header) == 40, "Wrong type size");
        stream.ReadRaw(&ddm);
    }
    uint actualNumFacesInLevel = 0;
    for (BSPModel &model : pBModels) {
        actualNumFacesInLevel += model.pFaces.size();
    }

    //  The ddm.uNumX values are only written in SaveLoad::Save, and
    //  only used for this check. Is it for forwards compatibility?
    bool object_count_in_level_changed_since_save =
        ddm.uNumFacesInBModels &&
        ddm.uNumBModels &&
        ddm.uNumDecorations &&
        (ddm.uNumFacesInBModels != actualNumFacesInLevel ||
         ddm.uNumBModels != pBModels.size() ||
         ddm.uNumDecorations != pLevelDecorations.size());

    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)
        respawn_interval_days = 0x1BAF800;

    bool should_respawn =
        days_played - ddm.uLastRepawnDay >= respawn_interval_days ||
        !ddm.uLastRepawnDay;

    std::array<char, 968> Src {};
    std::array<char, 968> Dst {};

    if (object_count_in_level_changed_since_save || should_respawn) {
        if (object_count_in_level_changed_since_save) {
            Dst.fill(0);
            Src.fill(0);
        }
        if (should_respawn) {
            stream.ReadRaw(&Dst);
            stream.ReadRaw(&Src);
        }

        ddm.uLastRepawnDay = days_played;
        if (!object_count_in_level_changed_since_save)
            ++ddm.uNumRespawns;

        *outdoors_was_respawned = true;
        stream.Reset(pGames_LOD->LoadCompressed(ddm_filename));
        stream.SkipBytes(sizeof(DDM_DLV_Header));
    } else {
        *outdoors_was_respawned = 0;
    }
    stream.ReadRaw(&uFullyRevealedCellOnMap);
    stream.ReadRaw(&uPartiallyRevealedCellOnMap);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    if (*outdoors_was_respawned) {
        memcpy(uFullyRevealedCellOnMap, &Dst, 968);
        memcpy(uPartiallyRevealedCellOnMap, &Src, 968);
    }

    for (BSPModel &model : pBModels) {
        for (ODMFace &face : model.pFaces)
            stream.ReadRaw(&face.uAttributes);

        for (ODMFace &face : model.pFaces) {
            if (face.sCogTriggeredID) {
                if (face.HasEventHint()) {
                    face.uAttributes |= FACE_HAS_EVENT;
                } else {
                    face.uAttributes &= ~FACE_HAS_EVENT;
                }
            }
        }

        // calculate bounding sphere for model
        Vec3f topLeft = Vec3f(model.pBoundingBox.x1, model.pBoundingBox.y1, model.pBoundingBox.z1);
        Vec3f bottomRight = Vec3f(model.pBoundingBox.x2, model.pBoundingBox.y2, model.pBoundingBox.z2);
        model.vBoundingCenter = ((topLeft + bottomRight) / 2.0f).ToInt();
        model.sBoundingRadius = (topLeft - model.vBoundingCenter.ToFloat()).Length();
    }

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    for (uint i = 0; i < pLevelDecorations.size(); ++i)
        stream.ReadRaw(&pLevelDecorations[i].uFlags);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки
    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    stream.ReadLegacyVector<Actor_MM7>(&pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки
    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    stream.ReadLegacyVector<SpriteObject_MM7>(&pSpriteObjects);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    stream.ReadVector(&vChests);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    static_assert(sizeof(stru_5E4C90_MapPersistVars) == 0xC8);
    stream.ReadRaw(&stru_5E4C90_MapPersistVars);

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    static_assert(sizeof(loc_time) == 0x38);
    stream.ReadRaw(&loc_time);

    pTileTable->InitializeTileset(Tileset_Dirt);
    pTileTable->InitializeTileset(Tileset_Snow);
    pTileTable->InitializeTileset(pTileTypes[0].tileset);
    pTileTable->InitializeTileset(pTileTypes[1].tileset);
    pTileTable->InitializeTileset(pTileTypes[2].tileset);
    pTileTable->InitializeTileset(pTileTypes[3].tileset);
    this->ground_tileset = byte_6BE124_cfg_textures_DefaultGroundTexture.data();
    TileDesc *v98 = pTileTable->GetTileById(pTileTypes[0].uTileID);

    main_tile_texture = v98->GetTexture();
    // sMainTile_BitmapID = pBitmaps_LOD->LoadTexture(v98->pTileName,
    // TEXTURE_DEFAULT); if (sMainTile_BitmapID != -1)
    //    pBitmaps_LOD->pTextures[sMainTile_BitmapID].palette_id2 =
    //    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[sMainTile_BitmapID].palette_id1);

    _47F0E2();

    // LABEL_150:
    if (pWeather->bRenderSnow) {  // Ritor1: it's include for snow
        strcpy(loc_time.sky_texture_name, "sky19");
    } else if (loc_time.last_visit) {
        if (loc_time.last_visit.GetDays() % 28 != pParty->uCurrentDayOfMonth) {
            int sky_to_use;
            if (vrng->Random(100) >= 20)
                sky_to_use = dword_4EC268[vrng->Random(dword_4EC2A8)];
            else
                sky_to_use = dword_4EC28C[vrng->Random(dword_4EC2AC)];
            sprintf(loc_time.sky_texture_name, "plansky%d", sky_to_use);
        }
    } else {
        strcpy(loc_time.sky_texture_name, "plansky3");
    }

    this->sky_texture = assets->GetBitmap(loc_time.sky_texture_name);

    //pPaletteManager->RecalculateAll();
    //    pSoundList->LoadSound(SOUND_RunDirt, 0);  //For Dirt
    //    tyle(для звука хождения по грязи)
    //    pSoundList->LoadSound(SOUND_WalkDirt, 0);  //для
    //    бега
    //    pSoundList->LoadSound(SOUND_RunRoad, 0);  //для звука
    //    хождения по дороге pSoundList->LoadSound(SOUND_WalkRoad, 0);
    //    pSoundList->LoadSound(SOUND_RunWood, 0);  //для звука
    //    хождения по дереву pSoundList->LoadSound(SOUND_WalkWood, 0);
    for (int i = 0; i < 3; ++i) {
        switch (pTileTypes[i].tileset) {
            case Tileset_Grass:
                //            pSoundList->LoadSound(SOUND_RunGrass, 0);  //для
                //            звука хождения по траве
                //            pSoundList->LoadSound(SOUND_WalkGrass, 0);
                break;
            case Tileset_Snow:
                //            pSoundList->LoadSound(SOUND_RunSnow, 0);  //по
                //            снегу pSoundList->LoadSound(SOUND_WalkSnow, 0);
                break;
            case Tileset_Desert:
                //            pSoundList->LoadSound(SOUND_RunDesert, 0);  //по
                //            пустыне pSoundList->LoadSound(SOUND_WalkDesert,
                //            0);
                break;
            case Tileset_CooledLava:
                //            pSoundList->LoadSound(SOUND_RunCooledLava, 0);//по
                //            лаве pSoundList->LoadSound(SOUND_WalkCooledLava,
                //            0);
                break;
            case Tileset_Water:
                //            pSoundList->LoadSound(SOUND_RunWater, 0);  //по
                //            воде pSoundList->LoadSound(SOUND_WalkWater, 0);
                break;
            case Tileset_Badlands:
                //            pSoundList->LoadSound(SOUND_RunBadlands, 0);
                //            //для звука ходьбы по бесплодным землям
                //            pSoundList->LoadSound(SOUND_WalkBadlands, 0);
                break;
            case Tileset_Swamp:
                //            pSoundList->LoadSound(SOUND_RunSwamp, 0);  //по
                //            болоту pSoundList->LoadSound(SOUND_WalkSwamp, 0);
                break;
            default:
                break;
        }
    }

    return true;
}

int OutdoorLocation::GetTileIdByTileMapId(signed int a2) {
    int result;  // eax@2
    int v3;             // eax@3

    if (a2 >= 90) {
        v3 = (a2 - 90) / 36;
        if (v3 && v3 != 1 && v3 != 2) {
            if (v3 == Tileset_CooledLava)
                result = this->pTileTypes[3].uTileID;
            else
                result = a2;
        } else {
            result = this->pTileTypes[v3].uTileID;
        }
    } else {
        result = 0;
    }
    return result;
}

//----- (0047ED08) --------------------------------------------------------
TileDesc *OutdoorLocation::GetTileDescByGrid(int sX, int sY) {
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

    if (engine->config->graphics.SeasonsChange.Get()) {
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

    return &pTileTable->pTiles[v3];
}

//----- (0047ED83) --------------------------------------------------------
int OutdoorLocation::_47ED83(signed int gridX, signed int gridY) {
    if (gridX < 0 || gridX > 127 || gridY < 0 || gridY > 127)
        return 0;

    return this->pTerrain.pTilemap[128 * gridY + gridX];
}

//----- (0047EDB3) --------------------------------------------------------
int OutdoorLocation::GetTileAttribByGrid(int gridX, int gridY) {
    if (gridX < 0 || gridX > 127 || gridY < 0 || gridY > 127)
        return 0;

    int v3 = this->pTerrain.pTilemap[gridY * 128 + gridX];
    if (v3 >= 90)
        v3 = v3 + this->pTileTypes[(v3 - 90) / 36].uTileID - 36 * ((v3 - 90) / 36) - 90;
    return pTileTable->pTiles[v3].uAttributes;
}

//----- (0047EE16) --------------------------------------------------------
int OutdoorLocation::DoGetHeightOnTerrain(signed int sX, signed int sZ) {
    if (sX < 0 || sX > 127 || sZ < 0 || sZ > 127)
        return 0;

    return 32 * pTerrain.pHeightmap[sZ * 128 + sX];
}

//----- (0047EE49) --------------------------------------------------------
int OutdoorLocation::GetSoundIdByPosition(signed int X_pos, signed int Y_pos,
                                          int running) {
    signed int v4;  // eax@5
    signed int v5;  // eax@7
                    //  int v6; // eax@8
    signed int v8;  // eax@9
    int modif = 0;

    if (X_pos < 0 || X_pos > 127 || Y_pos < 0 || Y_pos > 127)
        v4 = 0;
    else
        v4 = this->pTerrain.pTilemap[128 * Y_pos + X_pos];
    v5 = GetTileIdByTileMapId(v4);
    if (running) modif = -39;
    if (!v5) return 92 + modif;

    switch (pTileTable->pTiles[v5].tileset) {
        case 0:
            return 93 + modif;
        case 1:
            return 97 + modif;
        case 2:
            return 91 + modif;
        case 3:
            return 90 + modif;
        case 4:
            return 101 + modif;
        case 5:
            return 95 + modif;
        case 6:
            return 88 + modif;
        case 7:
            return 100 + modif;
        case 8:
            return 93 + modif;
        default:
            v8 = pTileTable->pTiles[v5].tileset;
            if ((v8 > 9 && v8 <= 17) || (v8 > 21 && v8 <= 27))
                return 96 + modif;
            else
                return 95 + modif;
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

//----- (0047F0E2) --------------------------------------------------------
bool OutdoorLocation::_47F0E2() {
    for (uint i = 0; i < (signed int)pBitmaps_LOD->uNumLoadedFiles; ++i) {
        // if ( i != -1 ? (int)&pBitmaps_LOD->pTextures[i] : 0 )
        pBitmaps_LOD->pTextures[i].header.uDecompressedSize =
            this->pTerrain._47CB57(
                pBitmaps_LOD->pTextures[i].paletted_pixels,
                pBitmaps_LOD->pTextures[i].header.palette_id2,
                pBitmaps_LOD->pTextures[i].header.uTextureWidth *
                pBitmaps_LOD->pTextures[i].header.uTextureHeight);
    }
    return 1;
}

//----- (0047F138) --------------------------------------------------------
bool OutdoorLocation::PrepareDecorations() {
    int v1 = 0;
    int v8 = 0;
    if (pCurrentMapName == "out09.odm") {
        v8 = 1;
    }

    for (uint i = 0; i < pLevelDecorations.size(); ++i) {
        LevelDecoration *decor = &pLevelDecorations[i];

        pDecorationList->InitializeDecorationSprite(decor->uDecorationDescID);
        const DecorationDesc *decoration = pDecorationList->GetDecoration(decor->uDecorationDescID);
        if (decoration->uSoundID && _6807E0_num_decorations_with_sounds_6807B8 < 9) {
            // pSoundList->LoadSound(decoration.uSoundID, 0);
            _6807B8_level_decorations_ids[_6807E0_num_decorations_with_sounds_6807B8++] = i;
        }
        if (v8 && decor->uCog == 20)
            decor->uFlags |= LEVEL_DECORATION_OBELISK_CHEST;
        if (!decor->uEventID) {
            if (decor->IsInteractive()) {
                if (v1 < 124) {
                    decor->_idx_in_stru123 = v1 + 75;
                    if (!stru_5E4C90_MapPersistVars._decor_events[v1++])
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
                        pSpriteObjects[i].containing_item.uEnchantmentType = grng->Random(15) + 5;
                    pItemTable->SetSpecialBonus(&pSpriteObjects[i].containing_item);
                }
            }
        }
    }
    pGameLoadingUI_ProgressBar->Progress();
}

//----- (0047F2D3) --------------------------------------------------------
bool OutdoorLocation::InitalizeActors(int a1) {
    bool alert_status;  // [sp+348h] [bp-8h]@1
                       //  int v9; // [sp+34Ch] [bp-4h]@1

    alert_status = false;
    for (int i = 0; i < pActors.size(); ++i) {
        if (!(pActors[i].uAttributes & ACTOR_UNKNOW7)) {
            if (!alert_status) {
                pActors[i].uCurrentActionTime = 0;
                pActors[i].uCurrentActionLength = 0;
                if (pActors[i].uAttributes & ACTOR_UNKNOW11)
                    pActors[i].uAIState = AIState::Disabled;
                if (pActors[i].uAIState != AIState::Removed &&
                    pActors[i].uAIState != AIState::Disabled &&
                    (pActors[i].sCurrentHP == 0 ||
                     pActors[i].pMonsterInfo.uHP == 0))
                    pActors[i].uAIState = AIState::Dead;
                pActors[i].vVelocity.x = 0;
                pActors[i].vVelocity.y = 0;
                pActors[i].vVelocity.z = 0;
                pActors[i].UpdateAnimation();
                pActors[i].pMonsterInfo.uHostilityType =
                    MonsterInfo::Hostility_Friendly;
                pActors[i].PrepareSprites(0);
            } else {
                pActors[i].uAIState = AIState::Disabled;
                pActors[i].uAttributes |= ACTOR_UNKNOW11;
            }
        } else if (a1 == 0) {
            pActors[i].uAIState = AIState::Disabled;
            pActors[i].uAttributes |= ACTOR_UNKNOW11;
        } else if (alert_status) {
            pActors[i].uCurrentActionTime = 0;
            pActors[i].uCurrentActionLength = 0;
            if (pActors[i].uAttributes & ACTOR_UNKNOW11)
                pActors[i].uAIState = AIState::Disabled;
            if (pActors[i].uAIState != AIState::Removed &&
                pActors[i].uAIState != AIState::Disabled &&
                (pActors[i].sCurrentHP == 0 ||
                 pActors[i].pMonsterInfo.uHP == 0))
                pActors[i].uAIState = AIState::Dead;
            pActors[i].vVelocity.x = 0;
            pActors[i].vVelocity.y = 0;
            pActors[i].vVelocity.z = 0;
            pActors[i].UpdateAnimation();
            pActors[i].pMonsterInfo.uHostilityType =
                MonsterInfo::Hostility_Friendly;
            pActors[i].PrepareSprites(0);
        } else {
            pActors[i].uAIState = AIState::Disabled;
            pActors[i].uAttributes |= ACTOR_UNKNOW11;
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
        pActors[i].uAttributes &= ~ACTOR_VISIBLE;
        if (pActors[i].uAIState == Removed || pActors[i].uAIState == Disabled) {
            continue;
        }

        if (uNumBillboardsToDraw >= 500) return;

        // view culling
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            bool onlist = false;
            for (uint j = 0; j < pBspRenderer->uNumVisibleNotEmptySectors; j++) {
                if (pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[j] == pActors[i].uSectorID) {
                    onlist = true;
                    break;
                }
            }
            if (!onlist) continue;
        } else {
            if (!IsCylinderInFrustum(pActors[i].vPosition.ToFloat(), pActors[i].uActorRadius)) continue;
        }

        int z = pActors[i].vPosition.z;
        int x = pActors[i].vPosition.x;
        int y = pActors[i].vPosition.y;

        Angle_To_Cam = TrigLUT.Atan2(
            pActors[i].vPosition.x - pCamera3D->vCameraPos.x,
            pActors[i].vPosition.y - pCamera3D->vCameraPos.y);

        Sprite_Octant = ((signed int)(TrigLUT.uIntegerPi +
                            ((signed int)TrigLUT.uIntegerPi >> 3) + pActors[i].uYawAngle -
                            Angle_To_Cam) >> 8) & 7;

        Cur_Action_Time = pActors[i].uCurrentActionTime;
        if (pParty->bTurnBasedModeOn) {
            if (pActors[i].uCurrentActionAnimation == ANIM_Walking)
                Cur_Action_Time = 32 * i + pMiscTimer->uTotalGameTimeElapsed;
        } else {
            if (pActors[i].uCurrentActionAnimation == ANIM_Walking)
                Cur_Action_Time = 32 * i + pEventTimer->uTotalGameTimeElapsed;
        }

        if (pActors[i].pActorBuffs[ACTOR_BUFF_STONED].Active() ||
            pActors[i].pActorBuffs[ACTOR_BUFF_PARALYZED].Active())
            Cur_Action_Time = 0;

        int v49 = 0;
        float v4 = 0.0f;
        if (pActors[i].uAIState == Summoned) {
            if (PID_TYPE(pActors[i].uSummonerID) != OBJECT_Actor ||
                pActors[PID_ID(pActors[i].uSummonerID)]
                .pMonsterInfo.uSpecialAbilityDamageDiceSides != 1) {
                z += floorf(pActors[i].uActorHeight * 0.5f + 0.5f);
            } else {
                v49 = 1;
                spell_fx_renderer->_4A7F74(pActors[i].vPosition.x, pActors[i].vPosition.y, z);
                v4 = (1.0 - (double)pActors[i].uCurrentActionTime /
                    (double)pActors[i].uCurrentActionLength) *
                    (double)(2 * pActors[i].uActorHeight);
                z -= floorf(v4 + 0.5f);
                if (z > pActors[i].vPosition.z) z = pActors[i].vPosition.z;
            }
        }


        if (pActors[i].uAIState == Summoned && !v49)
            frame = pSpriteFrameTable->GetFrame(uSpriteID_Spell11, Cur_Action_Time);
        else if (pActors[i].uAIState == Resurrected)
            frame = pSpriteFrameTable->GetFrameBy_x(
                pActors[i].pSpriteIDs[pActors[i].uCurrentActionAnimation], Cur_Action_Time);
        else
            frame = pSpriteFrameTable->GetFrame(
                pActors[i].pSpriteIDs[pActors[i].uCurrentActionAnimation], Cur_Action_Time);

        // no sprite frame to draw
        if (frame->icon_name == "null") continue;
        if (frame->hw_sprites[Sprite_Octant]->texture->GetHeight() == 0 || frame->hw_sprites[Sprite_Octant]->texture->GetWidth() == 0)
            __debugbreak();

        int flags = 0;
        // v16 = (int *)frame->uFlags;
        if (frame->uFlags & 2) flags = 2;
        if (frame->uFlags & 0x40000) flags |= 0x40;
        if (frame->uFlags & 0x20000) flags |= 0x80;
        if ((256 << Sprite_Octant) & frame->uFlags) flags |= 4;
        if (frame->uGlowRadius) {
            pMobileLightsStack->AddLight(Vec3f(x, y, z), pActors[i].uSectorID, frame->uGlowRadius, 0xFFu,
                                         0xFFu, 0xFFu, _4E94D3_light_type);
        }

        int view_x = 0, view_y = 0, view_z = 0;
        bool visible = pCamera3D->ViewClip(x, y, z, &view_x, &view_y, &view_z);

        if (visible) {
            if (2 * abs(view_x) >= abs(view_y)) {
                int projected_x = 0;
                int projected_y = 0;
                pCamera3D->Project(view_x, view_y, view_z, &projected_x, &projected_y);

                float proj_scale = frame->scale * (pCamera3D->ViewPlaneDist_X) / (view_x);
                int screen_space_half_width = static_cast<int>(proj_scale * frame->hw_sprites[Sprite_Octant]->uBufferWidth / 2.0f);
                int screen_space_height = static_cast<int>(proj_scale * frame->hw_sprites[Sprite_Octant]->uBufferHeight);

                if (projected_x + screen_space_half_width >= (signed int)pViewport->uViewportTL_X &&
                    projected_x - screen_space_half_width <= (signed int)pViewport->uViewportBR_X) {
                    if (projected_y >= pViewport->uViewportTL_Y && (projected_y - screen_space_height) <= pViewport->uViewportBR_Y) { // test
                        ++uNumBillboardsToDraw;
                        ++uNumSpritesDrawnThisFrame;

                        pActors[i].uAttributes |= ACTOR_VISIBLE;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].hwsprite = frame->hw_sprites[Sprite_Octant];
                        pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID = pActors[i].uSectorID;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].uPaletteIndex = frame->GetPaletteIndex();

                        pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x = proj_scale;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = proj_scale;

                        if (pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].Active() &&
                            pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].uPower > 0) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1]
                                .screenspace_projection_factor_y = 1.0f / pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].uPower *
                                pBillboardRenderList[uNumBillboardsToDraw - 1]
                                .screenspace_projection_factor_y;
                        } else if (pActors[i].pActorBuffs[ACTOR_BUFF_MASS_DISTORTION].Active()) {
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
                            pMonsterList->pMonsters[pActors[i].pMonsterInfo.uID - 1].sTintColor;  // *((int *)&v35[v36] - 36);
                        if (pActors[i].pActorBuffs[ACTOR_BUFF_STONED].Active()) {
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
                      int *bmodel_pid, int bWaterWalk) {
    std::array<int, 20> current_Face_id;                   // dword_721110
    std::array<int, 20> current_BModel_id;                 // dword_721160
    std::array<int, 20> odm_floor_level;                   // idb
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

            int slack = engine->config->gameplay.FloorChecksEps.Get();
            if (!face.Contains(pos, model.index, slack, FACE_XY_PLANE))
                continue;

            int floor_level;
            if (face.uPolygonType == POLYGON_Floor) {
                floor_level = model.pVertices[face.pVertexIDs[0]].z;
            } else {
                floor_level = face.zCalc.Calculate(pos.x, pos.y);
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
        *bmodel_pid = 0;
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
        *bmodel_pid = 0;
    else
        *bmodel_pid = current_Face_id[current_idx] | (current_BModel_id[current_idx] << 6);

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

    int dx = abs(pos_x - grid_pos_x1);
    int dy = abs(grid_pos_y1 - pos_y);
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

    Vec3f n = Cross(side2, side1);
    float mag = n.Length();
    if (fabsf(mag) < 1e-6f) {
        *out = Vec3i(0, 0, 65536);
    } else {
        *out = (n / mag).ToFixpoint();
    }
}
//----- (0046BE0A) --------------------------------------------------------
void ODM_UpdateUserInputAndOther() {
    bool v0;        // eax@5
    char pOut[32];  // [sp+8h] [bp-20h]@5

    UpdateObjects();
    ODM_ProcessPartyActions();
    if (pParty->vPosition.x < -22528 || pParty->vPosition.x > 22528 ||
        pParty->vPosition.y < -22528 || pParty->vPosition.y > 22528) {
        pOutdoor->level_filename = pCurrentMapName;
        v0 = pOutdoor->GetTravelDestination(pParty->vPosition.x,
                                            pParty->vPosition.y, pOut, 32);
        if (!engine->IsUnderwater() && (pParty->IsAirborne() || (pParty->uFlags & (PARTY_FLAGS_1_STANDING_ON_WATER | PARTY_FLAGS_1_WATER_DAMAGE)) ||
                             pParty->uFlags & PARTY_FLAGS_1_BURNING || pParty->bFlying) ||
            !v0) {
            if (pParty->vPosition.x < -22528) pParty->vPosition.x = -22528;
            if (pParty->vPosition.x > 22528) pParty->vPosition.x = 22528;
            if (pParty->vPosition.y < -22528) pParty->vPosition.y = -22528;
            if (pParty->vPosition.y > 22528) pParty->vPosition.y = 22528;
        } else {
            pAudioPlayer->PauseSounds(-1);
            pDialogueWindow = new GUIWindow_Travel();  // TravelUI_Load();
        }
    }
    UpdateActors_ODM();
    check_event_triggers();
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

    rest_ui_sky_frame_current = assets->GetImage_ColorKey(
        fmt::format("TERRA{:03}", pParty->uCurrentMinute / 6 + 10 * pParty->uCurrentHour));
}

OutdoorLocation::OutdoorLocation() {
    this->log = EngineIoc::ResolveLogger();
    this->decal_builder = EngineIoc::ResolveDecalBuilder();
    this->spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();

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
    bool bWaterWalk = false;
    pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
    if (pParty->WaterWalkActive()) {
        bWaterWalk = true;
        stru_5E4C90_MapPersistVars._decor_events[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uOverlayID + 119] |= 1;
        if (!pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff &&
            pParty->pPlayers[pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uCaster - 1].sMana <= 0)
            bWaterWalk = false;
    }

    int bmodel_standing_on_pid;
    bool is_on_water = false;

    int curent_floor_level = ODM_GetFloorLevel(pParty->vPosition, pParty->uPartyHeight,
                                    &is_on_water, &bmodel_standing_on_pid, bWaterWalk);
    int is_not_on_bmodel = bmodel_standing_on_pid == 0;
    int ground_level = curent_floor_level + 1;

    bool bFeatherFall = pParty->FeatherFallActive() || pParty->WearsItemAnywhere(ITEM_ARTIFACT_LADYS_ESCORT);
    if (bFeatherFall)
        pParty->uFallStartZ = curent_floor_level;
    else
        curent_floor_level = pParty->uFallStartZ;

    int faceID_ceiling = 0;
    int ceiling_height = -1;

    if (pParty->bFlying)
        ceiling_height = GetCeilingHeight(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z + pParty->uPartyHeight, &faceID_ceiling);

    bool hovering = false;
    if (pParty->vPosition.z <= ground_level) {  // landing from flight
        ceiling_height = -1;
        pParty->bFlying = false;
        pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
    } else {
        hovering = true;
    }
    bool not_high_fall = pParty->vPosition.z - ground_level <= 32;

    // check if we should be flying
    if (!engine->IsUnderwater() && !pParty->pPartyBuffs[PARTY_BUFF_FLY].Active())
        pParty->bFlying = false;

    // is party standing on any trigger faces
    int trigger_id{ 0 };
    if (!hovering) {
        if (pParty->floor_face_pid != PID(OBJECT_Face, bmodel_standing_on_pid) && bmodel_standing_on_pid) {
            int BModel_id = bmodel_standing_on_pid >> 6;
            if (BModel_id < pOutdoor->pBModels.size()) {
                int face_id = bmodel_standing_on_pid & 0x3F;
                if (pOutdoor->pBModels[BModel_id].pFaces[face_id].uAttributes & FACE_PRESSURE_PLATE) {
                    trigger_id = pOutdoor->pBModels[BModel_id].pFaces[face_id].sCogTriggeredID;
                }
            }
        }
        pParty->floor_face_pid = PID(OBJECT_Face, bmodel_standing_on_pid);
    }

    // set params before input
    int party_y_speed = 0;
    int party_x_speed = 0;
    int party_z_speed = pParty->uFallSpeed;
    if (pParty->bFlying) {
        party_z_speed = 0;
    }
    int save_old_flight_height = pParty->sPartySavedFlightZ;

    bool partyAtHighSlope = IsTerrainSlopeTooHigh(pParty->vPosition.x, pParty->vPosition.y);
    bool party_running_flag = false;
    bool party_walking_flag = false;
    bool bNoFlightBob = false;

    int party_new_rot_Z = pParty->sRotationZ;
    int party_new_rot_y = pParty->sRotationY;

    int64_t dturn = ((int64_t) pEventTimer->dt_fixpoint * pParty->y_rotation_speed * TrigLUT.uIntegerPi / 180) >> 16;
    while (pPartyActionQueue->uNumActions) {
        switch (pPartyActionQueue->Next()) {
            case PARTY_FlyUp:
            {
                if (!pParty->FlyActive() && !engine->IsUnderwater()) break;

                pParty->bFlying = false;
                if (engine->IsUnderwater() ||
                    pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff ||
                    (pParty->pPlayers[pParty->pPartyBuffs[PARTY_BUFF_FLY].uCaster - 1].sMana > 0 ||
                    engine->config->debug.AllMagic.Get())) {
                    if (pParty->sPartySavedFlightZ < engine->config->gameplay.MaxFlightHeight.Get() || hovering) {
                        pParty->bFlying = true;
                        pParty->uFallSpeed = 0;
                        bNoFlightBob = true;
                        pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
                        if (pParty->sPartySavedFlightZ < engine->config->gameplay.MaxFlightHeight.Get()) {
                            party_z_speed = pParty->uWalkSpeed * 4;
                            save_old_flight_height = pParty->vPosition.z;
                        }
                    }
                }
            } break;

            case PARTY_FlyDown:
                if (pParty->FlyActive() || engine->IsUnderwater()) {
                    pParty->bFlying = false;
                    if (engine->IsUnderwater() ||
                        pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff ||
                        (pParty->pPlayers[pParty->pPartyBuffs[PARTY_BUFF_FLY].uCaster - 1].sMana > 0 ||
                        engine->config->debug.AllMagic.Get())) {
                        save_old_flight_height = pParty->vPosition.z;
                        pParty->uFallSpeed = 0;
                        party_z_speed = -pParty->uWalkSpeed * 4;
                        pParty->bFlying = true;
                        bNoFlightBob = true;
                        pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
                    }
                }
                break;

            case PARTY_TurnLeft:
                if (engine->config->settings.TurnSpeed.Get() > 0)
                    party_new_rot_Z += engine->config->settings.TurnSpeed.Get();  // discrete turn
                else
                    party_new_rot_Z += dturn * fTurnSpeedMultiplier;  // time-based smooth turn

                party_new_rot_Z &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_TurnRight:
                if (engine->config->settings.TurnSpeed.Get() > 0)
                    party_new_rot_Z -= engine->config->settings.TurnSpeed.Get();
                else
                    party_new_rot_Z -= dturn * fTurnSpeedMultiplier;

                party_new_rot_Z &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_FastTurnLeft:
                if (engine->config->settings.TurnSpeed.Get() > 0)
                    party_new_rot_Z += engine->config->settings.TurnSpeed.Get();
                else
                    party_new_rot_Z += 2.0f * fTurnSpeedMultiplier * dturn;

                party_new_rot_Z &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_FastTurnRight:
                if (engine->config->settings.TurnSpeed.Get() > 0)
                    party_new_rot_Z -= engine->config->settings.TurnSpeed.Get();
                else
                    party_new_rot_Z -= 2.0f * fTurnSpeedMultiplier * dturn;

                party_new_rot_Z &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_StrafeLeft:
            {
                float sin_y = sinf(2 * pi_double * party_new_rot_Z / 2048.0);
                int dx = sin_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                party_x_speed -= 3 * dx / 4;

                float cos_y = cosf(2 * pi_double * party_new_rot_Z / 2048.0);
                int dy = cos_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                party_y_speed += 3 * dy / 4;

                party_walking_flag = true;
            } break;

            case PARTY_StrafeRight:
            {
                float sin_y = sinf(2 * pi_double * party_new_rot_Z / 2048.0);
                int dx = sin_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                party_x_speed += 3 * dx / 4;

                float cos_y = cosf(2 * pi_double * party_new_rot_Z / 2048.0);
                int dy = cos_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                party_y_speed -= 3 * dy / 4;

                party_walking_flag = true;
            } break;

            case PARTY_WalkForward:
            {
                float sin_y = sinf(2 * pi_double * party_new_rot_Z / 2048.0),
                      cos_y = cosf(2 * pi_double * party_new_rot_Z / 2048.0);

                int dx = cos_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                int dy = sin_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;

                if (engine->config->debug.TurboSpeed.Get()) {
                    party_x_speed += dx * 12;
                    party_y_speed += dy * 12;
                } else {
                    party_x_speed += dx;
                    party_y_speed += dy;
                }

                party_walking_flag = true;
            } break;

            case PARTY_RunForward:
            {
                float sin_y = sinf(2 * pi_double * party_new_rot_Z / 2048.0);
                float cos_y = cosf(2 * pi_double * party_new_rot_Z / 2048.0);

                int dx = cos_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                int dy = sin_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;

                if (pParty->bFlying) {
                    if (engine->config->debug.TurboSpeed.Get()) {
                        party_x_speed += dx * 24;
                        party_y_speed += dy * 24;
                    } else {
                        party_x_speed += 4 * dx;
                        party_y_speed += 4 * dy;
                    }
                } else if (partyAtHighSlope && !bmodel_standing_on_pid) {
                    party_x_speed += dx;
                    party_y_speed += dy;
                    party_walking_flag = true;
                } else {
                    if (engine->config->debug.TurboSpeed.Get()) {
                        party_x_speed += dx * 12;
                        party_y_speed += dy * 12;
                    } else {
                        party_x_speed += 2 * dx;
                        party_y_speed += 2 * dy;
                    }

                    party_running_flag = true;
                }
            } break;

            case PARTY_WalkBackward: {
                float sin_y = sinf(2 * pi_double * party_new_rot_Z / 2048.0);
                float cos_y = cosf(2 * pi_double * party_new_rot_Z / 2048.0);

                int dx = cos_y * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                party_x_speed -= dx;

                int dy = sin_y * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                party_y_speed -= dy;
                party_walking_flag = true;
            } break;

            case PARTY_RunBackward:
            {
                float sin_y = sinf(2 * pi_double * party_new_rot_Z / 2048.0);
                float cos_y = cosf(2 * pi_double * party_new_rot_Z / 2048.0);

                int dx = cos_y * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                int dy = sin_y * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;

                if (pParty->bFlying) {
                    party_x_speed -= 4 * dx;
                    party_y_speed -= 4 * dy;
                } else {
                    party_x_speed -= dx;
                    party_y_speed -= dy;
                    party_walking_flag = true;
                }
            } break;

            case PARTY_CenterView:
                party_new_rot_y = 0;
                break;

            case PARTY_LookUp:
                party_new_rot_y += engine->config->settings.VerticalTurnSpeed.Get();
                if (party_new_rot_y > 128) party_new_rot_y = 128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_LookUp, 0);
                break;

            case PARTY_LookDown:
                party_new_rot_y -= engine->config->settings.VerticalTurnSpeed.Get();
                if (party_new_rot_y < -128) party_new_rot_y = -128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_LookDown, 0);
                break;

            case PARTY_Jump:
                if ((!partyAtHighSlope || bmodel_standing_on_pid) &&
                    // to avoid jump hesitancy when moving downhill
                    (!hovering || (pParty->vPosition.z <= ground_level + 20 && party_z_speed <= 0)) &&
                    pParty->jump_strength &&
                    !(pParty->uFlags & PARTY_FLAGS_1_WATER_DAMAGE) &&
                    !(pParty->uFlags & PARTY_FLAGS_1_BURNING)) {
                    hovering = true;
                    party_z_speed += pParty->jump_strength * 96;
                    // boost party upwards slightly so we dont "land" straight away
                    pParty->vPosition.z += 1;
                }
                break;

            case PARTY_Land:
                if (pParty->bFlying) {
                    pParty->uFlags |= PARTY_FLAGS_1_LANDING;
                    pParty->uFallSpeed = 0;
                }
                pParty->bFlying = false;
                pPartyActionQueue->uNumActions = 0;
                break;

            default:
                assert(false);
        }
    }

    // set party look angles
    pParty->sRotationZ = party_new_rot_Z;
    pParty->sRotationY = party_new_rot_y;

    int party_new_x = pParty->vPosition.x;
    int party_new_Y = pParty->vPosition.y;
    int party_new_Z = pParty->vPosition.z;

    // TODO(pskelton): this is only a partial fix for #520
    if (!pParty->FlyActive())
        pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
    //-------------------------------------------
    if (pParty->bFlying) {
        // TODO(pskelton): check tickcount usage here - bob up and down in the air
        if (bNoFlightBob) {
            party_new_Z = save_old_flight_height;
        } else {
            party_new_Z = save_old_flight_height + 4 * TrigLUT.Cos(platform->tickCount());
        }

        if (pParty->FlyActive())
            stru_5E4C90_MapPersistVars._decor_events[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].uOverlayID + 119] &= 0xFE;
        pParty->uFallStartZ = party_new_Z;
    } else if (party_new_Z < ground_level) {
        if (is_on_water && party_z_speed)
            SpriteObject::Create_Splash_Object(party_new_x, party_new_Y, ground_level);
        party_z_speed = 0;
        party_new_Z = ground_level;
        pParty->uFallStartZ = ground_level;
        save_old_flight_height = party_new_Z;
        if (pParty->FlyActive())
            stru_5E4C90_MapPersistVars._decor_events[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].uOverlayID + 119] |= 1;
    } else {
        save_old_flight_height = party_new_Z;
        if (pParty->FlyActive())
            stru_5E4C90_MapPersistVars._decor_events[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].uOverlayID + 119] |= 1;
    }
    //------------------------------------------

    bool bpartyslope{ false };
    int another_z_speed;
    if (hovering && !pParty->bFlying) {  // add gravity
        another_z_speed = party_z_speed + (-(pEventTimer->uTimeElapsed * GetGravityStrength()) << 1);
        party_z_speed += (-(pEventTimer->uTimeElapsed * GetGravityStrength()) << 1);
    } else if (!partyAtHighSlope) {
        another_z_speed = party_z_speed;
    } else if (!hovering) {
        if (!bmodel_standing_on_pid) {
            // rolling down the hill
            // how it's done: you get a little bit pushed in the air along
            // terrain normal, getting in the air and falling to the gravity,
            // gradually sliding downwards. nice trick
            party_new_Z = ground_level;
            Vec3i v98;
            ODM_GetTerrainNormalAt(party_new_x, party_new_Y, &v98);
            int v35 = party_z_speed + (8 * -(pEventTimer->uTimeElapsed * GetGravityStrength()));
            int dot = abs(party_x_speed * v98.x + party_y_speed * v98.y + v35 * v98.z) >> 16;
            party_x_speed += fixpoint_mul(dot, v98.x);
            party_y_speed += fixpoint_mul(dot, v98.y);
            another_z_speed = v35 + fixpoint_mul(dot, v98.z);
            party_z_speed = another_z_speed;
            bpartyslope = true;
        }
    } else {
        another_z_speed = party_z_speed;
    }

    if (hovering) {
        if (!engine->IsUnderwater() && another_z_speed <= 0) {
            if (another_z_speed < -500 && !pParty->bFlying &&
                pParty->vPosition.z - ground_level > 1000 &&
                !pParty->FeatherFallActive() &&
                !(pParty->uFlags & PARTY_FLAGS_1_LANDING)) {  // falling scream
                for (int i = 0; i < 4; ++i) {
                    if (!pParty->pPlayers[i].HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) &&
                        !pParty->pPlayers[i].WearsItem(ITEM_ARTIFACT_HERMES_SANDALS, ITEM_SLOT_BOOTS) &&
                        pParty->pPlayers[i].CanAct())
                        pParty->pPlayers[i].PlaySound(SPEECH_Falling, 0);
                }
            }
        }
    } else {
      pParty->uFallStartZ = party_new_Z;
    }

    if (party_x_speed * party_x_speed + party_y_speed * party_y_speed < 400 && !partyAtHighSlope) {
        party_y_speed = 0;
        party_x_speed = 0;
    }

    // --(Collisions)-------------------------------------------------------------------
    collision_state.ignored_face_id = -1;
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = pParty->radius;
    collision_state.radius_hi = pParty->radius / 2.0f;
    collision_state.check_hi = true;
    // make 100 attempts to satisfy collisions
    for (uint i = 0; i < 100; i++) {
        collision_state.position_hi.x = party_new_x;
        collision_state.position_hi.y = party_new_Y;
        collision_state.position_hi.z = (pParty->uPartyHeight - 32) + party_new_Z + 1;

        collision_state.position_lo.x = party_new_x;
        collision_state.position_lo.y = party_new_Y;
        collision_state.position_lo.z = collision_state.radius_lo + party_new_Z + 1;

        collision_state.velocity.x = party_x_speed;
        collision_state.velocity.y = party_y_speed;
        collision_state.velocity.z = party_z_speed;

        collision_state.uSectorID = 0;

        int frame_movement_dt = 0;
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
            frame_movement_dt = 13312;
        if (collision_state.PrepareAndCheckIfStationary(frame_movement_dt))
            break;

        CollideOutdoorWithModels(true);
        CollideOutdoorWithDecorations(WorldPosToGridCellX(pParty->vPosition.x), WorldPosToGridCellY(pParty->vPosition.y));
        _46ED8A_collide_against_sprite_objects(4);

        for (uint actor_id = 0; actor_id < (signed int)pActors.size(); ++actor_id)
            CollideWithActor(actor_id, 0);

        int new_pos_low_y{};
        int new_pos_low_x{};
        int new_pos_low_z{};
        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            new_pos_low_x = collision_state.new_position_lo.x;
            new_pos_low_y = collision_state.new_position_lo.y;
            new_pos_low_z = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
        } else {
            new_pos_low_x = party_new_x + collision_state.adjusted_move_distance * collision_state.direction.x;
            new_pos_low_y = party_new_Y + collision_state.adjusted_move_distance * collision_state.direction.y;
            new_pos_low_z = party_new_Z + collision_state.adjusted_move_distance * collision_state.direction.z;
        }

        int allnewfloor = ODM_GetFloorLevel(Vec3i(new_pos_low_x, new_pos_low_y, new_pos_low_z), pParty->uPartyHeight, &is_on_water, &bmodel_standing_on_pid, 0);
        int party_y_pid;
        int x_advance_floor = ODM_GetFloorLevel(Vec3i(new_pos_low_x, party_new_Y, new_pos_low_z), pParty->uPartyHeight, &is_on_water, &party_y_pid, 0);
        int party_x_pid;
        int y_advance_floor = ODM_GetFloorLevel(Vec3i(party_new_x, new_pos_low_y, new_pos_low_z), pParty->uPartyHeight, &is_on_water, &party_x_pid, 0);
        bool terr_slope_advance_x = IsTerrainSlopeTooHigh(new_pos_low_x, party_new_Y);
        bool terr_slope_advance_y = IsTerrainSlopeTooHigh(party_new_x, new_pos_low_y);

        is_not_on_bmodel = false;
        if (!party_y_pid && !party_x_pid && !bmodel_standing_on_pid) is_not_on_bmodel = true;

        int move_in_y = 1;
        int move_in_x = 1;
        if (engine->IsUnderwater() || !is_not_on_bmodel) {
            party_new_x = new_pos_low_x;
            party_new_Y = new_pos_low_y;
        } else {
            if (terr_slope_advance_x && x_advance_floor > party_new_Z) move_in_x = 0;
            if (terr_slope_advance_y && y_advance_floor > party_new_Z) move_in_y = 0;

            if (move_in_x) {
                party_new_x = new_pos_low_x;
                if (move_in_y) party_new_Y = new_pos_low_y;
            } else if (move_in_y) {
                party_new_Y = new_pos_low_y;
            } else {
                if (IsTerrainSlopeTooHigh(new_pos_low_x, new_pos_low_y) && allnewfloor <= party_new_Z) {
                    // move down the hill is allowed
                    party_new_x = new_pos_low_x;
                    party_new_Y = new_pos_low_y;
                }
            }
        }

        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            if (!is_not_on_bmodel) {
                party_new_x = collision_state.new_position_lo.x;
                party_new_Y = collision_state.new_position_lo.y;
            }
            party_new_Z = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
            break;
        }

        collision_state.total_move_distance += collision_state.adjusted_move_distance;
        party_new_x = new_pos_low_x;
        party_new_Y = new_pos_low_y;
        party_new_Z = new_pos_low_z;
        int collisionPID = collision_state.pid;

        if (PID_TYPE(collision_state.pid) == OBJECT_Actor) {
            if (pParty->Invisible())
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
        }

        if (PID_TYPE(collision_state.pid) == OBJECT_Decoration) {
            int atanDecoration = TrigLUT.Atan2(
                new_pos_low_x - pLevelDecorations[(signed int)collision_state.pid >> 3].vPosition.x,
                new_pos_low_y - pLevelDecorations[(signed int)collision_state.pid >> 3].vPosition.y);
            party_x_speed = TrigLUT.Cos(atanDecoration) * integer_sqrt(party_x_speed * party_x_speed + party_y_speed * party_y_speed);
            party_y_speed = TrigLUT.Sin(atanDecoration) * integer_sqrt(party_x_speed * party_x_speed + party_y_speed * party_y_speed);
        }

        if (PID_TYPE(collision_state.pid) == OBJECT_Face) {
            pParty->bFlying = false;
            BSPModel *pModel = &pOutdoor->pBModels[(signed int)collision_state.pid >> 9];
            ODMFace *pODMFace = &pModel->pFaces[((signed int)collision_state.pid >> 3) & 0x3F];
            int bSmallZDelta = (pODMFace->pBoundingBox.z2 - pODMFace->pBoundingBox.z1) <= 32;
            bool bFaceSlopeTooSteep = pODMFace->pFacePlaneOLD.vNormal.z < 46378;

            if (engine->IsUnderwater())
                bFaceSlopeTooSteep = false;

            if (pODMFace->uPolygonType == POLYGON_Floor) {
                if (party_z_speed < 0) party_z_speed = 0;
                party_new_Z = pModel->pVertices[pODMFace->pVertexIDs[0]].z + 1;
                if (party_x_speed * party_x_speed + party_y_speed * party_y_speed < 400) {
                    party_x_speed = 0;
                    party_y_speed = 0;
                }
                if (pParty->floor_face_pid != collisionPID &&
                    pODMFace->Pressure_Plate()) {
                    pParty->floor_face_pid = collisionPID;
                    trigger_id = pODMFace->sCogTriggeredID;  // this one triggers tour
                                                             // events??
                }
            }

            if (!bSmallZDelta && (pODMFace->uPolygonType != POLYGON_InBetweenFloorAndWall || bFaceSlopeTooSteep)) {  // упёрся в столб
                int dot = abs(party_y_speed * pODMFace->pFacePlaneOLD.vNormal.y +
                           party_z_speed * pODMFace->pFacePlaneOLD.vNormal.z +
                           party_x_speed * pODMFace->pFacePlaneOLD.vNormal.x) >> 16;
                if ((collision_state.speed / 8) > dot)
                    dot = collision_state.speed / 8;
                party_x_speed += fixpoint_mul(dot, pODMFace->pFacePlaneOLD.vNormal.x);
                party_y_speed += fixpoint_mul(dot, pODMFace->pFacePlaneOLD.vNormal.y);
                int v54 = 0;
                if (!bFaceSlopeTooSteep)
                    v54 = fixpoint_mul(dot, pODMFace->pFacePlaneOLD.vNormal.z);
                pParty->uFallSpeed += v54;
                int v55 = collision_state.radius_lo -
                    pODMFace->pFacePlaneOLD.SignedDistanceTo(new_pos_low_x, new_pos_low_y, new_pos_low_z);
                if (v55 > 0) {
                    party_new_x = new_pos_low_x + fixpoint_mul(pODMFace->pFacePlaneOLD.vNormal.x, v55);
                    party_new_Y = new_pos_low_y + fixpoint_mul(pODMFace->pFacePlaneOLD.vNormal.y, v55);
                    if (!bFaceSlopeTooSteep)
                        party_new_Z = new_pos_low_z + fixpoint_mul(pODMFace->pFacePlaneOLD.vNormal.z, v55);
                }
                if (pParty->floor_face_pid != collision_state.pid &&
                    pODMFace->Pressure_Plate()) {
                    pParty->floor_face_pid = collision_state.pid;
                    trigger_id = pODMFace->sCogTriggeredID;  //
                }
            }

            if (pODMFace->uPolygonType == POLYGON_InBetweenFloorAndWall) {
                int dot = abs(party_y_speed * pODMFace->pFacePlaneOLD.vNormal.y +
                           party_z_speed * pODMFace->pFacePlaneOLD.vNormal.z +
                           party_x_speed * pODMFace->pFacePlaneOLD.vNormal.x) >> 16;
                if ((collision_state.speed / 8) > dot)
                    dot = collision_state.speed / 8;
                party_x_speed += fixpoint_mul(dot, pODMFace->pFacePlaneOLD.vNormal.x);
                party_y_speed += fixpoint_mul(dot, pODMFace->pFacePlaneOLD.vNormal.y);
                party_z_speed += fixpoint_mul(dot, pODMFace->pFacePlaneOLD.vNormal.z);
                bpartyslope = true;
                if (party_x_speed * party_x_speed + party_y_speed * party_y_speed >= 400) {
                    if (pParty->floor_face_pid != collision_state.pid &&
                        pODMFace->Pressure_Plate()) {
                        pParty->floor_face_pid = collision_state.pid;
                        trigger_id = pODMFace->sCogTriggeredID;  //
                    }
                } else {
                    party_x_speed = 0;
                    party_z_speed = 0;
                    party_y_speed = 0;
                }
            }
        }

        // ~0.9x reduce party speed and try again
        party_x_speed = fixpoint_mul(58500, party_x_speed);
        party_y_speed = fixpoint_mul(58500, party_y_speed);
        party_z_speed = fixpoint_mul(58500, party_z_speed);
    }
    if (bpartyslope)
        pParty->uFallStartZ = party_new_Z;

    // walking / running sounds ------------------------
    if (engine->config->settings.WalkSound.Get() && pParty->walk_sound_timer) {
        if (pParty->walk_sound_timer >= pEventTimer->uTimeElapsed)
            pParty->walk_sound_timer -= pEventTimer->uTimeElapsed;
        else
            pParty->walk_sound_timer = 0;
    }

    // save up distance deltas so walks sounds play at high fps with small delta
    int pX_ = pParty->vPosition.x - party_new_x;
    int pY_ = pParty->vPosition.y - party_new_Y;
    int pZ_ = pParty->vPosition.z - party_new_Z;
    pParty->_movementTally += integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_);

    if (engine->config->settings.WalkSound.Get() && pParty->walk_sound_timer <= 0) {
        pAudioPlayer->StopAll(804);  // stop sound
        if (party_running_flag && (!hovering || not_high_fall)) {
            if (pParty->_movementTally >= 16) {
                pParty->_movementTally = 0;
                if (!is_not_on_bmodel &&
                    pOutdoor->pBModels[pParty->floor_face_pid >> 9]
                    .pFaces[(pParty->floor_face_pid >> 3) & 0x3F].Visible()) {
                    pAudioPlayer->PlaySound(SOUND_RunWood, PID_INVALID /*804*/, 1, -1, 0, 0);  // бег на 3D Modelи
                } else {
                    int v87 = pOutdoor->GetSoundIdByPosition(
                        WorldPosToGridCellX(pParty->vPosition.x), WorldPosToGridCellY(pParty->vPosition.y), 1);
                    pAudioPlayer->PlaySound((SoundID)v87, PID_INVALID /*804*/, 1, -1, 0, 0);  // бег по земле 56
                }
                pParty->walk_sound_timer = 96;  // таймер для бега
            }
        } else if (party_walking_flag && (!hovering || not_high_fall)) {
            if (pParty->_movementTally >= 8) {
                pParty->_movementTally = 0;
                if (!is_not_on_bmodel &&
                    pOutdoor->pBModels[pParty->floor_face_pid >> 9]
                    .pFaces[(pParty->floor_face_pid >> 3) & 0x3F].Visible()) {
                    pAudioPlayer->PlaySound(SOUND_WalkWood, PID_INVALID /*804*/, 1, -1, 0, 0);  // хождение на 3D Modelи
                } else {
                    int v87 = pOutdoor->GetSoundIdByPosition(
                        WorldPosToGridCellX(pParty->vPosition.x), WorldPosToGridCellY(pParty->vPosition.y), 0);
                    pAudioPlayer->PlaySound((SoundID)v87, PID_INVALID /*804*/, 1, -1, 0, 0);  // хождение по земле
                }
                pParty->walk_sound_timer = 144;  // таймер для ходьбы
            }
        }
    }

    // TODO(pskelton): this will trigger during accumulation - and StopAll doesnt work
    // mute the walking sound when stopping
    if (pParty->_movementTally < 8)
        pAudioPlayer->StopAll(804);
    //------------------------------------------------------------------------

    if (!hovering || not_high_fall)
        pParty->SetAirborne(false);
    else
        pParty->SetAirborne(true);

    int current_party_x_grid = WorldPosToGridCellX(pParty->vPosition.x);
    int current_party_y_grid = WorldPosToGridCellY(pParty->vPosition.y);
    int new_party_x_grid = WorldPosToGridCellX(party_new_x);
    int new_party_y_grid = WorldPosToGridCellY(party_new_Y);

    // this gets if tile is not water
    unsigned int currently_not_water = (~(unsigned int) pOutdoor->GetTileAttribByGrid(current_party_x_grid, current_party_y_grid) / 2) & 1;
    int new_x_not_water = (~(unsigned int) pOutdoor->GetTileAttribByGrid(new_party_x_grid, current_party_y_grid) / 2) & 1;
    int new_y_not_water = (~(unsigned int) pOutdoor->GetTileAttribByGrid(current_party_x_grid, new_party_y_grid) / 2) & 1;

    // -(update party co-ords)---------------------------------------
    bool notWater{ false };
    if (new_party_x_grid == current_party_x_grid && new_party_y_grid == current_party_y_grid && currently_not_water/*new_x_not_water && new_y_not_water*/) notWater = true;

    if (!is_not_on_bmodel)
        notWater = true;

    if (notWater) {
        pParty->vPosition.x = party_new_x;
        pParty->vPosition.y = party_new_Y;

        if (bpartyslope) {
            pParty->uFallSpeed = party_new_Z - pParty->vPosition.z;
        } else {
            pParty->uFallSpeed = party_z_speed;
        }

        pParty->vPosition.z = party_new_Z;
        pParty->sPartySavedFlightZ = save_old_flight_height;

        pParty->uFlags &= ~(PARTY_FLAGS_1_BURNING | PARTY_FLAGS_1_WATER_DAMAGE);
    } else {
        // we are on/approaching water tile
        bool second_move_x;
        bool second_move_y;

        if (pParty->bFlying || !not_high_fall || bWaterWalk || !currently_not_water)
            second_move_x = 1;
        else
            second_move_x = new_x_not_water != 0;

        bool party_drowning_flag = false;

        if (!pParty->bFlying && not_high_fall && !bWaterWalk) {
            if (currently_not_water) {
                second_move_y = new_y_not_water != 0;
            } else {
                party_drowning_flag = true;
                second_move_y = true;
            }
        } else {
            second_move_y = true;
        }

        if (second_move_x) pParty->vPosition.x = party_new_x;
        if (second_move_y) pParty->vPosition.y = party_new_Y;

        if (second_move_y || second_move_x) {
            if (bWaterWalk) {
                pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
                stru_5E4C90_MapPersistVars._decor_events[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uOverlayID + 119] |= 1;
                if (!new_x_not_water || !new_y_not_water) {
                    if (!pParty->bFlying) {
                        pParty->uFlags |= PARTY_FLAGS_1_STANDING_ON_WATER;
                        stru_5E4C90_MapPersistVars._decor_events[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uOverlayID + 119] &= 0xFFFE;
                    }
                }
            }
        } else if (engine->config->settings.WalkSound.Get() && pParty->walk_sound_timer <= 0) {
            pAudioPlayer->StopAll(804);
            pParty->walk_sound_timer = 64;
        }

        pParty->vPosition.z = party_new_Z;
        pParty->uFallSpeed = party_z_speed;
        pParty->sPartySavedFlightZ = save_old_flight_height;

        pParty->uFlags &= ~(PARTY_FLAGS_1_BURNING | PARTY_FLAGS_1_WATER_DAMAGE);

        if (party_drowning_flag) {
            bool onWater = false;
            int pTerrainHeight = GetTerrainHeightsAroundParty2(pParty->vPosition.x, pParty->vPosition.y, &onWater, 1);
            if (pParty->vPosition.z <= pTerrainHeight + 1) {
                pParty->uFlags |= PARTY_FLAGS_1_WATER_DAMAGE;
            }
        }
    }

    // height restriction
    if (pParty->vPosition.z > 8160) {
        pParty->uFallStartZ = 8160;
        pParty->vPosition.z = 8160;
    }

    // Falling damage
    if (!trigger_id ||
        (EventProcessor(trigger_id, 0, 1), pParty->vPosition.x == party_new_x) &&
            pParty->vPosition.y == party_new_Y && pParty->vPosition.z == party_new_Z) {
        if (pParty->vPosition.z <= ground_level) {
            pParty->uFallSpeed = 0;
            pParty->vPosition.z = ground_level;
            if (pParty->uFallStartZ - party_new_Z > 512 && !bFeatherFall &&
                party_new_Z <= ground_level &&
                !engine->IsUnderwater()) {
                if (pParty->uFlags & PARTY_FLAGS_1_LANDING) {
                    pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
                } else {
                    pParty->GiveFallDamage(pParty->uFallStartZ - pParty->vPosition.z);
                }
            }
            pParty->uFallStartZ = party_new_Z;
        }
        if (faceID_ceiling && pParty->vPosition.z < ceiling_height && (pParty->uPartyHeight + pParty->vPosition.z) >= ceiling_height) {
            pParty->vPosition.z = ceiling_height - pParty->uPartyHeight - 1;
            pParty->sPartySavedFlightZ = pParty->vPosition.z;
        }
    }
}

int GetCeilingHeight(int Party_X, signed int Party_Y, int Party_ZHeight, int *pFaceID) {
    std::array<int, 20> face_indices;
    std::array<int, 20> model_indices;
    std::array<int, 20> ceiling_height_level;
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

            int slack = engine->config->gameplay.FloorChecksEps.Get();
            if (!face.Contains(Vec3i(Party_X, Party_Y, 0), model.index, slack, FACE_XY_PLANE))
                continue;

            if (ceiling_count >= 20)
                break;

            int height_level;
            if (face.uPolygonType == POLYGON_Ceiling)
                height_level = model.pVertices[face.pVertexIDs[0]].z;
            else
                height_level = face.zCalc.Calculate(Party_X, Party_Y);

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
    if (engine->config->debug.NoActors.Get())
        return;  // uNumActors = 0;

    for (unsigned int Actor_ITR = 0; Actor_ITR < pActors.size(); ++Actor_ITR) {
        if (pActors[Actor_ITR].uAIState == Removed || pActors[Actor_ITR].uAIState == Disabled ||
            pActors[Actor_ITR].uAIState == Summoned || !pActors[Actor_ITR].uMovementSpeed)
                continue;

        bool Water_Walk = MonsterStats::BelongsToSupertype(pActors[Actor_ITR].pMonsterInfo.uID, MONSTER_SUPERTYPE_WATER_ELEMENTAL);

        pActors[Actor_ITR].uSectorID = 0;

        bool uIsFlying = pActors[Actor_ITR].pMonsterInfo.uFlying;
        if (!pActors[Actor_ITR].CanAct())
            uIsFlying = 0;

        bool Slope_High = IsTerrainSlopeTooHigh(pActors[Actor_ITR].vPosition.x, pActors[Actor_ITR].vPosition.y);
        int Model_On_PID = 0;
        bool uIsOnWater = false;
        int Floor_Level = ODM_GetFloorLevel(pActors[Actor_ITR].vPosition, pActors[Actor_ITR].uActorHeight, &uIsOnWater, &Model_On_PID, Water_Walk);
        bool Actor_On_Terrain = Model_On_PID == 0;

        bool uIsAboveFloor = (pActors[Actor_ITR].vPosition.z > (Floor_Level + 1));

        // make bloodsplat when the ground is hit
        if (!pActors[Actor_ITR].donebloodsplat) {
            if (pActors[Actor_ITR].uAIState == Dead || pActors[Actor_ITR].uAIState == Dying) {
                if (pActors[Actor_ITR].vPosition.z < Floor_Level + 30) { // 30 to provide small error / rounding factor
                    if (pMonsterStats->pInfos[pActors[Actor_ITR].pMonsterInfo.uID].bBloodSplatOnDeath) {
                        if (engine->config->graphics.BloodSplats.Get()) {
                            float splatRadius = pActors[Actor_ITR].uActorRadius * engine->config->graphics.BloodSplatsMultiplier.Get();
                            decal_builder->AddBloodsplat(pActors[Actor_ITR].vPosition.x, pActors[Actor_ITR].vPosition.y, Floor_Level + 30, 1.0, 0.0, 0.0, splatRadius);
                        }
                        pActors[Actor_ITR].donebloodsplat = true;
                    }
                }
            }
        }

        if (pActors[Actor_ITR].uAIState == Dead && uIsOnWater && !uIsAboveFloor) {
            pActors[Actor_ITR].uAIState = Removed;
            continue;
        }

        // MOVEMENT
        if (pActors[Actor_ITR].uCurrentActionAnimation == ANIM_Walking) {
            int Actor_Speed = pActors[Actor_ITR].uMovementSpeed;
            if (pActors[Actor_ITR].pActorBuffs[ACTOR_BUFF_SLOWED].Active())
                Actor_Speed = Actor_Speed * 0.5;
            if (pActors[Actor_ITR].uAIState == Fleeing || pActors[Actor_ITR].uAIState == Pursuing)
                Actor_Speed *= 2;
            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_WAIT) {
                Actor_Speed *= debug_turn_based_monster_movespeed_mul;
            }
            if (Actor_Speed > 1000)
                Actor_Speed = 1000;

            pActors[Actor_ITR].vVelocity.x = TrigLUT.Cos(pActors[Actor_ITR].uYawAngle) * Actor_Speed;
            pActors[Actor_ITR].vVelocity.y = TrigLUT.Sin(pActors[Actor_ITR].uYawAngle) * Actor_Speed;
            if (uIsFlying) {
                pActors[Actor_ITR].vVelocity.z = TrigLUT.Sin(pActors[Actor_ITR].uPitchAngle) * Actor_Speed;
            }
        } else {
            pActors[Actor_ITR].vVelocity.x = fixpoint_mul(55000, pActors[Actor_ITR].vVelocity.x);
            pActors[Actor_ITR].vVelocity.y = fixpoint_mul(55000, pActors[Actor_ITR].vVelocity.y);
            if (uIsFlying)
                pActors[Actor_ITR].vVelocity.z = fixpoint_mul(55000, pActors[Actor_ITR].vVelocity.z);
        }

        // BELOW FLOOR - POP UPWARDS
        if (pActors[Actor_ITR].vPosition.z < Floor_Level) {
            pActors[Actor_ITR].vPosition.z = Floor_Level;
            pActors[Actor_ITR].vVelocity.z = uIsFlying != 0 ? 0x14 : 0;
        }
        // GRAVITY
        if (!uIsAboveFloor || uIsFlying) {
            if (Slope_High && !uIsAboveFloor && Actor_On_Terrain) {
                Vec3i Terrain_Norm;
                pActors[Actor_ITR].vPosition.z = Floor_Level;
                ODM_GetTerrainNormalAt(pActors[Actor_ITR].vPosition.x, pActors[Actor_ITR].vPosition.y, &Terrain_Norm);
                uint16_t Gravity = GetGravityStrength();

                pActors[Actor_ITR].vVelocity.z += -16 * pEventTimer->uTimeElapsed * Gravity;
                int v73 = abs(Terrain_Norm.x * pActors[Actor_ITR].vVelocity.x +
                              Terrain_Norm.z * pActors[Actor_ITR].vVelocity.z +
                              Terrain_Norm.y * pActors[Actor_ITR].vVelocity.y) >> 15;

                pActors[Actor_ITR].vVelocity.x += fixpoint_mul(v73, Terrain_Norm.x);
                pActors[Actor_ITR].vVelocity.y += fixpoint_mul(v73, Terrain_Norm.y);
                pActors[Actor_ITR].uYawAngle -= 32;
                // pActors[Actor_ITR].vVelocity.z += fixpoint_mul(v73, Terrain_Norm.z);
            }
        } else {
            pActors[Actor_ITR].vVelocity.z -= pEventTimer->uTimeElapsed * GetGravityStrength();
        }

        // ARMAGEDDON PANIC
        if (pParty->armageddon_timer != 0 && pActors[Actor_ITR].CanAct()) {
            pActors[Actor_ITR].vVelocity.x += grng->Random(100) - 50;
            pActors[Actor_ITR].vVelocity.y += grng->Random(100) - 50;
            pActors[Actor_ITR].vVelocity.z += grng->Random(100) - 20;
            pActors[Actor_ITR].uAIState = Stunned;
            pActors[Actor_ITR].uYawAngle += grng->Random(32) - 16;
            pActors[Actor_ITR].UpdateAnimation();
        }

        // MOVING TOO SLOW
        if (pActors[Actor_ITR].vVelocity.x * pActors[Actor_ITR].vVelocity.x +
                pActors[Actor_ITR].vVelocity.y * pActors[Actor_ITR].vVelocity.y < 400 && Slope_High == 0) {
            pActors[Actor_ITR].vVelocity.y = 0;
            pActors[Actor_ITR].vVelocity.x = 0;
        }

        // COLLISIONS
        signed int Act_Radius = pActors[Actor_ITR].uActorRadius;
        if (!uIsFlying)
            Act_Radius = 40;

        collision_state.check_hi = true;
        collision_state.ignored_face_id = -1;
        collision_state.radius_hi = Act_Radius;
        collision_state.radius_lo = Act_Radius;
        collision_state.total_move_distance = 0;

        for (int attempt = 0; attempt < 100; ++attempt) {
            collision_state.position_hi.x = pActors[Actor_ITR].vPosition.x;
            collision_state.position_lo.x = collision_state.position_hi.x;
            collision_state.position_hi.y = pActors[Actor_ITR].vPosition.y;
            collision_state.position_lo.y = collision_state.position_hi.y;
            int Act_Z_Pos = pActors[Actor_ITR].vPosition.z;
            collision_state.position_lo.z = Act_Z_Pos + Act_Radius + 1;
            collision_state.position_hi.z = Act_Z_Pos - Act_Radius + pActors[Actor_ITR].uActorHeight - 1;
            if (collision_state.position_hi.z < collision_state.position_lo.z)
                collision_state.position_hi.z = Act_Z_Pos + Act_Radius + 1;
            collision_state.velocity.x = pActors[Actor_ITR].vVelocity.x;
            collision_state.uSectorID = 0;
            collision_state.velocity.y = pActors[Actor_ITR].vVelocity.y;
            collision_state.velocity.z = pActors[Actor_ITR].vVelocity.z;
            if (collision_state.PrepareAndCheckIfStationary(0)) break;
            CollideOutdoorWithModels(true);
            CollideOutdoorWithDecorations(WorldPosToGridCellX(pActors[Actor_ITR].vPosition.x), WorldPosToGridCellY(pActors[Actor_ITR].vPosition.y));
            CollideWithParty(false);
            _46ED8A_collide_against_sprite_objects(PID(OBJECT_Actor, Actor_ITR));
            int v31 = 0;
            signed int i;
            for (i = 0; v31 < ai_arrays_size; ++v31) {
                unsigned int v33 = ai_near_actors_ids[v31];
                if (v33 != Actor_ITR && CollideWithActor(v33, 40))
                    ++i;
            }
            int v71 = i > 1;
            if (collision_state.adjusted_move_distance < collision_state.move_distance)
                Slope_High = collision_state.adjusted_move_distance * collision_state.direction.z;
            // v34 = 0;
            int v35 = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
            bool bOnWater = false;
            int Splash_Model_On;
            int Splash_Floor = ODM_GetFloorLevel(
                collision_state.new_position_lo.ToInt() - Vec3i(0, 0, collision_state.radius_lo + 1),
                pActors[Actor_ITR].uActorHeight, &bOnWater, &Splash_Model_On, 0);
            if (uIsOnWater) {
                if (v35 < Splash_Floor + 60) {
                    if (pActors[Actor_ITR].uAIState == Dead || pActors[Actor_ITR].uAIState == Dying ||
                        pActors[Actor_ITR].uAIState == Removed || pActors[Actor_ITR].uAIState == Disabled) {
                        int Splash_Z = Floor_Level + 60;
                        if (Splash_Model_On)
                            Splash_Z = Splash_Floor + 30;

                        SpriteObject::Create_Splash_Object(pActors[Actor_ITR].vPosition.x, pActors[Actor_ITR].vPosition.y, Splash_Z);
                        pActors[Actor_ITR].uAIState = Removed;
                        return;
                    }
                }
            }

            if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
                pActors[Actor_ITR].vPosition.x = collision_state.new_position_lo.x;
                pActors[Actor_ITR].vPosition.y = collision_state.new_position_lo.y;
                pActors[Actor_ITR].vPosition.z = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
                break;
            }

            pActors[Actor_ITR].vPosition.x += collision_state.adjusted_move_distance * collision_state.direction.x;
            pActors[Actor_ITR].vPosition.y += collision_state.adjusted_move_distance * collision_state.direction.y;
            pActors[Actor_ITR].vPosition.z += collision_state.adjusted_move_distance * collision_state.direction.z;
            collision_state.total_move_distance += collision_state.adjusted_move_distance;
            unsigned int v39 = PID_ID(collision_state.pid);
            int Angle_To_Decor;
            signed int Coll_Speed;
            switch (PID_TYPE(collision_state.pid)) {
                case OBJECT_Actor:
                    if (pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->turn_stage != TE_MOVEMENT || !pParty->bTurnBasedModeOn) {
                        // if(pParty->bTurnBasedModeOn)
                        // v34 = 0;
                        if (pActors[Actor_ITR].pMonsterInfo.uHostilityType) {
                            if (v71 == 0)
                                Actor::AI_Flee(Actor_ITR, collision_state.pid, 0, nullptr);
                            else
                                Actor::AI_StandOrBored(Actor_ITR, 4, 0, nullptr);
                        } else if (v71) {
                            Actor::AI_StandOrBored(Actor_ITR, 4, 0, nullptr);
                        } else if (pActors[v39].pMonsterInfo.uHostilityType == MonsterInfo::Hostility_Friendly) {
                            Actor::AI_Flee(Actor_ITR, collision_state.pid, 0, nullptr);
                        } else {
                            Actor::AI_FaceObject(Actor_ITR, collision_state.pid, 0, nullptr);
                        }
                    }
                    break;
                case OBJECT_Player:
                    if (!pActors[Actor_ITR].GetActorsRelation(0)) {
                        Actor::AI_FaceObject(Actor_ITR, collision_state.pid, 0, nullptr);
                        break;
                    }

                    pActors[Actor_ITR].vVelocity.y = 0;
                    pActors[Actor_ITR].vVelocity.x = 0;

                    if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
                        pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
                    }
                    break;
                case OBJECT_Decoration:
                    Coll_Speed = integer_sqrt(pActors[Actor_ITR].vVelocity.x * pActors[Actor_ITR].vVelocity.x +
                                              pActors[Actor_ITR].vVelocity.y * pActors[Actor_ITR].vVelocity.y);
                    Angle_To_Decor = TrigLUT.Atan2(pActors[Actor_ITR].vPosition.x - pLevelDecorations[v39].vPosition.x,
                                                   pActors[Actor_ITR].vPosition.y - pLevelDecorations[v39].vPosition.y);

                    pActors[Actor_ITR].vVelocity.x = TrigLUT.Cos(Angle_To_Decor) * Coll_Speed;
                    pActors[Actor_ITR].vVelocity.y = TrigLUT.Sin(Angle_To_Decor) * Coll_Speed;
                    break;
                case OBJECT_Face: {
                    ODMFace * face = &pOutdoor->pBModels[collision_state.pid >> 9].pFaces[v39 & 0x3F];
                    if (!face->Ethereal()) {
                        if (face->uPolygonType == POLYGON_Floor) {
                            pActors[Actor_ITR].vVelocity.z = 0;
                            pActors[Actor_ITR].vPosition.z = pOutdoor->pBModels[collision_state.pid >> 9].pVertices[face->pVertexIDs[0]].z + 1;
                            if (pActors[Actor_ITR].vVelocity.x * pActors[Actor_ITR].vVelocity.x +
                                    pActors[Actor_ITR].vVelocity.y * pActors[Actor_ITR].vVelocity.y < 400) {
                                pActors[Actor_ITR].vVelocity.y = 0;
                                pActors[Actor_ITR].vVelocity.x = 0;
                            }
                        } else {
                           int v72b = abs(face->pFacePlaneOLD.vNormal.y * pActors[Actor_ITR].vVelocity.y +
                                       face->pFacePlaneOLD.vNormal.z * pActors[Actor_ITR].vVelocity.z +
                                       face->pFacePlaneOLD.vNormal.x * pActors[Actor_ITR].vVelocity.x) >> 16;
                            if ((collision_state.speed / 8) > v72b)
                                v72b = collision_state.speed / 8;

                            pActors[Actor_ITR].vVelocity.x += fixpoint_mul(v72b, face->pFacePlaneOLD.vNormal.x);
                            pActors[Actor_ITR].vVelocity.y += fixpoint_mul(v72b, face->pFacePlaneOLD.vNormal.y);
                            pActors[Actor_ITR].vVelocity.z += fixpoint_mul(v72b, face->pFacePlaneOLD.vNormal.z);
                            if (face->uPolygonType != POLYGON_InBetweenFloorAndWall) {
                                int v46 = collision_state.radius_lo - face->pFacePlaneOLD.SignedDistanceTo(pActors[Actor_ITR].vPosition);
                                if (v46 > 0) {
                                    pActors[Actor_ITR].vPosition.x += fixpoint_mul(v46, face->pFacePlaneOLD.vNormal.x);
                                    pActors[Actor_ITR].vPosition.y += fixpoint_mul(v46, face->pFacePlaneOLD.vNormal.y);
                                    pActors[Actor_ITR].vPosition.z += fixpoint_mul(v46, face->pFacePlaneOLD.vNormal.z);
                                }
                                pActors[Actor_ITR].uYawAngle = TrigLUT.Atan2(pActors[Actor_ITR].vVelocity.x, pActors[Actor_ITR].vVelocity.y);
                            }
                        }
                    }
                }
                    break;
                default:
                    break;
            }

            pActors[Actor_ITR].vVelocity.x = fixpoint_mul(58500, pActors[Actor_ITR].vVelocity.x);
            pActors[Actor_ITR].vVelocity.y = fixpoint_mul(58500, pActors[Actor_ITR].vVelocity.y);
            pActors[Actor_ITR].vVelocity.z = fixpoint_mul(58500, pActors[Actor_ITR].vVelocity.z);

            Act_Radius = collision_state.radius_lo;
        }

        // WATER TILE CHECKING
        if (!Water_Walk) {
            // tile on (1) tile heading (2)
            unsigned int Tile_1_Land = ((unsigned int)~pOutdoor->GetTileAttribByGrid(
                    WorldPosToGridCellX(pActors[Actor_ITR].vPosition.x),
                    WorldPosToGridCellY(pActors[Actor_ITR].vPosition.y)) >>
                1) & 1;
            unsigned int Tile_2_Land = ((unsigned int)~pOutdoor->GetTileAttribByGrid(
                    WorldPosToGridCellX(pActors[Actor_ITR].vPosition.x + pActors[Actor_ITR].vVelocity.x),
                    WorldPosToGridCellY(pActors[Actor_ITR].vPosition.y + pActors[Actor_ITR].vVelocity.y)) >>
                1) & 1;

            if (!uIsFlying && Tile_1_Land && !Tile_2_Land) {
                // approaching water - turn away
                if (pActors[Actor_ITR].CanAct()) {
                    pActors[Actor_ITR].uYawAngle -= 32;
                    pActors[Actor_ITR].uCurrentActionTime = 0;
                    pActors[Actor_ITR].uCurrentActionLength = 128;
                    pActors[Actor_ITR].uAIState = Fleeing;
                }
            }
            if (!uIsFlying && Tile_1_Land == 0 && !uIsAboveFloor && Actor_On_Terrain) {
                // on water and shouldnt be
                unsigned int Tile_Test_Land = 0;  // reset land found
                int Grid_X = WorldPosToGridCellX(pActors[Actor_ITR].vPosition.x);
                int Grid_Z = WorldPosToGridCellY(pActors[Actor_ITR].vPosition.y);
                for (int i = Grid_X - 1; i <= Grid_X + 1; i++) {
                    // scan surrounding cells for land
                    for (int j = Grid_Z - 1; j <= Grid_Z + 1; j++) {
                        Tile_Test_Land = ((unsigned int)~pOutdoor->GetTileAttribByGrid(i, j) >> 1) & 1;
                        if (Tile_Test_Land) {  // found land
                            int target_x = GridCellToWorldPosX(i);
                            int target_y = GridCellToWorldPosY(j);
                            if (pActors[Actor_ITR].CanAct()) {  // head to land
                                pActors[Actor_ITR].uYawAngle = TrigLUT.Atan2(target_x - pActors[Actor_ITR].vPosition.x,
                                                                             target_y - pActors[Actor_ITR].vPosition.y);
                                pActors[Actor_ITR].uCurrentActionTime = 0;
                                pActors[Actor_ITR].uCurrentActionLength = 128;
                                pActors[Actor_ITR].uAIState = Fleeing;
                                break;
                            }
                        }
                    }
                    if (Tile_Test_Land) {  // break out nested loop
                        break;
                    }
                }
                if (!Tile_Test_Land) {
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
    if (_A750D8_player_speech_timer) _A750D8_player_speech_timer = 0;
    int map_id = pMapStats->GetMapInfo(pCurrentMapName);
    unsigned int respawn_interval = 0;
    if (map_id) {
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

    if (outdoor_was_respawned && map_id) {
        for (uint i = 0; i < pOutdoor->pSpawnPoints.size(); ++i) {
            SpawnPoint *spawn = &pOutdoor->pSpawnPoints[i];

            if (spawn->uKind == OBJECT_Actor)
                SpawnEncounter(map_info, spawn, 0, 0, 0);
            else
                map_info->SpawnRandomTreasure(spawn);
        }
        RespawnGlobalDecorations();
    }
    pOutdoor->PrepareDecorations();
    pOutdoor->ArrangeSpriteObjects();
    pOutdoor->InitalizeActors(map_id);
    pOutdoor->MessWithLUN();
    pOutdoor->level_filename = pFilename;
    pWeather->Initialize();
    pCamera3D->sRotationZ = pParty->sRotationZ;
    pCamera3D->sRotationY = pParty->sRotationY;
    // pODMRenderParams->RotationToInts();
    pOutdoor->UpdateSunlightVectors();

    for (int i = 0; i < 20000; ++i) {
        array_77EC08[i].ptr_38 = &SkyBillboard;

        array_77EC08[i].ptr_48 = nullptr;
    }

    MM7Initialization();
}
// returns 0xXXYYZZ fog color
unsigned int GetLevelFogColor() {
    if (engine->IsUnderwater()) {
        return 0xFF258F5C;
    }

    if (day_attrib & DAY_ATTRIB_FOG) {
        if (pWeather->bNight) {  // night-time fog
            if (false) {
                logger->Warning("decompilation can be inaccurate, please send savegame to Nomad");
                __debugbreak();
            }
            int v2 = -(pWeather->bNight != 1);
            return (v2 & 0xE0E0E1) - 0xE0E0E1;
        } else {
            int64_t v1 = (int64_t)((1.0 - pOutdoor->fFogDensity) * 200.0 +
                                   pOutdoor->fFogDensity * 31.0);
            return v1 |
                   (((unsigned int)v1 | (((unsigned int)v1 | 0xFFFFFF00) << 8))
                    << 8);
        }
    }

    return 0;
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

    int dx = abs(pos_x - party_grid_x1), dz = abs(party_grid_z1 - pos_y);

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
    if (pOutdoor->GetTileAttribByGrid(grid_x, grid_y) & 2)
        *pIsOnWater = true;
    v14 = 0;
    if (!bFloatAboveWater && *pIsOnWater) v14 = -60;
    if (y_x1z1 != y_x2z1 || y_x2z1 != y_x2z2 || y_x2z2 != y_x1z2) {
        if (abs(grid_y1 - y) >= abs(x - grid_x1)) {
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
double OutdoorLocation::GetPolygonMinZ(RenderVertexSoft* pVertices, unsigned int unumverts) {
    double result = FLT_MAX;
    for (uint i = 0; i < unumverts; i++) {
        if (pVertices[i].vWorldPosition.z < result) {
            result = pVertices[i].vWorldPosition.z;
        }
    }
    return result;
}

//----- (00436A40) --------------------------------------------------------
double OutdoorLocation::GetPolygonMaxZ(RenderVertexSoft* pVertex, unsigned int unumverts) {
    double result = FLT_MIN;
    for (uint i = 0; i < unumverts; i++) {
        if (pVertex[i].vWorldPosition.z > result)
            result = pVertex[i].vWorldPosition.z;
    }
    return result;
}
