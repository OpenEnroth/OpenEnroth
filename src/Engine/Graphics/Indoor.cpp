#include "Engine/Graphics/Indoor.h"

#include <algorithm>
#include <limits>
#include <ranges>
#include <string>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Collisions.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/TextureFrameTable.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/PortalFunctions.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Time/Timer.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/LOD.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"
#include "Library/LodFormats/LodFormats.h"

#include "Utility/String/Ascii.h"
#include "Utility/Math/TrigLut.h"
#include "Utility/Math/FixPoint.h"
#include "Utility/Exception.h"

IndoorLocation *pIndoor = nullptr;
BLVRenderParams *pBLVRenderParams = new BLVRenderParams;

// TODO(captainurist): move to SoundEnums.h?
static constexpr IndexedArray<SoundId, MAP_FIRST, MAP_LAST> pDoorSoundIDsByLocationID = {
    {MAP_EMERALD_ISLAND,            SOUND_wood_door0101},
    {MAP_HARMONDALE,                SOUND_wood_door0101},
    {MAP_ERATHIA,                   SOUND_wood_door0101},
    {MAP_TULAREAN_FOREST,           SOUND_wood_door0101},
    {MAP_DEYJA,                     SOUND_wood_door0101},
    {MAP_BRACADA_DESERT,            SOUND_wood_door0101},
    {MAP_CELESTE,                   SOUND_wood_door0101},
    {MAP_PIT,                       SOUND_wood_door0101},
    {MAP_EVENMORN_ISLAND,           SOUND_wood_door0101},
    {MAP_MOUNT_NIGHON,              SOUND_wood_door0101},
    {MAP_BARROW_DOWNS,              SOUND_wood_door0101},
    {MAP_LAND_OF_THE_GIANTS,        SOUND_wood_door0101},
    {MAP_TATALIA,                   SOUND_wood_door0101},
    {MAP_AVLEE,                     SOUND_wood_door0101},
    {MAP_SHOALS,                    SOUND_wood_door0101},
    {MAP_DRAGON_CAVES,              SOUND_stone_door0301},
    {MAP_LORD_MARKHAMS_MANOR,       SOUND_wood_door0201},
    {MAP_BANDIT_CAVES,              SOUND_wood_door0401},
    {MAP_HAUNTED_MANSION,           SOUND_wood_door0501},
    {MAP_TEMPLE_OF_THE_MOON,        SOUND_wood_door0301},
    {MAP_CASTLE_HARMONDALE,         SOUND_wood_door0501},
    {MAP_WHITE_CLIFF_CAVE,          SOUND_wood_door0201},
    {MAP_ERATHIAN_SEWERS,           SOUND_stone_door0101},
    {MAP_FORT_RIVERSTRIDE,          SOUND_wood_door0201},
    {MAP_TULAREAN_CAVES,            SOUND_wood_door0101},
    {MAP_CLANKERS_LABORATORY,       SOUND_wood_door0501},
    {MAP_HALL_OF_THE_PIT,           SOUND_wood_door0501},
    {MAP_WATCHTOWER_6,              SOUND_wood_door0401},
    {MAP_SCHOOL_OF_SORCERY,         SOUND_wood_door0501},
    {MAP_RED_DWARF_MINES,           SOUND_wood_door0501},
    {MAP_WALLS_OF_MIST,             SOUND_wood_door0301},
    {MAP_TEMPLE_OF_THE_LIGHT,       SOUND_wood_door0101},
    {MAP_BREEDING_ZONE,             SOUND_stone_door0301},
    {MAP_TEMPLE_OF_THE_DARK,        SOUND_stone_door0401},
    {MAP_GRAND_TEMPLE_OF_THE_MOON,  SOUND_wood_door0101},
    {MAP_GRAND_TEMPLE_OF_THE_SUN,   SOUND_stone_door0101},
    {MAP_THUNDERFIST_MOUNTAIN,      SOUND_stone_door0401},
    {MAP_MAZE,                      SOUND_stone_door0301},
    {MAP_STONE_CITY,                SOUND_wood_door0401},
    {MAP_COLONY_ZOD,                SOUND_wood_door0201},
    {MAP_MERCENARY_GUILD,           SOUND_stone_door0501},
    {MAP_TIDEWATER_CAVERNS,         SOUND_wood_door0301},
    {MAP_WINE_CELLAR,               SOUND_wood_door0101},
    {MAP_TITANS_STRONGHOLD,         SOUND_wood_door0101},
    {MAP_TEMPLE_OF_BAA,             SOUND_wood_door0101},
    {MAP_HALL_UNDER_THE_HILL,       SOUND_wood_door0101},
    {MAP_LINCOLN,                   SOUND_wood_door0101},
    {MAP_CASTLE_GRYPHONHEART,       SOUND_wood_door0101},
    {MAP_CASTLE_NAVAN,              SOUND_wood_door0101},
    {MAP_CASTLE_LAMBENT,            SOUND_wood_door0101},
    {MAP_CASTLE_GLOAMING,           SOUND_wood_door0101},
    {MAP_DRAGONS_LAIR,              SOUND_wood_door0101},
    {MAP_BARROW_VII,                SOUND_wood_door0101},
    {MAP_BARROW_IV,                 SOUND_wood_door0101},
    {MAP_BARROW_II,                 SOUND_wood_door0101},
    {MAP_BARROW_XIV,                SOUND_wood_door0101},
    {MAP_BARROW_III,                SOUND_wood_door0101},
    {MAP_BARROW_IX,                 SOUND_wood_door0101},
    {MAP_BARROW_VI,                 SOUND_wood_door0101},
    {MAP_BARROW_I,                  SOUND_wood_door0101},
    {MAP_BARROW_VIII,               SOUND_wood_door0101},
    {MAP_BARROW_XIII,               SOUND_wood_door0101},
    {MAP_BARROW_X,                  SOUND_stone_door0301},
    {MAP_BARROW_XII,                SOUND_wood_door0301},
    {MAP_BARROW_V,                  SOUND_stone_door0101},
    {MAP_BARROW_XI,                 SOUND_wood_door0101},
    {MAP_BARROW_XV,                 SOUND_wood_door0101},
    {MAP_ZOKARRS_TOMB,              SOUND_stone_door0301},
    {MAP_NIGHON_TUNNELS,            SOUND_wood_door0301},
    {MAP_TUNNELS_TO_EEOFOL,         SOUND_stone_door0101},
    {MAP_WILLIAM_SETAGS_TOWER,      SOUND_wood_door0101},
    {MAP_WROMTHRAXS_CAVE,           SOUND_wood_door0101},
    {MAP_HIDDEN_TOMB,               SOUND_stone_door0301},
    {MAP_STRANGE_TEMPLE,            SOUND_wood_door0301},
    {MAP_SMALL_HOUSE,               SOUND_stone_door0101},
    {MAP_ARENA,                     SOUND_wood_door0101}
};

// all locations which should have special tranfer message
// dragon caves, markham, bandit cave, haunted mansion
// barrow 7, barrow 9, barrow 10, setag tower
// wromthrax cave, toberti, hidden tomb
std::array<const char *, 11> _4E6BDC_loc_names = {
    "mdt12.blv", "d18.blv",   "mdt14.blv", "d37.blv",
    "mdk01.blv", "mdt01.blv", "mdr01.blv", "mdt10.blv",
    "mdt09.blv", "mdt15.blv", "mdt11.blv"};

//----- (0043F39E) --------------------------------------------------------
void PrepareDrawLists_BLV() {
    // unsigned int v7;  // ebx@8
    BLVSector *v8;    // esi@8

    pBLVRenderParams->Reset();
    uNumDecorationsDrawnThisFrame = 0;
    uNumSpritesDrawnThisFrame = 0;
    uNumBillboardsToDraw = 0;

    pMobileLightsStack->uNumLightsActive = 0;
    //pStationaryLightsStack->uNumLightsActive = 0;
    engine->StackPartyTorchLight();

    PrepareBspRenderList_BLV();

    render->DrawSpriteObjects();
    pOutdoor->PrepareActorsDrawList();

     for (unsigned i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
         int v7 = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
         v8 = &pIndoor->pSectors[pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i]];

        for (unsigned j = 0; j < v8->uNumDecorations; ++j)
            pIndoor->PrepareDecorationsRenderList_BLV(v8->pDecorationIDs[j], v7);
     }

    FindBillboardsLightLevels_BLV();
}


//----- (004407D9) --------------------------------------------------------
void BLVRenderParams::Reset() {
    this->uPartySectorID = pIndoor->GetSector(pParty->pos);
    this->uPartyEyeSectorID = pIndoor->GetSector(pParty->pos + Vec3f(0, 0, pParty->eyeLevel));

    if (!this->uPartySectorID) {
        assert(false);  // shouldnt happen, please provide savegame
    }


    {
        this->uViewportX = pViewport->uScreen_TL_X;
        this->uViewportY = pViewport->uScreen_TL_Y;
        this->uViewportZ = pViewport->uScreen_BR_X;
        this->uViewportW = pViewport->uScreen_BR_Y;

        this->uViewportWidth = uViewportZ - uViewportX + 1;
        this->uViewportHeight = uViewportW - uViewportY + 1;
        this->uViewportCenterX = (uViewportZ + uViewportX) / 2;
        this->uViewportCenterY = (uViewportY + uViewportW) / 2;
    }

    this->uTargetWidth = render->GetRenderDimensions().w;
    this->uTargetHeight = render->GetRenderDimensions().h;
    this->pTargetZBuffer = render->pActiveZBuffer;
    this->uNumFacesRenderedThisFrame = 0;
}


//----- (00440B44) --------------------------------------------------------
void IndoorLocation::DrawIndoorFaces(bool bD3D) {
    render->DrawIndoorFaces();
}



//----- (00441BD4) --------------------------------------------------------
void IndoorLocation::Draw() {
    PrepareDrawLists_BLV();
    if (pBLVRenderParams->uPartySectorID)
        DrawIndoorFaces(true);
    render->TransformBillboardsAndSetPalettesODM();
    engine->DrawParticles();
    trail_particle_generator.UpdateParticles();
}

//----- (004C0EF2) --------------------------------------------------------
void BLVFace::FromODM(ODMFace *face) {
    this->facePlane = face->facePlane;
    this->uAttributes = face->uAttributes;
    this->pBounding = face->pBoundingBox;
    this->zCalc = face->zCalc;
    this->uPolygonType = face->uPolygonType;
    this->uNumVertices = face->uNumVertices;
    this->resource = face->resource;
    this->pVertexIDs = face->pVertexIDs.data();
}

