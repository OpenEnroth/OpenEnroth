#include "Engine/Graphics/Outdoor.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Tables/TileFrameTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/ZlibWrapper.h"
#include "Engine/stru123.h"

#include "Engine/Graphics/DecalBuilder.h"

#include "Engine/Serialization/LegacyImages.h"

#include "Platform/Api.h"

#include "DecorationList.h"
#include "Level/Decoration.h"
#include "LightmapBuilder.h"
#include "Lights.h"
#include "PaletteManager.h"
#include "ParticleEngine.h"
#include "Sprites.h"
#include "Viewport.h"
#include "Weather.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIRest.h"
#include "GUI/UI/UITransition.h"

#include "Media/Audio/AudioPlayer.h"

using EngineIoc = Engine_::IocContainer;

MapStartPoint uLevel_StartingPointType;

OutdoorLocation *pOutdoor = new OutdoorLocation;
ODMRenderParams *pODMRenderParams;

SkyBillboardStruct SkyBillboard;  // skybox planes
std::array<struct Polygon, 2000 + 18000> array_77EC08;

struct FogProbabilityTableEntry {
    unsigned char small_fog_chance;
    unsigned char average_fog_chance;
    unsigned char dense_fog_chance;
    unsigned char __unused__;
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
    pIndoorCameraD3D->debug_flags = 0;
    if (viewparams->draw_d3d_outlines)
        pIndoorCameraD3D->debug_flags |= ODM_RENDER_DRAW_D3D_OUTLINES;

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
    pODMRenderParams->uMapGridCellZ = WorldPosToGridCellZ(pParty->vPosition.y);

    assert(pODMRenderParams->uMapGridCellX <= 127 && pODMRenderParams->uMapGridCellZ <= 127);

    // if (bRedraw) {
        // sub_487DA9(); // wipes poly array feild 108 doesnt do anything
    //}

    if (pParty->uCurrentMinute != pOutdoor->uLastSunlightUpdateMinute)
        pOutdoor->UpdateSunlightVectors();

    pOutdoor->UpdateFog();
    // pIndoorCameraD3D->sr_Reset_list_0037C();

    // if (render->pRenderD3D) // d3d - redraw always
    {
        SkyBillboard.CalcSkyFrustumVec(65536, 0, 0, 0, 65536, 0);  // sky box frustum
        render->DrawOutdoorSkyD3D();
        render->RenderTerrainD3D();
        render->DrawBuildingsD3D();

        // render->DrawBezierTerrain();
    }

    pMobileLightsStack->uNumLightsActive = 0;
    pStationaryLightsStack->uNumLightsActive = 0;

    engine->PushStationaryLights(-1);
    // engine->PrepareBloodsplats(); // not used?
    if (bRedraw)
        UpdateDiscoveredArea(WorldPosToGridCellX(pParty->vPosition.x),
                             WorldPosToGridCellZ(pParty->vPosition.y),
                             1);
    engine->SetForceRedraw(false);
    if (render->IsUsingSpecular())
        lightmap_builder->uFlags |= LIGHTMAP_FLAGS_USE_SPECULAR;
    else
        lightmap_builder->uFlags &= ~LIGHTMAP_FLAGS_USE_SPECULAR;

    uNumDecorationsDrawnThisFrame = 0;
    uNumSpritesDrawnThisFrame = 0;
    uNumBillboardsToDraw = 0;

    PrepareActorsDrawList();
    if (!pODMRenderParams->bDoNotRenderDecorations)
        render->PrepareDecorationsRenderList_ODM();

    render->DrawSpriteObjects_ODM();
    render->TransformBillboardsAndSetPalettesODM();
}

