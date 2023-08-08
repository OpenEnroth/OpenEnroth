#include "Engine/Graphics/Indoor.h"

#include <algorithm>
#include <limits>

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
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"
#include "Library/Logger/Logger.h"

#include "Utility/Memory/FreeDeleter.h"
#include "Utility/Math/TrigLut.h"
#include "Utility/Math/FixPoint.h"
#include "Utility/Exception.h"

// TODO(pskelton): make this neater
static DecalBuilder *decal_builder = EngineIocContainer::ResolveDecalBuilder();

IndoorLocation *pIndoor = new IndoorLocation;
BLVRenderParams *pBLVRenderParams = new BLVRenderParams;

static constexpr IndexedArray<uint16_t, MAP_FIRST, MAP_LAST> pDoorSoundIDsByLocationID = {
    {MAP_EMERALD_ISLAND, 300},
    {MAP_HARMONDALE, 300},
    {MAP_ERATHIA, 300},
    {MAP_TULAREAN_FOREST, 300},
    {MAP_DEYJA, 300},
    {MAP_BRACADA_DESERT, 300},
    {MAP_CELESTE, 300},
    {MAP_PIT, 300},
    {MAP_EVENMORN_ISLAND, 300},
    {MAP_MOUNT_NIGHON, 300},
    {MAP_BARROW_DOWNS, 300},
    {MAP_LAND_OF_THE_GIANTS, 300},
    {MAP_TATALIA, 300},
    {MAP_AVLEE, 300},
    {MAP_SHOALS, 300},
    {MAP_DRAGON_CAVES, 404},
    {MAP_LORD_MARKHAMS_MANOR, 302},
    {MAP_BANDIT_CAVES, 306},
    {MAP_HAUNTED_MANSION, 308},
    {MAP_TEMPLE_OF_THE_MOON, 304},
    {MAP_CASTLE_HARMONDALE, 308},
    {MAP_WHITE_CLIFF_CAVE, 302},
    {MAP_ERATHIAN_SEWERS, 400},
    {MAP_FORT_RIVERSTRIDE, 302},
    {MAP_TULAREAN_CAVES, 300},
    {MAP_CLANKERS_LABORATORY, 308},
    {MAP_HALL_OF_THE_PIT, 308},
    {MAP_WATCHTOWER_6, 306},
    {MAP_SCHOOL_OF_SORCERY, 308},
    {MAP_RED_DWARF_MINES, 308},
    {MAP_WALLS_OF_MIST, 304},
    {MAP_TEMPLE_OF_THE_LIGHT, 300},
    {MAP_BREEDING_ZONE, 404},
    {MAP_TEMPLE_OF_THE_DARK, 406},
    {MAP_GRAND_TEMPLE_OF_THE_MOON, 300},
    {MAP_GRAND_TEMPLE_OF_THE_SUN, 400},
    {MAP_THUNDERFIST_MOUNTAIN, 406},
    {MAP_MAZE, 404},
    {MAP_STONE_CITY, 306},
    {MAP_COLONY_ZOD, 302},
    {MAP_MERCENARY_GUILD, 408},
    {MAP_TIDEWATER_CAVERNS, 304},
    {MAP_WINE_CELLAR, 300},
    {MAP_TITANS_STRONGHOLD, 300},
    {MAP_TEMPLE_OF_BAA, 300},
    {MAP_HALL_UNDER_THE_HILL, 300},
    {MAP_LINCOLN, 300},
    {MAP_CASTLE_GRYPHONHEART, 300},
    {MAP_CASTLE_NAVAN, 300},
    {MAP_CASTLE_LAMBENT, 300},
    {MAP_CASTLE_GLOAMING, 300},
    {MAP_DRAGONS_LAIR, 300},
    {MAP_BARROW_VII, 300},
    {MAP_BARROW_IV, 300},
    {MAP_BARROW_II, 300},
    {MAP_BARROW_XIV, 300},
    {MAP_BARROW_III, 300},
    {MAP_BARROW_IX, 300},
    {MAP_BARROW_VI, 300},
    {MAP_BARROW_I, 300},
    {MAP_BARROW_VIII, 300},
    {MAP_BARROW_XIII, 300},
    {MAP_BARROW_X, 404},
    {MAP_BARROW_XII, 304},
    {MAP_BARROW_V, 400},
    {MAP_BARROW_XI, 300},
    {MAP_BARROW_XV, 300},
    {MAP_ZOKARRS_TOMB, 404},
    {MAP_NIGHON_TUNNELS, 304},
    {MAP_TUNNELS_TO_EEOFOL, 400},
    {MAP_WILLIAM_SETAGS_TOWER, 300},
    {MAP_WROMTHRAXS_CAVE, 300},
    {MAP_HIDDEN_TOMB, 404},
    {MAP_STRANGE_TEMPLE, 304},
    {MAP_SMALL_HOUSE, 400},
    {MAP_ARENA, 300}
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

     for (uint i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
         int v7 = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
         v8 = &pIndoor->pSectors[pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i]];

        for (uint j = 0; j < v8->uNumDecorations; ++j)
            pIndoor->PrepareDecorationsRenderList_BLV(v8->pDecorationIDs[j], v7);
     }

    FindBillboardsLightLevels_BLV();
}