//----- (004AE5BA) --------------------------------------------------------
GraphicsImage *BLVFace::GetTexture() {
    if (this->IsTextureFrameTable())
        // TODO(captainurist): using pEventTimer here is weird. This means that e.g. cleric in the haunted mansion is
        //                     not animated in turn-based mode. Use misc timer? Also see ODMFace::GetTexture.
        return pTextureFrameTable->GetFrameTexture(
            (int64_t)this->resource, pEventTimer->time());
    else
        return static_cast<GraphicsImage *>(this->resource);
}

void BLVFace::SetTexture(std::string_view filename) {
    if (this->IsTextureFrameTable()) {
        this->resource = (void *)pTextureFrameTable->FindTextureByName(filename);
        if (this->resource != (void *)-1) {
            return;
        }

        // Failed to find animated texture so disable
        this->resource = nullptr;
        this->ToggleIsTextureFrameTable();
    }

    this->resource = assets->getBitmap(filename);
    this->texlayer = -1;
    this->texunit = -1;
}

//----- (00498B15) --------------------------------------------------------
void IndoorLocation::Release() {
    this->ptr_0002B4_doors_ddata.clear();
    this->ptr_0002B0_sector_rdata.clear();
    this->ptr_0002B8_sector_lrdata.clear();
    this->pLFaces.clear();
    this->pSpawnPoints.clear();
    this->pSectors.clear();
    this->pFaces.clear();
    this->pFaceExtras.clear();
    this->pVertices.clear();
    this->pNodes.clear();
    this->pDoors.clear();
    this->pLights.clear();
    this->pMapOutlines.clear();

    render->ReleaseBSP();

    this->bLoaded = 0;
}

//----- (00444810) --------------------------------------------------------
// index of special transfer message, 0 otherwise
unsigned int IndoorLocation::GetLocationIndex(std::string_view locationName) {
    for (unsigned i = 0; i < _4E6BDC_loc_names.size(); ++i)
        if (ascii::noCaseEquals(locationName, _4E6BDC_loc_names[i]))
            return i + 1;
    return 0;
}

void IndoorLocation::toggleLight(signed int sLightID, unsigned int bToggle) {
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR &&
        (sLightID <= pIndoor->pLights.size() - 1) && (sLightID >= 0)) {
        if (bToggle)
            pIndoor->pLights[sLightID].uAtributes &= 0xFFFFFFF7;
        else
            pIndoor->pLights[sLightID].uAtributes |= 8;
    }
}

//----- (00498E0A) --------------------------------------------------------
void IndoorLocation::Load(std::string_view filename, int num_days_played, int respawn_interval_days, bool *indoor_was_respawned) {
    decal_builder->Reset(0);

    assert(!bLoaded); // BLV is already loaded!

    auto blv_filename = std::string(filename);
    blv_filename.replace(blv_filename.length() - 4, 4, ".blv");

    this->filename = std::string(filename);

    Release();

    bLoaded = true;

    IndoorLocation_MM7 location;
    deserialize(lod::decodeCompressed(pGames_LOD->read(blv_filename)), &location); // read throws if file doesn't exist.
    reconstruct(location, this);

    std::string dlv_filename = fmt::format("{}.dlv", filename.substr(0, filename.size() - 4));

    bool respawnInitial = false; // Perform initial location respawn?
    bool respawnTimed = false; // Perform timed location respawn?
    IndoorDelta_MM7 delta;
    if (Blob blob = lod::decodeCompressed(pSave_LOD->read(dlv_filename))) {
        try {
            deserialize(blob, &delta, tags::context(location));

            // Level was changed externally and we have a save there? Don't crash, just respawn.
            if (delta.header.totalFacesCount > 0 && delta.header.decorationCount > 0 &&
                (delta.header.totalFacesCount != pFaces.size() || delta.header.decorationCount != pLevelDecorations.size()))
                respawnInitial = true;

            // Entering the level for the 1st time?
            if (delta.header.info.lastRespawnDay == 0)
                respawnInitial = true;

            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)
                respawn_interval_days = 0x1BAF800;

            if (!respawnInitial && num_days_played - delta.header.info.lastRespawnDay >= respawn_interval_days && pMapStats->GetMapInfo(filename) != MAP_CASTLE_HARMONDALE)
                respawnTimed = true;
        } catch (const Exception &e) {
            logger->error("Failed to load '{}', respawning location: {}", dlv_filename, e.what());
            respawnInitial = true;
        }
    }

    assert(respawnInitial + respawnTimed <= 1);

    if (respawnInitial) {
        deserialize(lod::decodeCompressed(pGames_LOD->read(dlv_filename)), &delta, tags::context(location));
        *indoor_was_respawned = true;
    } else if (respawnTimed) {
        auto header = delta.header;
        auto visibleOutlines = delta.visibleOutlines;
        deserialize(lod::decodeCompressed(pGames_LOD->read(dlv_filename)), &delta, tags::context(location));
        delta.header = header;
        delta.visibleOutlines = visibleOutlines;
        *indoor_was_respawned = true;
    } else {
        *indoor_was_respawned = false;
    }

    reconstruct(delta, this);

    if (respawnTimed || respawnInitial)
        dlv.lastRespawnDay = num_days_played;
    if (respawnTimed)
        dlv.respawnCount++;
}

//----- (0049AC17) --------------------------------------------------------
int IndoorLocation::GetSector(float sX, float sY, float sZ) {
    if (uCurrentlyLoadedLevelType != LEVEL_INDOOR)
        return 0;

    if (pSectors.size() < 2) {
        // assert(false);
        return 0;
    }

     // holds faces the coords are above
    int FoundFaceStore[5] = { 0 };
    int NumFoundFaceStore = 0;
    int backupboundingsector{ 0 };
    std::optional<int> foundSector;
    bool singleSectorFound = false;

    // loop through sectors
    for (unsigned i = 1; i < pSectors.size(); ++i) {
        if (NumFoundFaceStore >= 5) break;

        BLVSector *pSector = &pSectors[i];

        if (!pSector->pBounding.intersectsCuboid(Vec3f(sX, sY, sZ), Vec3f(5, 5, 64)))
            continue;  // outside sector bounding

        if (!backupboundingsector) backupboundingsector = i;

        int FloorsAndPortals = pSector->uNumFloors + pSector->uNumPortals;

        // nothing in sector to check against so skip
        if (!FloorsAndPortals) continue;
        if (!pSector->pFloors) continue;

        if (!foundSector) {
            foundSector = i;
            singleSectorFound = true;
        } else if (*foundSector != i) {
            singleSectorFound = false;
        }

        // loop over check faces
        for (unsigned z = 0; z < FloorsAndPortals; ++z) {
            int uFaceID;
            if (z < pSector->uNumFloors)
                uFaceID = pSector->pFloors[z];
            else
                uFaceID = pSector->pPortals[z - pSector->uNumFloors];

            BLVFace *pFace = &pFaces[uFaceID];
            if (pFace->uPolygonType != POLYGON_Floor && pFace->uPolygonType != POLYGON_InBetweenFloorAndWall)
                continue;

            // add found faces into store
            if (pFace->Contains(Vec3f(sX, sY, 0), MODEL_INDOOR, engine->config->gameplay.FloorChecksEps.value(), FACE_XY_PLANE))
                FoundFaceStore[NumFoundFaceStore++] = uFaceID;
            if (NumFoundFaceStore >= 5)
                break; // TODO(captainurist): we do get here sometimes (e.g. in dragon cave), increase limit?
        }
    }

    // only one face found
    if (NumFoundFaceStore == 1)
        return this->pFaces[FoundFaceStore[0]].uSectorID;

    // only one sector found
    if (singleSectorFound) return *foundSector;

    // No face found - outside of level
    if (!NumFoundFaceStore) {
        if (!backupboundingsector) {
            logger->warning("GetSector fail: {}, {}, {}", sX, sY, sZ);
            return 0;
        } else {
            logger->warning("GetSector: Returning backup sector bounding!");
            return backupboundingsector;
        }
    }

    // when multiple possibilities are found - cycle through and use the closer one to party
    int pSectorID = 0, backupID = 0;
    int MinZDist = INT32_MAX, backupDist = INT32_MAX;
    if (NumFoundFaceStore > 0) {
        int CalcZDist = MinZDist;
        for (int s = 0; s < NumFoundFaceStore; ++s) {
            // calc distance between this face and party
            if (this->pFaces[FoundFaceStore[s]].uPolygonType == POLYGON_Floor)
                CalcZDist = sZ - this->pVertices[*this->pFaces[FoundFaceStore[s]].pVertexIDs].z;
            if (this->pFaces[FoundFaceStore[s]].uPolygonType == POLYGON_InBetweenFloorAndWall) {
                CalcZDist = sZ - this->pFaces[FoundFaceStore[s]].zCalc.calculate(sX, sY);
            }

            // use this face if its smaller than the current min - prefer faces below party
            if (CalcZDist < MinZDist) {
                if (CalcZDist >= 0) {
                    pSectorID = this->pFaces[FoundFaceStore[s]].uSectorID;
                    MinZDist = CalcZDist;
                } else {
                    backupID = this->pFaces[FoundFaceStore[s]].uSectorID;
                    backupDist = std::abs(CalcZDist);
                }
            }
        }

        if (pSectorID == 0) {
            if (backupID == 0) {
                assert(false); // doesnt choose - so default to first - SHOULDNT GET HERE
                pSectorID = this->pFaces[FoundFaceStore[0]].uSectorID;
            } else {
                // there is a face above the party to use
                pSectorID = backupID;
            }
        }
    }

    return pSectorID;
}