//----- (00441CFF) --------------------------------------------------------
void OutdoorLocation::Draw() {
    bool redrawWorld = true;
    if (!(pParty->uFlags & 2) && !engine->config->ForceRedraw())
        redrawWorld = false;
    pOutdoor->ExecDraw(redrawWorld);

    engine->DrawParticles();
    // pWeather->Draw();//если раскомментировать скорость снега быстрее
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
int OutdoorLocation::GetSomeOtherTileInfo(int sX, int sY) {
    // OutdoorLocation *v3; // esi@1
    unsigned int v4;  // edi@1
    unsigned int v5;  // eax@1
                      //  int result; // eax@5

    /*  v3 = this;
      v4 = WorldPosToGridCellZ(sY);
      v5 = WorldPosToGridCellX(sX);
      if ( (v5 & 0x80000000u) != 0 || (signed int)v5 > 127 || (v4 & 0x80000000u)
      != 0 || (signed int)v4 > 127 ) result = 0; else result =
      ActuallyGetSomeOtherTileInfo(v5, v4); return result;*/
    v4 = WorldPosToGridCellZ(sY);
    v5 = WorldPosToGridCellX(sX);
    if (v5 < 0 || v5 > 127 || v4 < 0 || v4 > 127) return 0;
    return ActuallyGetSomeOtherTileInfo(v5, v4);
}

//----- (00488EEF) --------------------------------------------------------
TileDesc *OutdoorLocation::GetTile(signed int sX, signed int sY) {
    signed int v4;  // edi@1
    signed int v5;  // eax@1

    v4 = WorldPosToGridCellZ(sY);
    v5 = WorldPosToGridCellX(sX);
    if (v5 < 0 || v5 > 127 || v4 < 0 || v4 > 127) return nullptr;

    return this->DoGetTile(v5, v4);
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
bool OutdoorLocation::Initialize(const String &filename, int days_played,
                                 int respawn_interval_days, int *thisa) {
    decal_builder->Reset(0);

    if (!filename.empty()) {
        Release();
        assets->ReleaseAllImages();
        pSprites_LOD->DeleteSomeOtherSprites();
        pSpriteFrameTable->ResetSomeSpriteFlags();

        if (!this->Load(filename, days_played, respawn_interval_days, thisa)) {
            logger->Warning(L"Couldn't Load Map!");
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
    int mapNumberAsInt;         // eax@3
    signed int direction;       // esi@7
    signed int destinationMap;  // eax@23
    char Str[140];              // [sp+8h] [bp-78h]@3

    strcpy(Str, this->level_filename.c_str());
    _strlwr(Str);
    mapNumberAsStr = strtok(Str, "out");
    mapNumberAsStr[2] = 0;
    mapNumberAsInt = atoi(mapNumberAsStr);
    if (a5 < 10 || this->level_filename.length() != 9 || mapNumberAsInt < 1 ||
        mapNumberAsInt > 15)  //длина  .odm и количество локаций
        return 0;
    if (sPartyX < -22528)  //граница карты
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
    sprintf(pOut, "out%02d.odm", destinationMap);  //локация направления
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
    this->field_D3C = (int)this->pSpriteIDs_LUN;
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
        pSpriteFrameTable->InitializeSprite(
            this->pSpriteIDs_LUN[i]);  // v2 += 2;
    pSpriteFrameTable->InitializeSprite(this->uSpriteID_LUN_SUN);
}

//----- (004892E6) --------------------------------------------------------
void OutdoorLocation::UpdateSunlightVectors() {
    unsigned int v3;  // edi@3
    double v8;        // st7@4

    if (pParty->uCurrentHour >= 5 && pParty->uCurrentHour < 21) {
        v3 = pParty->uCurrentMinute + 60 * (pParty->uCurrentHour - 5);
        this->inv_sunlight_y = 0;
        this->inv_sunlight_x =
            stru_5C6E00->Cos((v3 * stru_5C6E00->uIntegerPi) / 960);
        this->inv_sunlight_z =
            stru_5C6E00->Sin((v3 * stru_5C6E00->uIntegerPi) / 960);
        this->vSunlight.x = -this->inv_sunlight_x;
        this->vSunlight.y = -this->inv_sunlight_y;
        this->vSunlight.z = -this->inv_sunlight_z;
        if (v3 >= 480)
            v8 = 960 - v3;
        else
            v8 = v3;
        this->max_terrain_dimming_level = (int)(20.0 - v8 / 480.0 * 20.0);
        this->uLastSunlightUpdateMinute = pParty->uCurrentMinute;
    }
}

//----- (004893C1) --------------------------------------------------------
void OutdoorLocation::UpdateFog() {
    fFogDensity = GetFogDensityByTime();
}

int OutdoorLocation::GetNumFoodRequiredToRestInCurrentPos(int x, int y, int z) {
    bool is_on_water = false;
    int bmodel_standing_on_pid = 0;
    ODM_GetFloorLevel(x, y, z, pParty->uDefaultPartyHeight, &is_on_water, &bmodel_standing_on_pid, 0);
    if (pParty->uFlags & 8 || bmodel_standing_on_pid || is_on_water)  // на bmodel, и или на воде
        return 2;
    int v7 = _47ED83(WorldPosToGridCellX(pParty->vPosition.x),
                     WorldPosToGridCellZ(pParty->vPosition.y) - 1);
    switch (pTileTable->pTiles[GetTileIdByTileMapId(v7)].tileset) {
        case Tileset_Grass:  // на траве
            return 1;
        case Tileset_Snow:  // на снегу
            return 3;
        case Tilset_Desert:  // на песке
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

    uint chance = rand() % 100;

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

//----- (00482170) --------------------------------------------------------
bool ODMFace::IsBackfaceNotCulled(RenderVertexSoft *a2,
                                  struct Polygon *polygon) {
    unsigned int numOfVertices;    // edx@1
    RenderVertexSoft *currVertex;  // ecx@2
    double v7;                     // st7@5
    double v8;                     // st6@5
    double v9;                     // st5@5
    float v18;                     // [sp+8h] [bp-38h]@5
    float v19;                     // [sp+10h] [bp-30h]@5
    float v20;                     // [sp+14h] [bp-2Ch]@5
    float v21;                     // [sp+18h] [bp-28h]@5
    float v22;                     // [sp+1Ch] [bp-24h]@5
    float v23;                     // [sp+24h] [bp-1Ch]@5
    float v24;                     // [sp+28h] [bp-18h]@5
    float v25;                     // [sp+30h] [bp-10h]@5
    float v26;                     // [sp+34h] [bp-Ch]@5
    float v27;                     // [sp+38h] [bp-8h]@5
    float v28;                     // [sp+3Ch] [bp-4h]@5
    float a3a;                     // [sp+48h] [bp+8h]@5
    float a3b;                     // [sp+48h] [bp+8h]@17
    float a3c;                     // [sp+48h] [bp+8h]@17
    float a3d;                     // [sp+48h] [bp+8h]@17
    float a3e;                     // [sp+48h] [bp+8h]@17

    numOfVertices = polygon->uNumVertices;
    if (numOfVertices < 3) return false;
    currVertex = &a2[numOfVertices - 1];
    if (a2->vWorldPosition.z == a2[1].vWorldPosition.z &&
        a2[1].vWorldPosition.z == currVertex->vWorldPosition.z)
        polygon->flags |= 0x10u;

    v28 = a2[1].vWorldPosition.x - a2->vWorldPosition.x;
    v27 = a2[1].vWorldPosition.y - a2->vWorldPosition.y;
    a3a = a2[1].vWorldPosition.z - a2->vWorldPosition.z;

    for (int i = 0; i < numOfVertices; i++) {
        v7 = currVertex->vWorldPosition.x - a2->vWorldPosition.x;
        v8 = currVertex->vWorldPosition.y - a2->vWorldPosition.y;
        v9 = currVertex->vWorldPosition.z - a2->vWorldPosition.z;
        v26 = v27 * v9 - v8 * a3a;
        v24 = v7 * a3a - v9 * v28;
        v25 = v8 * v28 - v7 * v27;
        if (v24 != 0.0 || v25 != 0.0 || v26 != 0.0) break;
        currVertex--;
    }

    if (((double)pIndoorCameraD3D->vPartyPos.x - a2->vWorldPosition.x) * v26 +
            ((double)pIndoorCameraD3D->vPartyPos.z - a2->vWorldPosition.z) *
                v25 +
            ((double)pIndoorCameraD3D->vPartyPos.y - a2->vWorldPosition.y) *
                v24 >
        0.0) {
        v19 = a2[1].vWorldViewPosition.x - a2->vWorldViewPosition.x;
        v18 = a2[1].vWorldViewPosition.y - a2->vWorldViewPosition.y;
        v20 = a2[1].vWorldViewPosition.z - a2->vWorldViewPosition.z;
        v21 = currVertex->vWorldViewPosition.x - a2->vWorldViewPosition.x;
        v22 = currVertex->vWorldViewPosition.y - a2->vWorldViewPosition.y;
        v23 = currVertex->vWorldViewPosition.z - a2->vWorldViewPosition.z;

        a3b = v23 * v18 - v22 * v20;
        polygon->v_18.x = bankersRounding(a3b);
        a3c = v21 * v20 - v23 * v19;
        polygon->v_18.y = bankersRounding(a3c);
        a3d = v22 * v19 - v21 * v18;
        polygon->v_18.z = bankersRounding(a3d);
        polygon->_normalize_v_18();
        a3e = -((double)polygon->v_18.x * a2->vWorldViewPosition.x) -
              (double)polygon->v_18.y * a2->vWorldViewPosition.y -
              (double)polygon->v_18.z * a2->vWorldViewPosition.z;
        polygon->field_24 = bankersRounding(a3e);
        return true;
    } else {
        return false;
    }
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
    free(this->pHeightmap);
    pHeightmap = nullptr;
    free(pTilemap);
    pTilemap = nullptr;
    free(pAttributemap);
    pAttributemap = nullptr;
    free(pDmap);
    pDmap = nullptr;

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
    unsigned __int8 *pMapHeight;  // ebx@15
    int v17;                      // eax@15
    int v18;                      // ecx@15
    int v19;                      // esi@15
    int v20;                      // edi@15
    int v21;                      // edx@15
    int v22;                      // ecx@15
    int v23;                      // ebx@15
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
                        pMapHeight = this->pHeightmap;
                        v17 = (int)(&pMapHeight[v13 * this->field_10] + v14);
                        v18 = -v13;
                        v19 = (64 - v13) << 9;
                        v20 = 32 * *(char *)v17;
                        v21 = 32 * *(char *)(v17 + 1);

                        v22 = (v18 + 63) << 9;
                        v41 = v22;
                        v23 = (int)(&pMapHeight[this->field_10 * (v13 + 1)] +
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
                        *((char *)this->pDmap + v44 + 1) = (signed __int64)v31;

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
                        *((char *)this->pDmap + v44) = (signed __int64)v38;
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
                        //  unsigned __int16 *v5; // edx@3
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
                        //  unsigned __int16 *v17; // [sp+104h] [bp-24h]@3
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
      v25 = (signed __int64)(a3a * v22);
      i = (signed __int64)(a3a * v23);
      v12 = 0;
      a2b = num_b_bits + num_g_bits;
      while ( 1 )
      {
        v13 = v17[v12];
        a3b = abs((__int64)(signed __int64)(v11 * v6) - ((signed int)(r_mask &
    v17[v12]) >> a2b)); BYTE3(a3b) = abs((signed)v25 - ((signed int)(g_mask &
    v13) >> num_b_bits)) + a3b; v16[v12++] = abs((signed)i - (signed)(b_mask &
    v13)) + BYTE3(a3b); if ( v12 >= 256 ) break;
      }
      result = 0;
      v14 = (char *)&pPaletteManager->field_D1600[42][23][116];
      v15 = 0;
      do
      {
        if ( (unsigned __int8)v16[v15] < (signed int)v14 )
        {
          v14 = (char *)(unsigned __int8)v16[v15];
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
    memset(this->pHeightmap, 0, 0x4000u);
    memset(this->pTilemap, 90, 0x4000u);
    memset(this->pAttributemap, 0, 0x4000u);
    memset(this->pDmap, 0, 0x8000u);
    this->field_12 = 128;
    this->field_10 = 128;
    this->field_16 = 7;
    this->field_14 = 7;
    this->field_1C = 127;
    this->field_18 = 127;
    return true;
}

//----- (0047CD44) --------------------------------------------------------
bool OutdoorLocationTerrain::Initialize() {
    pHeightmap = (unsigned __int8 *)malloc(0x4000);  // height map
    pTilemap = (unsigned __int8 *)malloc(0x4000);    // tile map
    pAttributemap = (unsigned __int8 *)malloc(0x4000);  //карта атрибутов
    pDmap = (struct DMap *)malloc(0x8000);
    if (pHeightmap && pTilemap && pAttributemap && pDmap)
        return true;
    else
        return false;
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
    free(this->pSpawnPoints);
    this->pSpawnPoints = 0;
    this->pTerrain.Initialize();
    this->pTerrain.ZeroLandscape();
    this->pTerrain.FillDMap(0, 0, 128, 128);

    free(this->pCmap);
    this->pCmap = malloc(0x8000);

    free(this->pOMAP);
    this->pOMAP = (unsigned int *)malloc(0x10000);
    memset(this->pOMAP, 0, 0x10000);

    this->numFaceIDListElems = 0;

    free(this->pFaceIDLIST);
    this->pFaceIDLIST = (unsigned __int16 *)malloc(2);
    this->pFaceIDLIST[0] = 0;

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

    while (!pBModels.empty()) {
        pBModels.back().Release();
        pBModels.pop_back();
    }

    free(pSpawnPoints);
    pSpawnPoints = nullptr;
    uNumSpawnPoints = 0;

    pTerrain.Release();

    free(pCmap);
    pCmap = nullptr;
    free(pOMAP);
    pOMAP = nullptr;
    free(pFaceIDLIST);
    pFaceIDLIST = nullptr;
    free(pTerrainNormals);
    pTerrainNormals = nullptr;
}

bool OutdoorLocation::Load(const String &filename, int days_played,
                           int respawn_interval_days, int *thisa) {
    if (engine->IsUnderwater()) {
        pPaletteManager->pPalette_tintColor[0] = 0x10;
        pPaletteManager->pPalette_tintColor[1] = 0xC2;
        pPaletteManager->pPalette_tintColor[2] = 0x99;
        pPaletteManager->SetMistColor(37, 143, 92);
    } else {
        pPaletteManager->pPalette_tintColor[0] = 0;
        pPaletteManager->pPalette_tintColor[1] = 0;
        pPaletteManager->pPalette_tintColor[2] = 0;
        if (pPaletteManager->pPalette_mistColor[0] != 0x80 ||
            pPaletteManager->pPalette_mistColor[1] != 0x80 ||
            pPaletteManager->pPalette_mistColor[2] != 0x80) {
            pPaletteManager->SetMistColor(128, 128, 128);
            pPaletteManager->RecalculateAll();
        }
    }

    _6807E0_num_decorations_with_sounds_6807B8 = 0;

    assert(sizeof(BSPModelData) == 188);

    if (!pGames_LOD->DoesContainerExist(filename)) {
        Error("Unable to find %s in Games.LOD", filename.c_str());
    }

    String minimap_filename = filename.substr(0, filename.length() - 4);
    viewparams->location_minimap = assets->GetImage_Solid(minimap_filename);

    auto odm_filename = String(filename);
    odm_filename.replace(odm_filename.length() - 4, 4, ".odm");

    void *pSrcMem = pGames_LOD->LoadCompressed(odm_filename);
    uint8_t *pSrc = (uint8_t*)pSrcMem;

#pragma pack(push, 1)
    struct ODMInfo {
        char level_filename[32];
        char location_filename[32];
        char location_file_description[32];
        char sky_texture_filename[32];
        char ground_tileset[32];
    };
#pragma pack(pop)

    static_assert(sizeof(ODMInfo) == 160, "Wrong type size");
    ODMInfo *odm_info = (ODMInfo*)pSrc;
    this->level_filename = String(odm_info->level_filename, 32);
    this->location_filename = String(odm_info->level_filename, 32);
    this->location_file_description = String(odm_info->location_file_description, 32);
    this->sky_texture_filename = String(odm_info->sky_texture_filename, 32);
    this->ground_tileset = String(odm_info->ground_tileset, 32);
    pSrc += sizeof(ODMInfo);

    static_assert(sizeof(OutdoorLocationTileType) == 4, "Wrong type size");
    memcpy(pTileTypes, pSrc, sizeof(pTileTypes));
    pSrc += sizeof(pTileTypes);

    LoadTileGroupIds();
    LoadRoadTileset();
    this->ground_tileset = "grastyl";

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    // *******************Terrain**************************//
    pTerrain.Initialize();
    memcpy(pTerrain.pHeightmap, pSrc, 0x4000);  // карта высот
    pSrc += 0x4000;

    memcpy(pTerrain.pTilemap, pSrc, 0x4000);  // карта тайлов
    pSrc += 0x4000;

    memcpy(pTerrain.pAttributemap, pSrc, 0x4000);  // карта аттрибутов
    pSrc += 0x4000;

    // v43 = (char *)v43 + 16384;
    // v108 = (int)ptr_D4;
    free(pCmap);
    pCmap = malloc(0x8000);
    pTerrain.FillDMap(0, 0, 128, 128);  //

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    memcpy(&uNumTerrainNormals, pSrc, 4);  // количество нормалей
    pSrc += 4;
    memcpy(pTerrainSomeOtherData.data(), pSrc, 0x20000);
    pSrc += 0x20000;

    memcpy(pTerrainNormalIndices.data(), pSrc, 0x10000);  //индексы нормалей
    pSrc += 0x10000;

    pTerrainNormals = (Vec3_float_ *)malloc(
        sizeof(Vec3_float_) * uNumTerrainNormals);  //карта нормалей
    memcpy(pTerrainNormals, pSrc, 12 * uNumTerrainNormals);
    pSrc += 12 * uNumTerrainNormals;

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    // ************BModels************************//
    pSrc = pBModels.Load(pSrc);
    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    // ******************Decorations**********************//
    memcpy(&uNumLevelDecorations, pSrc, 4);
    // uSourceLen = (char *)uSourceLen + 4;
    if (uNumLevelDecorations > 3000) logger->Warning(L"Can't load file! Too many decorations");

    assert(sizeof(LevelDecoration) == 32);
    // pFilename = (char *)(32 * uNumLevelDecorations);
    memcpy(pLevelDecorations.data(), pSrc + 4,
           uNumLevelDecorations * sizeof(LevelDecoration));
    pSrc += 4 + sizeof(LevelDecoration) * uNumLevelDecorations;

    pGameLoadingUI_ProgressBar->Progress();

    // v151 = 0;
    // uSourceLen = (char *)uSourceLen + (int)pFilename;
    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        char name[256];
        memcpy(name, pSrc, sizeof(LevelDecoration));
        pSrc += sizeof(LevelDecoration);
        pLevelDecorations[i].uDecorationDescID =
            pDecorationList->GetDecorIdByName(name);
    }

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    memcpy(&numFaceIDListElems, pSrc, 4);

    // uSourceLen = (char *)uSourceLen + 4;
    // v108 = (int)pFaceIDLIST;
    free(pFaceIDLIST);
    pFaceIDLIST = nullptr;

    uint faceIDListSize = 2 * numFaceIDListElems;
    pFaceIDLIST = (unsigned short *)malloc(faceIDListSize);

    memcpy(pFaceIDLIST, pSrc + 4, faceIDListSize);
    pSrc += 4 + faceIDListSize;

    // uSourceLen = (char *)uSourceLen + (int)pFilename;
    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    free(pOMAP);
    // v69 = malloc(0, 0x10000u, "OMAP");
    pOMAP = (unsigned int *)malloc(0x10000);
    // v108 = 65536;
    // pOMAP = (unsigned int *)v69;
    memcpy(pOMAP, pSrc, 65536);
    pSrc += 65536;

    // uSourceLen = (char *)uSourceLen + 65536;
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumSpawnPoints, pSrc, 4);
    // uSourceLen = (char *)uSourceLen + 4;
    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    assert(sizeof(SpawnPointMM7) == 24);
    uint spawnPointsSize = uNumSpawnPoints * sizeof(SpawnPointMM7);
    pSpawnPoints = (SpawnPointMM7 *)malloc(spawnPointsSize);
    // v72 = uNumSpawnPoints;
    // pSpawnPoints = v71;
    memcpy(pSpawnPoints, pSrc + 4, spawnPointsSize);
    pSrc += 4 + spawnPointsSize;

    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    // ****************.ddm file*********************//
    free(pSrcMem);

    auto ddm_filename = filename;
    ddm_filename = ddm_filename.replace(ddm_filename.length() - 4, 4, ".ddm");
    pSrcMem = pNew_LOD->LoadCompressed(ddm_filename);

    if (pSrcMem != nullptr) {
        pSrc = (uint8_t*)pSrcMem;

        static_assert(sizeof(DDM_DLV_Header) == 40, "Wrong type size");
        memcpy(&ddm, pSrc, sizeof(DDM_DLV_Header));
        pSrc += sizeof(DDM_DLV_Header);
    }
    uint actualNumFacesInLevel = 0;
    for (BSPModel &model : pBModels) {
        actualNumFacesInLevel += model.pFaces.size();
    }

    bool Str2 = false;
    if (ddm.uNumFacesInBModels) {
        if (ddm.uNumBModels) {
            if (ddm.uNumDecorations) {
                if (ddm.uNumFacesInBModels != actualNumFacesInLevel ||
                    ddm.uNumBModels != pBModels.size() ||
                    ddm.uNumDecorations != uNumLevelDecorations)
                    Str2 = true;
            }
        }
    }

    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_2000) {
        respawn_interval_days = 0x1BAF800;
    }

    char Src[968];
    char Dst[968];
    int v108;

    if (Str2 || days_played - ddm.uLastRepawnDay >= respawn_interval_days ||
        !ddm.uLastRepawnDay) {
        if (Str2) {
            memset(Dst, 0, 968);
            memset(Src, 0, 968);
        }
        if (days_played - ddm.uLastRepawnDay >= respawn_interval_days ||
            !ddm.uLastRepawnDay) {
            memcpy(Dst, pSrc, 968);
            memcpy(Src, pSrc + 968, 968);
        }
        free(pSrcMem);

        ddm.uLastRepawnDay = days_played;
        if (Str2 == 0) ++ddm.uNumRespawns;
        v108 = 0;
        *thisa = 1;
        pSrcMem = pGames_LOD->LoadCompressed(ddm_filename);
        pSrc = (uint8_t*)pSrcMem;
        pSrc += sizeof(DDM_DLV_Header);
    } else {
        *thisa = 0;
    }
    memcpy(uFullyRevealedCellOnMap, pSrc, 968);
    memcpy(uPartiallyRevealedCellOnMap, pSrc + 968, 968);
    pSrc += 2 * 968;

    pGameLoadingUI_ProgressBar->Progress();  // прогресс загрузки

    if (*thisa) {
        memcpy(uFullyRevealedCellOnMap, Dst, 968);
        memcpy(uPartiallyRevealedCellOnMap, Src, 968);
    }

    for (BSPModel &model : pBModels) {
        for (ODMFace &face : model.pFaces) {
            memcpy(&face.uAttributes, pSrc, 4);
            pSrc += 4;
        }

        for (ODMFace &face : model.pFaces) {
            if (face.sCogTriggeredID) {
                if (face.HasEventHint()) {
                    face.uAttributes |= FACE_HAS_EVENT_HINT;
                } else {
                    face.uAttributes &= ~FACE_HAS_EVENT_HINT;  // ~0x00001000
                }
            }
        }
    }

    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        memcpy(&pLevelDecorations[i].uFlags, pSrc, 2);
        pSrc += 2;
    }

    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    memcpy(&uNumActors, pSrc, 4);
    if (uNumActors > 500) logger->Warning(L"Can't load file!");

    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    assert(sizeof(Actor) == 836);
    // pFilename = (char *)(836 * uNumActors);
    memcpy(pActors.data(), pSrc + 4, uNumActors * sizeof(Actor));
    pSrc += 4 + uNumActors * sizeof(Actor);
    // v92 = (char *)v91 + (int)pFilename;
    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    memcpy(&uNumSpriteObjects, pSrc, 4);
    assert(uNumSpriteObjects <= 1000 && "Too many objects");
    assert(sizeof(SpriteObject) == 112);

    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    // pFilename = (char *)(112 * uNumSpriteObjects);
    memcpy(pSpriteObjects.data(), pSrc + 4,
           uNumSpriteObjects * sizeof(SpriteObject));
    pSrc += 4 + uNumSpriteObjects * sizeof(SpriteObject);

    // v94 = (char *)v93 + (int)pFilename;
    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    pSrc = (uint8_t*)ChestsDeserialize((char*)pSrc);

    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки

    memcpy(&stru_5E4C90_MapPersistVars, pSrc, 0xC8);
    pSrc += 0xC8;

    pGameLoadingUI_ProgressBar->Progress();  //прогресс загрузки
    memcpy(&loc_time, pSrc, 0x38u);

    free(pSrcMem);

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
            if (rand() % 100 >= 20)
                v108 = dword_4EC268[rand() % dword_4EC2A8];
            else
                v108 = dword_4EC28C[rand() % dword_4EC2AC];
            sprintf(loc_time.sky_texture_name, "plansky%d", v108);
        }
    } else {
        strcpy(loc_time.sky_texture_name, "plansky3");
    }

    this->sky_texture = assets->GetBitmap(loc_time.sky_texture_name);

    pPaletteManager->RecalculateAll();
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
            case Tilset_Desert:
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
TileDesc *OutdoorLocation::DoGetTile(int sX, int sY) {
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

    if (engine->config->seasons_change) {
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
int OutdoorLocation::_47ED83(signed int a2, signed int a3) {
    if (a2 < 0 || a2 > 127 || a3 < 0 || a3 > 127)
        return 0;

    return *(&this->pTerrain.pTilemap[128 * a3] + a2);
}

//----- (0047EDB3) --------------------------------------------------------
int OutdoorLocation::ActuallyGetSomeOtherTileInfo(signed int sX,
                                                  signed int sY) {
    int v3;  // esi@5

    if (sX < 0 || sX > 127 || sY < 0 || sY > 127)
        return 0;

    v3 = this->pTerrain.pTilemap[sY * 128 + sX];
    if (v3 >= 90)
        v3 = v3 + this->pTileTypes[(v3 - 90) / 36].uTileID -
             36 * ((v3 - 90) / 36) - 90;
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
int OutdoorLocation::UpdateDiscoveredArea(int X_grid_pos, int Y_grid_poa,
                                          int a4) {
    for (int i = -10; i < 10; i++) {
        int currYpos = Y_grid_poa + i - 20;
        for (int j = -10; j < 10; j++) {
            int currXpos = X_grid_pos + j - 20;
            int distanceSquared = i * i + j * j;
            if (distanceSquared <= 100 && currYpos >= 0 && currYpos <= 87 &&
                currXpos >= 0 && currXpos <= 87) {
                unsigned char v13 = 1 << (7 - currXpos % 8);
                this->uPartiallyRevealedCellOnMap[currYpos][currXpos / 8] |=
                    v13;
                if (distanceSquared <= 49)
                    this->uFullyRevealedCellOnMap[currYpos][currXpos / 8] |=
                        v13;
            }
        }
    }
    return 1;
}

//----- (0047F04C) --------------------------------------------------------
bool OutdoorLocation::IsMapCellFullyRevealed(signed int x_pos,
                                             signed int y_pos) {
    if (x_pos < 0 || x_pos >= 88 || y_pos < 0 || y_pos >= 88)
        return false;
    else
        return (uFullyRevealedCellOnMap[y_pos][x_pos / 8] &
                (1 << (7 - (x_pos) % 8))) != 0;
}

//----- (0047F097) --------------------------------------------------------
bool OutdoorLocation::IsMapCellPartiallyRevealed(signed int x_pos,
                                                 signed int y_pos) {
    if (x_pos < 0 || x_pos >= 88 || y_pos < 0 || y_pos >= 88)
        return false;
    else
        return (uPartiallyRevealedCellOnMap[y_pos][x_pos / 8] &
                (1 << (7 - (x_pos) % 8))) != 0;
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

    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        LevelDecoration *decor = &pLevelDecorations[i];

        pDecorationList->InitializeDecorationSprite(decor->uDecorationDescID);
        DecorationDesc *decoration = pDecorationList->GetDecoration(decor->uDecorationDescID);
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
    if ((int)uNumSpriteObjects > 0) {
        for (int i = 0; i < (signed int)uNumSpriteObjects; ++i) {
            if (pSpriteObjects[i].uObjectDescID) {
                if (!(pSpriteObjects[i].uAttributes & 8) && !pSpriteObjects[i].IsUnpickable()) {
                    bool bOnWater = false;
                    pSpriteObjects[i].vPosition.z =
                        GetTerrainHeightsAroundParty2(
                            pSpriteObjects[i].vPosition.x,
                            pSpriteObjects[i].vPosition.y, &bOnWater, 0);
                }
                if (pSpriteObjects[i].containing_item.uItemID) {
                    if (pSpriteObjects[i].containing_item.uItemID != 220 &&
                        pItemsTable->pItems[pSpriteObjects[i].containing_item.uItemID].uEquipType == EQUIP_POTION &&
                        !pSpriteObjects[i].containing_item.uEnchantmentType)
                        pSpriteObjects[i].containing_item.uEnchantmentType = rand() % 15 + 5;
                    pItemsTable->SetSpecialBonus(&pSpriteObjects[i].containing_item);
                }
            }
        }
    }
    pGameLoadingUI_ProgressBar->Progress();
}

//----- (0047F2D3) --------------------------------------------------------
bool OutdoorLocation::InitalizeActors(int a1) {
    int alert_status;  // [sp+348h] [bp-8h]@1
                       //  int v9; // [sp+34Ch] [bp-4h]@1

    alert_status = 0;
    for (int i = 0; i < uNumActors; ++i) {
        if (!(pActors[i].uAttributes & ACTOR_UNKNOW7)) {
            if (alert_status != 1) {
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
        } else if (alert_status != 0) {
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

//----- (0047B42C) --------------------------------------------------------
void OutdoorLocation::PrepareActorsDrawList() {
    int z;             // esi@5
    float v4;          // ST48_4@8
    unsigned int Angle_To_Cam;   // eax@11
    signed int Cur_Action_Time;    // eax@16
    SpriteFrame *v14;  // eax@24
    SpriteFrame *v15;  // ebx@25
                       //    int v22; // ecx@41
                       //    int v23; // ST5C_4@43
    int Sprite_Octant;           // [sp+24h] [bp-3Ch]@11
                       //    int v48; // [sp+30h] [bp-30h]@41
    signed int v49;    // [sp+34h] [bp-2Ch]@5
                       //    int v51; // [sp+34h] [bp-2Ch]@41
    int y;             // [sp+40h] [bp-20h]@5
    int x;             // [sp+44h] [bp-1Ch]@5
    __int16 v62;       // [sp+5Ch] [bp-4h]@25

    for (int i = 0; i < uNumActors; ++i) {
        pActors[i].uAttributes &= ~ACTOR_VISIBLE;
        if (pActors[i].uAIState == Removed || pActors[i].uAIState == Disabled) {
            continue;
        }

        z = pActors[i].vPosition.z;
        v49 = 0;
        x = pActors[i].vPosition.x;
        y = pActors[i].vPosition.y;
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

        Angle_To_Cam = stru_5C6E00->Atan2(
            pActors[i].vPosition.x - pIndoorCameraD3D->vPartyPos.x,
            pActors[i].vPosition.y - pIndoorCameraD3D->vPartyPos.y);

        // int v9 = 0;
        // HEXRAYS_LOWORD(v9) = pActors[i].uYawAngle;
        Sprite_Octant = ((signed int)(stru_5C6E00->uIntegerPi +
                            ((signed int)stru_5C6E00->uIntegerPi >> 3) + pActors[i].uYawAngle -
                            Angle_To_Cam) >> 8) & 7;


        if (pParty->bTurnBasedModeOn) {
            Cur_Action_Time = pActors[i].uCurrentActionTime;
            if (pActors[i].uCurrentActionAnimation == 1)
                Cur_Action_Time = 32 * i + pMiscTimer->uTotalGameTimeElapsed;
        } else {
            Cur_Action_Time = pActors[i].uCurrentActionTime;
            if (pActors[i].uCurrentActionAnimation == 1)
                Cur_Action_Time = 32 * i + pEventTimer->uTotalGameTimeElapsed;
        }

        if (pActors[i].pActorBuffs[ACTOR_BUFF_STONED].Active() ||
            pActors[i].pActorBuffs[ACTOR_BUFF_PARALYZED].Active())
            Cur_Action_Time = 0;
        if (pActors[i].uAIState == Summoned && !v49)
            v14 = pSpriteFrameTable->GetFrame(uSpriteID_Spell11, Cur_Action_Time);
        else if (pActors[i].uAIState == Resurrected)
            v14 = pSpriteFrameTable->GetFrameBy_x(
                pActors[i].pSpriteIDs[pActors[i].uCurrentActionAnimation], Cur_Action_Time);
        else
            v14 = pSpriteFrameTable->GetFrame(
                pActors[i].pSpriteIDs[pActors[i].uCurrentActionAnimation], Cur_Action_Time);

        // no sprite frame to draw
        if (v14->icon_name == "null") continue  /* __debugbreak()*/;
        if (v14->hw_sprites[0] == nullptr) __debugbreak();

        v62 = 0;
        v15 = v14;
        // v16 = (int *)v14->uFlags;
        if (v14->uFlags & 2) v62 = 2;
        if (v14->uFlags & 0x40000) v62 |= 0x40;
        if (v14->uFlags & 0x20000) v62 |= 0x80;
        if ((256 << Sprite_Octant) & v14->uFlags) v62 |= 4;
        if (v15->uGlowRadius) {
            pMobileLightsStack->AddLight(x, y, z, 0, v15->uGlowRadius, 0xFFu,
                                         0xFFu, 0xFFu, _4E94D3_light_type);
        }

        int view_x = 0, view_y = 0, view_z = 0;
        bool visible = pIndoorCameraD3D->ViewClip(x, y, z, &view_x, &view_y, &view_z);

        if (visible) {
            if (abs(view_x) >= abs(view_y)) {
                int projected_x = 0;
                int projected_y = 0;
                pIndoorCameraD3D->Project(view_x, view_y, view_z, &projected_x,
                                          &projected_y);

                if (uNumBillboardsToDraw >= 500) return;
                ++uNumBillboardsToDraw;
                ++uNumSpritesDrawnThisFrame;

                pActors[i].uAttributes |= ACTOR_VISIBLE;
                pBillboardRenderList[uNumBillboardsToDraw - 1].hwsprite = v15->hw_sprites[Sprite_Octant];

                if (v15->hw_sprites[Sprite_Octant]->texture->GetHeight() == 0 || v15->hw_sprites[Sprite_Octant]->texture->GetWidth() == 0)
                    __debugbreak();

                pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID = 0;
                pBillboardRenderList[uNumBillboardsToDraw - 1].uPalette = v15->uPaletteIndex;

                auto _v26 = fixed::FromInt(pODMRenderParams->int_fov_rad) /
                            fixed::FromInt(view_x);
                pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x = v15->scale.GetFloat() * _v26.GetFloat();
                pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = v15->scale.GetFloat() * _v26.GetFloat();

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

                pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = v62 | 0x200;
                pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = v15;
                pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor =
                    pMonsterList->pMonsters[pActors[i].pMonsterInfo.uID - 1].sTintColor;  // *((int *)&v35[v36] - 36);
                if (pActors[i].pActorBuffs[ACTOR_BUFF_STONED].Active()) {
                    pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E =
                        v62 | 0x200;
                }
            }
        }
    }
}

int ODM_GetFloorLevel(int X, signed int Y, int Z, int __unused, bool *pIsOnWater,
                      int *bmodel_pid, int bWaterWalk) {
    int v18;                  // edx@26
    int v19;                  // eax@28
    int v22;                  // edx@30
    __int64 v23;              // qtt@30
    int v24;                  // eax@36
    int v25;                  // ecx@38
    int current_floor_level;  // ecx@43
    int v29;                  // edx@44
    int v39;                  // [sp+20h] [bp-20h]@9
    bool current_vertices_Y;  // [sp+30h] [bp-10h]@22
    bool next_vertices_Y;     // [sp+34h] [bp-Ch]@24
    int number_hits;          // [sp+58h] [bp+18h]@22
    int next_floor_level;     // [sp+58h] [bp+18h]@43

    int v46 = 1;
    current_BModel_id[0] = -1;
    current_Face_id[0] = -1;
    odm_floor_level[0] =
        GetTerrainHeightsAroundParty2(X, Y, pIsOnWater, bWaterWalk);

    for (BSPModel &model : pOutdoor->pBModels) {
        if (X <= model.sMaxX && X >= model.sMinX && Y <= model.sMaxY &&
            Y >= model.sMinY) {
            if (!model.pFaces.empty()) {
                v39 = 0;
                for (ODMFace &face : model.pFaces) {
                    if (face.Ethereal()) {
                        continue;
                    }
                    if ((face.uPolygonType == POLYGON_Floor ||
                         face.uPolygonType == POLYGON_InBetweenFloorAndWall) &&
                        X <= face.pBoundingBox.x2 &&
                        X >= face.pBoundingBox.x1 &&
                        Y <= face.pBoundingBox.y2 &&
                        Y >= face.pBoundingBox.y1) {
                        for (uint i = 0; i < face.uNumVertices; ++i) {
                            odm_floor_face_vert_coord_X[2 * i] =
                                face.pXInterceptDisplacements[i] +
                                model.pVertices.pVertices[face.pVertexIDs[i]].x;
                            odm_floor_face_vert_coord_Y[2 * i] =
                                face.pYInterceptDisplacements[i] +
                                model.pVertices.pVertices[face.pVertexIDs[i]].y;
                            odm_floor_face_vert_coord_X[2 * i + 1] =
                                face.pXInterceptDisplacements[i] +
                                model.pVertices
                                    .pVertices[face.pVertexIDs[i + 1]]
                                    .x;
                            odm_floor_face_vert_coord_Y[2 * i + 1] =
                                face.pYInterceptDisplacements[i] +
                                model.pVertices
                                    .pVertices[face.pVertexIDs[i + 1]]
                                    .y;
                        }
                        odm_floor_face_vert_coord_X[2 * face.uNumVertices] =
                            odm_floor_face_vert_coord_X[0];
                        odm_floor_face_vert_coord_Y[2 * face.uNumVertices] =
                            odm_floor_face_vert_coord_Y[0];

                        current_vertices_Y =
                            odm_floor_face_vert_coord_Y[0] >= Y;
                        number_hits = 0;
                        if (2 * face.uNumVertices > 0) {
                            for (int i = 0; i < 2 * face.uNumVertices; ++i) {
                                if (number_hits >= 2) break;
                                // v36 = odm_floor_face_vert_coord_Y[i + 1];
                                next_vertices_Y =
                                    odm_floor_face_vert_coord_Y[i + 1] >= Y;
                                if (current_vertices_Y !=
                                    next_vertices_Y) {  // проверка по Y
                                    v18 =
                                        odm_floor_face_vert_coord_X[i + 1] >= X
                                            ? 0
                                            : 2;
                                    v19 = v18 |
                                          (odm_floor_face_vert_coord_X[i] < X);
                                    if (v19 != 3) {
                                        if (!v19) {
                                            ++number_hits;
                                        } else {
                                            HEXRAYS_LODWORD(v23) =
                                                (Y -
                                                 odm_floor_face_vert_coord_Y[i])
                                                << 16;
                                            HEXRAYS_HIDWORD(v23) =
                                                (Y - odm_floor_face_vert_coord_Y
                                                         [i]) >>
                                                16;
                                            v22 =
                                                ((((odm_floor_face_vert_coord_X
                                                        [i + 1] -
                                                    odm_floor_face_vert_coord_X
                                                        [i]) *
                                                   v23 /
                                                   (odm_floor_face_vert_coord_Y
                                                        [i + 1] -
                                                    odm_floor_face_vert_coord_Y
                                                        [i])) >>
                                                  16) +
                                                 odm_floor_face_vert_coord_X
                                                     [i]);
                                            if (v22 >= X) ++number_hits;
                                        }
                                    }
                                }
                                current_vertices_Y = next_vertices_Y;
                            }
                            if (number_hits == 1) {
                                if (v46 >= 20) break;
                                if (face.uPolygonType == POLYGON_Floor) {
                                    v24 = model.pVertices
                                        .pVertices[face.pVertexIDs[0]]
                                        .z;
                                } else {
                                    int a = fixpoint_mul(face.zCalc1, X);
                                    int b = fixpoint_mul(face.zCalc2, Y);
                                    int c = ((int64_t)face.zCalc3 >> 16);
                                    v24 = a + b + c;
                                }
                                v25 = v46++;
                                odm_floor_level[v25] = v24;
                                current_BModel_id[v25] = model.index;
                                current_Face_id[v25] = face.index;
                            }
                        }
                    }
                }
            }
        }
    }
    if (v46 == 1) {
        *bmodel_pid = 0;
        return odm_floor_level[0];
    }
    current_floor_level = 0;
    v29 = 0;
    if (v46 <= 1) {
        *bmodel_pid = 0;
    } else {
        current_floor_level = odm_floor_level[0];
        for (uint i = 1; i < v46; ++i) {
            next_floor_level = odm_floor_level[i];
            if (current_floor_level <= Z + 5) {
                if (next_floor_level > current_floor_level &&
                    next_floor_level <= Z + 5) {
                    current_floor_level = next_floor_level;
                    v29 = i;
                }
            } else if (next_floor_level < current_floor_level) {
                current_floor_level = next_floor_level;
                v29 = i;
            }
        }
        if (!v29)
            *bmodel_pid = 0;
        else
            *bmodel_pid = current_Face_id[v29] | (current_BModel_id[v29] << 6);
    }
    if (v29) {
        *pIsOnWater = false;
        if (pOutdoor->pBModels[current_BModel_id[v29]]
                .pFaces[current_Face_id[v29]]
                .Fluid())
            *pIsOnWater = true;
    }
    if (odm_floor_level[v29] >= odm_floor_level[0])
        odm_floor_level[0] = odm_floor_level[v29];
    return odm_floor_level[0];
}

// not sure if right- or left-handed coordinate space assumed, so this could be
// normal of inverse normal
// for a right-handed system, that would be an inverse normal
//----- (0046DCC8) --------------------------------------------------------
void ODM_GetTerrainNormalAt(int pos_x, int pos_z, Vec3_int_ *out) {
    uint grid_x = WorldPosToGridCellX(pos_x);
    uint grid_z = WorldPosToGridCellZ(pos_z) - 1;

    int grid_pos_x1 = GridCellToWorldPosX(grid_x);
    int grid_pos_x2 = GridCellToWorldPosX(grid_x + 1);
    int grid_pos_z1 = GridCellToWorldPosZ(grid_z);
    int grid_pos_z2 = GridCellToWorldPosZ(grid_z + 1);

    int x1z1_y = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_z);
    int x2z1_y = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_z);
    int x2z2_y = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_z + 1);
    int x1z2_y = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_z + 1);

    float side1_dx, side1_dy, side1_dz, side2_dx, side2_dy, side2_dz;

    int dx = abs(pos_x - grid_pos_x1), dz = abs(grid_pos_z1 - pos_z);
    if (dz >= dx) {
        side2_dx = (double)(grid_pos_x2 - grid_pos_x1);
        side2_dz =
            0.0;  // (double)(grid_pos_z2 - grid_pos_z2);  // bug?  z2 - z2
        side2_dy = (double)(x2z2_y - x1z2_y);

        side1_dx = 0.0;  // (double)(grid_pos_x1 - grid_pos_x1);
        side1_dz = (double)(grid_pos_z1 - grid_pos_z2);  //       z1 - z2 yes
        side1_dy = (double)(x1z1_y - x1z2_y);
        // logger->Warning(L"%S %S %u\n", __FILE__, __FUNCTION__, __LINE__);
        /*       |\
           side1 |  \
                 |____\
                 side 2      */
    } else {
        side2_dx = (double)(grid_pos_x1 - grid_pos_x2);
        side2_dz = 0.0;  // (double)(grid_pos_z1 - grid_pos_z1);
        side2_dy = (double)(x1z1_y - x2z1_y);

        side1_dx = 0.0;  // (double)(grid_pos_x2 - grid_pos_x1);
        side1_dz = (double)(grid_pos_z2 - grid_pos_z1);
        side1_dy = (double)(x2z2_y - x2z1_y);
        /*   side 2
             _____
             \    |
               \  | side 1
                 \|       */
    }

    float nx = side1_dy * side2_dz - side1_dz * side2_dy;
    float ny = side1_dx * side2_dy - side1_dy * side2_dx;
    float nz = side1_dz * side2_dx - side1_dx * side2_dz;

    float mag = sqrt(nx * nx + ny * ny + nz * nz);
    if (fabsf(mag) < 1e-6f) {
        out->y = 0;
        out->x = 0;
        out->z = 65536;
    } else {
        float invmag = 1.0 / mag;
        out->x = invmag * nx * 65536.0;
        out->y = invmag * ny * 65536.0;
        out->z = invmag * nz * 65536.0;
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
        if (!engine->IsUnderwater() && (pParty->uFlags & (PARTY_FLAGS_1_STANDING_ON_WATER |
                                               PARTY_FLAGS_1_FALLING | 0x04) ||
                             pParty->uFlags & 0x0200 || pParty->bFlying) ||
            !v0) {
            if (pParty->vPosition.x < -22528) pParty->vPosition.x = -22528;
            if (pParty->vPosition.x > 22528) pParty->vPosition.x = 22528;
            if (pParty->vPosition.y < -22528) pParty->vPosition.y = -22528;
            if (pParty->vPosition.y > 22528) pParty->vPosition.y = 22528;
        } else {
            pAudioPlayer->StopChannels(-1, -1);
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
        StringPrintf("TERRA%03d", pParty->uCurrentMinute / 6 + 10 * pParty->uCurrentHour),
        0x7FF);
}

OutdoorLocation::OutdoorLocation() {
    this->log = EngineIoc::ResolveLogger();
    this->decal_builder = EngineIoc::ResolveDecalBuilder();
    this->spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();
    this->lightmap_builder = EngineIoc::ResolveLightmapBuilder();

    subconstuctor();
    uLastSunlightUpdateMinute = 0;
}

void OutdoorLocation::subconstuctor() {
    // OutdoorLocationTerrain::OutdoorLocationTerrain(&this->pTerrain);
    field_F0 = 0;
    field_F4 = 0x40000000u;
    // DLVHeader::DLVHeader(&v1->ddm);
    pSpawnPoints = 0;
    pCmap = 0;
    pFaceIDLIST = 0;
    pOMAP = 0;
}

//----- (00481E55) --------------------------------------------------------
void ODM_Project(unsigned int uNumVertices) {
    for (uint i = 0; i < uNumVertices; i++) {
        memcpy(&VertexRenderList[i], &array_507D30[i],
               sizeof(VertexRenderList[i]));
        VertexRenderList[i].vWorldViewProjX =
            (double)pViewport->uScreenCenterX -
            ((double)pODMRenderParams->int_fov_rad * array_507D30[i]._rhw) *
                array_507D30[i].vWorldViewPosition.y;
        VertexRenderList[i].vWorldViewProjY =
            (double)pViewport->uScreenCenterY -
            ((double)pODMRenderParams->int_fov_rad * array_507D30[i]._rhw) *
                array_507D30[i].vWorldViewPosition.z;
    }
}

//----- (00485F64) --------------------------------------------------------
void ODMRenderParams::Initialize() {  // this seems to be called several times during loading
    int v1;             // eax@1
    int v2;             // eax@2
    signed __int64 v3;  // qtt@4
    int v4;             // eax@4

    this->uCameraFovInDegrees = 75;

    v1 = stru_5C6E00->uPiMask & 0xD5;
    if (v1 >= (signed int)stru_5C6E00->uIntegerHalfPi)
        v2 = -stru_5C6E00->pTanTable[stru_5C6E00->uIntegerPi - v1];
    else
        v2 = stru_5C6E00->pTanTable[v1];
    HEXRAYS_LODWORD(v3) = (viewparams->uSomeZ - viewparams->uSomeX) << 31;
    HEXRAYS_HIDWORD(v3) = (viewparams->uSomeZ - viewparams->uSomeX) << 15 >> 16;
    v4 = (signed int)(v3 / v2) >> 16;
    this->int_fov_rad = v4;
    this->field_4C = 360000;
    this->int_fov_rad_inv = 65536 / v4;
    this->field_50 = 115;
    // sr_6BE060[1] = 1;
    // RotationToInts();
}

//----- (00473893) --------------------------------------------------------
void ODM_ProcessPartyActions() {
    int v1;            // edi@1
    int v2;            // ebx@1
    int floor_level;   // eax@14
    int v34;           // esi@143
    int v35;           // esi@147
    int v36;           // eax@155
    int v40;           // esi@162
    bool v42;          // eax@180
    signed int v43;    // ecx@184
    signed int v44;    // edx@184
    int v45;           // ecx@200
    BSPModel *pModel;  // eax@203
    bool pModel_;
    ODMFace *pODMFace;   // esi@203
    int v48;             // eax@203
    int v54;             // eax@215
    int v55;             // eax@217
    unsigned int v66;    // esi@263
    signed int v68;      // ecx@263
    int v69;             // eax@263
    bool v77;            // edx@297
    bool v78;            // ecx@303
    int v79;             // ecx@314
    __int16 v80;         // dx@317
    int pTerrainHeight;  // eax@321
    int v87;             // [sp-20h] [bp-B4h]@248
    int v97;             // [sp+Ch] [bp-88h]@180
    Vec3_int_ v98;
    bool not_high_fall;  // [sp+1Ch] [bp-78h]@33
    int v102;            // [sp+20h] [bp-74h]@1
    int trigger_id = 0;  // [sp+24h] [bp-70h]@1
    bool bFeatherFall;   // [sp+28h] [bp-6Ch]@4
    int bonus;
    int on_ground;            // [sp+2Ch] [bp-68h]@24
    bool bWaterWalk;          // [sp+30h] [bp-64h]@1
    int ceiling_height;       // [sp+3Ch] [bp-58h]@28
    int v110;                 // [sp+40h] [bp-54h]@180
    int v111;                 // [sp+44h] [bp-50h]@14
    bool hovering;            // [sp+48h] [bp-4Ch]@1
    int v113;                 // [sp+4Ch] [bp-48h]@1
    bool party_running_flag;  // [sp+50h] [bp-44h]@1
    int _walk_speed;          // [sp+54h] [bp-40h]@48
    int pX;                   // [sp+58h] [bp-3Ch]@1
    int pY;                   // [sp+5Ch] [bp-38h]@1
    int party_new_Z;          // [sp+74h] [bp-20h]@1
    int v118;                 // [sp+60h] [bp-34h]@1
    int _angle_x;             // [sp+68h] [bp-2Ch]@48
    unsigned int v122;        // [sp+70h] [bp-24h]@180

    bool party_walking_flag;  // [sp+78h] [bp-1Ch]@1
    int _angle_y;             // [sp+7Ch] [bp-18h]@48
    int v128;                 // [sp+88h] [bp-Ch]@1
    int v129;                 // [sp+8Ch] [bp-8h]@92

    v1 = 0;
    v2 = 0;
    // *(float *)&v128 = 0.0;
    int fall_speed = pParty->uFallSpeed;
    v128 = 0;
    v129 = 0;

    pX = pParty->vPosition.x;
    pY = pParty->vPosition.y;
    party_new_Z = pParty->vPosition.z;

    v113 = pParty->field_6F0;
    hovering = false;
    bool partyAtHighSlope =
        IsTerrainSlopeTooHigh(pParty->vPosition.x, pParty->vPosition.y);
    party_running_flag = false;
    party_walking_flag = false;
    v102 = 0;
    pModel_ = false;
    bWaterWalk = false;
    //************************************
    //Проверка падение пера
    if (!pParty->FeatherFallActive()) {
        bFeatherFall = false;
        for (int i = 0; i < 4; ++i)
            if (pParty->pPlayers[i].WearsItemAnyWhere(
                    ITEM_ARTIFACT_LADYS_ESCORT)) {  // seems like flying boots
                bFeatherFall = true;
                break;
            }
    } else {
        bFeatherFall = true;
    }
    //************************************
    //Проверка хождения по воде
    pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
    if (pParty->WaterWalkActive()) {
        bWaterWalk = true;
        *(short *)&stru_5E4C90_MapPersistVars._decor_events
             [20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uOverlayID +
              119] |= 1;
        if (!(pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uFlags & 1) &&
            pParty->pPlayers
                    [pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uCaster - 1]
                        .sMana <= 0)
            bWaterWalk = false;
    }
    //*************************************
    //определение уровня пола
    int bmodel_standing_on_pid;  //данные 3D model'и
    bool is_on_water = false;     //на воду
    floor_level =
        ODM_GetFloorLevel(pX, pY, party_new_Z, pParty->uPartyHeight,
                          &is_on_water, &bmodel_standing_on_pid, bWaterWalk);
    int is_not_on_bmodel = bmodel_standing_on_pid == 0;  //не на 3D model

    v111 = floor_level;  //???
    //************************************
    //определение высоты падения
    if (bFeatherFall)  //падение пера
        pParty->uFallStartY = floor_level;
    else
        floor_level = pParty->uFallStartY;
    //*************************************
    //падение на 3D Model
    if (floor_level - party_new_Z > 512 && !bFeatherFall &&
        party_new_Z <= v111 + 1) {
        if (pParty->uFlags & PARTY_FLAGS_1_LANDING) {
            pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
        } else {
            for (int i = 0; i < 4; ++i) {  // receive falling damage
                if (!pParty->pPlayers[i].HasEnchantedItemEquipped(72) &&
                    !pParty->pPlayers[i].WearsItem(ITEM_ARTIFACT_HERMES_SANDALS,
                        EQUIP_BOOTS)) {
                    pParty->pPlayers[i].ReceiveDamage(
                        (int)((pParty->uFallStartY - party_new_Z) *
                        (uint64_t)(pParty->pPlayers[i]
                            .GetMaxHealth() /
                            10)) /
                        256,
                        DMGT_PHISYCAL);
                    bonus = 20 - pParty->pPlayers[i].GetParameterBonus(
                        pParty->pPlayers[i].GetActualEndurance());
                    pParty->pPlayers[i].SetRecoveryTime(
                        (signed __int64)((double)bonus *
                            flt_6BE3A4_debug_recmod1 *
                            2.133333333333333));
                }
            }
        }
    }
    //*********************************
    //определение высоты потолка
    ceiling_height = -1;
    if (pParty->bFlying)  //в полёте
        ceiling_height =
            GetCeilingHeight(pX, pY, party_new_Z + pParty->uPartyHeight,
                             (int)&v102);  //высота потолка
    // v107 = bmodel_standing_on_pid == 0;
    on_ground = v111 + 1;  //на земле
    //**************************************

    if (party_new_Z <= on_ground) {  //полёт: посадка
        ceiling_height = -1;
        pParty->bFlying = false;
    } else {
      hovering = true;
    }
    not_high_fall = party_new_Z - v111 <= 32;
    //****************************************
    // timer update(обновить таймер звука ходьбы)
    if (!engine->config->NoWalkSound() && pParty->walk_sound_timer) {
        if (pParty->walk_sound_timer >= pEventTimer->uTimeElapsed)
            pParty->walk_sound_timer -= pEventTimer->uTimeElapsed;
        else
            pParty->walk_sound_timer = 0;
    }

    //****************************************
    // check if we should be flying
    if (!engine->IsUnderwater() && !pParty->pPartyBuffs[PARTY_BUFF_FLY].Active())
        pParty->bFlying = false;

    //*****************************************
    // установить на чём стоит группа
    if (!hovering) {  // не в воздухе
        if (pParty->floor_face_pid !=
            PID(OBJECT_BModel, bmodel_standing_on_pid)) {
            if (bmodel_standing_on_pid) {
                int BModel_id = bmodel_standing_on_pid >> 6;
                if (BModel_id < pOutdoor->pBModels.size()) {
                    int face_id = bmodel_standing_on_pid & 0x3F;
                    if (pOutdoor->pBModels[BModel_id]
                            .pFaces[face_id]
                            .uAttributes &
                        FACE_PRESSURE_PLATE) {
                        pParty->floor_face_pid =
                            PID(OBJECT_BModel, bmodel_standing_on_pid);
                        trigger_id =
                            pOutdoor->pBModels[BModel_id]
                                .pFaces[face_id]
                                .sCogTriggeredID;  // EVT, панель имеет событие
                    }
                }
            }
        }
        pParty->floor_face_pid =
            PID(OBJECT_BModel, bmodel_standing_on_pid);  // 6 - на земле
    }
    //***********************************************
    _walk_speed = pParty->uWalkSpeed;
    _angle_y = pParty->sRotationY;
    _angle_x = pParty->sRotationX;
    // v126 = pEventTimer->dt_in_some_format;
    /*v119 = (Player **)((unsigned __int64)(pEventTimer->dt_in_some_format
                                        * (signed __int64)((signed
       int)(pParty->field_20_prolly_turn_speed
                                                                      *
       stru_5C6E00->uIntegerPi) / 180)) >> 16);*/
    __int64 dturn =
        (unsigned __int64)(pEventTimer->dt_in_some_format *
                           (signed __int64)((signed int)(pParty
                                                             ->y_rotation_speed *
                                                         stru_5C6E00
                                                             ->uIntegerPi) /
                                            180)) >>
        16;
    while (pPartyActionQueue->uNumActions) {
        switch (pPartyActionQueue->Next()) {
            case PARTY_FlyUp:  //полёт вверх
            {
                if (!pParty->FlyActive() && !engine->IsUnderwater()) break;

                pParty->bFlying = false;
                if (engine->IsUnderwater() ||
                    pParty->pPartyBuffs[PARTY_BUFF_FLY].uFlags & 1 ||
                    (pParty->pPlayers
                            [pParty->pPartyBuffs[PARTY_BUFF_FLY].uCaster - 1]
                                .sMana > 0 || engine->config->debug_all_magic)) {
                    if (pParty->vPosition.z < engine->config->max_flight_height || hovering) {
                        party_new_Z += 30;
                        v113 += 30;
                        pParty->bFlying = true;
                        if (party_new_Z > engine->config->max_flight_height) {
                            party_new_Z = engine->config->max_flight_height;
                            v113 = engine->config->max_flight_height;
                        }
                        v1 = 0;
                        v2 = 0;
                        fall_speed = 0;
                        *(float *)&v128 = 0.0;
                        if (v102 && party_new_Z < ceiling_height &&
                            (signed int)(pParty->uPartyHeight + party_new_Z) >=
                                ceiling_height) {  // столкновение с потолком
                            pParty->field_6E0 = 0;
                            pParty->field_6E4 = 0;
                            pPartyActionQueue->uNumActions = 0;
                            pParty->uFlags |= PARTY_FLAGS_1_LANDING;
                            pParty->vPosition.z =
                                ceiling_height - pParty->uPartyHeight - 31;
                            pParty->field_6F0 = party_new_Z;
                            pParty->bFlying = false;
                            party_new_Z =
                                ceiling_height - pParty->uPartyHeight - 31;
                            v113 = pParty->field_6F0;
                        }
                        pParty->uFallSpeed = 0;
                        pModel_ = true;
                    }
                }
            } break;

            case PARTY_FlyDown:  //полёт вниз
                if (pParty->FlyActive() || engine->IsUnderwater()) {
                    pParty->bFlying = false;
                    if (engine->IsUnderwater() ||
                        pParty->pPartyBuffs[PARTY_BUFF_FLY].uFlags & 1 ||
                        pParty->pPlayers[pParty->pPartyBuffs[PARTY_BUFF_FLY].uCaster - 1].sMana > 0) {
                            // *(int *)&pParty->pArtifactsFound[6972 *
                            // pParty->pPartyBuffs[PARTY_BUFF_FLY].uCaster + 10] > 0 )
                        party_new_Z -= 30;
                        v113 -= 30;
                        pParty->uFallSpeed = 0;
                        fall_speed = 0;
                        pParty->bFlying = true;
                        pModel_ = true;
                        if (party_new_Z <= v111) {
                            pParty->bFlying = false;
                            pPartyActionQueue->uNumActions = 0;
                        }
                    }
                }
                break;

            case PARTY_TurnLeft:  //поворот влево
                if (engine->config->turn_speed > 0)
                    _angle_y += engine->config->turn_speed;  // descrete turn
                else
                    _angle_y += dturn * fTurnSpeedMultiplier;  // time-based smooth turn

                _angle_y &= stru_5C6E00->uDoublePiMask;
                break;

            case PARTY_TurnRight:  //поворот вправо
                if (engine->config->turn_speed > 0)
                    _angle_y -= engine->config->turn_speed;
                else
                    _angle_y -= dturn * fTurnSpeedMultiplier;

                _angle_y &= stru_5C6E00->uDoublePiMask;
                break;

            case PARTY_FastTurnLeft:  //быстрый поворот влево
                if (engine->config->turn_speed > 0)
                    _angle_y += engine->config->turn_speed;
                else
                    _angle_y += 2.0f * fTurnSpeedMultiplier * (double)dturn;

                _angle_y &= stru_5C6E00->uDoublePiMask;
                break;

            case PARTY_FastTurnRight:  //быстрый поворот вправо
                if (engine->config->turn_speed > 0)
                    _angle_y -= engine->config->turn_speed;
                else
                    _angle_y -= 2.0f * fTurnSpeedMultiplier * (double)dturn;

                _angle_y &= stru_5C6E00->uDoublePiMask;
                break;

            case PARTY_StrafeLeft:  //хождение боком в влево
            {
                *(float *)&v128 = pParty->uWalkSpeed;

                float sin_y = sinf(2 * pi_double * _angle_y / 2048.0);
                int dx = sin_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                v2 -= 3 * dx / 4;

                float cos_y = cosf(2 * pi_double * _angle_y / 2048.0);
                int dy = cos_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                v1 += 3 * dy / 4;

                v128 = v1;
                party_walking_flag = true;
            } break;

            case PARTY_StrafeRight:  //хождение боком в вправо
            {
                *(float *)&v128 = pParty->uWalkSpeed;

                float sin_y = sinf(2 * pi_double * _angle_y / 2048.0);
                int dx = sin_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                v2 += 3 * dx / 4;

                float cos_y = cosf(2 * pi_double * _angle_y / 2048.0);
                int dy = cos_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                v1 -= 3 * dy / 4;

                v128 = v1;
                party_walking_flag = true;
            } break;

            case PARTY_WalkForward:  // идти вперёд
            {
                *(float *)&v128 = _walk_speed;

                float sin_y = sinf(2 * pi_double * _angle_y / 2048.0),
                      cos_y = cosf(2 * pi_double * _angle_y / 2048.0);

                int dx = cos_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                int dy = sin_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;

                if (engine->config->debug_turbo_speed) {
                    v2 += dx * 12;
                    v1 += dy * 12;
                } else {
                    v2 += dx;
                    v1 += dy;
                }

                v128 = v1;
                party_walking_flag = true;
            } break;

            case PARTY_RunForward:  //бежать вперёд
            {
                *(float *)&v128 = _walk_speed;

                float sin_y = sinf(2 * pi_double * _angle_y / 2048.0);
                float cos_y = cosf(2 * pi_double * _angle_y / 2048.0);

                int dx = cos_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                int dy = sin_y * pParty->uWalkSpeed * fWalkSpeedMultiplier;

                if (pParty->bFlying) {  // лететь вперёд
                    v2 += 4 * dx;
                    v1 += 4 * dy;

                    v128 = v1;
                } else if (partyAtHighSlope &&
                           !bmodel_standing_on_pid) {  // сбегание со склона
                    v2 += dx;
                    v1 += dy;

                    v128 = v1;
                    party_walking_flag = true;
                } else {
                    /*v2 += (unsigned __int64)(stru_5C6E00->Cos(_angle_y)
                                     * (signed __int64)(signed int)(2 *
                    (unsigned __int64)(signed __int64)((double)_walk_speed *
                    fWalkSpeedMultiplier))) >> 16; v1 += (unsigned
                    __int64)((signed int)stru_5C6E00->Sin(_angle_y)
                                     * (signed __int64)(signed int)(2 *
                    (unsigned __int64)(signed __int64)((double)_walk_speed *
                    fWalkSpeedMultiplier))) >> 16;*/
                    if (engine->config->debug_turbo_speed) {
                        v2 += dx * 12;
                        v1 += dy * 12;
                    } else {
                        v2 += 2 * dx;
                        v1 += 2 * dy;
                    }

                    v128 = v1;
                    party_running_flag = true;
                }
            } break;

            case PARTY_WalkBackward: {  // идти назад
                *(float *)&v128 = _walk_speed;

                float sin_y = sinf(2 * pi_double * _angle_y / 2048.0);
                float cos_y = cosf(2 * pi_double * _angle_y / 2048.0);

                int dx = cos_y * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                v2 -= dx;

                int dy =
                    sin_y * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                v1 -= dy;

                v128 = v1;
                party_walking_flag = true;
            } break;

            case PARTY_RunBackward:  //бежать назад
            {
                float sin_y = sinf(2 * pi_double * _angle_y / 2048.0);
                float cos_y = cosf(2 * pi_double * _angle_y / 2048.0);

                int dx =
                    cos_y * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                int dy =
                    sin_y * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;

                if (pParty->bFlying) {
                    v2 -= 4 * dx;
                    v1 -= 4 * dy;
                    v128 = v1;
                } else {
                    v2 -= dx;
                    v1 -= dy;

                    v128 = v1;
                    party_walking_flag = true;
                }
            } break;

            case PARTY_CenterView:  //смотреть прямо
                _angle_x = 0;
                break;

            case PARTY_LookUp:  //смотреть вверх
                _angle_x +=
                    (signed __int64)(flt_6BE150_look_up_down_dangle * 25.0);
                if (_angle_x > 128) _angle_x = 128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_63, 0);
                break;

            case PARTY_LookDown:  //смотреть вниз
                _angle_x +=
                    (signed __int64)(flt_6BE150_look_up_down_dangle * -25.0);
                if (_angle_x < -128) _angle_x = -128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_64, 0);
                break;

            case PARTY_Jump:  //прыжок
                if ((!partyAtHighSlope || bmodel_standing_on_pid) &&
                    !hovering && pParty->field_24 && !(pParty->uFlags & 4) &&
                    !(pParty->uFlags & 0x200)) {
                    // v126 = pParty->field_24 << 6;
                    hovering = true;
                    fall_speed =
                        (signed __int64)((double)(pParty->field_24 << 6) * 1.5 +
                                         (double)fall_speed);
                }
                break;

            case PARTY_Land:  //приземление(клавиша Home)
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

    pParty->sRotationY = _angle_y;
    pParty->sRotationX = _angle_x;
    //-------------------------------------------
    if (pParty->bFlying) {
        v129 = fixpoint_mul(4, stru_5C6E00->Cos(OS_GetTime()));
        party_new_Z = v113 + v129;
        if (pModel_) party_new_Z = v113;
        if (pParty->FlyActive())
            stru_5E4C90_MapPersistVars._decor_events
                [20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].uOverlayID + 119] &=
                0xFE;
        pParty->uFallStartY = party_new_Z;
    } else if (party_new_Z < v111) {
        if (is_on_water && fall_speed)
            SpriteObject::sub_42F960_create_object(pX, pY, v111);
        fall_speed = 0;
        party_new_Z = v111;
        pParty->uFallStartY = v111;
        v113 = party_new_Z;
        if (pParty->FlyActive())
            stru_5E4C90_MapPersistVars._decor_events
                [20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].uOverlayID + 119] |=
                1;
    } else {
        v113 = party_new_Z;
        if (pParty->FlyActive())
            stru_5E4C90_MapPersistVars._decor_events
                [20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].uOverlayID + 119] |=
                1;
    }
    //------------------------------------------
    if (hovering && !pParty->bFlying) {  // расчёт скорости падения
        // v33 = -(pEventTimer->uTimeElapsed * GetGravityStrength());
        v34 = fall_speed +
              (-(pEventTimer->uTimeElapsed * GetGravityStrength()) << 1);
        fall_speed += (-(pEventTimer->uTimeElapsed * GetGravityStrength())
                       << 1);  // y(t) = 2*gt
    } else if (!partyAtHighSlope) {
        v34 = fall_speed;
    } else if (!hovering) {
        if (!bmodel_standing_on_pid) {
            // rolling down the hill
            // how it's done: you get a little bit pushed in the air along
            // terrain normal, getting in the air and falling to the gravity,
            // gradually sliding downwards. nice trick
            party_new_Z = v111;
            ODM_GetTerrainNormalAt(pX, pY, &v98);
            v35 = fall_speed +
                  (8 * -(pEventTimer->uTimeElapsed * GetGravityStrength()));
            v129 = abs(v2 * v98.x + v1 * v98.y + v35 * v98.z) >> 16;
            v2 += fixpoint_mul(v129, v98.x);
            v1 += fixpoint_mul(v129, v98.y);
            v34 = v35 + fixpoint_mul(v129, v98.z);
            v128 = v1;
            fall_speed = v34;
        }
    } else {
        v34 = fall_speed;
    }

    if (hovering) {  // блок для крика падения
        if (!engine->IsUnderwater() && v34 <= 0) {
            if (v34 < -500 && !pParty->bFlying &&
                pParty->vPosition.z - v111 > 1000 &&
                !pParty->FeatherFallActive()) {  // falling scream
                for (int i = 0; i < 4; ++i) {
                    if (!pParty->pPlayers[i].HasEnchantedItemEquipped(72) &&
                        !pParty->pPlayers[i].WearsItem(
                            ITEM_ARTIFACT_HERMES_SANDALS, EQUIP_BOOTS) &&
                        pParty->pPlayers[i].CanAct())
                        pParty->pPlayers[i].PlaySound(SPEECH_Falling_scream,
                                                      0);  //крик падения
                }
            }
        }
    } else {
      pParty->uFallStartY = party_new_Z;
    }

    if (v2 * v2 + v1 * v1 < 400 && !partyAtHighSlope) {
        *(float *)&v128 = 0.0;
        v2 = 0;
    }
    //--(столкновения)-------------------------------------------------------------------
    stru_721530.field_84 = -1;
    stru_721530.field_70 = 0;
    stru_721530.prolly_normal_d = pParty->field_14_radius;
    stru_721530.field_8_radius = pParty->field_14_radius / 2;
    stru_721530.field_0 = 1;
    stru_721530.height = pParty->uPartyHeight - 32;
    for (uint i = 0; i < 100; i++) {
        stru_721530.position.x = pX;
        stru_721530.position.y = pY;
        stru_721530.position.z = stru_721530.height + party_new_Z + 1;

        stru_721530.normal.x = pX;
        stru_721530.normal.y = pY;
        stru_721530.normal.z = stru_721530.prolly_normal_d + party_new_Z + 1;

        stru_721530.velocity.x = v2;
        stru_721530.velocity.y = v128;
        stru_721530.velocity.z = fall_speed;

        stru_721530.uSectorID = 0;
        v36 = 0;
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
            v36 = 13312;
        }
        if (stru_721530._47050A(v36)) break;
        _46E889_collide_against_bmodels(1);
        // v37 = WorldPosToGridCellZ(pParty->vPosition.y);
        // v38 = WorldPosToGridCellX(pParty->vPosition.x);
        _46E26D_collide_against_sprites(
            WorldPosToGridCellX(pParty->vPosition.x),
            WorldPosToGridCellZ(pParty->vPosition.y));
        _46ED8A_collide_against_sprite_objects(4);
        for (uint actor_id = 0; actor_id < (signed int)uNumActors; ++actor_id)
            Actor::_46DF1A_collide_against_actor(actor_id, 0);
        if (stru_721530.field_7C >= stru_721530.field_6C) {
            _angle_x = stru_721530.normal2.x;
            _angle_y = stru_721530.normal2.y;
            v40 = stru_721530.normal2.z - stru_721530.prolly_normal_d - 1;
        } else {
            _angle_x = pX + fixpoint_mul(stru_721530.field_7C,
                                         stru_721530.direction.x);
            _angle_y = pY + fixpoint_mul(stru_721530.field_7C,
                                         stru_721530.direction.y);
            // pModel = (BSPModel *)fixpoint_mul(stru_721530.field_7C,
            // stru_721530.direction.z);
            v40 = fixpoint_mul(stru_721530.field_7C, stru_721530.direction.z) +
                  party_new_Z;
        }
        v122 = v40;
        ODM_GetFloorLevel(_angle_x, _angle_y, v40, pParty->uPartyHeight,
                          &is_on_water, &bmodel_standing_on_pid, 0);
        v129 = ODM_GetFloorLevel(_angle_x, pY, v40, pParty->uPartyHeight,
                                 &is_on_water, &v97, 0);
        int v119 = ODM_GetFloorLevel(pX, _angle_y, v40, pParty->uPartyHeight,
                                     &is_on_water, &v110, 0);
        bool v42_ = (BSPModel *)IsTerrainSlopeTooHigh(_angle_x, pY);
        v42 = IsTerrainSlopeTooHigh(pX, _angle_y);
        is_not_on_bmodel = false;
        // v118 = v42;
        if (!v97 && !v110 && !bmodel_standing_on_pid) is_not_on_bmodel = true;
        v43 = 1;
        v44 = 1;
        if (engine->IsUnderwater() || !is_not_on_bmodel) {
            pX = _angle_x;
            if (v43) pY = _angle_y;
        } else {
            if (v42_ && v129 > party_new_Z) v44 = 0;
            if (v42 && v119 > party_new_Z) v43 = 0;
            if (v44) {
                pX = _angle_x;
                if (v43) pY = _angle_y;
            } else if (v43) {
                pY = _angle_y;
            } else {
                int new_ = ODM_GetFloorLevel(_angle_x, _angle_y, v40,
                                             pParty->uPartyHeight, &is_on_water,
                                             &bmodel_standing_on_pid, 0);
                if (IsTerrainSlopeTooHigh(_angle_x, _angle_y) &&
                    new_ <= party_new_Z) {
                    v43 = 1;
                    pX = _angle_x;
                    if (v43) pY = _angle_y;
                }
            }
        }
        if (stru_721530.field_7C >= stru_721530.field_6C) {
            if (!is_not_on_bmodel) {
                pX = stru_721530.normal2.x;
                pY = stru_721530.normal2.y;
            }
            party_new_Z =
                stru_721530.normal2.z - stru_721530.prolly_normal_d - 1;
            break;
        }
        stru_721530.field_70 += stru_721530.field_7C;
        pX = _angle_x;
        pY = _angle_y;
        v45 = stru_721530.pid;
        party_new_Z = v40;

        if (PID_TYPE(stru_721530.pid) == OBJECT_Actor) {
            if (pParty->Invisible())
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
            viewparams->bRedrawGameUI = true;
        }

        if (PID_TYPE(stru_721530.pid) == OBJECT_Decoration) {
            v129 = stru_5C6E00->Atan2(
                _angle_x - pLevelDecorations[(signed int)stru_721530.pid >> 3]
                               .vPosition.x,
                _angle_y - pLevelDecorations[(signed int)stru_721530.pid >> 3]
                               .vPosition.y);
            v2 = fixpoint_mul(stru_5C6E00->Cos(v129),
                              integer_sqrt(v2 * v2 + v128 * v128));
            v122 = fixpoint_mul(stru_5C6E00->Sin(v129),
                                integer_sqrt(v2 * v2 + v128 * v128));
            v128 = fixpoint_mul(stru_5C6E00->Sin(v129),
                                integer_sqrt(v2 * v2 + v128 * v128));
        }

        if (PID_TYPE(stru_721530.pid) == OBJECT_BModel) {
            pParty->bFlying = false;
            pModel = &pOutdoor->pBModels[(signed int)stru_721530.pid >> 9];
            pODMFace =
                &pModel->pFaces[((signed int)stru_721530.pid >> 3) & 0x3F];
            v48 = pODMFace->pBoundingBox.z2 - pODMFace->pBoundingBox.z1;
            v129 = v48 <= 32;
            v119 = pODMFace->pFacePlane.vNormal.z < 46378;
            if (engine->IsUnderwater())
                v119 = 0;
            if (pODMFace->uPolygonType == POLYGON_Floor) {
                if (fall_speed < 0) fall_speed = 0;
                party_new_Z =
                    pModel->pVertices.pVertices[pODMFace->pVertexIDs[0]].z + 1;
                if (v2 * v2 + v128 * v128 < 400) {
                    v2 = 0;
                    *(float *)&v128 = 0.0;
                }
                if (pParty->floor_face_pid != v45 &&
                    pODMFace->Pressure_Plate()) {
                    pParty->floor_face_pid = v45;
                    trigger_id =
                        pODMFace->sCogTriggeredID;  // this one triggers tour
                                                    // events??
                }
            }
            if (!v129 &&
                (pODMFace->uPolygonType != POLYGON_InBetweenFloorAndWall ||
                 v119)) {  // упёрся в столб
                v118 = abs(v128 * pODMFace->pFacePlane.vNormal.y +
                           fall_speed * pODMFace->pFacePlane.vNormal.z +
                           v2 * pODMFace->pFacePlane.vNormal.x) >>
                       16;
                if ((stru_721530.speed >> 3) > v118)
                    v118 = stru_721530.speed >> 3;
                v2 += fixpoint_mul(v118, pODMFace->pFacePlane.vNormal.x);
                v128 += fixpoint_mul(v118, pODMFace->pFacePlane.vNormal.y);
                v54 = 0;
                if (!v119)
                    v54 = fixpoint_mul(v118, pODMFace->pFacePlane.vNormal.z);
                pParty->uFallSpeed += v54;
                v55 =
                    stru_721530.prolly_normal_d -
                    ((signed int)(pODMFace->pFacePlane.dist +
                                  v122 * pODMFace->pFacePlane.vNormal.z +
                                  _angle_y * pODMFace->pFacePlane.vNormal.y +
                                  _angle_x * pODMFace->pFacePlane.vNormal.x) >>
                     16);
                if (v55 > 0) {
                    pX = _angle_x +
                         fixpoint_mul(pODMFace->pFacePlane.vNormal.x, v55);
                    pY = _angle_y +
                         fixpoint_mul(pODMFace->pFacePlane.vNormal.y, v55);
                    if (!v119)
                        party_new_Z =
                            v122 +
                            fixpoint_mul(pODMFace->pFacePlane.vNormal.z, v55);
                }
                if (pParty->floor_face_pid != stru_721530.pid &&
                    pODMFace->Pressure_Plate()) {
                    pParty->floor_face_pid = stru_721530.pid;
                    trigger_id = pODMFace->sCogTriggeredID;  //
                }
            }
            if (pODMFace->uPolygonType == POLYGON_InBetweenFloorAndWall) {
                v118 = abs(v128 * pODMFace->pFacePlane.vNormal.y +
                           fall_speed * pODMFace->pFacePlane.vNormal.z +
                           v2 * pODMFace->pFacePlane.vNormal.x) >>
                       16;
                if ((stru_721530.speed >> 3) > v118)
                    v118 = stru_721530.speed >> 3;
                v2 += fixpoint_mul(v118, pODMFace->pFacePlane.vNormal.x);
                v128 += fixpoint_mul(v118, pODMFace->pFacePlane.vNormal.y);
                fall_speed +=
                    fixpoint_mul(v118, pODMFace->pFacePlane.vNormal.z);
                if (v2 * v2 + v128 * v128 >= 400) {
                    if (pParty->floor_face_pid != stru_721530.pid &&
                        pODMFace->Pressure_Plate()) {
                        pParty->floor_face_pid = stru_721530.pid;
                        trigger_id = pODMFace->sCogTriggeredID;  //
                    }
                } else {
                    v2 = 0;
                    fall_speed = 0;
                    *(float *)&v128 = 0.0;
                }
            }
        }
        v2 = fixpoint_mul(58500, v2);
        v128 = fixpoint_mul(58500, v128);
        v122 = fixpoint_mul(58500, v122);
        fall_speed = fixpoint_mul(58500, fall_speed);
    }

    //Воспроизведение звуков ходьбы/бега------------------------
    uint pX_ = abs(pParty->vPosition.x - pX);
    uint pY_ = abs(pParty->vPosition.y - pY);
    uint pZ_ = abs(pParty->vPosition.z - party_new_Z);
    if (!engine->config->NoWalkSound() && pParty->walk_sound_timer <= 0) {
        pAudioPlayer->StopAll(804);  // stop sound
        if (party_running_flag && (!hovering || not_high_fall)) {
            if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) >= 16) {
                if (!is_not_on_bmodel &&
                    pOutdoor->pBModels[pParty->floor_face_pid >> 9]
                    .pFaces[(pParty->floor_face_pid >> 3) & 0x3F].Visible()) {
                    pAudioPlayer->PlaySound(SOUND_RunWood, -1 /*804*/, 1, -1, 0, 0);  //бег на 3D Modelи
                } else {
                    v87 = pOutdoor->GetSoundIdByPosition(
                        WorldPosToGridCellX(pParty->vPosition.x),
                        WorldPosToGridCellZ(pParty->vPosition.y) - 1, 1);
                    pAudioPlayer->PlaySound((SoundID)v87, -1 /*804*/, 1, -1, 0, 0);  //бег по земле 56
                }
                pParty->walk_sound_timer = 96;  // таймер для бега
            }
        } else if (party_walking_flag && (!hovering || not_high_fall)) {
            if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) >= 8) {
                if (!is_not_on_bmodel &&
                    pOutdoor->pBModels[pParty->floor_face_pid >> 9]
                    .pFaces[(pParty->floor_face_pid >> 3) & 0x3F].Visible()) {
                    pAudioPlayer->PlaySound(SOUND_WalkWood, -1 /*804*/, 1, -1, 0, 0);  // хождение на 3D Modelи
                } else {
                    v87 = pOutdoor->GetSoundIdByPosition(
                        WorldPosToGridCellX(pParty->vPosition.x),
                        WorldPosToGridCellZ(pParty->vPosition.y) - 1, 0);
                    pAudioPlayer->PlaySound((SoundID)v87, -1 /*804*/, 1, -1, 0, 0);  // хождение по земле
                }
                pParty->walk_sound_timer = 144;  // таймер для ходьбы
            }
        }
    }

    if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) < 8)  //отключить  звук ходьбы при остановке
        pAudioPlayer->StopAll(804);
    //------------------------------------------------------------------------
    if (!hovering || !not_high_fall)  //  или не высокое падение
        pParty->uFlags &= ~PARTY_FLAGS_1_FALLING;
    else
        pParty->uFlags |= PARTY_FLAGS_1_FALLING;
    int pMap_X = WorldPosToGridCellX(pParty->vPosition.x);
    int pMap_Y = WorldPosToGridCellZ(pParty->vPosition.y) - 1;
    unsigned int v114_a = WorldPosToGridCellX(pX);
    v66 = WorldPosToGridCellZ(pY) - 1;
    unsigned int v122_a = (~(unsigned int)pOutdoor->ActuallyGetSomeOtherTileInfo(pMap_X, pMap_Y) / 2) & 1;
    v122 = (~(unsigned int)pOutdoor->ActuallyGetSomeOtherTileInfo(v114_a, pMap_Y) / 2) & 1;
    v69 = (~(unsigned int)pOutdoor->ActuallyGetSomeOtherTileInfo(pMap_X, v66) / 2) & 1;

    //-(обновление координат группы)---------------------------------------
    v68 = 0;
    if (v114_a == pMap_X && v66 == pMap_Y && v122 && v69) v68 = 1;
    if (!is_not_on_bmodel)  // на bmodel,и
        v68 = 1;
    if (v68) {
        pParty->vPosition.x = pX;
        pParty->vPosition.y = pY;
        pParty->vPosition.z = party_new_Z;
        pParty->field_6F0 = v113;
        pParty->uFallSpeed = fall_speed;
        if (party_new_Z > 8160) {  // ограничение высоты
            party_new_Z = 8160;
            pParty->uFallStartY = 8160;
            pParty->vPosition.z = 8160;
        }

        if (!trigger_id  //падение на землю
            || (EventProcessor(trigger_id, 0, 1), pParty->vPosition.x == pX) &&
                   pParty->vPosition.y == pY &&
                   pParty->vPosition.z == party_new_Z) {
            if (pParty->vPosition.z < v111) {
                pParty->uFallSpeed = 0;
                // v73 = v105;
                pParty->vPosition.z = on_ground;
                if (pParty->uFallStartY - party_new_Z > 512 && !bFeatherFall &&
                    party_new_Z <= on_ground &&
                    !engine->IsUnderwater()) {  // Fall to the ground(падение на землю с
                                     // высоты)
                    if (pParty->uFlags & PARTY_FLAGS_1_LANDING) {
                        pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
                    } else {
                        for (uint i = 1; i <= 4; ++i) {
                            pPlayers[i]->ReceiveDamage(
                                (int)((pParty->uFallStartY -
                                              party_new_Z) *
                                             (uint64_t)((double)pPlayers[i]->GetMaxHealth() * 0.1)) /
                                    256,
                                DMGT_PHISYCAL);
                            v110 = 20 - pPlayers[i]->GetParameterBonus(
                                            pPlayers[i]->GetActualEndurance());
                            pPlayers[i]->SetRecoveryTime(
                                (signed __int64)((double)v110 *
                                                 flt_6BE3A4_debug_recmod1 *
                                                 2.133333333333333));
                        }
                        // v73 = pParty->vPosition.z;
                    }
                }
                pParty->uFallStartY = party_new_Z;
            }
            if (v102 && pParty->vPosition.z < ceiling_height) {
                if ((signed int)(pParty->uPartyHeight + pParty->vPosition.z) >=
                    ceiling_height) {
                    pParty->vPosition.z =
                        ceiling_height - pParty->uPartyHeight - 1;
                    pParty->field_6F0 =
                        ceiling_height - pParty->uPartyHeight - 1;
                }
            }
            pParty->uFlags &= ~0x204;
        }
        return;
    }
    //-----------------------------------------------------------------
    // v76 = pParty->bFlying;
    if (pParty->bFlying || !not_high_fall || bWaterWalk ||
        !v122_a)  // полёт или высокое падение или хождение по воде или
        v77 = 1;
    else
        v77 = v122 != 0;
    bool party_drowning_flag = false;
    if (!pParty->bFlying && not_high_fall &&
        !bWaterWalk) {  // не полёт и не высокое падение и не хождение по воде
        if (v122_a) {
            v78 = v69 != 0;
        } else {
            party_drowning_flag = true;  //утопление
            v78 = true;
        }
    } else {
      v78 = true;
    }

    if (v77) pParty->vPosition.x = pX;
    if (v78) pParty->vPosition.y = pY;

    if (v78 || v77) {
        if (bWaterWalk) {
            pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
            // v79 = 20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uOverlayID
            // + 6180178;
            // *(short *)&stru_5E4C90._decor_events[20 *
            // pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uOverlayID + 119] |=
            // 1u;
            v79 =
                (int)&stru_5E4C90_MapPersistVars._decor_events
                    [20 *
                         pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uOverlayID +
                     119];
            *(short *)v79 |= 1;
            if (!v122 || !v69) {
                if (!pParty->bFlying) {
                    v80 = *(short *)v79;
                    pParty->uFlags |= PARTY_FLAGS_1_STANDING_ON_WATER;
                    *(short *)v79 = v80 & 0xFFFE;
                }
            }
        }
    } else if (!engine->config->NoWalkSound() && pParty->walk_sound_timer <= 0) {
        pAudioPlayer->StopAll(804);
        pParty->walk_sound_timer = 64;
    }

    // v81 = pZ;
    // v82 = pZ;
    pParty->vPosition.z = party_new_Z;
    if (party_new_Z > 8160) {  // опять ограничение высоты
        // v82 = 8160;
        pParty->uFallStartY = 8160;
        pParty->vPosition.z = 8160;
    }
    HEXRAYS_LOWORD(pParty->uFlags) &= 0xFDFBu;
    pParty->uFallSpeed = fall_speed;
    pParty->field_6F0 = v113;
    if (party_drowning_flag) {  // группа тонет
        bool onWater = false;
        pTerrainHeight = GetTerrainHeightsAroundParty2(pParty->vPosition.x, pParty->vPosition.y, &onWater, 1);
        if (pParty->vPosition.z <= pTerrainHeight + 1) {  //положение группы всегда +1
            pParty->uFlags |= PARTY_FLAGS_1_WATER_DAMAGE;
        }
    }

    if (!trigger_id  //падение на воду
        ||
        (EventProcessor(trigger_id, 0, 1), pParty->vPosition.x == pX) &&
            pParty->vPosition.y == pY && pParty->vPosition.z == party_new_Z) {
        if (pParty->vPosition.z < v111) {
            // v82 = on_ground;
            pParty->uFallSpeed = 0;
            pParty->vPosition.z = on_ground;
            if (pParty->uFallStartY - party_new_Z > 512 && !bFeatherFall &&
                party_new_Z <= on_ground &&
                !engine->IsUnderwater()) {  // Fall to the water(падение на воду с высоты)
                if (pParty->uFlags & PARTY_FLAGS_1_LANDING) {
                    pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
                } else {
                    for (uint i = 1; i <= 4; ++i) {
                        v110 = pPlayers[i]->GetMaxHealth();
                        pPlayers[i]->ReceiveDamage(
                            (int)((pParty->uFallStartY - party_new_Z) *
                                         (uint64_t)((double)v110 * 0.1)) / 256,
                            DMGT_PHISYCAL);
                        v110 = 20 - pPlayers[i]->GetParameterBonus(
                                        pPlayers[i]->GetActualEndurance());
                        pPlayers[i]->SetRecoveryTime(
                            (signed __int64)((double)v110 *
                                             flt_6BE3A4_debug_recmod1 *
                                             2.133333333333333));
                    }
                    // v82 = pParty->vPosition.z;
                }
            }
            pParty->uFallStartY = party_new_Z;
        }
        if (v102 && pParty->vPosition.z < ceiling_height &&
            (signed int)(pParty->uPartyHeight + pParty->vPosition.z) >=
                ceiling_height) {
            pParty->vPosition.z =
                pParty->vPosition.z + pParty->uPartyHeight - ceiling_height + 1;
            pParty->field_6F0 =
                pParty->vPosition.z + pParty->uPartyHeight - ceiling_height + 1;
        }
    }
}