//----- (004407D9) --------------------------------------------------------
void BLVRenderParams::Reset() {
    this->uPartySectorID = pIndoor->GetSector(pParty->pos);
    this->uPartyEyeSectorID = pIndoor->GetSector(pParty->pos + Vec3i(0, 0, pParty->eyeLevel));

    if (!this->uPartySectorID) {
        __debugbreak();  // shouldnt happen, please provide savegame
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
        return pTextureFrameTable->GetFrameTexture(
            (int64_t)this->resource, pEventTimer->uTotalTimeElapsed);
    else
        return static_cast<GraphicsImage *>(this->resource);
}

void BLVFace::SetTexture(const std::string &filename) {
    if (this->IsTextureFrameTable()) {
        this->resource =
            (void *)pTextureFrameTable->FindTextureByName(filename);
        if (this->resource != (void *)-1) {
            return;
        }

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
unsigned int IndoorLocation::GetLocationIndex(const std::string &locationName) {
    for (uint i = 0; i < _4E6BDC_loc_names.size(); ++i)
        if (iequals(locationName, _4E6BDC_loc_names[i]))
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
void IndoorLocation::Load(const std::string &filename, int num_days_played, int respawn_interval_days, bool *indoor_was_respawned) {
    decal_builder->Reset(0);

    if (bLoaded)
        Error("BLV is already loaded");

    auto blv_filename = std::string(filename);
    blv_filename.replace(blv_filename.length() - 4, 4, ".blv");

    this->filename = std::string(filename);
    if (!pGames_LOD->exists(blv_filename))
        Error("Unable to find %s in Games.LOD", blv_filename.c_str());

    Release();

    bLoaded = true;

    IndoorLocation_MM7 location;
    deserialize(pGames_LOD->read(blv_filename), &location);
    reconstruct(location, this);

    std::string dlv_filename = filename;
    dlv_filename.replace(dlv_filename.length() - 4, 4, ".dlv");

    bool respawnInitial = false; // Perform initial location respawn?
    bool respawnTimed = false; // Perform timed location respawn?
    IndoorDelta_MM7 delta;
    if (Blob blob = pSave_LOD->read(dlv_filename)) {
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

            if (!respawnInitial && num_days_played - delta.header.info.lastRespawnDay >= respawn_interval_days && pCurrentMapName != "d29.dlv")
                respawnTimed = true;
        } catch (const Exception &e) {
            logger->error("Failed to load '{}', respawning location: {}", dlv_filename, e.what());
            respawnInitial = true;
        }
    }

    assert(respawnInitial + respawnTimed <= 1);

    if (respawnInitial) {
        deserialize(pGames_LOD->read(dlv_filename), &delta, tags::context(location));
        *indoor_was_respawned = true;
    } else if (respawnTimed) {
        auto header = delta.header;
        auto visibleOutlines = delta.visibleOutlines;
        deserialize(pGames_LOD->read(dlv_filename), &delta, tags::context(location));
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
int IndoorLocation::GetSector(int sX, int sY, int sZ) {
    if (uCurrentlyLoadedLevelType != LEVEL_INDOOR)
        return 0;

    if (pSectors.size() < 2) {
        // __debugbreak();
        return 0;
    }

     // holds faces the coords are above
    int FoundFaceStore[5] = { 0 };
    int NumFoundFaceStore = 0;
    int backupboundingsector{ 0 };

    // loop through sectors
    for (uint i = 1; i < pSectors.size(); ++i) {
        if (NumFoundFaceStore >= 5) break;

        BLVSector *pSector = &pSectors[i];

        if (!pSector->pBounding.intersectsCuboid(Vec3i(sX, sY, sZ), Vec3i(5, 5, 64)))
            continue;  // outside sector bounding

        if (!backupboundingsector) backupboundingsector = i;

        int FloorsAndPortals = pSector->uNumFloors + pSector->uNumPortals;

        // nothing in secotr to check against so skip
        if (!FloorsAndPortals) continue;
        if (!pSector->pFloors) continue;

        // loop over check faces
        for (uint z = 0; z < FloorsAndPortals; ++z) {
            int uFaceID;
            if (z < pSector->uNumFloors)
                uFaceID = pSector->pFloors[z];
            else
                uFaceID = pSector->pPortals[z - pSector->uNumFloors];

            BLVFace *pFace = &pFaces[uFaceID];
            if (pFace->uPolygonType != POLYGON_Floor && pFace->uPolygonType != POLYGON_InBetweenFloorAndWall)
                continue;

            // add found faces into store
            if (pFace->Contains(Vec3i(sX, sY, 0), MODEL_INDOOR, engine->config->gameplay.FloorChecksEps.value(), FACE_XY_PLANE))
                FoundFaceStore[NumFoundFaceStore++] = uFaceID;
            if (NumFoundFaceStore >= 5)
                break; // TODO(captainurist): we do get here sometimes (e.g. in dragon cave), increase limit?
        }
    }

    // only one face found
    if (NumFoundFaceStore == 1)
        return this->pFaces[FoundFaceStore[0]].uSectorID;

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
    int pSectorID = 0;
    int MinZDist = 0xFFFFFFu;
    if (NumFoundFaceStore > 0) {
        int CalcZDist = MinZDist;
        for (int s = 0; s < NumFoundFaceStore; ++s) {
            // calc distance between this face and party
            if (this->pFaces[FoundFaceStore[s]].uPolygonType == POLYGON_Floor)
                CalcZDist = std::abs(sZ - this->pVertices[*this->pFaces[FoundFaceStore[s]].pVertexIDs].z);
            if (this->pFaces[FoundFaceStore[s]].uPolygonType == POLYGON_InBetweenFloorAndWall) {
                CalcZDist = std::abs(sZ - this->pFaces[FoundFaceStore[s]].zCalc.calculate(sX, sY));
            }

            // use this face if its smaller than the current min
            // if (CalcZDist >= 0) {
                if (CalcZDist < MinZDist) {
                    pSectorID = this->pFaces[FoundFaceStore[s]].uSectorID;
                    MinZDist = CalcZDist;
                }
           // }
        }

        // doesnt choose - so default to first - SHOULDNT GET HERE
        if (pSectorID == 0) {
            __debugbreak();
            pSectorID = this->pFaces[FoundFaceStore[0]].uSectorID;
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

bool BLVFace::Contains(const Vec3i &pos, int model_idx, int slack, FaceAttributes override_plane) const {
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

    int u;
    int v;
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

        int edge_x = points.u[i] + (points.u[j] - points.u[i]) * (v - points.v[i]) / (points.v[j] - points.v[i]);
        if (u < edge_x)
            inside = !inside;
    }
    return inside;
#endif

    // The polygons we're dealing with are convex, so instead of the usual ray casting algorithm we can simply
    // check that the point in question lies on the same side relative to all of the polygon's edges.
    int sign = 0;
    for (int i = 0, j = this->uNumVertices - 1; i < this->uNumVertices; j = i++) {
        int a_u = points.u[j] - points.u[i];
        int a_v = points.v[j] - points.v[i];
        int b_u = u - points.u[i];
        int b_v = v - points.v[i];
        int cross_product = a_u * b_v - a_v * b_u; // That's |a| * |b| * sin(a,b)
        if (cross_product == 0)
            continue;

        if (slack > 0) {
            // distance(point, line) = (a x b) / |a|,
            // so the condition below just checks that distance is less than slack.
            int64_t a_len_sqr = a_u * a_u + a_v * a_v;
            if (static_cast<int64_t>(cross_product) * cross_product < a_len_sqr * slack * slack)
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
    SoundID eDoorSoundID = (SoundID)300;
    if (dword_6BE13C_uCurrentlyLoadedLocationID != MAP_INVALID)
        eDoorSoundID = (SoundID)pDoorSoundIDsByLocationID[dword_6BE13C_uCurrentlyLoadedLocationID];

    // loop over all doors
    for (uint i = 0; i < pIndoor->pDoors.size(); ++i) {
        BLVDoor *door = &pIndoor->pDoors[i];

        // door not moving currently
        if (door->uState == BLVDoor::Closed || door->uState == BLVDoor::Open) {
            door->uAttributes &= ~DOOR_SETTING_UP;
            continue;
        }
        bool shouldPlaySound = !(door->uAttributes & (DOOR_SETTING_UP | DOOR_NOSOUND)) && door->uNumVertices != 0;

        door->uTimeSinceTriggered += pEventTimer->uTimeElapsed;

        int openDistance;     // [sp+60h] [bp-4h]@6
        if (door->uState == BLVDoor::Opening) {
            openDistance = (door->uTimeSinceTriggered * door->uCloseSpeed) / 128;

            if (openDistance >= door->uMoveLength) {
                openDistance = door->uMoveLength;
                door->uState = BLVDoor::Open;
                if (shouldPlaySound)
                    pAudioPlayer->playSound((SoundID)((int)eDoorSoundID + 1), SOUND_MODE_PID, Pid(OBJECT_Door, i));
            } else if (shouldPlaySound) {
                pAudioPlayer->playSound(eDoorSoundID, SOUND_MODE_PID, Pid(OBJECT_Door, i));
            }
        } else {
            assert(door->uState == BLVDoor::Closing);

            int closeDistance = (door->uTimeSinceTriggered * door->uOpenSpeed) / 128;
            if (closeDistance >= door->uMoveLength) {
                openDistance = 0;
                door->uState = BLVDoor::Closed;
                if (shouldPlaySound)
                    pAudioPlayer->playSound((SoundID)((int)eDoorSoundID + 1), SOUND_MODE_PID, Pid(OBJECT_Door, i));
            } else {
                openDistance = door->uMoveLength - closeDistance;
                if (shouldPlaySound)
                    pAudioPlayer->playSound(eDoorSoundID, SOUND_MODE_PID, Pid(OBJECT_Door, i));
            }
        }

        // adjust verts to how open the door is
        for (int j = 0; j < door->uNumVertices; ++j) {
            pIndoor->pVertices[door->pVertexIDs[j]].x =
                fixpoint_mul(door->vDirection.x, openDistance) + door->pXOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].y =
                fixpoint_mul(door->vDirection.y, openDistance) + door->pYOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].z =
                fixpoint_mul(door->vDirection.z, openDistance) + door->pZOffsets[j];
        }

        for (int j = 0; j < door->uNumFaces; ++j) {
            BLVFace *face = &pIndoor->pFaces[door->pFaceIDs[j]];
            const Vec3s &facePoint = pIndoor->pVertices[face->pVertexIDs[0]];
            face->facePlane.dist = -dot(facePoint.toFloat(), face->facePlane.normal);
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
            for (uint k = 0; k < face->uNumVertices; ++k) {
                Vec3f point = pIndoor->pVertices[face->pVertexIDs[k]].toFloat();
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
                float udot = dot(door->vDirection.toFloatFromFixpoint(), u);
                float vdot = dot(door->vDirection.toFloatFromFixpoint(), v);
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
        int floorZ = GetIndoorFloorZ(actor.pos, &actor.sectorId, &uFaceID);

        if (actor.sectorId == 0 || floorZ <= -30000)
            continue;

        bool isFlying = actor.monsterInfo.uFlying;
        if (!actor.CanAct())
            isFlying = false;

        bool isAboveGround = false;
        if (actor.pos.z > floorZ + 1)
            isAboveGround = true;

        // make bloodsplat when the ground is hit
        if (!actor.donebloodsplat) {
            if (actor.aiState == Dead || actor.aiState == Dying) {
                if (actor.pos.z < floorZ + 30) { // 30 to provide small error / rounding factor
                    if (pMonsterStats->pInfos[actor.monsterInfo.uID].bBloodSplatOnDeath) {
                        if (engine->config->graphics.BloodSplats.value()) {
                            float splatRadius = actor.radius * engine->config->graphics.BloodSplatsMultiplier.value();
                            decal_builder->AddBloodsplat(Vec3f(actor.pos.x, actor.pos.y, floorZ + 30), colorTable.Red, splatRadius);
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

            actor.speed.x = TrigLUT.cos(actor.yawAngle) * moveSpeed;
            actor.speed.y = TrigLUT.sin(actor.yawAngle) * moveSpeed;
            if (isFlying)
                actor.speed.z = TrigLUT.sin(actor.pitchAngle) * moveSpeed;
        } else {
            // actor is not moving
            // fixpoint(55000) = 0.83923339843, appears to be velocity decay.
            actor.speed.x = fixpoint_mul(55000, actor.speed.x);
            actor.speed.y = fixpoint_mul(55000, actor.speed.y);
            if (isFlying)
                actor.speed.z = fixpoint_mul(55000, actor.speed.z);
        }

        if (actor.pos.z <= floorZ) {
            actor.pos.z = floorZ + 1;
            if (pIndoor->pFaces[uFaceID].uPolygonType == POLYGON_Floor) {
                if (actor.speed.z < 0)
                    actor.speed.z = 0;
            } else {
                // fixpoint(45000) = 0.68664550781, no idea what the actual semantics here is.
                if (pIndoor->pFaces[uFaceID].facePlane.normal.z < 0.68664550781f) // was 45000 fixpoint
                    actor.speed.z -= pEventTimer->uTimeElapsed * GetGravityStrength();
            }
        } else {
            if (isAboveGround && !isFlying)
                actor.speed.z += -8 * pEventTimer->uTimeElapsed * GetGravityStrength();
        }

        if (actor.speed.lengthSqr() >= 400) {
            ProcessActorCollisionsBLV(actor, isAboveGround, isFlying);
        } else {
            actor.speed = Vec3s(0, 0, 0);
            if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_SKY) {
                if (actor.aiState == Dead)
                    actor.aiState = Removed;
            }
        }
    }
}

//----- (00460A78) --------------------------------------------------------
void PrepareToLoadBLV(bool bLoading) {
    unsigned int respawn_interval;  // ebx@1
    MapInfo *map_info;              // edi@9
    bool v28;                       // zf@81
    bool alertStatus;                        // [sp+404h] [bp-10h]@1
    bool indoor_was_respawned = true;                      // [sp+40Ch] [bp-8h]@1

    respawn_interval = 0;
    pGameLoadingUI_ProgressBar->Reset(0x20u);
    bNoNPCHiring = false;
    uCurrentlyLoadedLevelType = LEVEL_INDOOR;
    pBLVRenderParams->uPartySectorID = 0;
    pBLVRenderParams->uPartyEyeSectorID = 0;

    engine->SetUnderwater(Is_out15odm_underwater());

    if ((pCurrentMapName == "out15.odm") || (pCurrentMapName == "d23.blv")) {
        bNoNPCHiring = true;
    }
    //pPaletteManager->pPalette_tintColor[0] = 0;
    //pPaletteManager->pPalette_tintColor[1] = 0;
    //pPaletteManager->pPalette_tintColor[2] = 0;
    //pPaletteManager->RecalculateAll();
    pParty->_delayedReactionTimer = 0;
    MAP_TYPE map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id != MAP_INVALID) {
        map_info = &pMapStats->pInfos[map_id];
        respawn_interval = pMapStats->pInfos[map_id].uRespawnIntervalDays;
        alertStatus = GetAlertStatus();
    } else {
        map_info = nullptr;
    }
    dword_6BE13C_uCurrentlyLoadedLocationID = map_id;

    pStationaryLightsStack->uNumLightsActive = 0;
    pIndoor->Load(pCurrentMapName, pParty->GetPlayingTime().GetDays() + 1, respawn_interval, &indoor_was_respawned);
    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)) {
        Actor::InitializeActors();
        SpriteObject::InitializeSpriteObjects();
    }
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN;
    if (map_id == MAP_INVALID)
        indoor_was_respawned = false;

    if (indoor_was_respawned) {
        for (uint i = 0; i < pIndoor->pSpawnPoints.size(); ++i) {
            auto spawn = &pIndoor->pSpawnPoints[i];
            if (spawn->uKind == OBJECT_Actor)
                SpawnEncounter(map_info, spawn, 0, 0, 0);
            else
                SpawnRandomTreasure(map_info, spawn);
        }
        RespawnGlobalDecorations();
    }

    for (uint i = 0; i < pIndoor->pDoors.size(); ++i) {
        if (pIndoor->pDoors[i].uAttributes & DOOR_TRIGGERED) {
            pIndoor->pDoors[i].uState = BLVDoor::Opening;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = DOOR_SETTING_UP;
        }

        if (pIndoor->pDoors[i].uState == BLVDoor::Closed) {
            pIndoor->pDoors[i].uState = BLVDoor::Closing;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = DOOR_SETTING_UP;
        } else if (pIndoor->pDoors[i].uState == BLVDoor::Open) {
            pIndoor->pDoors[i].uState = BLVDoor::Opening;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = DOOR_SETTING_UP;
        }
    }

    /*for (uint i = 0; i < pIndoor->uNumFaces; ++i)
    {
        if (pIndoor->pFaces[i].uBitmapID != -1)
            pBitmaps_LOD->pTextures[pIndoor->pFaces[i].uBitmapID].palette_id2 =
    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[pIndoor->pFaces[i].uBitmapID].palette_id1);
    }*/

    pGameLoadingUI_ProgressBar->Progress();
    decorationsWithSound.clear();

    int interactiveDecorationsNum = 0;
    for (uint i = 0; i < pLevelDecorations.size(); ++i) {
        pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);

        const DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);

        if (decoration->uSoundID) {
            decorationsWithSound.push_back(i);
        }

        if (!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            if (!decoration->DontDraw()) {
                if (decoration->uLightRadius) {
                    Color color = render->config->graphics.ColoredLights.value() ? decoration->uColoredLight : colorTable.White;
                    pStationaryLightsStack->AddLight(pLevelDecorations[i].vPosition.toFloat() +
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
                    pItemTable->pItems[pSpriteObjects[i].containing_item.uItemID].uEquipType == EQUIP_POTION &&
                    !pSpriteObjects[i].containing_item.uEnchantmentType)
                    pSpriteObjects[i].containing_item.uEnchantmentType = grng->random(15) + 5;
                pItemTable->SetSpecialBonus(&pSpriteObjects[i].containing_item);
            }
        }
    }

    // INDOOR initialize actors
    alertStatus = false;

    for (uint i = 0; i < pActors.size(); ++i) {
        if (pActors[i].attributes & ACTOR_UNKNOW7) {
            if (map_id == MAP_INVALID) {
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
            pActors[i].monsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
            if (pActors[i].monsterInfo.field_3E != 11 &&
                pActors[i].monsterInfo.field_3E != 19 &&
                (!pActors[i].currentHP || !pActors[i].monsterInfo.uHP)) {
                pActors[i].monsterInfo.field_3E = 5;
                pActors[i].UpdateAnimation();
            }
        } else {
            pActors[i].monsterInfo.field_3E = 19;
            pActors[i].attributes |= ACTOR_UNKNOW11;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    // Party to start position
    Actor this_;
    this_.monsterInfo.uID = 45;
    this_.PrepareSprites(0);
    if (!bLoading) {
        pParty->_viewPitch = 0;
        pParty->_viewYaw = 0;
        pParty->pos = Vec3i();
        pParty->speed = Vec3i();
        pParty->uFallStartZ = 0;
        TeleportToStartingPoint(uLevel_StartingPointType);
        pBLVRenderParams->Reset();
    }
    viewparams->_443365();
    PlayLevelMusic();

    // Active character speaks.
    if (!bLoading && indoor_was_respawned) {
        int id = pParty->getRandomActiveCharacterId(vrng.get());

        if (id != -1) {
            pParty->setDelayedReaction(SPEECH_ENTER_DUNGEON, id);
        }
    }
}

//----- (0046CEC3) --------------------------------------------------------
int BLV_GetFloorLevel(const Vec3i &pos, int uSectorID, int *pFaceID) {
    // stores faces and floor z levels
    int FacesFound = 0;
    int blv_floor_z[5] = { 0 };
    int blv_floor_id[5] = { 0 };

    BLVSector *pSector = &pIndoor->pSectors[uSectorID];

    // loop over all floor faces
    for (uint i = 0; i < pSector->uNumFloors; ++i) {
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
        int z_calc;
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
        for (uint i = 0; i < pSector->uNumPortals; ++i) {
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
            /*__debugbreak();*/
        }
        return blv_floor_z[0];
    }

    // no face found - probably wrong sector supplied
    if (!FacesFound) {
        logger->verbose("Floorlvl fail: {} {} {}", pos.x, pos.y, pos.z);

        if (pFaceID)
            *pFaceID = -1;
        return -30000;
    }

    // multiple faces found - pick nearest
    int result = blv_floor_z[0];
    int faceId = blv_floor_id[0];
    for (uint i = 1; i < FacesFound; ++i) {
        int v38 = blv_floor_z[i];

        if (std::abs(pos.z - v38) <= std::abs(pos.z - result)) {
            result = blv_floor_z[i];
            if (blv_floor_z[i] <= -29000) __debugbreak();
            faceId = blv_floor_id[i];
        }
    }

    if (result <= -29000) __debugbreak();

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
        particle.timeToLive = vrng->random(0x80) + 128; // was rand() & 0x80
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
    int v37 = pEventTimer->uTotalTimeElapsed;
    if (pParty->bTurnBasedModeOn) v37 = pMiscTimer->uTotalTimeElapsed;
    v10 = std::abs(pLevelDecorations[uDecorationID].vPosition.x +
              pLevelDecorations[uDecorationID].vPosition.y);
    v11 = pSpriteFrameTable->GetFrame(decoration->uSpriteID, v37 + v10);

    // error catching
    if (v11->icon_name == "null") __debugbreak();

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
                        __debugbreak();

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
bool Check_LineOfSight(const Vec3i &target, const Vec3i &from) {  // target from - true on clear
    int AngleToTarget = TrigLUT.atan2(from.x - target.x, from.y - target.y);
    bool LOS_Obscurred = 0;
    bool LOS_Obscurred2 = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        // offset 32 to side and check LOS
        Vec3i targetmod = target + Vec3i::fromPolarRetarded(32, AngleToTarget + TrigLUT.uIntegerHalfPi, 0);
        Vec3i frommod = from + Vec3i::fromPolarRetarded(32, AngleToTarget + TrigLUT.uIntegerHalfPi, 0);
        LOS_Obscurred2 = Check_LOS_Obscurred_Indoors(targetmod, frommod);

        // offset other side and repeat check
        targetmod = target + Vec3i::fromPolarRetarded(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0);
        frommod = from + Vec3i::fromPolarRetarded(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0);
        LOS_Obscurred = Check_LOS_Obscurred_Indoors(targetmod, frommod);
    } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        // TODO(pskelton): Need to add check against terrain
        // offset 32 to side and check LOS
        Vec3i targetmod = target + Vec3i::fromPolarRetarded(32, AngleToTarget + TrigLUT.uIntegerHalfPi, 0);
        Vec3i frommod = from + Vec3i::fromPolarRetarded(32, AngleToTarget + TrigLUT.uIntegerHalfPi, 0);
        LOS_Obscurred2 = Check_LOS_Obscurred_Outdoors_Bmodels(targetmod, frommod);

        // offset other side and repeat check
        targetmod = target + Vec3i::fromPolarRetarded(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0);
        frommod = from + Vec3i::fromPolarRetarded(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0);
        LOS_Obscurred = Check_LOS_Obscurred_Outdoors_Bmodels(targetmod, frommod);
    }

    bool result{ !LOS_Obscurred2 || !LOS_Obscurred };
    return result;  // true if LOS clear
}

bool Check_LOS_Obscurred_Indoors(const Vec3i &target, const Vec3i &from) {  // true if obscurred
    Vec3f dir = (from - target).toFloat();
    dir.normalize();

    BBoxi bbox = BBoxi::forPoints(from, target);

    for (int sectargetrflip = 0; sectargetrflip < 2; sectargetrflip++) {
        int SectargetrID = 0;
        if (sectargetrflip)
            SectargetrID = pIndoor->GetSector(target.x, target.y, target.z);
        else
            SectargetrID = pIndoor->GetSector(from.x, from.y, from.z);

        // loop over sectargetr faces
        for (int FaceLoop = 0; FaceLoop < pIndoor->pSectors[SectargetrID].uNumFaces; ++FaceLoop) {
            BLVFace *face = &pIndoor->pFaces[pIndoor->pSectors[SectargetrID].pFaceIDs[FaceLoop]];
            if (face->isPortal())
                continue;

            // dot product
            float dirDotNormal = dot(dir, face->facePlane.normal);
            bool FaceIsParallel = fuzzyIsNull(dirDotNormal);
            if (FaceIsParallel)
                continue;

            // skip further checks
            if (!bbox.intersects(face->pBounding))
                continue;

            float NegFacePlaceDist = -face->facePlane.signedDistanceTo(target.toFloat());
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
                if (IntersectionDist >= 0) {
                    Vec3i pos = target + (IntersectionDist * dir).toInt();
                    if (face->Contains(pos, MODEL_INDOOR)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Check_LOS_Obscurred_Outdoors_Bmodels(const Vec3i &target, const Vec3i &from) {  // true is obscurred
    Vec3f dir = (from - target).toFloat();
    dir.normalize();

    BBoxi bbox = BBoxi::forPoints(from, target);

    for (BSPModel &model : pOutdoor->pBModels) {
        if (CalcDistPointToLine(target.x, target.y, from.x, from.y, model.vPosition.x, model.vPosition.y) <= model.sBoundingRadius + 128) {
            for (ODMFace &face : model.pFaces) {
                float dirDotNormal = dot(dir, face.facePlane.normal);
                bool FaceIsParallel = fuzzyIsNull(dirDotNormal);
                if (FaceIsParallel)
                    continue;

                // bounds check
                if (!bbox.intersects(face.pBoundingBox))
                    continue;

                // point target plane distacne
                float NegFacePlaceDist = -face.facePlane.signedDistanceTo(target.toFloat());

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
                    if (IntersectionDist >= 0) {
                        Vec3i pos = target + (IntersectionDist * dir).toInt();
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
    unsigned int faceEvent = 0;

    bool party_running_flag = false;
    bool party_walking_flag = false;
    bool not_high_fall = false;
    bool on_water = false;
    bool bFeatherFall;

    int sectorId = pBLVRenderParams->uPartySectorID;
    int faceId = -1;
    int floorZ = GetIndoorFloorZ(pParty->pos + Vec3i(0, 0, 40), &sectorId, &faceId);

    if (pParty->bFlying)  // disable flight
        pParty->bFlying = false;

    if (floorZ == -30000 || faceId == -1) {
        floorZ = GetApproximateIndoorFloorZ(pParty->pos + Vec3i(0, 0, 40), &sectorId, &faceId);
        if (floorZ == -30000 || faceId == -1) {
            __debugbreak();  // level built with errors
            pParty->pos = blv_prev_party_pos;
            pParty->uFallStartZ = blv_prev_party_pos.z;
            return;
        }
    }

    blv_prev_party_pos = pParty->pos;

    int fall_start;
    if (pParty->FeatherFallActive() || pParty->wearsItemAnywhere(ITEM_ARTIFACT_LADYS_ESCORT)
        || pParty->uFlags & (PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING)) {
        fall_start = floorZ;
        bFeatherFall = true;
        pParty->uFallStartZ = floorZ;
    } else {
        bFeatherFall = false;
        fall_start = pParty->uFallStartZ;
    }

    if (fall_start - pParty->pos.z > 512 && !bFeatherFall && pParty->pos.z <= floorZ + 1) {  // fall damage
        if (pParty->uFlags & (PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING)) {
            // flying was previously used to prevent fall damage from jump spell
            pParty->uFlags &= ~(PARTY_FLAGS_1_LANDING | PARTY_FLAGS_1_JUMPING);
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
    int rotation =
        (static_cast<int64_t>(pEventTimer->dt_fixpoint) * pParty->_yawRotationSpeed * TrigLUT.uIntegerPi / 180) >> 16;

    pParty->speed = Vec3i(0, 0, pParty->speed.z);

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
                pParty->speed.x -= TrigLUT.sin(angle) * pParty->walkSpeed * fWalkSpeedMultiplier / 2;
                pParty->speed.y += TrigLUT.cos(angle) * pParty->walkSpeed * fWalkSpeedMultiplier / 2;
                party_walking_flag = true;
                break;

            case PARTY_StrafeRight:
                pParty->speed.y -= TrigLUT.cos(angle) * pParty->walkSpeed * fWalkSpeedMultiplier / 2;
                pParty->speed.x += TrigLUT.sin(angle) * pParty->walkSpeed * fWalkSpeedMultiplier / 2;
                party_walking_flag = true;
                break;

            case PARTY_WalkForward:
                pParty->speed.x += TrigLUT.cos(angle) * pParty->walkSpeed * fWalkSpeedMultiplier;
                pParty->speed.y += TrigLUT.sin(angle) * pParty->walkSpeed * fWalkSpeedMultiplier;
                party_walking_flag = true;
                break;

            case PARTY_WalkBackward:
                pParty->speed.x -= TrigLUT.cos(angle) * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                pParty->speed.y -= TrigLUT.sin(angle) * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                party_walking_flag = true;
                break;

            case PARTY_RunForward:
                pParty->speed.x += TrigLUT.cos(angle) * 2 * pParty->walkSpeed * fWalkSpeedMultiplier;
                pParty->speed.y += TrigLUT.sin(angle) * 2 * pParty->walkSpeed * fWalkSpeedMultiplier;
                party_running_flag = true;
                break;

            case PARTY_RunBackward:
                pParty->speed.x -= TrigLUT.cos(angle) * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
                pParty->speed.y -= TrigLUT.sin(angle) * pParty->walkSpeed * fBackwardWalkSpeedMultiplier;
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
                if ((!isAboveGround || pParty->pos.z <= floorZ + 6 && pParty->speed.z <= 0) && pParty->jump_strength) {
                    isAboveGround = true;
                    pParty->speed.z += pParty->jump_strength * 96;
                }
                break;
            default:
                break;
        }
    }

    if (isAboveGround) {
        pParty->speed.z += -2 * pEventTimer->uTimeElapsed * GetGravityStrength();
        if (pParty->speed.z < -500) {
            for (Character &character : pParty->pCharacters) {
                if (!character.HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) &&
                    !character.WearsItem(ITEM_ARTIFACT_HERMES_SANDALS, ITEM_SLOT_BOOTS)) {  // was 8
                    character.playEmotion(CHARACTER_EXPRESSION_SCARED, 0);
                }
            }
        }
    } else {
        if (pIndoor->pFaces[faceId].facePlane.normal.z < 0.5) {
            pParty->speed.z -= pEventTimer->uTimeElapsed * GetGravityStrength();
        } else {
            if (!(pParty->uFlags & PARTY_FLAGS_1_LANDING))
                pParty->speed.z = 0;
        }
    }

    if (!isAboveGround || pParty->speed.z > 0)
        pParty->uFallStartZ = pParty->pos.z;

    // If party movement delta is lower then this number then the party remains stationary.
    int64_t elapsed_time_bounded = std::min(pEventTimer->uTimeElapsed, 10000);
    int min_party_move_delta_sqr = 400 * elapsed_time_bounded * elapsed_time_bounded / 8;

    if (pParty->speed.xy().lengthSqr() < min_party_move_delta_sqr) {
        pParty->speed.x = 0;
        pParty->speed.y = 0;
    }

    Vec3i oldPos = pParty->pos;

    collision_state.ignored_face_id = -1;
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = pParty->radius;
    collision_state.radius_hi = pParty->radius / 2;
    collision_state.check_hi = true;
    for (uint i = 0; i < 100; i++) {
        collision_state.position_hi = pParty->pos.toFloat() + Vec3f(0, 0, pParty->height - 32 + 1);
        collision_state.position_lo = pParty->pos.toFloat() + Vec3f(0, 0, collision_state.radius_lo + 1);
        collision_state.velocity = pParty->speed.toFloat();

        collision_state.uSectorID = sectorId;
        int dt = 0; // zero means use actual dt
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
            dt = 13312; // fixpoint(13312) = 0.203125

        if (collision_state.PrepareAndCheckIfStationary(dt))
            break;

        for (uint j = 0; j < 100; ++j) {
            CollideIndoorWithGeometry(true);
            CollideIndoorWithDecorations();
            for (int k = 0; k < pActors.size(); ++k)
                CollideWithActor(k, 0);
            if (CollideIndoorWithPortals())
                break; // No portal collisions => can break.
        }

        Vec3i adjusted_pos = pParty->pos + (collision_state.adjusted_move_distance * collision_state.direction).toInt();
        int adjusted_floor_z = GetIndoorFloorZ(adjusted_pos + Vec3i(0, 0, 40), &collision_state.uSectorID, &faceId);
        if (adjusted_floor_z == -30000 || adjusted_floor_z - pParty->pos.z > 128)
            return; // TODO: whaaa?

        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            pParty->pos = (collision_state.new_position_lo - Vec3f(0, 0, collision_state.radius_lo + 1)).toIntTrunc();
            break; // And we're done with collisions.
        }

        collision_state.total_move_distance += collision_state.adjusted_move_distance;

        pParty->pos.x += collision_state.adjusted_move_distance * collision_state.direction.x;
        pParty->pos.y += collision_state.adjusted_move_distance * collision_state.direction.y;
        int new_party_z_tmp = pParty->pos.z +
            collision_state.adjusted_move_distance * collision_state.direction.z;

        if (collision_state.pid.type() == OBJECT_Actor) {
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset(); // Break invisibility when running into a monster.
        }

        if (collision_state.pid.type() == OBJECT_Decoration) {
            // Bounce back from a decoration & do another round of collision checks.
            // This way the party can "slide" along & past a decoration.
            int angle = TrigLUT.atan2(pParty->pos.x - pLevelDecorations[collision_state.pid.id()].vPosition.x,
                                      pParty->pos.y - pLevelDecorations[collision_state.pid.id()].vPosition.y);
            int len = integer_sqrt(pParty->speed.xy().lengthSqr());
            pParty->speed.x = TrigLUT.cos(angle) * len;
            pParty->speed.y = TrigLUT.sin(angle) * len;
        }

        if (collision_state.pid.type() == OBJECT_Face) {
            BLVFace *pFace = &pIndoor->pFaces[collision_state.pid.id()];
            if (pFace->uPolygonType == POLYGON_Floor) {
                if (pParty->speed.z < 0)
                    pParty->speed.z = 0;
                new_party_z_tmp = pIndoor->pVertices[*pFace->pVertexIDs].z + 1;
                if (pParty->uFallStartZ - new_party_z_tmp < 512)
                    pParty->uFallStartZ = new_party_z_tmp;
                if (pParty->speed.xy().lengthSqr() < min_party_move_delta_sqr) {
                    pParty->speed.y = 0;
                    pParty->speed.x = 0;
                }
                if (pParty->floor_face_id != collision_state.pid.id() && pFace->Pressure_Plate())
                    faceEvent = pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
            } else { // Not floor
                int speed_dot_normal = std::abs(
                    pParty->speed.x * pFace->facePlane.normal.x +
                    pParty->speed.y * pFace->facePlane.normal.y +
                    pParty->speed.z * pFace->facePlane.normal.z);

                if ((collision_state.speed / 8) > speed_dot_normal)
                    speed_dot_normal = collision_state.speed / 8;

                pParty->speed.x += speed_dot_normal * pFace->facePlane.normal.x;
                pParty->speed.y += speed_dot_normal * pFace->facePlane.normal.y;
                pParty->speed.z += speed_dot_normal * pFace->facePlane.normal.z;

                if (pFace->uPolygonType != POLYGON_InBetweenFloorAndWall) { // wall / ceiling
                    int distance_to_face = pFace->facePlane.signedDistanceTo(Vec3f(pParty->pos.x, pParty->pos.y, new_party_z_tmp)) -
                                           collision_state.radius_lo;
                    if (distance_to_face < 0) {
                        // We're too close to the face, push back.
                        pParty->pos.x += -distance_to_face * pFace->facePlane.normal.x;
                        pParty->pos.y += -distance_to_face * pFace->facePlane.normal.y;
                        new_party_z_tmp += -distance_to_face * pFace->facePlane.normal.z;
                    }
                    if (pParty->floor_face_id != collision_state.pid.id() && pFace->Pressure_Plate())
                        faceEvent = pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
                } else { // between floor & wall
                    if (pParty->speed.xy().lengthSqr() >= min_party_move_delta_sqr) {
                        if (pParty->floor_face_id != collision_state.pid.id() && pFace->Pressure_Plate())
                            faceEvent = pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
                    } else {
                        pParty->speed = Vec3i();
                    }
                }
            }
        }

        pParty->speed.x = fixpoint_mul(58500, pParty->speed.x);  // 58500 is roughly 0.89
        pParty->speed.y = fixpoint_mul(58500, pParty->speed.y);
        pParty->speed.z = fixpoint_mul(58500, pParty->speed.z);
    }

    // walking / running sounds ------------------------
    if (engine->config->settings.WalkSound.value()) {
        bool canStartNewSound = !pAudioPlayer->isWalkingSoundPlays();

        // Start sound processing only when actual movement is performed to avoid stopping sounds on high FPS
        if (pEventTimer->uTimeElapsed) {
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
                    SoundID sound = SOUND_Invalid;
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

    pParty->uFlags &= ~(PARTY_FLAGS_1_BURNING | PARTY_FLAGS_1_WATER_DAMAGE);
    pParty->_viewYaw = angle;
    pParty->_viewPitch = vertical_angle;

    if (!isAboveGround && pIndoor->pFaces[faceId].uAttributes & FACE_IsLava)
        pParty->uFlags |= PARTY_FLAGS_1_BURNING;

    if (faceEvent)
        eventProcessor(faceEvent, Pid(), 1);
}

void switchDoorAnimation(unsigned int uDoorID, int a2) {
    BLVDoor::State old_state;       // eax@1
    signed int door_id;  // esi@2

    if (pIndoor->pDoors.empty()) return;
    for (door_id = 0; door_id < 200; ++door_id) {
        if (pIndoor->pDoors[door_id].uDoorID == uDoorID) break;
    }
    if (door_id >= 200) {
        Error("Unable to find Door ID: %i!", uDoorID);
    }
    old_state = pIndoor->pDoors[door_id].uState;
    // old_state: 0 - в нижнем положении/закрыто
    //           2 - в верхнем положении/открыто,
    // a2: 1 - открыть
    //    2 - опустить/поднять
    if (a2 == 2) {
        if (pIndoor->pDoors[door_id].uState == BLVDoor::Closing ||
            pIndoor->pDoors[door_id].uState == BLVDoor::Opening)
            return;
        if (pIndoor->pDoors[door_id].uState != BLVDoor::Closed) {
            if (pIndoor->pDoors[door_id].uState != BLVDoor::Closed &&
                pIndoor->pDoors[door_id].uState != BLVDoor::Closing) {
                pIndoor->pDoors[door_id].uState = BLVDoor::Closing;
                if (old_state == BLVDoor::Open) {
                    pIndoor->pDoors[door_id].uTimeSinceTriggered = 0;
                    return;
                }
                if (pIndoor->pDoors[door_id].uTimeSinceTriggered != 15360) {
                    pIndoor->pDoors[door_id].uTimeSinceTriggered =
                        (pIndoor->pDoors[door_id].uMoveLength << 7) /
                            pIndoor->pDoors[door_id].uOpenSpeed -
                        ((signed int)(pIndoor->pDoors[door_id]
                                          .uTimeSinceTriggered *
                                      pIndoor->pDoors[door_id].uCloseSpeed) /
                             128
                         << 7) /
                            pIndoor->pDoors[door_id].uOpenSpeed;
                    return;
                }
                pIndoor->pDoors[door_id].uTimeSinceTriggered = 15360;
            }
            return;
        }
    } else {
        if (a2 == 0) {
            if (pIndoor->pDoors[door_id].uState != BLVDoor::Closed &&
                pIndoor->pDoors[door_id].uState != BLVDoor::Closing) {
                pIndoor->pDoors[door_id].uState = BLVDoor::Closing;
                if (old_state == BLVDoor::Open) {
                    pIndoor->pDoors[door_id].uTimeSinceTriggered = 0;
                    return;
                }
                if (pIndoor->pDoors[door_id].uTimeSinceTriggered != 15360) {
                    pIndoor->pDoors[door_id].uTimeSinceTriggered =
                        (pIndoor->pDoors[door_id].uMoveLength << 7) /
                            pIndoor->pDoors[door_id].uOpenSpeed -
                        ((signed int)(pIndoor->pDoors[door_id]
                                          .uTimeSinceTriggered *
                                      pIndoor->pDoors[door_id].uCloseSpeed) /
                             128
                         << 7) /
                            pIndoor->pDoors[door_id].uOpenSpeed;
                    return;
                }
                pIndoor->pDoors[door_id].uTimeSinceTriggered = 15360;
            }
            return;
        }
        if (a2 != 1) return;
    }
    if (old_state != BLVDoor::Open && old_state != BLVDoor::Opening) {
        pIndoor->pDoors[door_id].uState = BLVDoor::Opening;
        if (old_state == BLVDoor::Closed) {
            pIndoor->pDoors[door_id].uTimeSinceTriggered = 0;
            return;
        }
        if (pIndoor->pDoors[door_id].uTimeSinceTriggered != 15360) {
            pIndoor->pDoors[door_id].uTimeSinceTriggered =
                (pIndoor->pDoors[door_id].uMoveLength << 7) /
                    pIndoor->pDoors[door_id].uCloseSpeed -
                ((signed int)(pIndoor->pDoors[door_id].uTimeSinceTriggered *
                              pIndoor->pDoors[door_id].uOpenSpeed) /
                     128
                 << 7) /
                    pIndoor->pDoors[door_id].uCloseSpeed;
            return;
        }
        pIndoor->pDoors[door_id].uTimeSinceTriggered = 15360;
    }
    return;
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
    uint loop_cnt = 0;

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
            mon_sectorID = pIndoor->GetSector(enc_spawn_point.vPosition.x, enc_spawn_point.vPosition.y, pParty->pos.z);
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
int DropTreasureAt(ITEM_TREASURE_LEVEL trs_level, int trs_type, int x, int y, int z, uint16_t facing) {
    SpriteObject a1;
    pItemTable->generateItem(trs_level, trs_type, &a1.containing_item);
    a1.uType = (SPRITE_OBJECT_TYPE)pItemTable->pItems[a1.containing_item.uItemID].uSpriteID;
    a1.uObjectDescID = pObjectList->ObjectIDByItemID(a1.uType);
    a1.vPosition.x = x;
    a1.vPosition.y = y;
    a1.vPosition.z = z;
    a1.uFacing = facing;
    a1.uAttributes = 0;
    a1.uSectorID = pIndoor->GetSector(x, y, z);
    a1.uSpriteFrameID = 0;
    return a1.Create(0, 0, 0, 0);
}

void SpawnRandomTreasure(MapInfo *mapInfo, SpawnPoint *a2) {
    assert(a2->uKind == OBJECT_Item);

    SpriteObject a1a;
    a1a.containing_item.Reset();

    int v34 = 0;
    int v5 = grng->random(100);
    ITEM_TREASURE_LEVEL v13 = grng->randomSample(RemapTreasureLevel(a2->uItemIndex, mapInfo->Treasure_prob));
    if (v13 != ITEM_TREASURE_LEVEL_GUARANTEED_ARTIFACT) {
        // [0, 20) -- nothing
        // [20, 60) -- gold
        // [60, 100) -- item

        if (v5 < 20)
            return;

        if (v5 >= 60) {
            DropTreasureAt(v13, grng->random(27) + 20, a2->vPosition.x,
                           a2->vPosition.y,
                           a2->vPosition.z, 0);
            return;
        }

        if (a2->uItemIndex == ITEM_TREASURE_LEVEL_1) {
            a1a.containing_item.uItemID = ITEM_GOLD_SMALL;
            v34 = grng->random(51) + 50;
        } else if (a2->uItemIndex == ITEM_TREASURE_LEVEL_2) {
            a1a.containing_item.uItemID = ITEM_GOLD_SMALL;
            v34 = grng->random(101) + 100;
        } else if (a2->uItemIndex == ITEM_TREASURE_LEVEL_3) {
            a1a.containing_item.uItemID = ITEM_GOLD_MEDIUM;
            v34 = grng->random(301) + 200;
        } else if (a2->uItemIndex == ITEM_TREASURE_LEVEL_4) {
            a1a.containing_item.uItemID = ITEM_GOLD_MEDIUM;
            v34 = grng->random(501) + 500;
        } else if (a2->uItemIndex == ITEM_TREASURE_LEVEL_5) {
            a1a.containing_item.uItemID = ITEM_GOLD_LARGE;
            v34 = grng->random(1001) + 1000;
        } else if (a2->uItemIndex == ITEM_TREASURE_LEVEL_6) {
            a1a.containing_item.uItemID = ITEM_GOLD_LARGE;
            v34 = grng->random(3001) + 2000;
        }
        a1a.uType = (SPRITE_OBJECT_TYPE)pItemTable->pItems[a1a.containing_item.uItemID].uSpriteID;
        a1a.containing_item.SetIdentified();
        a1a.uObjectDescID = pObjectList->ObjectIDByItemID(a1a.uType);
        a1a.containing_item.special_enchantment = (ITEM_ENCHANTMENT)v34;
    } else {
        if (!a1a.containing_item.GenerateArtifact())
            return;
        a1a.uType = (SPRITE_OBJECT_TYPE)pItemTable->pItems[a1a.containing_item.uItemID].uSpriteID;
        a1a.uObjectDescID = pObjectList->ObjectIDByItemID(a1a.uType);
        a1a.containing_item.Reset();  // TODO(captainurist): this needs checking
    }
    a1a.vPosition.y = a2->vPosition.y;
    a1a.uAttributes = 0;
    a1a.uSoundID = 0;
    a1a.uFacing = 0;
    a1a.vPosition.z = a2->vPosition.z;
    a1a.vPosition.x = a2->vPosition.x;
    a1a.spell_skill = CHARACTER_SKILL_MASTERY_NONE;
    a1a.spell_level = 0;
    a1a.uSpellID = SPELL_NONE;
    a1a.spell_target_pid = Pid();
    a1a.spell_caster_pid = Pid();
    a1a.uSpriteFrameID = 0;
    a1a.uSectorID = pIndoor->GetSector(a2->vPosition.x, a2->vPosition.y, a2->vPosition.z);
    a1a.Create(0, 0, 0, 0);
}

//----- (0043F515) --------------------------------------------------------
void FindBillboardsLightLevels_BLV() {
    for (uint i = 0; i < uNumBillboardsToDraw; ++i) {
        if (pBillboardRenderList[i].field_1E & 2 ||
            uCurrentlyLoadedLevelType == LEVEL_INDOOR &&
                !pBillboardRenderList[i].uIndoorSectorID)
            pBillboardRenderList[i].dimming_level = 0;
        else
            pBillboardRenderList[i].dimming_level =
                _43F55F_get_billboard_light_level(&pBillboardRenderList[i], -1);
    }
}

int GetIndoorFloorZ(const Vec3i &pos, int *pSectorID, int *pFaceID) {
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
int GetApproximateIndoorFloorZ(const Vec3i &pos, int *pSectorID, int *pFaceID) {
    std::array<Vec3i, 5> attempts = {{
        pos + Vec3i(-2, 0, 40),
        pos + Vec3i(2, 0, 40),
        pos + Vec3i(0, -2, 40),
        pos + Vec3i(0, 2, 40),
        pos + Vec3i(0, 0, 140)
    }};

    int result;
    for (const Vec3i &attempt : attempts) {
        *pSectorID = 0; // Make sure GetIndoorFloorZ recalculates sector id from provided coordinates.
        result = GetIndoorFloorZ(attempt, pSectorID, pFaceID);
        if (result != -30000)
            return result;
    }
    return result; // Return the last result anyway.
}