//----- (00498A41) --------------------------------------------------------
void BLVFace::_get_normals(Vec3f *outU, Vec3f *outV) {
    // TODO(captainurist): code looks very similar to Camera3D::GetFacetOrientation
    if (this->uPolygonType == POLYGON_VerticalWall) {
        outU->x = -this->facePlane.normal.y;
        outU->y = this->facePlane.normal.x;
        outU->z = 0;

        outV->x = 0;
        outV->y = 0;
        outV->z = -1;

    } else if (this->uPolygonType == POLYGON_Floor ||
               this->uPolygonType == POLYGON_Ceiling) {
        outU->x = 1;
        outU->y = 0;
        outU->z = 0;

        outV->x = 0;
        outV->y = -1;
        outV->z = 0;

    } else if (this->uPolygonType == POLYGON_InBetweenFloorAndWall || this->uPolygonType == POLYGON_InBetweenCeilingAndWall) {
        if (std::abs(this->facePlane.normal.z) < 0.70863342285f) { // Was 46441 fixpoint
            outU->x = -this->facePlane.normal.y;
            outU->y = this->facePlane.normal.x;
            outU->z = 0;
            outU->normalize();

            outV->y = 0;
            outV->z = -1;
            outV->x = 0;
        } else {
            outU->x = 1;
            outU->y = 0;
            outU->z = 0;

            outV->x = 0;
            outV->y = -1;
            outV->z = 0;
        }
    }
    // LABEL_12:
    if (this->uAttributes & FACE_FlipNormalU) {
        outU->x = -outU->x;
        outU->y = -outU->y;
        outU->z = -outU->z;
    }
    if (this->uAttributes & FACE_FlipNormalV) {
        outV->x = -outV->x;
        outV->y = -outV->y;
        outV->z = -outV->z;
    }
    return;
}

void BLVFace::Flatten(FlatFace *points, int model_idx, FaceAttributes override_plane) const {
    assert(!override_plane ||
            override_plane == FACE_XY_PLANE || override_plane == FACE_YZ_PLANE || override_plane == FACE_XZ_PLANE);

    FaceAttributes plane = override_plane;
    if (!plane)
        plane = this->uAttributes & (FACE_XY_PLANE | FACE_YZ_PLANE | FACE_XZ_PLANE);

    auto do_flatten = [&](auto &&vertex_accessor) {
        if (plane & FACE_XY_PLANE) {
            for (int i = 0; i < this->uNumVertices; i++) {
                points->u[i] = vertex_accessor(i).x;
                points->v[i] = vertex_accessor(i).y;
            }
        } else if (plane & FACE_XZ_PLANE) {
            for (int i = 0; i < this->uNumVertices; i++) {
                points->u[i] = vertex_accessor(i).x;
                points->v[i] = vertex_accessor(i).z;
            }
        } else {
            for (int i = 0; i < this->uNumVertices; i++) {
                points->u[i] = vertex_accessor(i).y;
                points->v[i] = vertex_accessor(i).z;
            }
        }
    };

    if (model_idx == MODEL_INDOOR) {
        do_flatten([&](int index) -> const auto &{
            return pIndoor->pVertices[this->pVertexIDs[index]];
        });
    } else {
        do_flatten([&](int index) -> const auto &{
            return pOutdoor->pBModels[model_idx].pVertices[this->pVertexIDs[index]];
        });
    }
}

bool BLVFace::Contains(const Vec3f &pos, int model_idx, int slack, FaceAttributes override_plane) const {
    assert(!override_plane ||
            override_plane == FACE_XY_PLANE || override_plane == FACE_YZ_PLANE || override_plane == FACE_XZ_PLANE);

    // TODO(captainurist): uncomment this
    // float d = std::abs(this->facePlane.signedDistanceTo(pos.toFloat()));
    // assert(d < 0.01f);

    if (this->uNumVertices < 3)
        return false; // This does happen.

    FaceAttributes plane = override_plane;
    if (!plane)
        plane = this->uAttributes & (FACE_XY_PLANE | FACE_YZ_PLANE | FACE_XZ_PLANE);

    FlatFace points;
    Flatten(&points, model_idx, plane);

    float u;
    float v;
    if (plane & FACE_XY_PLANE) {
        u = pos.x;
        v = pos.y;
    } else if (plane & FACE_YZ_PLANE) {
        u = pos.y;
        v = pos.z;
    } else {
        u = pos.x;
        v = pos.z;
    }

#if 0
    // Old algo for reference.
    bool inside = false;
    for (int i = 0, j = this->uNumVertices - 1; i < this->uNumVertices; j = i++) {
        if ((points.v[i] > v) == (points.v[j] > v))
            continue;

        float edge_x = points.u[i] + (points.u[j] - points.u[i]) * (v - points.v[i]) / (points.v[j] - points.v[i]);
        if (u < edge_x)
            inside = !inside;
    }
    return inside;
#endif

    // The polygons we're dealing with are convex, so instead of the usual ray casting algorithm we can simply
    // check that the point in question lies on the same side relative to all of the polygon's edges.
    int sign = 0;
    for (int i = 0, j = this->uNumVertices - 1; i < this->uNumVertices; j = i++) {
        float a_u = points.u[j] - points.u[i];
        float a_v = points.v[j] - points.v[i];
        float b_u = u - points.u[i];
        float b_v = v - points.v[i];
        float cross_product = a_u * b_v - a_v * b_u; // That's |a| * |b| * sin(a,b)
        if (fuzzyIsNull(cross_product))
            continue;

        if (slack > 0) {
            // distance(point, line) = (a x b) / |a|,
            // so the condition below just checks that distance is less than slack.
            float a_len_sqr = a_u * a_u + a_v * a_v;
            if (cross_product * cross_product < a_len_sqr * slack * slack)
                continue;
        }

        int cross_sign = static_cast<int>(cross_product > 0) * 2 - 1;

        if (sign == 0) {
            sign = cross_sign;
        } else if (sign != cross_sign) {
            return false;
        }
    }

    // sign == 0 means we got an invalid polygon, so we return false in this case
    // (invalid polygons don't contain points).
    return sign != 0;
}

//----- (0044C23B) --------------------------------------------------------
bool BLVFaceExtra::HasEventHint() {
    return hasEventHint(this->uEventID);
}

//----- (0046F228) --------------------------------------------------------
void BLV_UpdateDoors() {
    SoundId eDoorSoundID = SOUND_wood_door0101;
    if (engine->_currentLoadedMapId != MAP_INVALID)
        eDoorSoundID = pDoorSoundIDsByLocationID[engine->_currentLoadedMapId];

    // loop over all doors
    for (unsigned i = 0; i < pIndoor->pDoors.size(); ++i) {
        BLVDoor *door = &pIndoor->pDoors[i];

        // door not moving currently
        if (door->uState == DOOR_CLOSED || door->uState == DOOR_OPEN) {
            door->uAttributes &= ~DOOR_SETTING_UP;
            continue;
        }
        bool shouldPlaySound = !(door->uAttributes & (DOOR_SETTING_UP | DOOR_NOSOUND)) && door->uNumVertices != 0;

        door->uTimeSinceTriggered += pEventTimer->dt();

        int openDistance;     // [sp+60h] [bp-4h]@6
        if (door->uState == DOOR_OPENING) {
            openDistance = door->uTimeSinceTriggered.realtimeMilliseconds() * door->uOpenSpeed / 1000;

            if (openDistance >= door->uMoveLength) {
                openDistance = door->uMoveLength;
                door->uState = DOOR_OPEN;
                if (shouldPlaySound)
                    pAudioPlayer->playSound(doorClosedSound(eDoorSoundID), SOUND_MODE_PID, Pid(OBJECT_Door, i));
            } else if (shouldPlaySound) {
                pAudioPlayer->playSound(eDoorSoundID, SOUND_MODE_PID, Pid(OBJECT_Door, i));
            }
        } else {
            assert(door->uState == DOOR_CLOSING);

            int closeDistance = door->uTimeSinceTriggered.realtimeMilliseconds() * door->uCloseSpeed / 1000;
            if (closeDistance >= door->uMoveLength) {
                openDistance = 0;
                door->uState = DOOR_CLOSED;
                if (shouldPlaySound)
                    pAudioPlayer->playSound(doorClosedSound(eDoorSoundID), SOUND_MODE_PID, Pid(OBJECT_Door, i));
            } else {
                openDistance = door->uMoveLength - closeDistance;
                if (shouldPlaySound)
                    pAudioPlayer->playSound(eDoorSoundID, SOUND_MODE_PID, Pid(OBJECT_Door, i));
            }
        }

        // adjust verts to how open the door is
        for (int j = 0; j < door->uNumVertices; ++j) {
            pIndoor->pVertices[door->pVertexIDs[j]].x = door->vDirection.x * openDistance + door->pXOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].y = door->vDirection.y * openDistance + door->pYOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].z = door->vDirection.z * openDistance + door->pZOffsets[j];
        }

        for (int j = 0; j < door->uNumFaces; ++j) {
            BLVFace *face = &pIndoor->pFaces[door->pFaceIDs[j]];
            const Vec3f &facePoint = pIndoor->pVertices[face->pVertexIDs[0]];
            face->facePlane.dist = -dot(facePoint, face->facePlane.normal);
            face->zCalc.init(face->facePlane);

            Vec3f v;
            Vec3f u;
            face->_get_normals(&u, &v);
            BLVFaceExtra *extras = &pIndoor->pFaceExtras[face->uFaceExtraID];
            extras->sTextureDeltaU = 0;
            extras->sTextureDeltaV = 0;

            float minU = std::numeric_limits<float>::max();
            float minV = std::numeric_limits<float>::max();
            float maxU = std::numeric_limits<float>::min();
            float maxV = std::numeric_limits<float>::min();
            for (unsigned k = 0; k < face->uNumVertices; ++k) {
                Vec3f point = pIndoor->pVertices[face->pVertexIDs[k]];
                float pointU = dot(point, u);
                float pointV = dot(point, v);
                minU = std::min(minU, pointU);
                minV = std::min(minV, pointV);
                maxU = std::max(maxU, pointU);
                maxV = std::max(maxV, pointV);
                face->pVertexUIDs[k] = pointU;
                face->pVertexVIDs[k] = pointV;
            }

            if (face->uAttributes & FACE_TexAlignLeft) {
                extras->sTextureDeltaU -= minU;
            } else if (face->uAttributes & FACE_TexAlignRight && face->resource) {
                extras->sTextureDeltaU -= maxU + face->GetTexture()->width();
            }

            if (face->uAttributes & FACE_TexAlignDown) {
                extras->sTextureDeltaV -= minV;
            } else if (face->uAttributes & FACE_TexAlignBottom && face->resource) {
                extras->sTextureDeltaV -= maxU + face->GetTexture()->height();
            }

            if (face->uAttributes & FACE_TexMoveByDoor) {
                float udot = dot(door->vDirection, u);
                float vdot = dot(door->vDirection, v);
                extras->sTextureDeltaU = -udot * openDistance + door->pDeltaUs[j];
                extras->sTextureDeltaV = -vdot * openDistance + door->pDeltaVs[j];
            }
        }
    }
}