int GetCeilingHeight(int Party_X, signed int Party_Y, int Party_ZHeight,
                     int pFaceID) {
    int v13;      // eax@25
    int v14;      // edx@27
    int v16;      // ST18_4@29
    int v17;      // edx@29
    int64_t v18;  // qtt@29
    int v19;      // eax@35
    int v20;      // ecx@37
    int v22;      // ebx@42
    int v27;      // [sp+10h] [bp-34h]@21
    bool v34;     // [sp+30h] [bp-14h]@21
    bool v35;     // [sp+34h] [bp-10h]@23
    int v37;      // [sp+38h] [bp-Ch]@21
    int v38;      // [sp+38h] [bp-Ch]@42
    int v39;      // [sp+3Ch] [bp-8h]@1

    dword_720ED0[0] = -1;
    dword_720E80[0] = -1;
    v39 = 1;
    ceiling_height_level[0] = 10000;  //нет потолка
    for (BSPModel &model : pOutdoor->pBModels) {
        if (Party_X <= model.sMaxX && Party_X >= model.sMinX &&
            Party_Y <= model.sMaxY && Party_Y >= model.sMinY) {
            for (ODMFace &face : model.pFaces) {
                if ((face.uPolygonType == POLYGON_Ceiling ||
                     face.uPolygonType == POLYGON_InBetweenCeilingAndWall) &&
                    !face.Ethereal() && Party_X <= face.pBoundingBox.x2 &&
                    Party_X >= face.pBoundingBox.x1 &&
                    Party_Y <= face.pBoundingBox.y2 &&
                    Party_Y >= face.pBoundingBox.y1) {
                    for (uint v = 0; v < face.uNumVertices; v++) {
                        word_720DB0_xs[2 * v] =
                            face.pXInterceptDisplacements[v] +
                            (short)model.pVertices.pVertices[face.pVertexIDs[v]]
                                .x;
                        word_720CE0_ys[2 * v] =
                            face.pXInterceptDisplacements[v] +
                            (short)model.pVertices.pVertices[face.pVertexIDs[v]]
                                .y;
                        word_720DB0_xs[2 * v + 1] =
                            face.pXInterceptDisplacements[v] +
                            (short)model.pVertices
                                .pVertices[face.pVertexIDs[v + 1]]
                                .x;
                        word_720CE0_ys[2 * v + 1] =
                            face.pXInterceptDisplacements[v] +
                            (short)model.pVertices
                                .pVertices[face.pVertexIDs[v + 1]]
                                .y;
                    }
                    v27 = 2 * face.uNumVertices;
                    word_720DB0_xs[2 * face.uNumVertices] = word_720DB0_xs[0];
                    word_720CE0_ys[2 * face.uNumVertices] = word_720CE0_ys[0];
                    v34 = word_720CE0_ys[0] >= Party_Y;
                    v37 = 0;
                    for (uint v = 0; v < v27; ++v) {
                        if (v37 >= 2) break;
                        v35 = word_720CE0_ys[v + 1] >= Party_Y;
                        if (v34 != v35) {
                            v13 = word_720DB0_xs[v + 1] >= Party_X ? 0 : 2;
                            v14 = v13 | (word_720DB0_xs[v] < Party_X);
                            if (v14 != 3) {
                                if (!v14 ||
                                    (v16 = word_720CE0_ys[v + 1] -
                                           word_720CE0_ys[v],
                                     v17 = Party_Y - word_720CE0_ys[v],
                                     HEXRAYS_LODWORD(v18) = v17 << 16,
                                     HEXRAYS_HIDWORD(v18) = v17 >> 16,
                                     (int)(((uint64_t)(
                                                ((int)word_720DB0_xs[v + 1] -
                                                 (int)word_720DB0_xs[v]) *
                                                v18 / v16) >>
                                            16) +
                                           word_720DB0_xs[v]) >= Party_X))
                                    ++v37;
                            }
                        }
                        v34 = v35;
                    }
                    if (v37 == 1) {
                        if (v39 >= 20) break;
                        if (face.uPolygonType == POLYGON_Ceiling)
                            v19 =
                                model.pVertices.pVertices[face.pVertexIDs[0]].z;
                        else
                            v19 = fixpoint_mul(face.zCalc1, Party_X) +
                                  fixpoint_mul(face.zCalc2, Party_Y) +
                                  HEXRAYS_HIWORD(face.zCalc3);
                        v20 = v39++;
                        ceiling_height_level[v20] = v19;
                        dword_720ED0[v20] = model.index;
                        dword_720E80[v20] = face.index;
                    }
                }
            }
        }
    }

    if (!v39) {
        pFaceID = 0;
        return ceiling_height_level[0];
    }
    v22 = 0;
    for (v38 = 0; v38 < v39; ++v38) {
        if (ceiling_height_level[v38] == ceiling_height_level[0])
            v22 = v38;
        else if (ceiling_height_level[v38] < ceiling_height_level[0] &&
                 ceiling_height_level[0] > Party_ZHeight + 15)
            v22 = v38;
        else if (ceiling_height_level[v38] > ceiling_height_level[0] &&
                 ceiling_height_level[v38] <= Party_ZHeight + 15)
            v22 = v38;
    }
    if (v22) {
        *(int *)pFaceID = dword_720E80[v22] | (dword_720ED0[v22] << 6);
        return ceiling_height_level[v22];  //если есть преграда
    }
    pFaceID = 0;
    return ceiling_height_level[v22];  // нет никакой преграды
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
    if (engine->config->no_actors)
        return;  // uNumActors = 0;

    for (unsigned int Actor_ITR = 0; Actor_ITR < uNumActors; ++Actor_ITR) {
        if (pActors[Actor_ITR].uAIState == Removed || pActors[Actor_ITR].uAIState == Disabled ||
            pActors[Actor_ITR].uAIState == Summoned || !pActors[Actor_ITR].uMovementSpeed)
                continue;

        bool Water_Walk = MonsterStats::BelongsToSupertype(pActors[Actor_ITR].pMonsterInfo.uID,
            MONSTER_SUPERTYPE_WATER_ELEMENTAL);

        pActors[Actor_ITR].uSectorID = 0;

        bool uIsFlying = pActors[Actor_ITR].pMonsterInfo.uFlying;
        if (!pActors[Actor_ITR].CanAct()) uIsFlying = 0;

        bool Slope_High = IsTerrainSlopeTooHigh(pActors[Actor_ITR].vPosition.x,
                                    pActors[Actor_ITR].vPosition.y);
        unsigned int Model_On_PID = 0;
        bool uIsOnWater = false;
        int Floor_Level = ODM_GetFloorLevel(
            pActors[Actor_ITR].vPosition.x, pActors[Actor_ITR].vPosition.y,
            pActors[Actor_ITR].vPosition.z, pActors[Actor_ITR].uActorHeight, &uIsOnWater,
            (int *)&Model_On_PID, Water_Walk);
        bool Actor_On_Terrain = Model_On_PID == 0;

        bool uIsAboveFloor = (pActors[Actor_ITR].vPosition.z > (Floor_Level + 1));

        if (pActors[Actor_ITR].uAIState == Dead && uIsOnWater && !uIsAboveFloor) {
            pActors[Actor_ITR].uAIState = Removed;
            continue;
        }

        // MOVEMENT
        if (pActors[Actor_ITR].uCurrentActionAnimation == ANIM_Walking) {
            signed int Actor_Speed = pActors[Actor_ITR].uMovementSpeed;
            if (pActors[Actor_ITR].pActorBuffs[ACTOR_BUFF_SLOWED].Active())
                Actor_Speed = (int64_t)((double)Actor_Speed * 0.5);
            if (pActors[Actor_ITR].uAIState == Fleeing ||
                pActors[Actor_ITR].uAIState == Pursuing)
                Actor_Speed *= 2;
            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_WAIT) {
                Actor_Speed *= flt_6BE3AC_debug_recmod1_x_1_6;
            }
            if (Actor_Speed > 1000) Actor_Speed = 1000;

            pActors[Actor_ITR].vVelocity.x =
                fixpoint_mul(stru_5C6E00->Cos(pActors[Actor_ITR].uYawAngle), Actor_Speed);
            pActors[Actor_ITR].vVelocity.y =
                fixpoint_mul(stru_5C6E00->Sin(pActors[Actor_ITR].uYawAngle), Actor_Speed);
            if (uIsFlying) {
                pActors[Actor_ITR].vVelocity.z = fixpoint_mul(
                    stru_5C6E00->Sin(pActors[Actor_ITR].uPitchAngle), Actor_Speed);
            }
        } else {
            pActors[Actor_ITR].vVelocity.x =
                fixpoint_mul(55000, pActors[Actor_ITR].vVelocity.x);
            pActors[Actor_ITR].vVelocity.y =
                fixpoint_mul(55000, pActors[Actor_ITR].vVelocity.y);
            if (uIsFlying)
                pActors[Actor_ITR].vVelocity.z =
                    fixpoint_mul(55000, pActors[Actor_ITR].vVelocity.z);
        }

        // BELOW FLOOR - POP UPWARDS
        if (pActors[Actor_ITR].vPosition.z < Floor_Level) {
            pActors[Actor_ITR].vPosition.z = Floor_Level;
            pActors[Actor_ITR].vVelocity.z = uIsFlying != 0 ? 0x14 : 0;
        }
        // GRAVITY
        if (!uIsAboveFloor || uIsFlying) {
            if (Slope_High && !uIsAboveFloor && Actor_On_Terrain) {
                Vec3_int_ Terrain_Norm;
                pActors[Actor_ITR].vPosition.z = Floor_Level;
                ODM_GetTerrainNormalAt(pActors[Actor_ITR].vPosition.x,
                                       pActors[Actor_ITR].vPosition.y, &Terrain_Norm);
                uint16_t Gravity = GetGravityStrength();

                pActors[Actor_ITR].vVelocity.z +=
                    -16 * (short)pEventTimer->uTimeElapsed * Gravity;
                int v73 = abs(Terrain_Norm.x * pActors[Actor_ITR].vVelocity.x +
                              Terrain_Norm.z * pActors[Actor_ITR].vVelocity.z +
                              Terrain_Norm.y * pActors[Actor_ITR].vVelocity.y) >> 15;

                pActors[Actor_ITR].vVelocity.x += fixpoint_mul(v73, Terrain_Norm.x);
                pActors[Actor_ITR].vVelocity.y += fixpoint_mul(v73, Terrain_Norm.y);
                pActors[Actor_ITR].uYawAngle -= 32;
                // pActors[Actor_ITR].vVelocity.z += fixpoint_mul(v73, Terrain_Norm.z);
            }
        } else {
            pActors[Actor_ITR].vVelocity.z -=
                (short)pEventTimer->uTimeElapsed * GetGravityStrength();
        }

        // ARMAGEDDON PANIC
        if (pParty->armageddon_timer != 0 && pActors[Actor_ITR].CanAct()) {
            pActors[Actor_ITR].vVelocity.x += rand() % 100 - 50;
            pActors[Actor_ITR].vVelocity.y += rand() % 100 - 50;
            pActors[Actor_ITR].vVelocity.z += rand() % 100 - 20;
            pActors[Actor_ITR].uAIState = Stunned;
            pActors[Actor_ITR].uYawAngle += rand() % 32 - 16;
            pActors[Actor_ITR].UpdateAnimation();
        }

        // MOVING TOO SLOW
        if (pActors[Actor_ITR].vVelocity.x * pActors[Actor_ITR].vVelocity.x +
                    pActors[Actor_ITR].vVelocity.y * pActors[Actor_ITR].vVelocity.y <
                400 &&
            Slope_High == 0) {
            pActors[Actor_ITR].vVelocity.y = 0;
            pActors[Actor_ITR].vVelocity.x = 0;
        }


        // COLLISIONS
        signed int Act_Radius = pActors[Actor_ITR].uActorRadius;
        if (!uIsFlying)
            Act_Radius = 40;


        stru_721530.field_0 = 1;
        stru_721530.field_84 = -1;
        stru_721530.field_8_radius = Act_Radius;
        stru_721530.prolly_normal_d = Act_Radius;
        stru_721530.height = pActors[Actor_ITR].uActorHeight;
        stru_721530.field_70 = 0;

        for (Model_On_PID = 0; Model_On_PID < 100; ++Model_On_PID) {
            stru_721530.position.x = pActors[Actor_ITR].vPosition.x;
            stru_721530.normal.x = stru_721530.position.x;
            stru_721530.position.y = pActors[Actor_ITR].vPosition.y;
            stru_721530.normal.y = stru_721530.position.y;
            int Act_Z_Pos = pActors[Actor_ITR].vPosition.z;
            stru_721530.normal.z = Act_Z_Pos + Act_Radius + 1;
            stru_721530.position.z = Act_Z_Pos - Act_Radius + stru_721530.height - 1;
            if (stru_721530.position.z < stru_721530.normal.z)
                stru_721530.position.z = Act_Z_Pos + Act_Radius + 1;
            stru_721530.velocity.x = pActors[Actor_ITR].vVelocity.x;
            stru_721530.uSectorID = 0;
            stru_721530.velocity.y = pActors[Actor_ITR].vVelocity.y;
            stru_721530.velocity.z = pActors[Actor_ITR].vVelocity.z;
            if (stru_721530._47050A(0)) break;
            _46E889_collide_against_bmodels(1);
            _46E26D_collide_against_sprites(WorldPosToGridCellX(pActors[Actor_ITR].vPosition.x), WorldPosToGridCellZ(pActors[Actor_ITR].vPosition.y));
            _46EF01_collision_chech_player(0);
            _46ED8A_collide_against_sprite_objects(PID(OBJECT_Actor, Actor_ITR));
            int v31 = 0;
            signed int i;
            for (i = 0; v31 < ai_arrays_size; ++v31) {
                unsigned int v33 = ai_near_actors_ids[v31];
                if (v33 != Actor_ITR && Actor::_46DF1A_collide_against_actor(v33, 40))
                    ++i;
            }
            int v71 = i > 1;
            if (stru_721530.field_7C < stru_721530.field_6C)
                Slope_High =
                    fixpoint_mul(stru_721530.field_7C, stru_721530.direction.z);
            // v34 = 0;
            int v35 = stru_721530.normal2.z - stru_721530.prolly_normal_d - 1;
            bool bOnWater = false;
            int Splash_Model_On;
            int Splash_Floor = ODM_GetFloorLevel(
                stru_721530.normal2.x, stru_721530.normal2.y,
                stru_721530.normal2.z - stru_721530.prolly_normal_d - 1,
                pActors[Actor_ITR].uActorHeight, &bOnWater, &Splash_Model_On, 0);
            if (uIsOnWater) {
                if (v35 < Splash_Floor + 60) {
                    if (pActors[Actor_ITR].uAIState == Dead ||
                        pActors[Actor_ITR].uAIState == Dying ||
                        pActors[Actor_ITR].uAIState == Removed ||
                        pActors[Actor_ITR].uAIState == Disabled) {
                        int Splash_Z = Floor_Level + 60;
                        if (Splash_Model_On)
                            Splash_Z = Splash_Floor + 30;

                        SpriteObject::sub_42F960_create_object(
                            pActors[Actor_ITR].vPosition.x, pActors[Actor_ITR].vPosition.y,
                            Splash_Z);
                        pActors[Actor_ITR].uAIState = Removed;
                        return;
                    }
                }
            }
            if (stru_721530.field_7C >= stru_721530.field_6C) {
                pActors[Actor_ITR].vPosition.x = (short)stru_721530.normal2.x;
                pActors[Actor_ITR].vPosition.y = (short)stru_721530.normal2.y;
                pActors[Actor_ITR].vPosition.z = (short)stru_721530.normal2.z -
                                           (short)stru_721530.prolly_normal_d -
                                           1;
                break;
            }

            pActors[Actor_ITR].vPosition.x +=
                fixpoint_mul(stru_721530.field_7C, stru_721530.direction.x);

            pActors[Actor_ITR].vPosition.y +=
                fixpoint_mul(stru_721530.field_7C, stru_721530.direction.y);

            pActors[Actor_ITR].vPosition.z +=
                fixpoint_mul(stru_721530.field_7C, stru_721530.direction.z);
            stru_721530.field_70 += stru_721530.field_7C;
            unsigned int v39 = PID_ID(stru_721530.pid);
            int Angle_To_Decor;
            signed int Coll_Speed;
            switch (PID_TYPE(stru_721530.pid)) {
                case OBJECT_Actor:
                    if (pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->turn_stage != TE_MOVEMENT ||
                        !pParty->bTurnBasedModeOn) {
                        // if(pParty->bTurnBasedModeOn)
                        // v34 = 0;
                        if (pActors[Actor_ITR].pMonsterInfo.uHostilityType) {
                            if (v71 == 0)
                                Actor::AI_Flee(Actor_ITR, stru_721530.pid, 0,
                                               (AIDirection *)0);
                            else
                                Actor::AI_StandOrBored(Actor_ITR, 4, 0,
                                                       (AIDirection *)0);
                        } else if (v71) {
                            Actor::AI_StandOrBored(Actor_ITR, 4, 0, (AIDirection *)0);
                        } else if (pActors[v39].pMonsterInfo.uHostilityType ==
                            MonsterInfo::Hostility_Friendly) {
                            Actor::AI_Flee(Actor_ITR, stru_721530.pid, 0,
                                (AIDirection *)0);
                        } else {
                            Actor::AI_FaceObject(Actor_ITR, stru_721530.pid, 0,
                                (AIDirection *)0);
                        }
                    }
                    break;
                case OBJECT_Player:
                    if (!pActors[Actor_ITR].GetActorsRelation(0)) {
                        Actor::AI_FaceObject(Actor_ITR, stru_721530.pid, 0,
                                             (AIDirection *)0);
                        break;
                    }

                    pActors[Actor_ITR].vVelocity.y = 0;
                    pActors[Actor_ITR].vVelocity.x = 0;

                    if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
                        pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
                    }
                    viewparams->bRedrawGameUI = 1;
                    break;
                case OBJECT_Decoration:
                    Coll_Speed = integer_sqrt(
                        pActors[Actor_ITR].vVelocity.x * pActors[Actor_ITR].vVelocity.x +
                        pActors[Actor_ITR].vVelocity.y * pActors[Actor_ITR].vVelocity.y);
                    Angle_To_Decor = stru_5C6E00->Atan2(
                        pActors[Actor_ITR].vPosition.x -
                            pLevelDecorations[v39].vPosition.x,
                        pActors[Actor_ITR].vPosition.y -
                            pLevelDecorations[v39].vPosition.y);

                    pActors[Actor_ITR].vVelocity.x =
                        fixpoint_mul(stru_5C6E00->Cos(Angle_To_Decor), Coll_Speed);
                    pActors[Actor_ITR].vVelocity.y =
                        fixpoint_mul(stru_5C6E00->Sin(Angle_To_Decor), Coll_Speed);
                    break;
                case OBJECT_BModel:
                    ODMFace * face = &pOutdoor->pBModels[stru_721530.pid >> 9]
                                .pFaces[v39 & 0x3F];
                    if (!face->Ethereal()) {
                        if (face->uPolygonType == 3) {
                            pActors[Actor_ITR].vVelocity.z = 0;
                            pActors[Actor_ITR].vPosition.z =
                                (short)pOutdoor->pBModels[stru_721530.pid >> 9]
                                    .pVertices.pVertices[face->pVertexIDs[0]]
                                    .z +
                                1;
                            if (pActors[Actor_ITR].vVelocity.x *
                                        pActors[Actor_ITR].vVelocity.x +
                                    pActors[Actor_ITR].vVelocity.y *
                                        pActors[Actor_ITR].vVelocity.y <
                                400) {
                                pActors[Actor_ITR].vVelocity.y = 0;
                                pActors[Actor_ITR].vVelocity.x = 0;
                            }
                        } else {
                           int v72b = abs(face->pFacePlane.vNormal.y *
                                           pActors[Actor_ITR].vVelocity.y +
                                       face->pFacePlane.vNormal.z *
                                           pActors[Actor_ITR].vVelocity.z +
                                       face->pFacePlane.vNormal.x *
                                           pActors[Actor_ITR].vVelocity.x) >>
                                   16;
                            if ((stru_721530.speed >> 3) > v72b)
                                v72b = stru_721530.speed >> 3;

                            pActors[Actor_ITR].vVelocity.x +=
                                fixpoint_mul(v72b, face->pFacePlane.vNormal.x);
                            pActors[Actor_ITR].vVelocity.y +=
                                fixpoint_mul(v72b, face->pFacePlane.vNormal.y);
                            pActors[Actor_ITR].vVelocity.z +=
                                fixpoint_mul(v72b, face->pFacePlane.vNormal.z);
                            if (face->uPolygonType != 4) {
                                int v46 = stru_721530.prolly_normal_d -
                                      ((face->pFacePlane.dist +
                                        face->pFacePlane.vNormal.x *
                                            pActors[Actor_ITR].vPosition.x +
                                        face->pFacePlane.vNormal.y *
                                            pActors[Actor_ITR].vPosition.y +
                                        face->pFacePlane.vNormal.z *
                                            pActors[Actor_ITR].vPosition.z) >>
                                       16);
                                if (v46 > 0) {
                                    pActors[Actor_ITR].vPosition.x += fixpoint_mul(
                                        v46, face->pFacePlane.vNormal.x);
                                    pActors[Actor_ITR].vPosition.y += fixpoint_mul(
                                        v46, face->pFacePlane.vNormal.y);
                                    pActors[Actor_ITR].vPosition.z += fixpoint_mul(
                                        v46, face->pFacePlane.vNormal.z);
                                }
                                pActors[Actor_ITR].uYawAngle = stru_5C6E00->Atan2(
                                    pActors[Actor_ITR].vVelocity.x,
                                    pActors[Actor_ITR].vVelocity.y);
                            }
                        }
                    }
                    break;
            }

            pActors[Actor_ITR].vVelocity.x =
                fixpoint_mul(58500, pActors[Actor_ITR].vVelocity.x);
            pActors[Actor_ITR].vVelocity.y =
                fixpoint_mul(58500, pActors[Actor_ITR].vVelocity.y);
            pActors[Actor_ITR].vVelocity.z =
                fixpoint_mul(58500, pActors[Actor_ITR].vVelocity.z);

            Act_Radius = stru_721530.prolly_normal_d;
        }

        // WATER TILE CHECKING
        if (!Water_Walk) {
            // tile on (1) tile heading (2)
            unsigned int Tile_1_Land = ((unsigned int)~pOutdoor->ActuallyGetSomeOtherTileInfo(
                WorldPosToGridCellX(pActors[Actor_ITR].vPosition.x),
                WorldPosToGridCellZ(pActors[Actor_ITR].vPosition.y) - 1) >>
                1) & 1;
            unsigned int Tile_2_Land = ((unsigned int)~pOutdoor->ActuallyGetSomeOtherTileInfo(
                WorldPosToGridCellX(pActors[Actor_ITR].vPosition.x +
                    pActors[Actor_ITR].vVelocity.x),
                WorldPosToGridCellZ(pActors[Actor_ITR].vPosition.y +
                    pActors[Actor_ITR].vVelocity.y) - 1) >>
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
                int Grid_Z = WorldPosToGridCellZ(pActors[Actor_ITR].vPosition.y);
                for (int i = Grid_X - 1; i <= Grid_X + 1; i++) {
                    // scan surrounding cells for land
                    for (int j = Grid_Z - 1; j <= Grid_Z + 1; j++) {
                        Tile_Test_Land = ((unsigned int)~pOutdoor->
                            ActuallyGetSomeOtherTileInfo(i, j - 1) >> 1) & 1;
                        if (Tile_Test_Land) {  // found land
                            int target_x = GridCellToWorldPosX(i);
                            int target_y = GridCellToWorldPosZ(j - 1);
                            if (pActors[Actor_ITR].CanAct()) {  // head to land
                                pActors[Actor_ITR].uYawAngle = stru_5C6E00->Atan2(target_x -
                                    pActors[Actor_ITR].vPosition.x, target_y -
                                    pActors[Actor_ITR].vPosition.y);
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
                    // logger->Warning(L"DROWNING");
                }
            }
        }
    }
}

//----- (0047A384) --------------------------------------------------------
void ODM_LoadAndInitialize(const String &pFilename, ODMRenderParams *thisa) {
    MapInfo *v4;            // edi@4
    // size_t v7;              // eax@19

    // thisa->AllocSoftwareDrawBuffers();
    pODMRenderParams->Initialize();
    pWeather->bRenderSnow = false;
    render->ClearZBuffer(0, 479);
    // thisa = (ODMRenderParams *)1;
    GetAlertStatus();
    if (_A750D8_player_speech_timer) _A750D8_player_speech_timer = 0;
    int v2 = pMapStats->GetMapInfo(pCurrentMapName);
    unsigned int respawn_interval = 0;
    if (v2) {
        v4 = &pMapStats->pInfos[v2];
        respawn_interval = v4->uRespawnIntervalDays;
    } else {
        v4 = (MapInfo*)1;
    }
    day_attrib &= ~DAY_ATTRIB_FOG;
    dword_6BE13C_uCurrentlyLoadedLocationID = v2;
    int v;
    pOutdoor->Initialize(pFilename, pParty->GetPlayingTime().GetDays() + 1, respawn_interval, &v);
    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_2000)) {
        Actor::InitializeActors();
        SpriteObject::InitializeSpriteObjects();
    }
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_2000;
    // v5 = 0;
    if (!v2) v = 0;
    if (v == 1) {
        // v13 = 0;
        for (uint i = 0; i < pOutdoor->uNumSpawnPoints; ++i) {
            SpawnPointMM7 *spawn = pOutdoor->pSpawnPoints + i;

            if (spawn->IsMonsterSpawn())
                SpawnEncounter(v4, spawn, 0, 0, 0);
            else
                v4->SpawnRandomTreasure(spawn);
        }
        RespawnGlobalDecorations();
    }
    pOutdoor->PrepareDecorations();
    pOutdoor->ArrangeSpriteObjects();
    pOutdoor->InitalizeActors(v2);
    pOutdoor->MessWithLUN();
    pOutdoor->level_filename = pFilename;
    pWeather->Initialize();
    pIndoorCameraD3D->sRotationY = pParty->sRotationY;
    pIndoorCameraD3D->sRotationX = pParty->sRotationX;
    // pODMRenderParams->RotationToInts();
    pOutdoor->UpdateSunlightVectors();

    float fov_rad;
    float fov_rad_inv;
    //----- (0042394D) --------------------------------------------------------
    // void IndoorCamera::Initialize(int degFov, unsigned int uViewportWidth,
    // unsigned int uViewportHeight)
    {
        // pIndoorCamera->Initialize(65, viewparams->uScreen_BttmR_X -
        // viewparams->uScreen_topL_X + 1,
        //                              viewparams->uScreen_BttmR_Y -
        //                              viewparams->uScreen_topL_Y + 1);

        int uViewportWidth =
            viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X + 1;

        extern float _calc_fov(int viewport_width, int angle_degree);
        fov_rad = _calc_fov(uViewportWidth, 65);
        fov_rad_inv = 65536.0 / fov_rad;
    }
    pODMRenderParams->int_fov_rad = (signed __int64)fov_rad;
    pODMRenderParams->int_fov_rad_inv = (signed __int64)fov_rad_inv;

    for (int i = 0; i < 20000; ++i) {
        array_77EC08[i].ptr_38 = &SkyBillboard;

        array_77EC08[i].ptr_48 = nullptr;
    }

    MM7Initialization();
}

