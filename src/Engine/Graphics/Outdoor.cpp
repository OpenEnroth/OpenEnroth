#include "Engine/Graphics/Outdoor.h"

#include <algorithm>
#include <memory>
#include <string>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Collisions.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Time/Timer.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/MapInfo.h"
#include "Engine/LOD.h"
#include "Engine/Seasons.h"
#include "Engine/Data/TileEnumFunctions.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIRest.h"
#include "GUI/UI/UITransition.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"
#include "Library/LodFormats/LodFormats.h"

#include "Utility/String/Ascii.h"
#include "Utility/Memory/FreeDeleter.h"
#include "Utility/Math/TrigLut.h"
#include "Utility/Exception.h"

#include "Io/Mouse.h"

MapStartPoint uLevel_StartingPointType;

OutdoorLocation *pOutdoor = nullptr;
ODMRenderParams *pODMRenderParams = nullptr;

SkyBillboardStruct SkyBillboard;  // skybox planes

static constexpr IndexedArray<std::array<MapId, 4>, MAP_EMERALD_ISLAND, MAP_SHOALS> footTravelDestinations = {
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

static constexpr IndexedArray<std::array<int, 4>, MAP_EMERALD_ISLAND, MAP_SHOALS> footTravelTimes = {
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

static constexpr IndexedArray<std::array<MapStartPoint, 4>, MAP_EMERALD_ISLAND, MAP_SHOALS> footTravelArrivalPoints = {
    // from                      north                south                east                 west
    {MAP_EMERALD_ISLAND,        {MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY}},
    {MAP_HARMONDALE,            {MAP_START_POINT_SOUTH, MAP_START_POINT_NORTH, MAP_START_POINT_SOUTH, MAP_START_POINT_EAST}},
    {MAP_ERATHIA,               {MAP_START_POINT_SOUTH, MAP_START_POINT_NORTH, MAP_START_POINT_WEST,  MAP_START_POINT_EAST}},
    {MAP_TULAREAN_FOREST,       {MAP_START_POINT_EAST,  MAP_START_POINT_NORTH, MAP_START_POINT_PARTY, MAP_START_POINT_EAST}},
    {MAP_DEYJA,                 {MAP_START_POINT_WEST,  MAP_START_POINT_NORTH, MAP_START_POINT_WEST,  MAP_START_POINT_NORTH}},
    {MAP_BRACADA_DESERT,        {MAP_START_POINT_SOUTH, MAP_START_POINT_PARTY, MAP_START_POINT_WEST,  MAP_START_POINT_PARTY}},
    {MAP_CELESTE,               {MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY}},
    {MAP_PIT,                   {MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY}},
    {MAP_EVENMORN_ISLAND,       {MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY}},
    {MAP_MOUNT_NIGHON,          {MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY}},
    {MAP_BARROW_DOWNS,          {MAP_START_POINT_SOUTH, MAP_START_POINT_EAST,  MAP_START_POINT_SOUTH, MAP_START_POINT_EAST}},
    {MAP_LAND_OF_THE_GIANTS,    {MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY}},
    {MAP_TATALIA,               {MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_WEST,  MAP_START_POINT_PARTY}},
    {MAP_AVLEE,                 {MAP_START_POINT_PARTY, MAP_START_POINT_NORTH, MAP_START_POINT_NORTH, MAP_START_POINT_PARTY}},
    {MAP_SHOALS,                {MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY, MAP_START_POINT_PARTY}},
};

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

    if (pParty->uCurrentMinute != pOutdoor->uLastSunlightUpdateMinute)
        pOutdoor->UpdateSunlightVectors();

    pOutdoor->UpdateFog();
    // pCamera3D->sr_Reset_list_0037C();

    SkyBillboard.CalcSkyFrustumVec(1, 0, 0, 0, 1, 0);  // sky box frustum
    render->DrawOutdoorSky();
    render->DrawOutdoorTerrain();
    render->DrawOutdoorBuildings();

    // TODO(pskelton): consider order of drawing / lighting
    pMobileLightsStack->uNumLightsActive = 0;
    pStationaryLightsStack->uNumLightsActive = 0;
    engine->StackPartyTorchLight();

    // engine->PrepareBloodsplats(); // not used?
    UpdateDiscoveredArea(worldToGrid(pParty->pos));

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
    pOutdoor->ExecDraw(true);

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

//----- (00488F5C) --------------------------------------------------------
bool OutdoorLocation::Initialize(std::string_view filename, int days_played,
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
        ::day_fogrange_3 = pCamera3D->GetFarClip();
        if (isMapUnderwater(engine->_currentLoadedMapId))
            SetUnderwaterFog();

        return true;
    }

    return false;
}

MapId OutdoorLocation::getTravelDestination(int partyX, int partyY) {
    int direction;
    MapId currentMap = engine->_currentLoadedMapId;
    MapId destinationMap;

    if (!isMapOutdoor(currentMap))
        return MAP_INVALID;

    // Check which side of the map
    if (partyX < -22528)
        direction = 3; // west
    else if (partyX > 22528)
        direction = 2; // east
    else if (partyY < -22528)
        direction = 1; // south
    else if (partyY > 22528)
        direction = 0; // north
    else
        return MAP_INVALID;

    if (currentMap == MAP_AVLEE && direction == 3) {  // to Shoals
        bool wholePartyUnderwaterSuitEquipped = true;
        for (Character &player : pParty->pCharacters) {
            if (!player.hasUnderwaterSuitEquipped()) {
                wholePartyUnderwaterSuitEquipped = false;
                break;
            }
        }

        if (wholePartyUnderwaterSuitEquipped) {
            uDefaultTravelTime_ByFoot = 1;
            uLevel_StartingPointType = MAP_START_POINT_EAST;
            pParty->uFlags &= ~(PARTY_FLAG_BURNING | PARTY_FLAG_STANDING_ON_WATER | PARTY_FLAG_WATER_DAMAGE);
            return MAP_SHOALS;
        }
    } else if (currentMap == MAP_SHOALS && direction == 2) {  // from Shoals
        uDefaultTravelTime_ByFoot = 1;
        uLevel_StartingPointType = MAP_START_POINT_WEST;
        pParty->uFlags &= ~(PARTY_FLAG_BURNING | PARTY_FLAG_STANDING_ON_WATER | PARTY_FLAG_WATER_DAMAGE);
        return MAP_AVLEE;
    }
    destinationMap = footTravelDestinations[currentMap][direction];
    if (destinationMap == MAP_INVALID)
        return MAP_INVALID;

    assert(destinationMap <= MAP_SHOALS);

    uDefaultTravelTime_ByFoot = footTravelTimes[currentMap][direction];
    uLevel_StartingPointType = footTravelArrivalPoints[currentMap][direction];
    return destinationMap;
}

//----- (004892E6) --------------------------------------------------------
void OutdoorLocation::UpdateSunlightVectors() {
    unsigned int minutes;  // edi@3
    double v8;        // st7@4

    if (pParty->uCurrentHour >= 5 && pParty->uCurrentHour < 21) {
        minutes = pParty->uCurrentMinute + 60 * (pParty->uCurrentHour - 5);

        this->vSunlight.x = std::cos((minutes * pi) / 960.0);
        this->vSunlight.y = 0;
        this->vSunlight.z = std::sin((minutes * pi) / 960.0);

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

int OutdoorLocation::getNumFoodRequiredToRestInCurrentPos(const Vec3f &pos) {
    bool is_on_water = false;
    int bmodel_standing_on_pid = 0;
    ODM_GetFloorLevel(pos, &is_on_water, &bmodel_standing_on_pid);
    if (pParty->isAirborne() || bmodel_standing_on_pid || is_on_water) {
        return 2;
    }

    return foodRequiredForTileset(pTerrain.tilesetByPos(pos));
}

//----- (00489487) --------------------------------------------------------
void OutdoorLocation::SetFog() {
    MapId map_id = engine->_currentLoadedMapId;
    if (map_id == MAP_INVALID || map_id == MAP_CELESTE ||
        map_id == MAP_PIT || map_id > MAP_SHOALS)
        return;

    unsigned chance = vrng->random(100);

    if (chance < fog_probability_table[map_id].small_fog_chance) {
        ::day_fogrange_1 = 4096;
        ::day_fogrange_2 = 8192;
        ::day_attrib |= MAP_WEATHER_FOGGY;
    } else if (chance <
               fog_probability_table[map_id].small_fog_chance +
                   fog_probability_table[map_id].average_fog_chance) {
        ::day_fogrange_2 = 4096;
        ::day_fogrange_1 = 0;
        ::day_attrib |= MAP_WEATHER_FOGGY;
    } else if (fog_probability_table[map_id].dense_fog_chance &&
               chance <
                   fog_probability_table[map_id].small_fog_chance +
                       fog_probability_table[map_id].average_fog_chance +
                       fog_probability_table[map_id].dense_fog_chance) {
        ::day_fogrange_2 = 2048;
        ::day_fogrange_1 = 0;
        ::day_attrib |= MAP_WEATHER_FOGGY;
    } else {
        ::day_attrib &= ~MAP_WEATHER_FOGGY;
    }

    ::day_fogrange_3 = pCamera3D->GetFarClip();

    if (isMapUnderwater(map_id))
        SetUnderwaterFog();
    pOutdoor->loc_time.day_fogrange_1 = ::day_fogrange_1;
    pOutdoor->loc_time.day_fogrange_2 = ::day_fogrange_2;
    pOutdoor->loc_time.day_attrib = ::day_attrib;
}

//----- (0047CDE2) --------------------------------------------------------
void OutdoorLocation::CreateDebugLocation() {
    this->level_filename = "blank";
    this->location_filename = "i6.odm";
    this->location_file_description = "MM6 Outdoor v1.00";

    this->pTerrain.createDebugTerrain();
    this->pSpawnPoints.clear();

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
    pFaceIDLIST.clear();

    // free shader data for outdoor location
    render->ReleaseTerrain();

    if (viewparams->location_minimap)
        viewparams->location_minimap->Release();
    viewparams->location_minimap = nullptr;
}

void OutdoorLocation::Load(std::string_view filename, int days_played, int respawn_interval_days, bool *outdoors_was_respawned) {
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

    std::string_view minimap_filename = filename.substr(0, filename.length() - 4);
    if (viewparams->location_minimap)
        viewparams->location_minimap->Release();
    viewparams->location_minimap = assets->getImage_Solid(minimap_filename);

    std::string odm_filename = std::string(filename);
    odm_filename.replace(odm_filename.length() - 4, 4, ".odm");

    OutdoorLocation_MM7 location;
    deserialize(lod::decodeCompressed(pGames_LOD->read(odm_filename)), &location); // read throws.
    reconstruct(location, this);

    // ****************.ddm file*********************//

    std::string ddm_filename = fmt::format("{}.ddm", filename.substr(0, filename.length() - 4));

    bool respawnInitial = false; // Perform initial location respawn?
    bool respawnTimed = false; // Perform timed location respawn?
    OutdoorDelta_MM7 delta;
    if (Blob blob = lod::decodeCompressed(pSave_LOD->read(ddm_filename))) {
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
        deserialize(lod::decodeCompressed(pGames_LOD->read(ddm_filename)), &delta, tags::context(location));
        *outdoors_was_respawned = true;
    } else if (respawnTimed) {
        auto header = delta.header;
        auto fullyRevealedCells = delta.fullyRevealedCells;
        auto partiallyRevealedCells = delta.partiallyRevealedCells;
        deserialize(lod::decodeCompressed(pGames_LOD->read(ddm_filename)), &delta, tags::context(location));
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

    // LABEL_150:
    if (pWeather->bRenderSnow) {  // Ritor1: it's include for snow
        loc_time.sky_texture_name = "sky19";
    } else if (loc_time.last_visit) {
        if (loc_time.last_visit.toDays() % 28 != pParty->uCurrentDayOfMonth) {
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

    if (engine->config->graphics.SeasonsChange.value())
        pOutdoor->pTerrain.changeSeason(pParty->uCurrentMonth);
}

//----- (0047EF60) --------------------------------------------------------
int OutdoorLocation::UpdateDiscoveredArea(Vec2i gridPos) {
    for (int i = -10; i < 10; i++) {
        int currYpos = gridPos.y + i - 20;
        for (int j = -10; j < 10; j++) {
            int currXpos = gridPos.x + j - 20;
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

    decorationsWithSound.clear();
    for (unsigned i = 0; i < pLevelDecorations.size(); ++i) {
        LevelDecoration *decor = &pLevelDecorations[i];

        pDecorationList->InitializeDecorationSprite(decor->uDecorationDescID);
        const DecorationDesc *decoration = pDecorationList->GetDecoration(decor->uDecorationDescID);

        if (decoration->uSoundID != SOUND_Invalid) {
            decorationsWithSound.push_back(i);
        }

        if ((engine->_currentLoadedMapId == MAP_EVENMORN_ISLAND) && decor->uCog == 20)
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
                    pSpriteObjects[i].vPosition.z = pOutdoor->pTerrain.heightByPos(pSpriteObjects[i].vPosition);
                }
                pSpriteObjects[i].containing_item.postGenerate(ITEM_SOURCE_MAP);
            }
        }
    }
    pGameLoadingUI_ProgressBar->Progress();
}

//----- (0047F2D3) --------------------------------------------------------
bool OutdoorLocation::InitalizeActors(MapId a1) {
    bool alert_status;  // [sp+348h] [bp-8h]@1
                       //  int v9; // [sp+34Ch] [bp-4h]@1

    alert_status = false;
    for (int i = 0; i < pActors.size(); ++i) {
        if (!(pActors[i].attributes & ACTOR_UNKNOW7)) {
            if (!alert_status) {
                pActors[i].currentActionTime = 0_ticks;
                pActors[i].currentActionLength = 0_ticks;
                if (pActors[i].attributes & ACTOR_UNKNOW11)
                    pActors[i].aiState = AIState::Disabled;
                if (pActors[i].aiState != AIState::Removed &&
                    pActors[i].aiState != AIState::Disabled &&
                    (pActors[i].currentHP == 0 ||
                     pActors[i].monsterInfo.hp == 0))
                    pActors[i].aiState = AIState::Dead;
                pActors[i].velocity.x = 0;
                pActors[i].velocity.y = 0;
                pActors[i].velocity.z = 0;
                pActors[i].UpdateAnimation();
                pActors[i].monsterInfo.hostilityType =
                    HOSTILITY_FRIENDLY;
                pActors[i].PrepareSprites(0);
            } else {
                pActors[i].aiState = AIState::Disabled;
                pActors[i].attributes |= ACTOR_UNKNOW11;
            }
        } else if (a1 == MAP_INVALID) {
            pActors[i].aiState = AIState::Disabled;
            pActors[i].attributes |= ACTOR_UNKNOW11;
        } else if (alert_status) {
            pActors[i].currentActionTime = 0_ticks;
            pActors[i].currentActionLength = 0_ticks;
            if (pActors[i].attributes & ACTOR_UNKNOW11)
                pActors[i].aiState = AIState::Disabled;
            if (pActors[i].aiState != AIState::Removed &&
                pActors[i].aiState != AIState::Disabled &&
                (pActors[i].currentHP == 0 ||
                 pActors[i].monsterInfo.hp == 0))
                pActors[i].aiState = AIState::Dead;
            pActors[i].velocity.x = 0;
            pActors[i].velocity.y = 0;
            pActors[i].velocity.z = 0;
            pActors[i].UpdateAnimation();
            pActors[i].monsterInfo.hostilityType =
                HOSTILITY_FRIENDLY;
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

// TODO: move to actors?
//  combined with IndoorLocation::PrepareActorRenderList_BLV() (0043FDED) ----
//----- (0047B42C) --------------------------------------------------------
void OutdoorLocation::PrepareActorsDrawList() {
    unsigned int Angle_To_Cam;   // eax@11
    Duration Cur_Action_Time;    // eax@16
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
            for (unsigned j = 0; j < pBspRenderer->uNumVisibleNotEmptySectors; j++) {
                if (pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[j] == pActors[i].sectorId) {
                    onlist = true;
                    break;
                }
            }
            if (!onlist) continue;
        } else {
            if (!IsCylinderInFrustum(pActors[i].pos, pActors[i].radius)) continue;
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
                Cur_Action_Time = i * 32_ticks + pMiscTimer->time();
        } else {
            if (pActors[i].currentActionAnimation == ANIM_Walking)
                Cur_Action_Time = i * 32_ticks + pEventTimer->time();
        }

        if (pActors[i].buffs[ACTOR_BUFF_STONED].Active() ||
            pActors[i].buffs[ACTOR_BUFF_PARALYZED].Active())
            Cur_Action_Time = 0_ticks;

        int v49 = 0;
        float v4 = 0.0f;
        if (pActors[i].aiState == Summoned) {
            if (pActors[i].summonerId.type() != OBJECT_Actor ||
                pActors[pActors[i].summonerId.id()]
                .monsterInfo.specialAbilityDamageDiceSides != 1) {
                z += floorf(pActors[i].height * 0.5f + 0.5f);
            } else {
                v49 = 1;
                spell_fx_renderer->_4A7F74(pActors[i].pos.x, pActors[i].pos.y, z);
                v4 = (1.0 - (double)pActors[i].currentActionTime.ticks() /
                            (double)pActors[i].currentActionLength.ticks()) *
                     (double)(2 * pActors[i].height);
                z -= floorf(v4 + 0.5f);
                if (z > pActors[i].pos.z) z = pActors[i].pos.z;
            }
        }


        if (pActors[i].aiState == Summoned && !v49)
            frame = pSpriteFrameTable->GetFrame(uSpriteID_Spell11, Cur_Action_Time);
        else if (pActors[i].aiState == Resurrected)
            frame = pSpriteFrameTable->GetFrameReversed(pActors[i].spriteIds[pActors[i].currentActionAnimation], Cur_Action_Time);
        else
            frame = pSpriteFrameTable->GetFrame(
                pActors[i].spriteIds[pActors[i].currentActionAnimation], Cur_Action_Time);

        // no sprite frame to draw
        if (frame->icon_name == "null") continue;
        if (frame->hw_sprites[Sprite_Octant]->texture->height() == 0 || frame->hw_sprites[Sprite_Octant]->texture->width() == 0)
            assert(false);

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

                float proj_scale = frame->scale * (pCamera3D->ViewPlaneDistPixels) / (view_x);
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
                        } else if (pActors[i].massDistortionTime) {
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
                        pBillboardRenderList[uNumBillboardsToDraw - 1].object_pid = Pid(OBJECT_Actor, i);
                        pBillboardRenderList[uNumBillboardsToDraw - 1].field_14_actor_id = i;

                        pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = flags | 0x200;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = frame;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor =
                            pMonsterList->monsters[pActors[i].monsterInfo.id].tintColor;  // *((int *)&v35[v36] - 36);
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

float ODM_GetFloorLevel(const Vec3f &pos, bool *pIsOnWater, int *faceId) {
    std::array<int, 20> current_Face_id{};                   // dword_721110
    std::array<int, 20> current_BModel_id{};                 // dword_721160
    std::array<float, 20> odm_floor_level{};                   // idb
    current_BModel_id[0] = -1;
    current_Face_id[0] = -1;
    odm_floor_level[0] = pOutdoor->pTerrain.heightByPos(pos);
    *pIsOnWater = pOutdoor->pTerrain.isWaterByPos(pos);

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

    float current_floor_level = odm_floor_level[0];
    int current_idx = 0;
    for (unsigned i = 1; i < surface_count; ++i) {
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

    if (current_idx)
        *pIsOnWater = pOutdoor->pBModels[current_BModel_id[current_idx]].pFaces[current_Face_id[current_idx]].Fluid();

    return std::max(odm_floor_level[0], odm_floor_level[current_idx]);
}

//----- (0046BE0A) --------------------------------------------------------
void ODM_UpdateUserInputAndOther() {
    ODM_ProcessPartyActions();

    if (pParty->pos.x < -22528 || pParty->pos.x > 22528 ||
        pParty->pos.y < -22528 || pParty->pos.y > 22528) {
        MapId mapid = pOutdoor->getTravelDestination(pParty->pos.x, pParty->pos.y);
        if (!engine->IsUnderwater() && (pParty->isAirborne() || (pParty->uFlags & (PARTY_FLAG_STANDING_ON_WATER | PARTY_FLAG_WATER_DAMAGE)) ||
                             pParty->uFlags & PARTY_FLAG_BURNING || pParty->bFlying) || mapid == MAP_INVALID) {
            if (pParty->pos.x < -22528) pParty->pos.x = -22528;
            if (pParty->pos.x > 22528) pParty->pos.x = 22528;
            if (pParty->pos.y < -22528) pParty->pos.y = -22528;
            if (pParty->pos.y > 22528) pParty->pos.y = 22528;
        } else {
            pDialogueWindow = new GUIWindow_Travel();  // TravelUI_Load();
        }
    }

    UpdateActors_ODM();
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
    this->decal_builder = EngineIocContainer::ResolveDecalBuilder();
    this->spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();

    this->sky_texture = nullptr;

    uLastSunlightUpdateMinute = 0;

    engine->config->graphics.SeasonsChange.addListener(this, [this](bool seasonsChange) {
        pTerrain.changeSeason(seasonsChange ? pParty->uCurrentMonth : 6);
        render->ReleaseTerrain();
    });
}

OutdoorLocation::~OutdoorLocation() {
    engine->config->graphics.SeasonsChange.removeListeners(this);
}

// TODO(pskelton): Magic numbers
// TODO(pskelton): check pointer maths for updating fly and waterwalk overlays
// TODO(pskelton): Split function up
// TODO(pskelton): Pass party as param
//----- (00473893) --------------------------------------------------------
void ODM_ProcessPartyActions() {
    bool waterWalkActive = false;
    pParty->uFlags &= ~PARTY_FLAG_STANDING_ON_WATER;
    if (pParty->WaterWalkActive()) {
        waterWalkActive = true;
        engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].overlayID + 119] |= 1;
        if (!pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff &&
            pParty->pCharacters[pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].caster - 1].mana <= 0)
            waterWalkActive = false;
    }

    int floorFaceId = 0;
    bool partyIsOnWater = false;

    float floorZ = ODM_GetFloorLevel(pParty->pos, &partyIsOnWater, &floorFaceId);
    bool partyNotOnModel = floorFaceId == 0;
    int currentGroundLevel = floorZ + 1;

    bool partyHasFeatherFall = pParty->FeatherFallActive() || pParty->wearsItemAnywhere(ITEM_ARTIFACT_LADYS_ESCORT)
                                    || pParty->uFlags & (PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING);
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
        pParty->uFlags &= ~(PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING);
    } else {
        partyNotTouchingFloor = true;
    }

    bool partyCloseToGround = pParty->pos.z - currentGroundLevel <= 32;

    // check if we should be flying
    if (!engine->IsUnderwater() && !pParty->FlyActive()) {
        pParty->bFlying = false;
        pParty->uFlags &= ~PARTY_FLAG_LANDING;
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
    Vec3f partyInputSpeed = Vec3f(0, 0, pParty->velocity.z);
    if (pParty->bFlying) {
        partyInputSpeed.z = 0;
    }
    int partyOldFlightZ = pParty->sPartySavedFlightZ;

    bool partyAtHighSlope = pOutdoor->pTerrain.isSlopeTooHighByPos(pParty->pos);
    bool partyIsRunning = false;
    bool partyIsWalking = false;
    bool noFlightBob = false;

    bool flyDown{ false };

    // TODO(captainurist): #time think about a better way to write this formula.
    int64_t dturn = pEventTimer->dt().ticks() * pParty->_yawRotationSpeed * TrigLUT.uIntegerPi / 180 / Duration::TICKS_PER_REALTIME_SECOND;
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
                        pParty->velocity.z = 0;
                        noFlightBob = true;
                        pParty->uFlags &= ~(PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING);
                        if (pParty->sPartySavedFlightZ < engine->config->gameplay.MaxFlightHeight.value()) {
                            partyInputSpeed.z = pParty->walkSpeed * 4;
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
                    pParty->_viewYaw += engine->config->settings.TurnSpeed.value();  // discrete turn
                else
                    pParty->_viewYaw += dturn * fTurnSpeedMultiplier;  // time-based smooth turn

                pParty->_viewYaw &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_TurnRight:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    pParty->_viewYaw -= engine->config->settings.TurnSpeed.value();
                else
                    pParty->_viewYaw -= dturn * fTurnSpeedMultiplier;

                pParty->_viewYaw &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_FastTurnLeft:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    pParty->_viewYaw += engine->config->settings.TurnSpeed.value();
                else
                    pParty->_viewYaw += 2.0f * fTurnSpeedMultiplier * dturn;

                pParty->_viewYaw &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_FastTurnRight:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    pParty->_viewYaw -= engine->config->settings.TurnSpeed.value();
                else
                    pParty->_viewYaw -= 2.0f * fTurnSpeedMultiplier * dturn;

                pParty->_viewYaw &= TrigLUT.uDoublePiMask;
                break;

            case PARTY_StrafeLeft:
            {
                float sin_y = sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
                float dx = sin_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                partyInputSpeed.x -= 3 * dx / 4;

                float cos_y = cosf(2 * pi_double * pParty->_viewYaw / 2048.0);
                float dy = cos_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                partyInputSpeed.y += 3 * dy / 4;

                partyIsWalking = true;
            } break;

            case PARTY_StrafeRight:
            {
                float sin_y = sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
                float dx = sin_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                partyInputSpeed.x += 3 * dx / 4;

                float cos_y = cosf(2 * pi_double * pParty->_viewYaw / 2048.0);
                float dy = cos_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                partyInputSpeed.y -= 3 * dy / 4;

                partyIsWalking = true;
            } break;

            case PARTY_WalkForward:
            {
                float sin_y = sinf(2 * pi_double * pParty->_viewYaw / 2048.0),
                      cos_y = cosf(2 * pi_double * pParty->_viewYaw / 2048.0);

                float dx = cos_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                float dy = sin_y * pParty->walkSpeed * fWalkSpeedMultiplier;

                if (engine->config->debug.TurboSpeed.value()) {
                    partyInputSpeed.x += dx * 12;
                    partyInputSpeed.y += dy * 12;
                } else {
                    partyInputSpeed.x += dx;
                    partyInputSpeed.y += dy;
                }

                partyIsWalking = true;
            } break;

            case PARTY_RunForward:
            {
                float sin_y = sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
                float cos_y = cosf(2 * pi_double * pParty->_viewYaw / 2048.0);

                float dx = cos_y * pParty->walkSpeed * fWalkSpeedMultiplier;
                float dy = sin_y * pParty->walkSpeed * fWalkSpeedMultiplier;

                if (pParty->bFlying) {
                    if (engine->config->debug.TurboSpeed.value()) {
                        partyInputSpeed.x += dx * 24;
                        partyInputSpeed.y += dy * 24;
                    } else {
                        partyInputSpeed.x += 4 * dx;
                        partyInputSpeed.y += 4 * dy;
                    }
                } else if (partyAtHighSlope && !floorFaceId) {
                    partyInputSpeed.x += dx;
                    partyInputSpeed.y += dy;
                    partyIsWalking = true;
                } else {
                    if (engine->config->debug.TurboSpeed.value()) {
                        partyInputSpeed.x += dx * 12;
                        partyInputSpeed.y += dy * 12;
                    } else {
                        partyInputSpeed.x += 2 * dx;
                        partyInputSpeed.y += 2 * dy;
                    }

                    partyIsRunning = true;
                }
            } break;

            case PARTY_WalkBackward: {
                float sin_y = sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
                float cos_y = cosf(2 * pi_double * pParty->_viewYaw / 2048.0);

                float dx = cos_y * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                partyInputSpeed.x -= dx;

                float dy = sin_y * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                partyInputSpeed.y -= dy;
                partyIsWalking = true;
            } break;

            case PARTY_RunBackward:
            {
                float sin_y = sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
                float cos_y = cosf(2 * pi_double * pParty->_viewYaw / 2048.0);

                float dx = cos_y * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                float dy = sin_y * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;

                if (pParty->bFlying) {
                    partyInputSpeed.x -= 4 * dx;
                    partyInputSpeed.y -= 4 * dy;
                } else {
                    partyInputSpeed.x -= dx;
                    partyInputSpeed.y -= dy;
                    partyIsWalking = true;
                }
            } break;

            case PARTY_CenterView:
                pParty->_viewPitch = 0;
                break;

            case PARTY_LookUp:
                pParty->_viewPitch += engine->config->settings.VerticalTurnSpeed.value();
                if (pParty->_viewPitch > 128) pParty->_viewPitch = 128;
                if (pParty->hasActiveCharacter()) {
                    pParty->activeCharacter().playReaction(SPEECH_LOOK_UP);
                }
                break;

            case PARTY_LookDown:
                pParty->_viewPitch -= engine->config->settings.VerticalTurnSpeed.value();
                if (pParty->_viewPitch < -128) pParty->_viewPitch = -128;
                if (pParty->hasActiveCharacter()) {
                    pParty->activeCharacter().playReaction(SPEECH_LOOK_DOWN);
                }
                break;

            case PARTY_MouseLook:
                mouse->DoMouseLook();
                break;

            case PARTY_Jump:
                if ((!partyAtHighSlope || floorFaceId) &&
                    // to avoid jump hesitancy when moving downhill
                    (!partyNotTouchingFloor || (partyCloseToGround && partyInputSpeed.z <= 0)) &&
                    pParty->jump_strength &&
                    !(pParty->uFlags & PARTY_FLAG_WATER_DAMAGE) &&
                    !(pParty->uFlags & PARTY_FLAG_BURNING)) {
                    partyNotTouchingFloor = true;
                    partyInputSpeed.z += pParty->jump_strength * 96.0f;
                    // boost party upwards slightly so we dont "land" straight away
                    pParty->pos.z += 1;
                }
                break;

            case PARTY_Land:
                if (pParty->bFlying) {
                    pParty->uFlags |= PARTY_FLAG_LANDING;
                }
                pPartyActionQueue->uNumActions = 0;
                break;

            default:
                assert(false);
        }
    }

    // Behaviour divergence from vanilla - now treat landing process as flying down for consistency
    if (pParty->uFlags & PARTY_FLAG_LANDING || flyDown) {
        if (pParty->FlyActive() || engine->IsUnderwater()) {
            pParty->bFlying = false;
            if (engine->IsUnderwater() ||
                pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff ||
                (pParty->pCharacters[pParty->pPartyBuffs[PARTY_BUFF_FLY].caster - 1].mana > 0 || engine->config->debug.AllMagic.value())) {
                partyOldFlightZ = pParty->pos.z;
                partyInputSpeed.z = -pParty->walkSpeed * 4;
                pParty->bFlying = true;
                noFlightBob = true;
                if (flyDown)
                    pParty->uFlags &= ~(PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING);
            }
        }
    }

    if (partyInputSpeed.xy().lengthSqr() < 400 && !partyAtHighSlope) {
        partyInputSpeed.y = 0;
        partyInputSpeed.x = 0;
    }

    Vec3f partyNewPos = pParty->pos;

    //-------------------------------------------
    if (pParty->bFlying) {
        // bob up and down in the air
        if (noFlightBob) {
            partyNewPos.z = partyOldFlightZ;
        } else {
            partyNewPos.z = partyOldFlightZ + 4 * TrigLUT.cos(pEventTimer->time().realtimeMilliseconds());
        }

        if (pParty->FlyActive())
            engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].overlayID + 119] &= 0xFE;
        pParty->uFallStartZ = partyNewPos.z;
    } else if (partyNewPos.z < currentGroundLevel) {
        partyNewPos.z = currentGroundLevel;
        partyInputSpeed.z = 0;
        pParty->uFallStartZ = currentGroundLevel;
        partyOldFlightZ = partyNewPos.z;
        if (pParty->FlyActive())
            engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].overlayID + 119] |= 1;
    } else {
        partyOldFlightZ = partyNewPos.z;
        if (pParty->FlyActive())
            engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_FLY].overlayID + 119] |= 1;
    }
    //------------------------------------------

    if (partyNotTouchingFloor && !pParty->bFlying) {  // add gravity
        partyInputSpeed.z += -2.0f * pEventTimer->dt().ticks() * GetGravityStrength();
    } else if (!partyNotTouchingFloor) {
        if (!floorFaceId) {
            // rolling down the hill
            // how it's done: you get a little bit pushed in the air along
            // terrain normal, getting in the air and falling to the gravity,
            // gradually sliding downwards. nice trick
            partyNewPos.z = currentGroundLevel;
            if (partyAtHighSlope) {
                Vec3f v98 = pOutdoor->pTerrain.normalByPos(partyNewPos);
                partyInputSpeed.z += (8 * -(pEventTimer->dt().ticks() * (int)GetGravityStrength()));
                float dotp = std::abs(dot(partyInputSpeed, v98));
                partyInputSpeed += dotp * v98;
            }
        }
    }

    if (partyNotTouchingFloor) {
        if (!engine->IsUnderwater() && partyInputSpeed.z <= 0) {
            if (partyInputSpeed.z < -500 && !pParty->bFlying &&
                pParty->pos.z - currentGroundLevel > 1000 &&
                !pParty->FeatherFallActive() &&
                !(pParty->uFlags & (PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING))) {  // falling scream
                for (int i = 0; i < 4; ++i) {
                    if (!pParty->pCharacters[i].HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) &&
                        !pParty->pCharacters[i].WearsItem(ITEM_ARTIFACT_HERMES_SANDALS, ITEM_SLOT_BOOTS) &&
                        pParty->pCharacters[i].CanAct())
                        pParty->pCharacters[i].playReaction(SPEECH_FALLING);
                }
            }
        }
    } else {
      pParty->uFallStartZ = partyNewPos.z;
    }

    // has the party collided with a outdoor model
    bool partyHasHitModel{ false };

    float savedZSpeed = partyInputSpeed.z;
    // horizontal
    partyInputSpeed.z = 0;
    ProcessPartyCollisionsODM(&partyNewPos, &partyInputSpeed, &floorFaceId, &partyNotOnModel, &partyHasHitModel, &triggerID);
    // vertical - only when horizonal motion hasnt caused height gain
    if (partyNewPos.z <= pParty->pos.z) {
        partyInputSpeed = Vec3f(0, 0, savedZSpeed);
        ProcessPartyCollisionsODM(&partyNewPos, &partyInputSpeed, &floorFaceId, &partyNotOnModel, &partyHasHitModel, &triggerID);
    }

    if (!partyNotTouchingFloor || partyCloseToGround)
        pParty->setAirborne(false);
    else
        pParty->setAirborne(true);

    Vec3f partyOldPosition = pParty->pos;
    Vec2i partyOldGridPos = worldToGrid(pParty->pos);
    Vec2i partyNewGridPos = worldToGrid(partyNewPos);

    // this gets if tile is not water
    bool partyCurrentOnLand = !pOutdoor->pTerrain.isWaterByGrid(partyOldGridPos);
    bool partyNewXOnLand = !pOutdoor->pTerrain.isWaterByGrid({partyNewGridPos.x, partyOldGridPos.y});
    bool partyNewYOnLand = !pOutdoor->pTerrain.isWaterByGrid({partyOldGridPos.x, partyNewGridPos.y});

    // -(update party co-ords)---------------------------------------
    bool notWater{ false };
    if (partyNewGridPos == partyOldGridPos && partyCurrentOnLand/*partyNewXOnLand && partyNewYOnLand*/)
        notWater = true;

    if (!partyNotOnModel)
        notWater = true;

    if (notWater) {
        pParty->pos.x = partyNewPos.x;
        pParty->pos.y = partyNewPos.y;

        pParty->velocity.z = partyInputSpeed.z;

        pParty->pos.z = partyNewPos.z;
        pParty->sPartySavedFlightZ = partyOldFlightZ;

        pParty->uFlags &= ~(PARTY_FLAG_BURNING | PARTY_FLAG_WATER_DAMAGE);
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

        if (waterMoveX) pParty->pos.x = partyNewPos.x;
        if (waterMoveY) pParty->pos.y = partyNewPos.y;

        if (waterMoveY || waterMoveX) {
            if (waterWalkActive) {
                pParty->uFlags &= ~PARTY_FLAG_STANDING_ON_WATER;
                engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].overlayID + 119] |= 1;
                if (!partyNewXOnLand || !partyNewYOnLand) {
                    if (!pParty->bFlying) {
                        pParty->uFlags |= PARTY_FLAG_STANDING_ON_WATER;
                        engine->_persistentVariables.decorVars[20 * pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].overlayID + 119] &= 0xFFFE;
                    }
                }
            }
        }

        pParty->pos.z = partyNewPos.z;
        pParty->velocity.z = partyInputSpeed.z;
        pParty->sPartySavedFlightZ = partyOldFlightZ;

        pParty->uFlags &= ~(PARTY_FLAG_BURNING | PARTY_FLAG_WATER_DAMAGE);

        if (partyDrowningFlag) {
            int pTerrainHeight = pOutdoor->pTerrain.heightByPos(pParty->pos);
            if (pParty->pos.z <= pTerrainHeight + 1) {
                pParty->uFlags |= PARTY_FLAG_WATER_DAMAGE;
            }
        }
    }

    // height restriction
    if (pParty->pos.z > 8160) {
        pParty->uFallStartZ = 8160;
        pParty->pos.z = 8160;
    }

    // new ground level
    float newFloorLevel = ODM_GetFloorLevel(partyNewPos, &partyIsOnWater, &floorFaceId);
    float newGroundLevel = newFloorLevel + 1;
    partyNewPos.z = std::max(partyNewPos.z, newGroundLevel);

    // Falling damage
    if (!triggerID ||
        (eventProcessor(triggerID, Pid(), 1), pParty->pos.x == partyNewPos.x) &&
        pParty->pos.y == partyNewPos.y && pParty->pos.z == partyNewPos.z) {
        if (((pParty->pos.z <= newGroundLevel || partyHasHitModel) && savedZSpeed < 0)) {
            pParty->velocity.z = 0;

            if (partyIsOnWater && savedZSpeed < -400.0f) { // Require that we have a bit of impact into water surface to cause a splash
                // -400 chosen so that it is just under z impact speed from standing jump
                // SpriteObject::createSplashObject(partyNewPos);
                // Party can never see its own splashes so just play the sound - only one splash at a time for party
                pAudioPlayer->playSound(SOUND_splash, SOUND_MODE_EXCLUSIVE, Pid::character(0));
            }

            if (!partyHasHitModel)
                pParty->pos.z = newGroundLevel;
            if (pParty->uFallStartZ - partyNewPos.z > 512 && !partyHasFeatherFall &&
                (partyNewPos.z <= newGroundLevel || partyHasHitModel) &&
                !engine->IsUnderwater()) {
                if (pParty->uFlags & (PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING)) {
                    pParty->uFlags &= ~(PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING);
                } else {
                    pParty->giveFallDamage(pParty->uFallStartZ - pParty->pos.z);
                }
            }
            pParty->uFallStartZ = partyNewPos.z;
        }
        if (ceilingFaceID && pParty->pos.z < ceilingHeight && (pParty->height + pParty->pos.z) >= ceilingHeight) {
            pParty->pos.z = ceilingHeight - pParty->height - 1;
            pParty->sPartySavedFlightZ = pParty->pos.z;
        }
    }

    // walking / running sounds ------------------------
    if (engine->config->settings.WalkSound.value()) {
        bool canStartNewSound = !pAudioPlayer->isWalkingSoundPlays();

        // Start sound processing only when actual movement is performed to avoid stopping sounds on high FPS
        if (pEventTimer->dt()) {
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
                    SoundId sound = SOUND_Invalid;
                    if (partyIsRunning) {
                        if (walkDelta >= 4) {
                            if (isModelWalk) {
                                sound = SOUND_RunWood;
                            } else {
                                sound = walkSoundForTileset(pOutdoor->pTerrain.tilesetByPos(partyOldPosition), true);
                            }
                        }
                    } else if (partyIsWalking) {
                        if (walkDelta >= 2) {
                            if (isModelWalk) {
                                sound = SOUND_RunWood;
                            } else {
                                sound = walkSoundForTileset(pOutdoor->pTerrain.tilesetByPos(partyOldPosition), false);
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
            if (!face.Contains(Vec3f(Party_X, Party_Y, 0), model.index, slack, FACE_XY_PLANE))
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

//----- (00464851) --------------------------------------------------------
void SetUnderwaterFog() {
    day_fogrange_1 = 50;
    day_fogrange_2 = 2000;
    day_fogrange_3 = 25000;
}

//----- (004706C6) --------------------------------------------------------
void UpdateActors_ODM() {
    if (engine->config->debug.NoActors.value())
        return;  // uNumActors = 0;

    for (unsigned int Actor_ITR = 0; Actor_ITR < pActors.size(); ++Actor_ITR) {
        if (pActors[Actor_ITR].aiState == Removed || pActors[Actor_ITR].aiState == Disabled ||
            pActors[Actor_ITR].aiState == Summoned || !pActors[Actor_ITR].moveSpeed)
                continue;

        bool Water_Walk = supertypeForMonsterId(pActors[Actor_ITR].monsterInfo.id) == MONSTER_SUPERTYPE_WATER_ELEMENTAL;

        pActors[Actor_ITR].sectorId = 0;

        bool uIsFlying = pActors[Actor_ITR].monsterInfo.flying;
        if (!pActors[Actor_ITR].CanAct())
            uIsFlying = 0;

        bool Slope_High = pOutdoor->pTerrain.isSlopeTooHighByPos(pActors[Actor_ITR].pos);
        int Model_On_PID = 0;
        bool uIsOnWater = false;
        float Floor_Level = ODM_GetFloorLevel(pActors[Actor_ITR].pos, &uIsOnWater, &Model_On_PID);
        bool Actor_On_Terrain = Model_On_PID == 0;

        bool uIsAboveFloor = (pActors[Actor_ITR].pos.z > (Floor_Level + 1));

        // make bloodsplat when the ground is hit
        if (!pActors[Actor_ITR].donebloodsplat) {
            if (pActors[Actor_ITR].aiState == Dead || pActors[Actor_ITR].aiState == Dying) {
                if (pActors[Actor_ITR].pos.z < Floor_Level + 30) { // 30 to provide small error / rounding factor
                    if (pMonsterStats->infos[pActors[Actor_ITR].monsterInfo.id].bloodSplatOnDeath) {
                        if (engine->config->graphics.BloodSplats.value()) {
                            float splatRadius = pActors[Actor_ITR].radius * engine->config->graphics.BloodSplatsMultiplier.value();
                            EngineIocContainer::ResolveDecalBuilder()->AddBloodsplat(Vec3f(pActors[Actor_ITR].pos.x, pActors[Actor_ITR].pos.y, Floor_Level + 30), colorTable.Red, splatRadius);
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

            pActors[Actor_ITR].velocity.x = TrigLUT.cos(pActors[Actor_ITR].yawAngle) * Actor_Speed;
            pActors[Actor_ITR].velocity.y = TrigLUT.sin(pActors[Actor_ITR].yawAngle) * Actor_Speed;
            if (uIsFlying) {
                pActors[Actor_ITR].velocity.z = TrigLUT.sin(pActors[Actor_ITR].pitchAngle) * Actor_Speed;
            }
        } else {
            pActors[Actor_ITR].velocity.x *= 0.83923339843f;
            pActors[Actor_ITR].velocity.y *= 0.83923339843f;
            if (uIsFlying)
                pActors[Actor_ITR].velocity.z *= 0.83923339843f;
        }

        // BELOW FLOOR - POP UPWARDS
        if (pActors[Actor_ITR].pos.z < Floor_Level) {
            pActors[Actor_ITR].pos.z = Floor_Level;
            pActors[Actor_ITR].velocity.z = uIsFlying != 0 ? 0x14 : 0;
        }
        // GRAVITY
        if (!uIsAboveFloor || uIsFlying) {
            if (Slope_High && !uIsAboveFloor && Actor_On_Terrain) {
                pActors[Actor_ITR].pos.z = Floor_Level;
                Vec3f Terrain_Norm = pOutdoor->pTerrain.normalByPos(pActors[Actor_ITR].pos);
                int Gravity = GetGravityStrength();

                pActors[Actor_ITR].velocity.z += -16 * pEventTimer->dt().ticks() * Gravity; //TODO(pskelton): common gravity code extract
                float v73 = std::abs(dot(Terrain_Norm, pActors[Actor_ITR].velocity)) * 2.0f;

                pActors[Actor_ITR].velocity.x += v73 * Terrain_Norm.x;
                pActors[Actor_ITR].velocity.y += v73 * Terrain_Norm.y;
                pActors[Actor_ITR].yawAngle -= 32;
                // pActors[Actor_ITR].vVelocity.z += fixpoint_mul(v73, Terrain_Norm.z);
            }
        } else {
            pActors[Actor_ITR].velocity.z -= pEventTimer->dt().ticks() * GetGravityStrength();
        }

        // ARMAGEDDON PANIC
        if (pParty->armageddon_timer && pActors[Actor_ITR].CanAct() && pParty->armageddonForceCount > 0) {
            pActors[Actor_ITR].velocity.x += grng->random(100) - 50;
            pActors[Actor_ITR].velocity.y += grng->random(100) - 50;
            pActors[Actor_ITR].velocity.z += grng->random(100) - 20;
            pActors[Actor_ITR].aiState = Stunned;
            pActors[Actor_ITR].yawAngle += grng->random(32) - 16;
            pActors[Actor_ITR].UpdateAnimation();
        }

        // TODO(pskelton): this cancels out the above - is this intended
        // MOVING TOO SLOW
        if (pActors[Actor_ITR].velocity.xy().lengthSqr() < 400 && Slope_High == 0) {
            pActors[Actor_ITR].velocity.y = 0;
            pActors[Actor_ITR].velocity.x = 0;
        }

        // COLLISIONS
        ProcessActorCollisionsODM(pActors[Actor_ITR], uIsFlying);

        // WATER TILE CHECKING
        if (!Water_Walk) {
            // tile on (1) tile heading (2)
            bool tile1IsLand, tile2IsLand;
            tile1IsLand = !pOutdoor->pTerrain.isWaterByPos(pActors[Actor_ITR].pos);
            tile2IsLand = !pOutdoor->pTerrain.isWaterByPos(pActors[Actor_ITR].pos + pActors[Actor_ITR].velocity);
            if (!uIsFlying && tile1IsLand && !tile2IsLand) {
                // approaching water - turn away
                if (pActors[Actor_ITR].CanAct()) {
                    pActors[Actor_ITR].yawAngle -= 32;
                    pActors[Actor_ITR].currentActionTime = 0_ticks;
                    pActors[Actor_ITR].currentActionLength = 128_ticks;
                    pActors[Actor_ITR].aiState = Fleeing;
                }
            }
            if (!uIsFlying && !tile1IsLand && !uIsAboveFloor && Actor_On_Terrain) {
                // on water and shouldnt be
                bool tileTestLand = false;  // reset land found
                Vec2i gridPos = worldToGrid(pActors[Actor_ITR].pos);
                for (int i = gridPos.x - 1; i <= gridPos.x + 1; i++) {
                    // scan surrounding cells for land
                    for (int j = gridPos.y - 1; j <= gridPos.y + 1; j++) {
                        tileTestLand = !pOutdoor->pTerrain.isWaterByGrid({i, j});
                        if (tileTestLand) {  // found land
                            Vec2i target = gridToWorld({i, j});
                            if (pActors[Actor_ITR].CanAct()) {  // head to land
                                pActors[Actor_ITR].yawAngle = TrigLUT.atan2(target.x - pActors[Actor_ITR].pos.x,
                                                                             target.y - pActors[Actor_ITR].pos.y);
                                pActors[Actor_ITR].currentActionTime = 0_ticks;
                                pActors[Actor_ITR].currentActionLength = 128_ticks;
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

/**
 * @offset 0x47A384
 */
static void loadAndPrepareODMInternal(MapId mapid) {
    MapInfo *map_info;
    bool outdoor_was_respawned;
    unsigned int respawn_interval = 0;
    std::string mapFilename;

    // thisa->AllocSoftwareDrawBuffers();
    pWeather->bRenderSnow = false;
    render->ClearZBuffer();
    // thisa = (ODMRenderParams *)1;
    GetAlertStatus(); // Result unused.
    pParty->_delayedReactionTimer = 0_ticks;
    if (mapid != MAP_INVALID) {
        mapFilename = pMapStats->pInfos[mapid].fileName;
        map_info = &pMapStats->pInfos[mapid];
        respawn_interval = map_info->respawnIntervalDays;

        assert(ascii::noCaseEquals(mapFilename.substr(mapFilename.rfind('.') + 1), "odm"));
    } else {
        // TODO(Nik-RE-dev): why there's logic for loading maps that are not listed in info?
        mapFilename = "";
        map_info = nullptr;
    }
    day_attrib &= ~MAP_WEATHER_FOGGY;
    pOutdoor->Initialize(mapFilename, pParty->GetPlayingTime().toDays() + 1, respawn_interval, &outdoor_was_respawned);

    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)) {
        Actor::InitializeActors();
        SpriteObject::InitializeSpriteObjects();
    }
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN;

    if (outdoor_was_respawned && mapid != MAP_INVALID) {
        for (unsigned i = 0; i < pOutdoor->pSpawnPoints.size(); ++i) {
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
    pOutdoor->InitalizeActors(mapid);
    pOutdoor->level_filename = mapFilename;
    pWeather->Initialize();
    pCamera3D->_viewYaw = pParty->_viewYaw;
    pCamera3D->_viewPitch = pParty->_viewPitch;
    // pODMRenderParams->RotationToInts();
    pOutdoor->UpdateSunlightVectors();

    MM7Initialization();
}

void loadAndPrepareODM(MapId mapid, bool bLoading) {
    pGameLoadingUI_ProgressBar->Reset(27);
    uCurrentlyLoadedLevelType = LEVEL_OUTDOOR;

    loadAndPrepareODMInternal(mapid);
    if (!bLoading)
        TeleportToStartingPoint(uLevel_StartingPointType);

    viewparams->_443365();
    PlayLevelMusic();

    //  level decoration sound
    for (int decorIdx : decorationsWithSound) {
        const DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[decorIdx].uDecorationDescID);
        pAudioPlayer->playSound(decoration->uSoundID, SOUND_MODE_PID, Pid(OBJECT_Decoration, decorIdx));
    }
}

// returns 0xXXYYZZ fog color
Color GetLevelFogColor() {
    if (engine->IsUnderwater()) {
        return colorTable.Eucalyptus;
    }

    if (day_attrib & MAP_WEATHER_FOGGY) {
        if (pWeather->bNight) {  // night-time fog
            if (false) {
                logger->error("decompilation can be inaccurate, please send savegame to Nomad");
                assert(false);
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
        !(day_attrib & MAP_WEATHER_FOGGY) && !engine->IsUnderwater())
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

//----- (00436A6D) --------------------------------------------------------
double OutdoorLocation::GetPolygonMinZ(RenderVertexSoft *pVertices, unsigned int unumverts) {
    double result = FLT_MAX;
    for (unsigned i = 0; i < unumverts; i++) {
        if (pVertices[i].vWorldPosition.z < result) {
            result = pVertices[i].vWorldPosition.z;
        }
    }
    return result;
}

//----- (00436A40) --------------------------------------------------------
double OutdoorLocation::GetPolygonMaxZ(RenderVertexSoft *pVertex, unsigned int unumverts) {
    double result = FLT_MIN;
    for (unsigned i = 0; i < unumverts; i++) {
        if (pVertex[i].vWorldPosition.z > result)
            result = pVertex[i].vWorldPosition.z;
    }
    return result;
}

// TODO(pskelton): move this - used both indoors and out
void TeleportToStartingPoint(MapStartPoint point) {
    DecorationId decID = pDecorationList->GetDecorIdByName(toString(point));

    if (decID != DECORATION_NULL) {
        for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
            if (pLevelDecorations[i].uDecorationDescID == decID) {
                pParty->pos = pLevelDecorations[i].vPosition;
                if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                    // Spawn point in Harmondale from Barrow Downs is up in the sky, vanilla worked it around by
                    // always placing the party on the ground.
                    // TODO: (Chaosit) dummy variables created for the sake of passing pointers
                    bool bOnWater = false;
                    int bModelPid;
                    pParty->pos.z = ODM_GetFloorLevel(pParty->pos, &bOnWater, &bModelPid);
                } else {
                    int face = -1;
                    pParty->pos.z = BLV_GetFloorLevel(pParty->pos, pIndoor->GetSector(pParty->pos), &face);
                }
                pParty->velocity = Vec3f();
                pParty->uFallStartZ = pParty->pos.z;
                pParty->_viewYaw = pLevelDecorations[i]._yawAngle;
                pParty->_viewPitch = 0;
            }
        }

        if (engine->_teleportPoint.isValid()) {
            engine->_teleportPoint.doTeleport(true);
        }
        engine->_teleportPoint.invalidate();
    }
}