//----- (0046F90C) --------------------------------------------------------
void UpdateActors_BLV() {
    if (engine->config->debug.NoActors.value())
        return;

    for (Actor &actor : pActors) {
        if (actor.aiState == Removed || actor.aiState == Disabled || actor.aiState == Summoned || actor.moveSpeed == 0)
            continue;

        int uFaceID;
        float floorZ = GetIndoorFloorZ(actor.pos, &actor.sectorId, &uFaceID);

        if (actor.sectorId == 0 || floorZ <= -30000)
            continue;

        bool isFlying = actor.monsterInfo.flying;
        if (!actor.CanAct())
            isFlying = false;

        bool isAboveGround = false;
        if (actor.pos.z > floorZ + 1)
            isAboveGround = true;

        // make bloodsplat when the ground is hit
        if (!actor.donebloodsplat) {
            if (actor.aiState == Dead || actor.aiState == Dying) {
                if (actor.pos.z < floorZ + 30) { // 30 to provide small error / rounding factor
                    if (pMonsterStats->infos[actor.monsterInfo.id].bloodSplatOnDeath) {
                        if (engine->config->graphics.BloodSplats.value()) {
                            float splatRadius = actor.radius * engine->config->graphics.BloodSplatsMultiplier.value();
                            EngineIocContainer::ResolveDecalBuilder()->AddBloodsplat(Vec3f(actor.pos.x, actor.pos.y, floorZ + 30), colorTable.Red, splatRadius);
                        }
                        actor.donebloodsplat = true;
                    }
                }
            }
        }

        if (actor.currentActionAnimation == ANIM_Walking) {  // actor is moving
            int moveSpeed = actor.moveSpeed;

            if (actor.buffs[ACTOR_BUFF_SLOWED].Active()) {
                if (actor.buffs[ACTOR_BUFF_SLOWED].power)
                    moveSpeed = actor.moveSpeed / actor.buffs[ACTOR_BUFF_SLOWED].power;
                else
                    moveSpeed = actor.moveSpeed / 2;
            }

            if (actor.aiState == Pursuing || actor.aiState == Fleeing)
                moveSpeed *= 2;

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_WAIT)
                moveSpeed = moveSpeed * debug_turn_based_monster_movespeed_mul;

            if (moveSpeed > 1000)
                moveSpeed = 1000;

            actor.velocity.x = TrigLUT.cos(actor.yawAngle) * moveSpeed;
            actor.velocity.y = TrigLUT.sin(actor.yawAngle) * moveSpeed;
            if (isFlying)
                actor.velocity.z = TrigLUT.sin(actor.pitchAngle) * moveSpeed;
        } else {
            // actor is not moving
            // fixpoint(55000) = 0.83923339843, appears to be velocity decay.
            actor.velocity.x *= 0.83923339843f;
            actor.velocity.y *= 0.83923339843f;
            if (isFlying)
                actor.velocity.z *= 0.83923339843f;
        }

        if (actor.pos.z <= floorZ) {
            actor.pos.z = floorZ + 1;
            if (pIndoor->pFaces[uFaceID].uPolygonType == POLYGON_Floor) {
                if (actor.velocity.z < 0)
                    actor.velocity.z = 0;
            } else {
                // fixpoint(45000) = 0.68664550781, no idea what the actual semantics here is.
                if (pIndoor->pFaces[uFaceID].facePlane.normal.z < 0.68664550781f) // was 45000 fixpoint
                    actor.velocity.z -= pEventTimer->dt().ticks() * GetGravityStrength();
            }
        } else {
            if (isAboveGround && !isFlying)
                actor.velocity.z += -8 * pEventTimer->dt().ticks() * GetGravityStrength();
        }

        if (actor.velocity.lengthSqr() >= 400) {
            ProcessActorCollisionsBLV(actor, isAboveGround, isFlying);
        } else {
            actor.velocity = Vec3f(0, 0, 0);
            if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_SKY) {
                if (actor.aiState == Dead)
                    actor.aiState = Removed;
            }
        }
    }
}