unsigned int GetLevelFogColor() {
    if (engine->IsUnderwater()) {
        return 0xFF258F5C;
    }

    if (day_attrib & DAY_ATTRIB_FOG) {
        if (pWeather->bNight) {  // night-time fog
            if (false) {
                logger->Warning(
                    L"decompilation can be inaccurate, please send savegame to "
                    L"Nomad");
                __debugbreak();
            }
            int v2 = -(pWeather->bNight != 1);
            return (v2 & 0xE0E0E1) - 0xE0E0E1;
        } else {
            __int64 v1 = (__int64)((1.0 - pOutdoor->fFogDensity) * 200.0 +
                                   pOutdoor->fFogDensity * 31.0);
            return v1 |
                   (((unsigned int)v1 | (((unsigned int)v1 | 0xFFFFFF00) << 8))
                    << 8);
        }
    }

    return 0;
}

int sub_47C3D7_get_fog_specular(int a1, int a2, float a3) {
    int v7;

    int v3 = pWeather->bNight;
    if (engine->IsUnderwater())
        v3 = 0;
    if (pParty->armageddon_timer ||
        !(day_attrib & DAY_ATTRIB_FOG) && !engine->IsUnderwater())
        return 0xFF000000;
    if (v3) {
        if (a3 < (double)day_fogrange_1) {
            v7 = 0;
            if (a3 == 0.0) v7 = 216;
            if (a2) v7 = 248;
            return (-1 - v7) << 24;
        } else {
            if (a3 > (double)day_fogrange_2) {
                v7 = 216;
                if (a3 == 0.0) v7 = 216;
                if (a2) v7 = 248;
                return (-1 - v7) << 24;
            }
            v7 = (signed __int64)((a3 - (double)day_fogrange_1) /
                                  ((double)day_fogrange_2 -
                                   (double)day_fogrange_1) *
                                  216.0);
        }
    } else {
        if (a3 < (double)day_fogrange_1) {
            v7 = 0;
            if (a3 == 0.0) v7 = 216;
            if (a2) v7 = 248;
            return (-1 - v7) << 24;
        } else {
            if (a3 > (double)day_fogrange_2) {
                v7 = 216;
                if (a3 == 0.0) v7 = 216;
                if (a2) v7 = 248;
                return (-1 - v7) << 24;
            } else {
                v7 =
                    floorf(((a3 - (double)day_fogrange_1) * 216.0 /
                    ((double)day_fogrange_2 - (double)day_fogrange_1)) +
                        0.5f);
            }
        }
    }
    if (v7 > 216) {
        v7 = 216;
    } else {
        if (a3 == 0.0) v7 = 216;
    }
    if (a2) v7 = 248;
    return (-1 - v7) << 24;
}

//----- (0047F44B) --------------------------------------------------------
unsigned int WorldPosToGridCellX(int sWorldPosX) {
    return (sWorldPosX >> 9) +
           64;  // sar is in original exe, resulting -880 / 512 = -1
                //                               and -880 sar 9 = -2
}

//----- (0047F458) --------------------------------------------------------
unsigned int WorldPosToGridCellZ(int sWorldPosZ) {
    return 64 - (sWorldPosZ >>
                 9);  // sar is in original exe, resulting -880 / 512 = -1
                      //                               and -880 sar 9 = -2
}

//----- (0047F469) --------------------------------------------------------
int GridCellToWorldPosX(int a1) { return (a1 - 64) << 9; }

//----- (0047F476) --------------------------------------------------------
int GridCellToWorldPosZ(int a1) { return (64 - a1) << 9; }


//----- (004823F4) --------------------------------------------------------
bool IsTerrainSlopeTooHigh(int pos_x, int pos_z) {
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
    unsigned int grid_z = WorldPosToGridCellZ(pos_z) - 1;

    int party_grid_x1 = GridCellToWorldPosX(grid_x);
    // dword_76D56C_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(grid_x + 1);
    // dword_76D570_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(grid_x + 1);
    // dword_76D574_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(grid_x);
    int party_grid_z1 = GridCellToWorldPosZ(grid_z);
    // dword_76D55C_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosZ(grid_z);
    // dword_76D560_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosZ(grid_z + 1);
    // dword_76D564_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosZ(grid_z + 1);
    int party_x1z1_y = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_z);
    int party_x2z1_y = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_z);
    int party_x2z2_y = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_z + 1);
    int party_x1z2_y = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_z + 1);
    // dword_76D554_terrain_cell_world_pos_around_party_y = v4;
    if (party_x1z1_y == party_x2z1_y && party_x2z1_y == party_x2z2_y &&
        party_x2z2_y == party_x1z2_y)
        return false;

    int dx = abs(pos_x - party_grid_x1), dz = abs(party_grid_z1 - pos_z);

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

    int y_min = min(y1, min(y2, y3));  // не верно при подъёме на склон
    int y_max = max(y1, max(y2, y3));
    return (y_max - y_min) > 512;
}