void loadAndPrepareBLV(MapId mapid, bool bLoading) {
    unsigned int respawn_interval;  // ebx@1
    MapInfo *map_info;              // edi@9
    bool v28;                       // zf@81
    bool alertStatus;                        // [sp+404h] [bp-10h]@1
    bool indoor_was_respawned = true;                      // [sp+40Ch] [bp-8h]@1
    std::string mapFilename;

    respawn_interval = 0;
    pGameLoadingUI_ProgressBar->Reset(0x20u);
    uCurrentlyLoadedLevelType = LEVEL_INDOOR;
    pBLVRenderParams->uPartySectorID = 0;
    pBLVRenderParams->uPartyEyeSectorID = 0;

    engine->SetUnderwater(isMapUnderwater(mapid));

    //pPaletteManager->pPalette_tintColor[0] = 0;
    //pPaletteManager->pPalette_tintColor[1] = 0;
    //pPaletteManager->pPalette_tintColor[2] = 0;
    //pPaletteManager->RecalculateAll();
    pParty->_delayedReactionTimer = 0_ticks;

    if (mapid != MAP_INVALID) {
        mapFilename = pMapStats->pInfos[mapid].fileName;
        map_info = &pMapStats->pInfos[mapid];
        respawn_interval = pMapStats->pInfos[mapid].respawnIntervalDays;
        alertStatus = GetAlertStatus();

        assert(ascii::noCaseEquals(mapFilename.substr(mapFilename.rfind('.') + 1), "blv"));
    } else {
        // TODO(Nik-RE-dev): why there's logic for loading maps that are not listed in info?
        mapFilename = "";
        map_info = nullptr;
    }

    pStationaryLightsStack->uNumLightsActive = 0;
    pIndoor->Load(mapFilename, pParty->GetPlayingTime().toDays() + 1, respawn_interval, &indoor_was_respawned);
    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)) {
        Actor::InitializeActors();
        SpriteObject::InitializeSpriteObjects();
    }
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN;
    if (mapid == MAP_INVALID)
        indoor_was_respawned = false;

    if (indoor_was_respawned) {
        for (unsigned i = 0; i < pIndoor->pSpawnPoints.size(); ++i) {
            auto spawn = &pIndoor->pSpawnPoints[i];
            if (spawn->uKind == OBJECT_Actor)
                SpawnEncounter(map_info, spawn, 0, 0, 0);
            else
                SpawnRandomTreasure(map_info, spawn);
        }
        RespawnGlobalDecorations();
    }

    // TODO(captainurist): that's some convoluted logic. We set up doors, set uTimeSinceTriggered to 120sec, and thus
    //                     they snap into place on the next frame. Just init them properly!
    for (unsigned i = 0; i < pIndoor->pDoors.size(); ++i) {
        if (pIndoor->pDoors[i].uAttributes & DOOR_TRIGGERED) {
            pIndoor->pDoors[i].uState = DOOR_OPENING;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360_ticks;
            pIndoor->pDoors[i].uAttributes = DOOR_SETTING_UP;
        }

        if (pIndoor->pDoors[i].uState == DOOR_CLOSED) {
            pIndoor->pDoors[i].uState = DOOR_CLOSING;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360_ticks;
            pIndoor->pDoors[i].uAttributes = DOOR_SETTING_UP;
        } else if (pIndoor->pDoors[i].uState == DOOR_OPEN) {
            pIndoor->pDoors[i].uState = DOOR_OPENING;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360_ticks;
            pIndoor->pDoors[i].uAttributes = DOOR_SETTING_UP;
        }
    }

    /*for (unsigned i = 0; i < pIndoor->uNumFaces; ++i)
    {
        if (pIndoor->pFaces[i].uBitmapID != -1)
            pBitmaps_LOD->pTextures[pIndoor->pFaces[i].uBitmapID].palette_id2 =
    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[pIndoor->pFaces[i].uBitmapID].palette_id1);
    }*/

    pGameLoadingUI_ProgressBar->Progress();
    decorationsWithSound.clear();

    int interactiveDecorationsNum = 0;
    for (unsigned i = 0; i < pLevelDecorations.size(); ++i) {
        pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);

        const DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);

        if (decoration->uSoundID != SOUND_Invalid) {
            decorationsWithSound.push_back(i);
        }

        if (!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            if (!decoration->DontDraw()) {
                if (decoration->uLightRadius) {
                    Color color = render->config->graphics.ColoredLights.value() ? decoration->uColoredLight : colorTable.White;
                    pStationaryLightsStack->AddLight(pLevelDecorations[i].vPosition +
                        Vec3f(0, 0, decoration->uDecorationHeight),
                        decoration->uLightRadius, color, _4E94D0_light_type);
                }
            }
        }

        if (!pLevelDecorations[i].uEventID) {
            if (pLevelDecorations[i].IsInteractive()) {
                if (interactiveDecorationsNum < 124) {
                    pLevelDecorations[i].eventVarId = interactiveDecorationsNum;
                    if (!engine->_persistentVariables.decorVars[interactiveDecorationsNum])
                        pLevelDecorations[i].uFlags |= LEVEL_DECORATION_INVISIBLE;
                    interactiveDecorationsNum++;
                }
            }
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    for (int i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uObjectDescID) {
            if (pSpriteObjects[i].containing_item.uItemID != ITEM_NULL) {
                if (pSpriteObjects[i].containing_item.uItemID != ITEM_POTION_BOTTLE &&
                    pItemTable->pItems[pSpriteObjects[i].containing_item.uItemID].uEquipType == ITEM_TYPE_POTION &&
                    !pSpriteObjects[i].containing_item.potionPower)
                    pSpriteObjects[i].containing_item.potionPower = grng->random(15) + 5;
                pItemTable->SetSpecialBonus(&pSpriteObjects[i].containing_item);
            }
        }
    }

    // INDOOR initialize actors
    alertStatus = false;

    for (unsigned i = 0; i < pActors.size(); ++i) {
        if (pActors[i].attributes & ACTOR_UNKNOW7) {
            if (mapid == MAP_INVALID) {
                pActors[i].monsterInfo.field_3E = 19;
                pActors[i].attributes |= ACTOR_UNKNOW11;
                continue;
            }
            v28 = !alertStatus;
        } else {
            v28 = alertStatus;
        }

        if (!v28) {
            pActors[i].PrepareSprites(0);
            pActors[i].monsterInfo.hostilityType = HOSTILITY_FRIENDLY;
            if (pActors[i].monsterInfo.field_3E != 11 &&
                pActors[i].monsterInfo.field_3E != 19 &&
                (!pActors[i].currentHP || !pActors[i].monsterInfo.hp)) {
                pActors[i].monsterInfo.field_3E = 5;
                pActors[i].UpdateAnimation();
            }
        } else {
            pActors[i].monsterInfo.field_3E = 19;
            pActors[i].attributes |= ACTOR_UNKNOW11;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    Actor this_;
    this_.monsterInfo.id = MONSTER_ELEMENTAL_LIGHT_C;
    this_.PrepareSprites(0); // TODO(captainurist): can drop this? Was loaded because light elementals can be summoned.

    // Party to start position
    if (!bLoading) {
        pParty->_viewPitch = 0;
        pParty->_viewYaw = 0;
        pParty->pos = Vec3f();
        pParty->velocity = Vec3f();
        pParty->uFallStartZ = 0;
        TeleportToStartingPoint(uLevel_StartingPointType);
        pBLVRenderParams->Reset();
    }
    viewparams->_443365();
    PlayLevelMusic();

    // Active character speaks.
    if (!bLoading && indoor_was_respawned) {
        int id = pParty->getRandomActiveCharacterId(vrng);

        if (id != -1) {
            pParty->setDelayedReaction(SPEECH_ENTER_DUNGEON, id);
        }
    }
}

//----- (0046CEC3) --------------------------------------------------------
float BLV_GetFloorLevel(const Vec3f &pos, int uSectorID, int *pFaceID) {
    // stores faces and floor z levels
    int FacesFound = 0;
    float blv_floor_z[5] = { 0 };
    int blv_floor_id[5] = { 0 };

    BLVSector *pSector = &pIndoor->pSectors[uSectorID];

    // loop over all floor faces
    for (unsigned i = 0; i < pSector->uNumFloors; ++i) {
        if (FacesFound >= 5)
            break;

        BLVFace *pFloor = &pIndoor->pFaces[pSector->pFloors[i]];
        if (pFloor->Ethereal())
            continue;

        if (!pFloor->Contains(pos, MODEL_INDOOR, engine->config->gameplay.FloorChecksEps.value(), FACE_XY_PLANE))
            continue;

        // TODO: Does POLYGON_Ceiling really belong here?
        // Returned z is then used like this in UpdateActors_BLV:
        //
        // if (actor.z <= z) {
        //     actor.z = z + 1;
        //
        // And if this z is ceiling z, then this will place the actor above the ceiling.
        float z_calc;
        if (pFloor->uPolygonType == POLYGON_Floor || pFloor->uPolygonType == POLYGON_Ceiling) {
            z_calc = pIndoor->pVertices[pFloor->pVertexIDs[0]].z;
        } else {
            z_calc = pFloor->zCalc.calculate(pos.x, pos.y);
        }

        blv_floor_z[FacesFound] = z_calc;
        blv_floor_id[FacesFound] = pSector->pFloors[i];
        FacesFound++;
    }

    // as above but for sector portal faces
    if (pSector->field_0 & 8) {
        for (unsigned i = 0; i < pSector->uNumPortals; ++i) {
            if (FacesFound >= 5) break;

            BLVFace *portal = &pIndoor->pFaces[pSector->pPortals[i]];
            if (portal->uPolygonType != POLYGON_Floor)
                continue;

            if(!portal->Contains(pos, MODEL_INDOOR, engine->config->gameplay.FloorChecksEps.value(), FACE_XY_PLANE))
                continue;

            blv_floor_z[FacesFound] = -29000;
            blv_floor_id[FacesFound] = pSector->pPortals[i];
            FacesFound++;
        }
    }

    // one face found
    if (FacesFound == 1) {
        if (pFaceID)
            *pFaceID = blv_floor_id[0];
        if (blv_floor_z[0] <= -29000) {
            /*assert(false);*/
        }
        return blv_floor_z[0];
    }

    // no face found - probably wrong sector supplied
    if (!FacesFound) {
        logger->trace("Floorlvl fail: {} {} {}", pos.x, pos.y, pos.z);

        if (pFaceID)
            *pFaceID = -1;
        return -30000;
    }

    // multiple faces found - pick nearest
    float result = blv_floor_z[0];
    int faceId = blv_floor_id[0];
    for (unsigned i = 1; i < FacesFound; ++i) {
        float v38 = blv_floor_z[i];

        if (std::abs(pos.z - v38) <= std::abs(pos.z - result)) {
            result = blv_floor_z[i];
            if (blv_floor_z[i] <= -29000) assert(false);
            faceId = blv_floor_id[i];
        }
    }

    if (result <= -29000) assert(false);

    if (pFaceID)
        *pFaceID = faceId;
    return result;
}

//----- (0043FA33) --------------------------------------------------------
void IndoorLocation::PrepareDecorationsRenderList_BLV(unsigned int uDecorationID, int uSectorID) {
    unsigned int v8;       // edi@5
    int v9;                // edi@5
    int v10;               // eax@7
    SpriteFrame *v11;      // eax@7
    Particle_sw particle;  // [sp+Ch] [bp-A0h]@3
    int v30;               // [sp+8Ch] [bp-20h]@7

    if (pLevelDecorations[uDecorationID].uFlags & LEVEL_DECORATION_INVISIBLE)
        return;

    const DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[uDecorationID].uDecorationDescID);

    if (decoration->uFlags & DECORATION_DESC_EMITS_FIRE) {
        memset(&particle, 0, sizeof(Particle_sw));  // fire,  like at the Pit's tavern
        particle.type = ParticleType_Bitmap | ParticleType_Rotating | ParticleType_Ascending;
        particle.uDiffuse = colorTable.OrangeyRed;
        particle.x = (double)pLevelDecorations[uDecorationID].vPosition.x;
        particle.y = (double)pLevelDecorations[uDecorationID].vPosition.y;
        particle.z = (double)pLevelDecorations[uDecorationID].vPosition.z;
        particle.r = 0.0;
        particle.g = 0.0;
        particle.b = 0.0;
        particle.particle_size = 1.0;
        particle.timeToLive = Duration::randomRealtimeSeconds(vrng, 1, 2); // was either 1 or 2 secs, we made it into [1, 2).
        particle.texture = spell_fx_renderer->effpar01;
        particle_engine->AddParticle(&particle);
        return;
    }

    if (decoration->uFlags & DECORATION_DESC_DONT_DRAW) {
        return;
    }

    v8 = pLevelDecorations[uDecorationID]._yawAngle +
         ((signed int)TrigLUT.uIntegerPi >> 3) - TrigLUT.atan2(pLevelDecorations[uDecorationID].vPosition.x - pCamera3D->vCameraPos.x,
                                                               pLevelDecorations[uDecorationID].vPosition.y - pCamera3D->vCameraPos.y);
    v9 = ((signed int)(TrigLUT.uIntegerPi + v8) >> 8) & 7;
    Duration v37 = pEventTimer->time();
    if (pParty->bTurnBasedModeOn) v37 = pMiscTimer->time();
    v10 = std::abs(pLevelDecorations[uDecorationID].vPosition.x +
              pLevelDecorations[uDecorationID].vPosition.y);
    v11 = pSpriteFrameTable->GetFrame(decoration->uSpriteID, v37 + Duration::fromTicks(v10));

    // error catching
    if (v11->icon_name == "null") assert(false);

    v30 = 0;
    if (v11->uFlags & 2) v30 = 2;
    if (v11->uFlags & 0x40000) v30 |= 0x40;
    if (v11->uFlags & 0x20000) v30 |= 0x80;
    if ((256 << v9) & v11->uFlags) v30 |= 4;

    int view_x = 0;
    int view_y = 0;
    int view_z = 0;
    bool visible =
        pCamera3D->ViewClip(pLevelDecorations[uDecorationID].vPosition.x,
                                   pLevelDecorations[uDecorationID].vPosition.y,
                                   pLevelDecorations[uDecorationID].vPosition.z,
                                   &view_x, &view_y, &view_z);

    if (visible) {
        if (2 * std::abs(view_x) >= std::abs(view_y)) {
            int projected_x = 0;
            int projected_y = 0;
            pCamera3D->Project(view_x, view_y, view_z, &projected_x, &projected_y);

            float billb_scale = v11->scale * pCamera3D->ViewPlaneDistPixels / view_x;

            int screen_space_half_width = static_cast<int>(billb_scale * v11->hw_sprites[(int64_t)v9]->uWidth / 2.0f);
            int screen_space_height = static_cast<int>(billb_scale * v11->hw_sprites[(int64_t)v9]->uHeight);

            if (projected_x + screen_space_half_width >= (signed int)pViewport->uViewportTL_X &&
                projected_x - screen_space_half_width <= (signed int)pViewport->uViewportBR_X) {
                if (projected_y >= pViewport->uViewportTL_Y && (projected_y - screen_space_height) <= pViewport->uViewportBR_Y) {
                    assert(uNumBillboardsToDraw < 500);
                    ++uNumBillboardsToDraw;
                    ++uNumDecorationsDrawnThisFrame;

                    pBillboardRenderList[uNumBillboardsToDraw - 1].hwsprite =
                        v11->hw_sprites[v9];

                    if (v11->hw_sprites[v9]->texture->height() == 0 || v11->hw_sprites[v9]->texture->width() == 0)
                        assert(false);

                    pBillboardRenderList[uNumBillboardsToDraw - 1].uPaletteIndex = v11->GetPaletteIndex();
                    pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID =
                        uSectorID;

                    pBillboardRenderList[uNumBillboardsToDraw - 1].fov_x =
                        pCamera3D->ViewPlaneDistPixels;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x = billb_scale;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = billb_scale;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = v30;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].world_x =
                        pLevelDecorations[uDecorationID].vPosition.x;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].world_y =
                        pLevelDecorations[uDecorationID].vPosition.y;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].world_z =
                        pLevelDecorations[uDecorationID].vPosition.z;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_x =
                        projected_x;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_y =
                        projected_y;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_z =
                        view_x;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].object_pid =
                        Pid(OBJECT_Decoration, uDecorationID);

                    pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor = Color();
                    pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = v11;
                }
            }
        }
    }
}