//----- (0048257A) --------------------------------------------------------
int GetTerrainHeightsAroundParty2(int a1, int a2, bool *pIsOnWater, int bFloatAboveWater) {
    //  int result; // eax@9
    int v8;          // ebx@11
    int v9;          // eax@11
    int v10;         // ecx@11
    int v13;         // [sp+10h] [bp-8h]@11
    signed int v14;  // [sp+14h] [bp-4h]@3
    int v15;         // [sp+24h] [bp+Ch]@11

    unsigned int grid_x = WorldPosToGridCellX(a1);
    unsigned int grid_z = WorldPosToGridCellZ(a2) - 1;

    int grid_x1 = GridCellToWorldPosX(grid_x),
        grid_x2 = GridCellToWorldPosX(grid_x + 1);
    int grid_z1 = GridCellToWorldPosZ(grid_z),
        grid_z2 = GridCellToWorldPosZ(grid_z + 1);

    int y_x1z1 = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_z),
        y_x2z1 = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_z),
        y_x2z2 = pOutdoor->DoGetHeightOnTerrain(grid_x + 1, grid_z + 1),
        y_x1z2 = pOutdoor->DoGetHeightOnTerrain(grid_x, grid_z + 1);
    // v4 = WorldPosToGridCellX(a1);
    // v5 = WorldPosToGridCellZ(v12) - 1;
    // dword_76D538_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(v4);
    // dword_76D53C_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(v4 + 1);
    // dword_76D540_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(v4 + 1);
    // dword_76D544_terrain_cell_world_pos_around_party_x =
    // GridCellToWorldPosX(v4);
    // dword_76D528_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosZ(v5);
    // dword_76D52C_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosZ(v5);
    // dword_76D530_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosZ(v5 + 1);
    // dword_76D534_terrain_cell_world_pos_around_party_z =
    // GridCellToWorldPosZ(v5 + 1);
    // dword_76D518_terrain_cell_world_pos_around_party_y =
    // pOutdoor->DoGetHeightOnTerrain(v4, v5);
    // dword_76D51C_terrain_cell_world_pos_around_party_y =
    // pOutdoor->DoGetHeightOnTerrain(v4 + 1, v5);
    // dword_76D520_terrain_cell_world_pos_around_party_y =
    // pOutdoor->DoGetHeightOnTerrain(v4 + 1, v5 + 1);
    // dword_76D524_terrain_cell_world_pos_around_party_y =
    // pOutdoor->DoGetHeightOnTerrain(v4, v5 + 1);
    *pIsOnWater = false;
    if (pOutdoor->ActuallyGetSomeOtherTileInfo(grid_x, grid_z) & 2)
        *pIsOnWater = true;
    v14 = 0;
    if (!bFloatAboveWater && *pIsOnWater) v14 = -60;
    if (y_x1z1 != y_x2z1 || y_x2z1 != y_x2z2 || y_x2z2 != y_x1z2) {
        if (abs(grid_z1 - a2) >= abs(a1 - grid_x1)) {
            v8 = y_x1z2;
            v9 = y_x2z2;
            v10 = y_x1z1;
            v15 = a1 - grid_x1;
            v13 = a2 - grid_z2;
        } else {
            v8 = y_x2z1;
            v9 = y_x1z1;
            v10 = y_x2z2;
            v15 = grid_x2 - a1;
            v13 = grid_z1 - a2;
        }
        return v14 + v8 + fixpoint_mul(v13, (v10 - v8) * 128) +
               fixpoint_mul(v15, (v9 - v8) * 128);
    } else {
        return y_x1z1;
    }
}