//----- (00407A1C) --------------------------------------------------------
bool Check_LineOfSight(const Vec3f &target, const Vec3f &from) {  // target from - true on clear
    int AngleToTarget = TrigLUT.atan2(from.x - target.x, from.y - target.y);
    bool LOS_Obscurred = false;
    bool LOS_Obscurred2 = false;
    bool LOS_ObscurredStr = false;

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        // check straight point to point
        LOS_ObscurredStr = Check_LOS_Obscurred_Indoors(target, from);

        if (LOS_ObscurredStr) {
            // offset 32 to side and check LOS
            Vec3f targetmod = target + Vec3f::fromPolar(32, AngleToTarget + TrigLUT.uIntegerHalfPi, 0);
            Vec3f frommod = from + Vec3f::fromPolar(32, AngleToTarget + TrigLUT.uIntegerHalfPi, 0);
            LOS_Obscurred2 = Check_LOS_Obscurred_Indoors(targetmod, frommod);

            if (LOS_Obscurred2) {
                // offset other side and repeat check
                targetmod = target + Vec3f::fromPolar(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0);
                frommod = from + Vec3f::fromPolar(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0);
                LOS_Obscurred = Check_LOS_Obscurred_Indoors(targetmod, frommod);
            }
        }
    } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        // TODO(pskelton): Need to add check against terrain
        // check straight point to point
        LOS_ObscurredStr = Check_LOS_Obscurred_Outdoors_Bmodels(target, from);

        if (LOS_ObscurredStr) {
            // offset 32 to side and check LOS
            Vec3f targetmod = target + Vec3f::fromPolar(32, AngleToTarget + TrigLUT.uIntegerHalfPi, 0);
            Vec3f frommod = from + Vec3f::fromPolar(32, AngleToTarget + TrigLUT.uIntegerHalfPi, 0);
            LOS_Obscurred2 = Check_LOS_Obscurred_Outdoors_Bmodels(targetmod, frommod);

            if (LOS_Obscurred2) {
                // offset other side and repeat check
                targetmod = target + Vec3f::fromPolar(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0);
                frommod = from + Vec3f::fromPolar(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0);
                LOS_Obscurred = Check_LOS_Obscurred_Outdoors_Bmodels(targetmod, frommod);
            }
        }
    }

    bool result{ !LOS_Obscurred2 || !LOS_Obscurred || !LOS_ObscurredStr };
    return result;  // true if LOS clear
}

bool Check_LOS_Obscurred_Indoors(const Vec3f &target, const Vec3f &from) {  // true if obscurred
    Vec3f dir = from - target;
    float dist = dir.length();
    dir.normalize();

    BBoxf bbox = BBoxf::forPoints(from, target);

    for (int sectargetrflip = 0; sectargetrflip < 2; sectargetrflip++) {
        int SectargetrID = 0;
        if (sectargetrflip)
            SectargetrID = pIndoor->GetSector(target);
        else
            SectargetrID = pIndoor->GetSector(from);

        // loop over sectargetr faces
        for (int FaceLoop = 0; FaceLoop < pIndoor->pSectors[SectargetrID].uNumFaces; ++FaceLoop) {
            BLVFace *face = &pIndoor->pFaces[pIndoor->pSectors[SectargetrID].pFaceIDs[FaceLoop]];
            if (face->isPortal() || face->Ethereal())
                continue;

            // dot product
            float dirDotNormal = dot(dir, face->facePlane.normal);
            bool FaceIsParallel = fuzzyIsNull(dirDotNormal);
            if (FaceIsParallel)
                continue;

            // skip further checks
            if (!bbox.intersects(face->pBounding))
                continue;

            float NegFacePlaceDist = -face->facePlane.signedDistanceTo(target);
            // are we on same side of plane
            if (dirDotNormal <= 0) {
                if (NegFacePlaceDist > 0)
                    continue;
            } else {
                if (NegFacePlaceDist < 0)
                    continue;
            }

            // TODO(captainurist): what's going on in this check?
            if (std::abs(NegFacePlaceDist) / 16384.0f <= std::abs(dirDotNormal)) {
                float IntersectionDist = NegFacePlaceDist / dirDotNormal;
                // less than zero means intersection is behind target point
                // greater than dist means intersection is behind the caster
                if (IntersectionDist >= 0.0 && IntersectionDist <= dist) {
                    Vec3f pos = target + (IntersectionDist * dir);
                    if (face->Contains(pos, MODEL_INDOOR)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Check_LOS_Obscurred_Outdoors_Bmodels(const Vec3f &target, const Vec3f &from) {  // true is obscurred
    Vec3f dir = from - target;
    float dist = dir.length();
    dir.normalize();

    BBoxf bbox = BBoxf::forPoints(from, target);

    for (BSPModel &model : pOutdoor->pBModels) {
        if (CalcDistPointToLine(target.x, target.y, from.x, from.y, model.vPosition.x, model.vPosition.y) <= model.sBoundingRadius + 128) {
            for (ODMFace &face : model.pFaces) {
                if (face.Ethereal()) continue;

                float dirDotNormal = dot(dir, face.facePlane.normal);
                bool FaceIsParallel = fuzzyIsNull(dirDotNormal);
                if (FaceIsParallel)
                    continue;

                // bounds check
                if (!bbox.intersects(face.pBoundingBox))
                    continue;

                // point target plane distacne
                float NegFacePlaceDist = -face.facePlane.signedDistanceTo(target);

                // are we on same side of plane
                if (dirDotNormal <= 0) {
                    // angle obtuse - is target underneath plane
                    if (NegFacePlaceDist > 0)
                        continue;  // can never hit
                } else {
                    // angle acute - is target above plane
                    if (NegFacePlaceDist < 0)
                        continue;  // can never hit
                }

                if (std::abs(NegFacePlaceDist) / 16384.0f <= std::abs(dirDotNormal)) {
                    // calc how far along line interesction is
                    float IntersectionDist = NegFacePlaceDist /  dirDotNormal;
                    // less than zero means intersection is behind target point
                    // greater than dist means intersection is behind the caster
                    if (IntersectionDist >= 0.0 && IntersectionDist <= dist) {
                        Vec3f pos = target + IntersectionDist * dir;
                        if (face.Contains(pos, model.index)) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

//----- (0046A334) --------------------------------------------------------
// TODO(Nik-RE-dev): does not belong here, it's common function for interaction for both indoor/outdoor
// TODO(Nik-RE-dev): get rid of external function declaration inside
char DoInteractionWithTopmostZObject(Pid pid) {
    auto id = pid.id();
    auto type = pid.type();

    // was SCREEN_BRANCHLESS_NPC_DIALOG
    if (current_screen_type != SCREEN_GAME) {
        return 1;
    }

    switch (type) {
        case OBJECT_Item: {  // take the item
            if (pSpriteObjects[id].IsUnpickable() || id >= pSpriteObjects.size() || !pSpriteObjects[id].uObjectDescID) {
                return 1;
            }

            extern void ItemInteraction(unsigned int item_id);
            ItemInteraction(id);
            break;
        }

        case OBJECT_Actor:
            if (pActors[id].aiState == Dying || pActors[id].aiState == Summoned)
                return 1;
            if (pActors[id].aiState == Dead) {
                pActors[id].LootActor();
            } else {
                extern bool CanInteractWithActor(unsigned int id);
                extern void InteractWithActor(unsigned int id);
                if (CanInteractWithActor(id)) {
                    if (pParty->hasActiveCharacter()) {
                        InteractWithActor(id);
                    } else {
                        engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
                    }
                }
            }
            break;

        case OBJECT_Decoration:
            extern void DecorationInteraction(unsigned int id, Pid pid);
            if (pParty->hasActiveCharacter()) {
                DecorationInteraction(id, pid);
            } else {
                engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
            }
            break;

        case OBJECT_Face:
            if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                int bmodel_id = id >> 6;
                int face_id = id & 0x3F;

                if (bmodel_id >= pOutdoor->pBModels.size()) {
                    return 1;
                }

                ODMFace &model = pOutdoor->pBModels[bmodel_id].pFaces[face_id];

                if (model.uAttributes & FACE_HAS_EVENT || model.sCogTriggeredID == 0) {
                    return 1;
                }

                if (pParty->hasActiveCharacter()) {
                    eventProcessor(pOutdoor->pBModels[bmodel_id].pFaces[face_id].sCogTriggeredID, pid, 1);
                } else {
                    engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
                }
            } else {
                if (!(pIndoor->pFaces[id].uAttributes & FACE_CLICKABLE)) {
                    engine->_statusBar->nothingHere();
                    return 1;
                }
                if (pIndoor->pFaces[id].uAttributes & FACE_HAS_EVENT || !pIndoor->pFaceExtras[pIndoor->pFaces[id].uFaceExtraID].uEventID) {
                    return 1;
                }

                if (pParty->hasActiveCharacter()) {
                    eventProcessor((int16_t)pIndoor->pFaceExtras[pIndoor->pFaces[id].uFaceExtraID].uEventID, pid, 1);
                } else {
                    engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
                }
            }
            return 0;
            break;

        default:
            logger->warning("Warning: Invalid ID reached!");
            return 1;
    }

    return 0;
}
//----- (0046BDF1) --------------------------------------------------------
void BLV_UpdateUserInputAndOther() {
    BLV_ProcessPartyActions();
    UpdateActors_BLV();
    BLV_UpdateDoors();
}

//----- (00472866) --------------------------------------------------------
void BLV_ProcessPartyActions() {  // could this be combined with odm process actions?
    int faceEvent = 0;

    bool party_running_flag = false;
    bool party_walking_flag = false;
    bool not_high_fall = false;
    bool on_water = false;
    bool bFeatherFall;

    int sectorId = pBLVRenderParams->uPartySectorID;
    int faceId = -1;
    float floorZ = GetIndoorFloorZ(pParty->pos + Vec3f(0, 0, pParty->radius), &sectorId, &faceId);

    if (pParty->bFlying)  // disable flight
        pParty->bFlying = false;

    if (floorZ == -30000 || faceId == -1) {
        floorZ = GetApproximateIndoorFloorZ(pParty->pos + Vec3f(0, 0, pParty->radius), &sectorId, &faceId);
        if (floorZ == -30000 || faceId == -1) {
            assert(false);  // level built with errors
            return;
        }
    }

    int fall_start;
    if (pParty->FeatherFallActive() || pParty->wearsItemAnywhere(ITEM_ARTIFACT_LADYS_ESCORT)
        || pParty->uFlags & (PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING)) {
        fall_start = floorZ;
        bFeatherFall = true;
        pParty->uFallStartZ = floorZ;
    } else {
        bFeatherFall = false;
        fall_start = pParty->uFallStartZ;
    }

    if (fall_start - pParty->pos.z > 512 && !bFeatherFall && pParty->pos.z <= floorZ + 1) {  // fall damage
        if (pParty->uFlags & (PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING)) {
            // flying was previously used to prevent fall damage from jump spell
            pParty->uFlags &= ~(PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING);
        } else {
            pParty->giveFallDamage(pParty->uFallStartZ - pParty->pos.z);
        }
    }

    bool isAboveGround = pParty->pos.z > floorZ + 1;

    if (!isAboveGround) {
        pParty->pos.z = floorZ + 1; // Snap to floor if party is below.
        pParty->uFallStartZ = pParty->pos.z;
    } else if (pParty->pos.z <= floorZ + 32) {
        not_high_fall = true;
        pParty->uFallStartZ = pParty->pos.z;
    }

    // not hovering & stepped onto a new face => activate potential pressure plate.
    if (!isAboveGround && pParty->floor_face_id != faceId) {
        if (pIndoor->pFaces[faceId].uAttributes & FACE_PRESSURE_PLATE)
            faceEvent = pIndoor->pFaceExtras[pIndoor->pFaces[faceId].uFaceExtraID].uEventID;
    }

    if (!isAboveGround)
        pParty->floor_face_id = faceId;

    // party is on water?
    if (pIndoor->pFaces[faceId].isFluid())
        on_water = true;

    // Party angle in XY plane.
    int angle = pParty->_viewYaw;

    // Vertical party angle (basically azimuthal angle in polar coordinates).
    int vertical_angle = pParty->_viewPitch;

    // Calculate rotation in ticks (1024 ticks per 180 degree).
    // TODO(captainurist): #time think about a better way to write this formula.
    int rotation =
        pEventTimer->dt().ticks() * pParty->_yawRotationSpeed * TrigLUT.uIntegerPi / 180 / Duration::TICKS_PER_REALTIME_SECOND;

    pParty->velocity = Vec3f(0, 0, pParty->velocity.z);

    while (pPartyActionQueue->uNumActions) {
        switch (pPartyActionQueue->Next()) {
            case PARTY_TurnLeft:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    angle = TrigLUT.uDoublePiMask & (angle + (int) engine->config->settings.TurnSpeed.value());
                else
                    angle = TrigLUT.uDoublePiMask & (angle + static_cast<int>(rotation * fTurnSpeedMultiplier));
                break;
            case PARTY_TurnRight:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    angle = TrigLUT.uDoublePiMask & (angle - (int) engine->config->settings.TurnSpeed.value());
                else
                    angle = TrigLUT.uDoublePiMask & (angle - static_cast<int>(rotation * fTurnSpeedMultiplier));
                break;

            case PARTY_FastTurnLeft:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    angle = TrigLUT.uDoublePiMask & (angle + (int) engine->config->settings.TurnSpeed.value());
                else
                    angle = TrigLUT.uDoublePiMask & (angle + static_cast<int>(2.0f * rotation * fTurnSpeedMultiplier));
                break;

            case PARTY_FastTurnRight:
                if (engine->config->settings.TurnSpeed.value() > 0)
                    angle = TrigLUT.uDoublePiMask & (angle - (int) engine->config->settings.TurnSpeed.value());
                else
                    angle = TrigLUT.uDoublePiMask & (angle - static_cast<int>(2.0f * rotation * fTurnSpeedMultiplier));
                break;

            case PARTY_StrafeLeft:
                pParty->velocity.x -= TrigLUT.sin(angle) * pParty->walkSpeed * fWalkSpeedMultiplier / 2;
                pParty->velocity.y += TrigLUT.cos(angle) * pParty->walkSpeed * fWalkSpeedMultiplier / 2;
                party_walking_flag = true;
                break;

            case PARTY_StrafeRight:
                pParty->velocity.y -= TrigLUT.cos(angle) * pParty->walkSpeed * fWalkSpeedMultiplier / 2;
                pParty->velocity.x += TrigLUT.sin(angle) * pParty->walkSpeed * fWalkSpeedMultiplier / 2;
                party_walking_flag = true;
                break;

            case PARTY_WalkForward:
                pParty->velocity.x += TrigLUT.cos(angle) * pParty->walkSpeed * fWalkSpeedMultiplier;
                pParty->velocity.y += TrigLUT.sin(angle) * pParty->walkSpeed * fWalkSpeedMultiplier;
                party_walking_flag = true;
                break;

            case PARTY_WalkBackward:
                pParty->velocity.x -= TrigLUT.cos(angle) * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                pParty->velocity.y -= TrigLUT.sin(angle) * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                party_walking_flag = true;
                break;

            case PARTY_RunForward:
                pParty->velocity.x += TrigLUT.cos(angle) * 2 * pParty->walkSpeed * fWalkSpeedMultiplier;
                pParty->velocity.y += TrigLUT.sin(angle) * 2 * pParty->walkSpeed * fWalkSpeedMultiplier;
                party_running_flag = true;
                break;

            case PARTY_RunBackward:
                pParty->velocity.x -= TrigLUT.cos(angle) * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                pParty->velocity.y -= TrigLUT.sin(angle) * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                party_walking_flag = true;
                break;

            case PARTY_LookUp:
                vertical_angle += engine->config->settings.VerticalTurnSpeed.value();
                if (vertical_angle > 128)
                    vertical_angle = 128;
                if (pParty->hasActiveCharacter())
                    pParty->activeCharacter().playReaction(SPEECH_LOOK_UP);
                break;

            case PARTY_LookDown:
                vertical_angle -= engine->config->settings.VerticalTurnSpeed.value();
                if (vertical_angle < -128)
                    vertical_angle = -128;
                if (pParty->hasActiveCharacter())
                    pParty->activeCharacter().playReaction(SPEECH_LOOK_DOWN);
                break;

            case PARTY_CenterView:
                vertical_angle = 0;
                break;

            case PARTY_Jump:
                if ((!isAboveGround || pParty->pos.z <= floorZ + 6 && pParty->velocity.z <= 0) && pParty->jump_strength) {
                    isAboveGround = true;
                    pParty->velocity.z += pParty->jump_strength * 96;
                }
                break;
            default:
                break;
        }
    }

    if (isAboveGround) {
        pParty->velocity.z += -2.0f * pEventTimer->dt().ticks() * GetGravityStrength();
        if (pParty->velocity.z < -500) {
            for (Character &character : pParty->pCharacters) {
                if (!character.HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) &&
                    !character.WearsItem(ITEM_ARTIFACT_HERMES_SANDALS, ITEM_SLOT_BOOTS)) {  // was 8
                    character.playEmotion(CHARACTER_EXPRESSION_SCARED, 0_ticks);
                }
            }
        }
    } else {
        if (pIndoor->pFaces[faceId].facePlane.normal.z < 0.5) {
            pParty->velocity.z -= 1.0f * pEventTimer->dt().ticks() * GetGravityStrength();
        } else {
            if (!(pParty->uFlags & PARTY_FLAG_LANDING))
                pParty->velocity.z = 0;
        }
    }

    if (!isAboveGround || pParty->velocity.z > 0)
        pParty->uFallStartZ = pParty->pos.z;

    // If party movement delta is lower then this number then the party remains stationary.
    int64_t elapsed_time_bounded = std::min(pEventTimer->dt().ticks(), static_cast<std::int64_t>(10000));
    int min_party_move_delta_sqr = 400 * elapsed_time_bounded * elapsed_time_bounded / 8;

    if (pParty->velocity.xy().lengthSqr() < min_party_move_delta_sqr) {
        pParty->velocity.x = 0;
        pParty->velocity.y = 0;
    }

    Vec3f oldPos = pParty->pos;
    Vec3f savedspeed = pParty->velocity;

    // horizontal
    pParty->velocity.z = 0;
    ProcessPartyCollisionsBLV(sectorId, min_party_move_delta_sqr, &faceId, &faceEvent);
    // vertical -  only when horizonal motion hasnt caused height gain
    if (pParty->pos.z <= oldPos.z) {
        pParty->velocity = Vec3f(0, 0, savedspeed.z);
        ProcessPartyCollisionsBLV(sectorId, min_party_move_delta_sqr, &faceId, &faceEvent);
    }

    // walking / running sounds ------------------------
    if (engine->config->settings.WalkSound.value()) {
        bool canStartNewSound = !pAudioPlayer->isWalkingSoundPlays();

        // Start sound processing only when actual movement is performed to avoid stopping sounds on high FPS
        if (pEventTimer->dt()) {
            // TODO(Nik-RE-dev): use calculated velocity of party and walk/run flags instead of delta
            int walkDelta = integer_sqrt((oldPos - pParty->pos).lengthSqr());

            if (walkDelta < 2) {
                // mute the walking sound when stopping
                pAudioPlayer->stopWalkingSounds();
            } else {
                // Delta limits for running/walking has been changed. Previously:
                // - for run limit was >= 16
                // - for walk limit was >= 8
                // - stop sound if delta < 8
                if (!isAboveGround || not_high_fall) {
                    SoundId sound = SOUND_Invalid;
                    if (party_running_flag) {
                        if (walkDelta >= 4) {
                            if (on_water) {
                                sound = SOUND_RunWaterIndoor;
                            } else if (pIndoor->pFaces[faceId].uAttributes & FACE_INDOOR_CARPET) {
                                sound = SOUND_RunCarpet;
                            } else {
                                // TODO(Nik-RE-dev): need to probe surface
                                sound = SOUND_RunWood;
                            }
                        }
                    } else if (party_walking_flag) {
                        if (walkDelta >= 2) {
                            if (on_water) {
                                sound = SOUND_WalkWaterIndoor;
                            } else if (pIndoor->pFaces[faceId].uAttributes & FACE_INDOOR_CARPET) {
                                sound = SOUND_WalkCarpet;
                            } else {
                                // TODO(Nik-RE-dev): need to probe surface
                                sound = SOUND_WalkWood;
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
    //-------------------------------------------------------------

    if (!isAboveGround || not_high_fall)
        pParty->setAirborne(false);
    else
        pParty->setAirborne(true);

    pParty->uFlags &= ~(PARTY_FLAG_BURNING | PARTY_FLAG_WATER_DAMAGE);
    pParty->_viewYaw = angle;
    pParty->_viewPitch = vertical_angle;

    if (!isAboveGround && pIndoor->pFaces[faceId].uAttributes & FACE_IsLava)
        pParty->uFlags |= PARTY_FLAG_BURNING;

    if (faceEvent)
        eventProcessor(faceEvent, Pid(), 1);
}

void switchDoorAnimation(unsigned int uDoorID, DoorAction a2) {
    auto pos = std::ranges::find(pIndoor->pDoors, uDoorID, &BLVDoor::uDoorID);
    if (pos == pIndoor->pDoors.end()) {
        logger->error("Unable to find Door ID: {}!", uDoorID);
        return;
    }

    BLVDoor &door = *pos;

    if (a2 == DOOR_ACTION_TRIGGER) {
        if (door.uState == DOOR_CLOSING || door.uState == DOOR_OPENING)
            return;

        door.uTimeSinceTriggered = 0_ticks;

        if (door.uState == DOOR_OPEN) {
            door.uState = DOOR_CLOSING;
        } else {
            assert(door.uState == DOOR_CLOSED);
            door.uState = DOOR_OPENING;
        }
    } else if (a2 == DOOR_ACTION_CLOSE) {
        if (door.uState == DOOR_CLOSED || door.uState == DOOR_CLOSING)
            return;

        if (door.uState == DOOR_OPEN) {
            door.uTimeSinceTriggered = 0_ticks;
        } else if (door.uTimeSinceTriggered != 15360_ticks) {
            assert(door.uState == DOOR_OPENING);
            int totalTimeMs = 1000 * door.uMoveLength / door.uCloseSpeed;
            int timeLeftMs = door.uTimeSinceTriggered.realtimeMilliseconds() * door.uOpenSpeed / door.uCloseSpeed;
            door.uTimeSinceTriggered = Duration::fromRealtimeMilliseconds(totalTimeMs - timeLeftMs);
        }
        door.uState = DOOR_CLOSING;
    } else if (a2 == DOOR_ACTION_OPEN) {
        if (door.uState == DOOR_OPEN || door.uState == DOOR_OPENING)
            return;

        if (door.uState == DOOR_CLOSED) {
            door.uTimeSinceTriggered = 0_ticks;
        } else if (door.uTimeSinceTriggered != 15360_ticks) {
            assert(door.uState == DOOR_CLOSING);
            int totalTimeMs = 1000 * door.uMoveLength / door.uOpenSpeed;
            int timeLeftMs = door.uTimeSinceTriggered.realtimeMilliseconds() * door.uCloseSpeed / door.uOpenSpeed;
            door.uTimeSinceTriggered = Duration::fromRealtimeMilliseconds(totalTimeMs - timeLeftMs);
        }
        door.uState = DOOR_OPENING;
    }
}

//----- (004088E9) --------------------------------------------------------
int CalcDistPointToLine(int x1, int y1, int x2, int y2, int x3, int y3) {
    // calculates distance from point x3y3 to line x1y1->x2y2

    signed int result;
    // calc line length
    result = integer_sqrt(std::abs(x2 - x1) * std::abs(x2 - x1) + std::abs(y2 - y1) * std::abs(y2 - y1));

    // orthogonal projection from line to point
    if (result)
        result = std::abs(((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / result);

    return result;
}

//----- (0045063B) --------------------------------------------------------
int SpawnEncounterMonsters(MapInfo *map_info, int enc_index) {
    // creates random spawn point for encounter
    bool failed_point = false;
    float angle_from_party;
    int dist_from_party;
    SpawnPoint enc_spawn_point;
    unsigned loop_cnt = 0;

    //// why check this ??
    // if (!uNumActors) return 0;

    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        int dist_y;
        int dist_x;
        bool not_in_model = false;
        bool bInWater = false;
        int modelPID = 0;

        // 100 attempts to make a usuable spawn point
        for (; loop_cnt < 100; ++loop_cnt) {
            // random x,y at distance from party
            dist_from_party = grng->random(1024) + 512;
            angle_from_party = (grng->random(TrigLUT.uIntegerDoublePi) * 2 * pi) / TrigLUT.uIntegerDoublePi;
            enc_spawn_point.vPosition.x = pParty->pos.x + std::cos(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.y = pParty->pos.y + std::sin(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.z = pParty->pos.z;
            enc_spawn_point.uKind = OBJECT_Actor;
            enc_spawn_point.uMonsterIndex = enc_index;

            // get proposed floor level
            enc_spawn_point.vPosition.z = ODM_GetFloorLevel(enc_spawn_point.vPosition, 0, &bInWater, &modelPID, 0);

            // check spawn point is not in a model
            for (BSPModel &model : pOutdoor->pBModels) {
                dist_y = std::abs(enc_spawn_point.vPosition.y - model.vBoundingCenter.y);
                dist_x = std::abs(enc_spawn_point.vPosition.x - model.vBoundingCenter.x);
                if (int_get_vector_length(dist_x, dist_y, 0) <
                    model.sBoundingRadius + 256) {
                    not_in_model = 1;
                    break;
                }
            }

            // break loop if point sucessful
            if (not_in_model) {
                break;
            }
        }
        failed_point = loop_cnt == 100;
    } else if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        int party_sectorID = pBLVRenderParams->uPartySectorID;
        int mon_sectorID;
        int indoor_floor_level;

        // 100 attempts to make a usuable spawn point
        for (loop_cnt = 0; loop_cnt < 100; ++loop_cnt) {
            // random x,y at distance from party
            dist_from_party = grng->random(512) + 256;
            angle_from_party = (grng->random(TrigLUT.uIntegerDoublePi) * 2 * pi) / TrigLUT.uIntegerDoublePi;
            enc_spawn_point.vPosition.x = pParty->pos.x + std::cos(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.y = pParty->pos.y + std::sin(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.z = pParty->pos.z;
            enc_spawn_point.uKind = OBJECT_Actor;
            enc_spawn_point.uMonsterIndex = enc_index;

            // get proposed sector
            mon_sectorID = pIndoor->GetSector(enc_spawn_point.vPosition);
            if (mon_sectorID == party_sectorID) {
                // check proposed floor level
                indoor_floor_level = BLV_GetFloorLevel(enc_spawn_point.vPosition, mon_sectorID);
                enc_spawn_point.vPosition.z = indoor_floor_level;
                if (indoor_floor_level != -30000) {
                    // break if spanwn point is okay
                    if (std::abs(indoor_floor_level - pParty->pos.z) <= 1024) break;
                }
            }
        }
        failed_point = loop_cnt == 100;
    }

    if (failed_point) {
        return false;
    } else {
        SpawnEncounter(map_info, &enc_spawn_point, 0, 0, 1);
    }

    return enc_index;
}

//----- (00450521) --------------------------------------------------------
int DropTreasureAt(ItemTreasureLevel trs_level, RandomItemType trs_type, Vec3f pos, uint16_t facing) {
    SpriteObject a1;
    pItemTable->generateItem(trs_level, trs_type, &a1.containing_item);
    a1.uType = pItemTable->pItems[a1.containing_item.uItemID].uSpriteID;
    a1.uObjectDescID = pObjectList->ObjectIDByItemID(a1.uType);
    a1.vPosition = pos;
    a1.uFacing = facing;
    a1.uAttributes = 0;
    a1.uSectorID = pIndoor->GetSector(a1.vPosition);
    a1.timeSinceCreated = 0_ticks;
    return a1.Create(0, 0, 0, 0);
}

void SpawnRandomTreasure(MapInfo *mapInfo, SpawnPoint *a2) {
    assert(a2->uKind == OBJECT_Item);

    SpriteObject a1a;
    a1a.containing_item.Reset();

    int v34 = 0;
    int v5 = grng->random(100);
    ItemTreasureLevel v13 = grng->randomSample(RemapTreasureLevel(a2->uItemIndex, mapInfo->mapTreasureLevel));
    if (v13 != ITEM_TREASURE_LEVEL_7) {
        // [0, 20) -- nothing
        // [20, 60) -- gold
        // [60, 100) -- item

        if (v5 < 20)
            return;

        if (v5 >= 60) {
            DropTreasureAt(v13, grng->randomSample(allSpawnableRandomItemTypes()), a2->vPosition, 0);
            return;
        }

        a1a.containing_item.generateGold(a2->uItemIndex);
        a1a.uType = pItemTable->pItems[a1a.containing_item.uItemID].uSpriteID;
        a1a.uObjectDescID = pObjectList->ObjectIDByItemID(a1a.uType);
    } else {
        if (!a1a.containing_item.GenerateArtifact())
            return;
        a1a.uType = pItemTable->pItems[a1a.containing_item.uItemID].uSpriteID;
        a1a.uObjectDescID = pObjectList->ObjectIDByItemID(a1a.uType);
        a1a.containing_item.Reset();  // TODO(captainurist): this needs checking
    }
    a1a.uAttributes = 0;
    a1a.uSoundID = 0;
    a1a.uFacing = 0;
    a1a.vPosition = a2->vPosition;
    a1a.spell_skill = CHARACTER_SKILL_MASTERY_NONE;
    a1a.spell_level = 0;
    a1a.uSpellID = SPELL_NONE;
    a1a.spell_target_pid = Pid();
    a1a.spell_caster_pid = Pid();
    a1a.timeSinceCreated = 0_ticks;
    a1a.uSectorID = pIndoor->GetSector(a2->vPosition);
    a1a.Create(0, 0, 0, 0);
}

//----- (0043F515) --------------------------------------------------------
void FindBillboardsLightLevels_BLV() {
    for (unsigned i = 0; i < uNumBillboardsToDraw; ++i) {
        if (pBillboardRenderList[i].field_1E & 2 ||
            uCurrentlyLoadedLevelType == LEVEL_INDOOR &&
                !pBillboardRenderList[i].uIndoorSectorID)
            pBillboardRenderList[i].dimming_level = 0;
        else
            pBillboardRenderList[i].dimming_level =
                _43F55F_get_billboard_light_level(&pBillboardRenderList[i], -1);
    }
}

float GetIndoorFloorZ(const Vec3f &pos, int *pSectorID, int *pFaceID) {
    if (*pSectorID != 0) {
        int result = BLV_GetFloorLevel(pos, *pSectorID, pFaceID);
        if (result != -30000 && result <= pos.z + 50)
            return result;
    }

    *pSectorID = pIndoor->GetSector(pos);
    if (*pSectorID == 0) {
        if (pFaceID)
            *pFaceID = -1;
        return -30000;
    }

    return BLV_GetFloorLevel(pos, *pSectorID, pFaceID);
}

//----- (0047272C) --------------------------------------------------------
float GetApproximateIndoorFloorZ(const Vec3f &pos, int *pSectorID, int *pFaceID) {
    std::array<Vec3f, 5> attempts = {{
        pos + Vec3f(-2, 0, 40),
        pos + Vec3f(2, 0, 40),
        pos + Vec3f(0, -2, 40),
        pos + Vec3f(0, 2, 40),
        pos + Vec3f(0, 0, 140)
    }};

    float result = -30000;
    for (const Vec3f &attempt : attempts) {
        *pSectorID = 0; // Make sure GetIndoorFloorZ recalculates sector id from provided coordinates.
        result = GetIndoorFloorZ(attempt, pSectorID, pFaceID);
        if (result != -30000)
            return result;
    }
    return result; // Return the last result anyway.
}

