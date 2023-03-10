#include "Engine/Graphics/Indoor.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Events.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Collisions.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/PortalFunctions.h"
#include "Engine/Graphics/ClippingFunctions.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Serialization/LegacyImages.h"
#include "Engine/Serialization/Deserializer.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/stru123.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"


#include "Utility/Memory/FreeDeleter.h"
#include "Utility/Math/TrigLut.h"
#include "Library/Random/Random.h"

// TODO(pskelton): make this neater
static DecalBuilder* decal_builder = EngineIoc::ResolveDecalBuilder();

IndoorLocation *pIndoor = new IndoorLocation;
BLVRenderParams *pBLVRenderParams = new BLVRenderParams;

LEVEL_TYPE uCurrentlyLoadedLevelType = LEVEL_null;

LightsData Lights;
stru337_unused _DLV_header_unused;
// std::array<stru352, 480> stru_F83B80;

uint16_t pDoorSoundIDsByLocationID[78] = {
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
    300, 300, 300, 404, 302, 306, 308, 304, 308, 302, 400, 302, 300,
    308, 308, 306, 308, 308, 304, 300, 404, 406, 300, 400, 406, 404,
    306, 302, 408, 304, 300, 300, 300, 300, 300, 300, 300, 300, 300,
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 404, 304,
    400, 300, 300, 404, 304, 400, 300, 300, 404, 304, 400, 300, 300};

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
    this->field_0_timer_ = pEventTimer->uTotalGameTimeElapsed;

    this->uPartySectorID = pIndoor->GetSector(pParty->vPosition);
    this->uPartyEyeSectorID = pIndoor->GetSector(pParty->vPosition + Vec3i(0, 0, pParty->sEyelevel));

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

    pParty->uFlags &= ~PARTY_FLAGS_1_ForceRedraw;
    engine->DrawParticles();
    trail_particle_generator.UpdateParticles();
}

//----- (004C0EF2) --------------------------------------------------------
void BLVFace::FromODM(ODMFace *face) {
    this->pFacePlane_old = face->pFacePlaneOLD;
    this->pFacePlane = face->pFacePlane;
    this->uAttributes = face->uAttributes;
    this->pBounding = face->pBoundingBox;
    this->zCalc = face->zCalc;
    this->pXInterceptDisplacements = face->pXInterceptDisplacements.data();
    this->pYInterceptDisplacements = face->pYInterceptDisplacements.data();
    this->pZInterceptDisplacements = face->pZInterceptDisplacements.data();
    this->uPolygonType = (PolygonType)face->uPolygonType;
    this->uNumVertices = face->uNumVertices;
    this->resource = face->resource;
    this->pVertexIDs = face->pVertexIDs.data();
}

//----- (004B0E07) --------------------------------------------------------
unsigned int FaceFlowTextureOffset(unsigned int uFaceID) {  // time texture offset
    Lights.pDeltaUV[0] = pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID].sTextureDeltaU;
    Lights.pDeltaUV[1] = pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID].sTextureDeltaV;

    unsigned int offset = platform->tickCount() >> 3;

    if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FlowDown) {
        Lights.pDeltaUV[1] -= offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetHeight() - 1);
    } else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FlowUp) {
        Lights.pDeltaUV[1] += offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetHeight() - 1);
    }

    if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FlowRight) {
        Lights.pDeltaUV[0] -= offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetWidth() - 1);
    } else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FlowLeft) {
        Lights.pDeltaUV[0] += offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetWidth() - 1);
    }

    return offset;
}

//----- (004AE5BA) --------------------------------------------------------
Texture *BLVFace::GetTexture() {
    if (this->IsTextureFrameTable())
        return pTextureFrameTable->GetFrameTexture(
            (int64_t)this->resource, pBLVRenderParams->field_0_timer_);
    else
        return (Texture *)this->resource;
}

void BLVFace::SetTexture(const std::string &filename) {
    if (this->IsTextureFrameTable()) {
        this->resource =
            (void *)pTextureFrameTable->FindTextureByName(filename.c_str());
        if (this->resource != (void *)-1) {
            return;
        }

        this->ToggleIsTextureFrameTable();
    }

    this->resource = assets->GetBitmap(filename);
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
unsigned int IndoorLocation::GetLocationIndex(const char *Str1) {
    for (uint i = 0; i < 11; ++i)
        if (iequals(Str1, _4E6BDC_loc_names[i]))
            return i + 1;
    return 0;
}

//----- (004488F7) --------------------------------------------------------
void IndoorLocation::ToggleLight(signed int sLightID, unsigned int bToggle) {
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor &&
        (sLightID <= pIndoor->pLights.size() - 1) && (sLightID >= 0)) {
        if (bToggle)
            pIndoor->pLights[sLightID].uAtributes &= 0xFFFFFFF7;
        else
            pIndoor->pLights[sLightID].uAtributes |= 8;
        pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
    }
}

//----- (00498E0A) --------------------------------------------------------
bool IndoorLocation::Load(const std::string &filename, int num_days_played,
                          int respawn_interval_days, char *pDest) {
    decal_builder->Reset(0);

    _6807E0_num_decorations_with_sounds_6807B8 = 0;

    if (bLoaded) {
        log->Warning("BLV is already loaded");
        return true;
    }

    auto blv_filename = std::string(filename);
    blv_filename.replace(blv_filename.length() - 4, 4, ".blv");

    this->filename = std::string(filename);
    if (!pGames_LOD->DoesContainerExist(blv_filename)) {
        Error("Unable to find %s in Games.LOD", blv_filename.c_str());
    }

    Release();

    BlobDeserializer stream(pGames_LOD->LoadCompressed(blv_filename));

    bLoaded = true;

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadRaw(&blv);
    stream.ReadVector(&pVertices);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadLegacyVector<BLVFace_MM7>(&pFaces);
    stream.ReadSizedVector(&pLFaces, blv.uFaces_fdata_Size / sizeof(uint16_t));

    for (uint i = 0, j = 0; i < pFaces.size(); ++i) {
        BLVFace *pFace = &pFaces[i];

        pFace->pVertexIDs = &pLFaces[j];

        j += pFace->uNumVertices + 1;
        pFace->pXInterceptDisplacements = (int16_t *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pYInterceptDisplacements = (int16_t *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pZInterceptDisplacements = (int16_t *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pVertexUIDs = (int16_t *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pVertexVIDs = (int16_t *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
    }

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < pFaces.size(); ++i) {
        BLVFace *pFace = &pFaces[i];

        std::string texName;
        stream.ReadSizedString(&texName, 10);
        pFace->SetTexture(texName);
    }

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadVector(&pFaceExtras);

    pGameLoadingUI_ProgressBar->Progress();

    // v108 = (char *)v107 + 36 * uNumFaceExtras;
    // v245 = 0;
    // *(int *)((char *)&uSourceLen + 1) = 0;
    for (uint i = 0; i < pFaceExtras.size(); ++i) {
        std::string texName;
        stream.ReadSizedString(&texName, 10);

        if (texName.empty())
            pFaceExtras[i].uAdditionalBitmapID = -1;
        else
            pFaceExtras[i].uAdditionalBitmapID = pBitmaps_LOD->LoadTexture(texName);
    }

    for (uint i = 0; i < pFaces.size(); ++i) {
        BLVFace *pFace = &pFaces[i];
        BLVFaceExtra *pFaceExtra = &pFaceExtras[pFace->uFaceExtraID];

        if (pFaceExtra->uEventID) {
            if (pFaceExtra->HasEventHint())
                pFace->uAttributes |= FACE_HAS_EVENT;
            else
                pFace->uAttributes &= ~FACE_HAS_EVENT;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadLegacyVector<BLVSector_MM7>(&pSectors);

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadSizedVector(&ptr_0002B0_sector_rdata, blv.uSector_rdata_Size / sizeof(uint16_t));
    ptr_0002B0_sector_rdata.push_back(0); // make the element past the end addressable.

    for (uint i = 0, j = 0; i < pSectors.size(); ++i) {
        BLVSector *pSector = &pSectors[i];

        pSector->pFloors = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumFloors;

        pSector->pWalls = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumWalls;

        pSector->pCeilings = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumCeilings;

        pSector->pFluids = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumFluids;

        pSector->pPortals = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumPortals;

        pSector->pFaceIDs = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumFaces;

        pSector->pCogs = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumCogs;

        pSector->pDecorationIDs = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumDecorations;

        pSector->pMarkers = &ptr_0002B0_sector_rdata[j];
        j += pSector->uNumMarkers;
    }

    stream.ReadSizedVector(&ptr_0002B8_sector_lrdata, blv.uSector_lrdata_Size / sizeof(uint16_t));
    ptr_0002B8_sector_lrdata.push_back(0); // make the element past the end addressable.

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0, j = 0; i < pSectors.size(); ++i) {
        pSectors[i].pLights = &ptr_0002B8_sector_lrdata[j];
        j += pSectors[i].uNumLights;
    }

    pGameLoadingUI_ProgressBar->Progress();

    uint32_t uNumDoors;
    stream.ReadRaw(&uNumDoors);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadVector(&pLevelDecorations);

    for (uint i = 0; i < pLevelDecorations.size(); ++i) {
        std::string name;
        stream.ReadSizedString(&name, 32);

        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(name);
    }

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadVector(&pLights);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadVector(&pNodes);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadLegacyVector<SpawnPoint_MM7>(&pSpawnPoints);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadVector(&pMapOutlines);

    std::string dlv_filename = std::string(filename);
    dlv_filename.replace(dlv_filename.length() - 4, 4, ".dlv");

    bool bResetSpawn = false;
    Blob blob = pNew_LOD->LoadCompressed(dlv_filename);
    if (blob) {
        stream.Reset(blob);
        stream.ReadRaw(&dlv);
    } else {
        bResetSpawn = true;
    }

    if (dlv.uNumFacesInBModels > 0) {
        if (dlv.uNumDecorations > 0) {
            if (dlv.uNumFacesInBModels != pFaces.size() ||
                dlv.uNumDecorations != pLevelDecorations.size())
                bResetSpawn = true;
        }
    }

    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN) {
        respawn_interval_days = 0x1BAF800;
    }

    bool bRespawnLocation = false;
    if (num_days_played - dlv.uLastRepawnDay >= respawn_interval_days &&
        (pCurrentMapName != "d29.dlv")) {
        bRespawnLocation = true;
    }

    std::array<char, 875> SavedOutlines = {{}};
    if (bResetSpawn || (bRespawnLocation || !dlv.uLastRepawnDay)) {
        if (bResetSpawn) {
            // do nothing, SavedOutlines are already filled with zeros.
        } else if (bRespawnLocation || !dlv.uLastRepawnDay) {
            stream.ReadRaw(&SavedOutlines);
        }

        dlv.uLastRepawnDay = num_days_played;
        if (!bResetSpawn) ++dlv.uNumRespawns;
        *(int *)pDest = 1;

        stream.Reset(pGames_LOD->LoadCompressed(dlv_filename));
        stream.SkipBytes(sizeof(DDM_DLV_Header));
    } else {
        *(int*)pDest = 0;
    }

    stream.ReadRaw(&_visible_outlines);

    if (*(int *)pDest)
        _visible_outlines = SavedOutlines;

    for (uint i = 0; i < pMapOutlines.size(); ++i) {
        BLVMapOutline *pVertex = &pMapOutlines[i];
        if ((uint8_t)(1 << (7 - i % 8)) & _visible_outlines[i / 8])
            pVertex->uFlags |= 1;
    }

    for (uint i = 0; i < pFaces.size(); ++i) {
        BLVFace *pFace = &pFaces[i];
        BLVFaceExtra *pFaceExtra = &pFaceExtras[pFace->uFaceExtraID];

        stream.ReadRaw(&pFace->uAttributes);

        if (pFaceExtra->uEventID) {
            if (pFaceExtra->HasEventHint())
                pFace->uAttributes |= FACE_HAS_EVENT;
            else
                pFace->uAttributes &= ~FACE_HAS_EVENT;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < pLevelDecorations.size(); ++i)
        stream.ReadRaw(&pLevelDecorations[i].uFlags);

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadLegacyVector<Actor_MM7>(&pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadLegacyVector<SpriteObject_MM7>(&pSpriteObjects);

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].containing_item.uItemID != ITEM_NULL && !(pSpriteObjects[i].uAttributes & SPRITE_MISSILE)) {
            pSpriteObjects[i].uType = (SPRITE_OBJECT_TYPE)pItemTable->pItems[pSpriteObjects[i].containing_item.uItemID].uSpriteID;
            pSpriteObjects[i].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[i].uType);
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadVector(&vChests);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadSizedLegacyVector<BLVDoor_MM7>(&pDoors, uNumDoors);

    // v201 = (const char *)blv.uDoors_ddata_Size;
    // v200 = (size_t)ptr_0002B4_doors_ddata;
    // v170 = malloc(ptr_0002B4_doors_ddata, blv.uDoors_ddata_Size, "L.DData");
    // v171 = blv.uDoors_ddata_Size;
    stream.ReadSizedVector(&ptr_0002B4_doors_ddata, blv.uDoors_ddata_Size / sizeof(uint16_t));
    ptr_0002B4_doors_ddata.push_back(0); // make the element past the end addressable.

    // Src = (BLVFace *)((char *)Src + v171);
    // v172 = 0;
    // v245 = 0;
    // if (uNumDoors > 0)
    for (uint i = 0, j = 0; i < uNumDoors; ++i) {
        BLVDoor *pDoor = &pDoors[i];

        pDoor->pVertexIDs = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumVertices;

        pDoor->pFaceIDs = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumFaces;

        pDoor->pSectorIDs = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumSectors;

        pDoor->pDeltaUs = (int16_t *)(&ptr_0002B4_doors_ddata[j]);
        j += pDoor->uNumFaces;

        pDoor->pDeltaVs = (int16_t *)(&ptr_0002B4_doors_ddata[j]);
        j += pDoor->uNumFaces;

        pDoor->pXOffsets = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumOffsets;

        pDoor->pYOffsets = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumOffsets;

        pDoor->pZOffsets = &ptr_0002B4_doors_ddata[j];
        j += pDoor->uNumOffsets;
    }
    // v190 = 0;
    // v245 = 0;
    for (uint i = 0; i < uNumDoors; ++i) {
        BLVDoor *pDoor = &pDoors[i];

        for (uint j = 0; j < pDoor->uNumFaces; ++j) {
            BLVFace *pFace = &pFaces[pDoor->pFaceIDs[j]];
            BLVFaceExtra *pFaceExtra = &pFaceExtras[pFace->uFaceExtraID];

            pDoor->pDeltaUs[j] = pFaceExtra->sTextureDeltaU;
            pDoor->pDeltaVs[j] = pFaceExtra->sTextureDeltaV;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadRaw(&stru_5E4C90_MapPersistVars);

    pGameLoadingUI_ProgressBar->Progress();

    stream.ReadRaw(&stru1);

    return 0;
}

//----- (0049AC17) --------------------------------------------------------
int IndoorLocation::GetSector(int sX, int sY, int sZ) {
    if (uCurrentlyLoadedLevelType != LEVEL_Indoor) return 0;
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

        if ((pSector->pBounding.x1 - 5) > sX || (pSector->pBounding.x2 + 5) < sX ||
            (pSector->pBounding.y1 - 5) > sY || (pSector->pBounding.y2 + 5) < sY ||
            (pSector->pBounding.z1 - 64) > sZ || (pSector->pBounding.z2 + 64) < sZ)
            continue;  // outside sector bounding

        if (!backupboundingsector) backupboundingsector = i;

        int FloorsAndPortals = pSector->uNumFloors + pSector->uNumPortals;

        // nothing in secotr to check against so skip
        if (!FloorsAndPortals) continue;
        if (!pSector->pFloors) continue;

        // loop over check faces
        for (uint z = 0; z < FloorsAndPortals; ++z) {
            uint uFaceID;
            if (z < pSector->uNumFloors)
                uFaceID = pSector->pFloors[z];
            else
                uFaceID = pSector->pPortals[z - pSector->uNumFloors];

            BLVFace *pFace = &pFaces[uFaceID];
            if (pFace->uPolygonType != POLYGON_Floor && pFace->uPolygonType != POLYGON_InBetweenFloorAndWall)
                continue;

            // add found faces into store
            if (pFace->Contains(Vec3i(sX, sY, 0), MODEL_INDOOR, engine->config->gameplay.FloorChecksEps.Get(), FACE_XY_PLANE))
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
            logger->Warning("GetSector fail: {}, {}, {}", sX, sY, sZ);
            return 0;
        } else {
            logger->Warning("GetSector: Returning backup sector bounding!");
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
                CalcZDist = abs(sZ - this->pVertices[*this->pFaces[FoundFaceStore[s]].pVertexIDs].z);
            if (this->pFaces[FoundFaceStore[s]].uPolygonType == POLYGON_InBetweenFloorAndWall) {
                CalcZDist = abs(sZ - this->pFaces[FoundFaceStore[s]].zCalc.Calculate(sX, sY));
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
void BLVFace::_get_normals(Vec3i *a2, Vec3i *a3) {
    if (this->uPolygonType == POLYGON_VerticalWall) {
        a2->x = -this->pFacePlane_old.vNormal.y;
        a2->y = this->pFacePlane_old.vNormal.x;
        a2->z = 0;

        a3->x = 0;
        a3->y = 0;
        a3->z = 0xFFFF0000u;

    } else if (this->uPolygonType == POLYGON_Floor ||
               this->uPolygonType == POLYGON_Ceiling) {
        a2->x = 0x10000u;
        a2->y = 0;
        a2->z = 0;

        a3->x = 0;
        a3->y = 0xFFFF0000u;
        a3->z = 0;

    } else if (this->uPolygonType == POLYGON_InBetweenFloorAndWall ||
               this->uPolygonType == POLYGON_InBetweenCeilingAndWall) {
        if (abs(this->pFacePlane_old.vNormal.z) < 46441) {
            Vec3f a1;
            a1.x = (double)-this->pFacePlane_old.vNormal.y;
            a1.y = (double)this->pFacePlane_old.vNormal.x;
            a1.z = 0.0;
            a1.Normalize();

            a2->x = (int64_t)(a1.x * 65536.0);
            a2->y = (int64_t)(a1.y * 65536.0);
            a2->z = 0;

            a3->y = 0;
            a3->z = 0xFFFF0000u;
            a3->x = 0;

        } else {
            a2->x = 0x10000u;
            a2->y = 0;
            a2->z = 0;

            a3->x = 0;
            a3->y = 0xFFFF0000u;
            a3->z = 0;
        }
    }
    // LABEL_12:
    if (this->uAttributes & FACE_FlipNormalU) {
        a2->x = -a2->x;
        a2->y = -a2->y;
        a2->z = -a2->z;
    }
    if (this->uAttributes & FACE_FlipNormalV) {
        a3->x = -a3->x;
        a3->y = -a3->y;
        a3->z = -a3->z;
    }
    return;
}

void BLVFace::Flatten(FlatFace *points, int model_idx, FaceAttributes override_plane) const {
    Assert(!override_plane ||
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
    Assert(!override_plane ||
            override_plane == FACE_XY_PLANE || override_plane == FACE_YZ_PLANE || override_plane == FACE_XZ_PLANE);

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
    int event_index = 0;
    if ((uLevelEVT_NumEvents - 1) <= 0) {
        return false;
    }
    while (pLevelEVT_Index[event_index].event_id != this->uEventID) {
        ++event_index;
        if (event_index >= (signed int)(uLevelEVT_NumEvents - 1)) return false;
    }
    _evt_raw *end_evt =
        (_evt_raw
             *)&pLevelEVT[pLevelEVT_Index[event_index + 1].uEventOffsetInEVT];
    _evt_raw *start_evt =
        (_evt_raw *)&pLevelEVT[pLevelEVT_Index[event_index].uEventOffsetInEVT];
    if ((end_evt->_e_type != EVENT_Exit) ||
        (start_evt->_e_type != EVENT_MouseOver)) {
        return false;
    } else {
        return true;
    }
}

//----- (0046F228) --------------------------------------------------------
void BLV_UpdateDoors() {
    // signed int v20;      // eax@24
    int v24;             // esi@25
    int v25;             // eax@25
    int64_t v27 {};  // qtt@27
    BLVFaceExtra *v28;   // esi@32
    int v32;             // eax@34
    Vec3s *v34;    // eax@35
    int v35;             // ecx@35
    int v36;             // edx@35
    signed int v37;      // eax@35
    signed int v38;      // edx@35
    int v39;             // eax@35
    int v40;             // edx@35
    Vec3s *v43;    // edi@36
    int v57;             // eax@58
    Vec3i v67;
    Vec3i v70;
    int v75;               // [sp+28h] [bp-3Ch]@36
    int v76;               // [sp+2Ch] [bp-38h]@36
    int v77;               // [sp+30h] [bp-34h]@36
    int v82;               // [sp+44h] [bp-20h]@35
    int v83;               // [sp+48h] [bp-1Ch]@34
    int v84;               // [sp+4Ch] [bp-18h]@34
    int j;               // [sp+5Ch] [bp-8h]@18
    int open_distance;     // [sp+60h] [bp-4h]@6

    SoundID eDoorSoundID = (SoundID)pDoorSoundIDsByLocationID[dword_6BE13C_uCurrentlyLoadedLocationID];

    // loop over all doors
    for (uint i = 0; i < pIndoor->pDoors.size(); ++i) {
        BLVDoor *door = &pIndoor->pDoors[i];

        // door not moving currently
        if (door->uState == BLVDoor::Closed || door->uState == BLVDoor::Open) {
            door->uAttributes &= ~DOOR_SETTING_UP;
            continue;
        }

        door->uTimeSinceTriggered += pEventTimer->uTimeElapsed;
        if (door->uState == BLVDoor::Opening) {
            open_distance = (door->uTimeSinceTriggered * door->uCloseSpeed) / 128;
            if (open_distance >= door->uMoveLength) {
                open_distance = door->uMoveLength;
                door->uState = BLVDoor::Open;
                if (!(door->uAttributes & (DOOR_SETTING_UP | DOOR_NOSOUND)) && door->uNumVertices != 0)
                    pAudioPlayer->PlaySound((SoundID)((int)eDoorSoundID + 1), PID(OBJECT_Door, i), 0, -1, 0, 0);
                // goto LABEL_18;
            } else if (!(door->uAttributes & (DOOR_SETTING_UP | DOOR_NOSOUND)) && door->uNumVertices != 0) {
                pAudioPlayer->PlaySound(eDoorSoundID, PID(OBJECT_Door, i), 1, -1, 0, 0);
            }
        } else {  // door closing
            signed int v5 = (signed int)(door->uTimeSinceTriggered * door->uOpenSpeed) / 128;
            if (v5 >= door->uMoveLength) {
                open_distance = 0;
                door->uState = BLVDoor::Closed;
                if (!(door->uAttributes & (DOOR_SETTING_UP | DOOR_NOSOUND)) && door->uNumVertices != 0)
                    pAudioPlayer->PlaySound((SoundID)((int)eDoorSoundID + 1), PID(OBJECT_Door, i), 0, -1, 0, 0);
                // goto LABEL_18;
            } else {
                open_distance = door->uMoveLength - v5;
                if (!(door->uAttributes & (DOOR_SETTING_UP | DOOR_NOSOUND)) && door->uNumVertices != 0)
                    pAudioPlayer->PlaySound(eDoorSoundID, PID(OBJECT_Door, i), 1, -1, 0, 0);
            }
        }

        // adjust verts to how open the door is
        for (uint j = 0; j < door->uNumVertices; ++j) {
            pIndoor->pVertices[door->pVertexIDs[j]].x =
                fixpoint_mul(door->vDirection.x, open_distance) + door->pXOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].y =
                fixpoint_mul(door->vDirection.y, open_distance) + door->pYOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].z =
                fixpoint_mul(door->vDirection.z, open_distance) + door->pZOffsets[j];
        }


        for (j = 0; j < door->uNumFaces; ++j) {
            BLVFace *face = &pIndoor->pFaces[door->pFaceIDs[j]];
            Vec3s *v17 = &pIndoor->pVertices[face->pVertexIDs[0]];
            face->pFacePlane_old.dist = -Dot(*v17, face->pFacePlane_old.vNormal);
            face->pFacePlane.dist = -Dot(v17->ToFloat(), face->pFacePlane.vNormal);
            if (face->pFacePlane_old.vNormal.z) {
                v24 = abs(face->pFacePlane_old.dist >> 15);
                v25 = abs(face->pFacePlane_old.vNormal.z);
                if (v24 > v25)
                    Error(
                        "Door Error\ndoor id: %i\nfacet no: %i\n\nOverflow "
                        "dividing facet->d [%i] by facet->nz [%i]",
                        door->uDoorID, door->pFaceIDs[j],
                        face->pFacePlane_old.dist,
                        face->pFacePlane_old.vNormal.z);
                HEXRAYS_LODWORD(v27) = face->pFacePlane_old.dist << 16;
                HEXRAYS_HIDWORD(v27) = face->pFacePlane_old.dist >> 16;
                face->zCalc.c = -v27 / face->pFacePlane_old.vNormal.z;
            }
            // if ( face->uAttributes & FACE_TexMoveByDoor || render->pRenderD3D
            // )
            face->_get_normals(&v70, &v67);
            v28 = &pIndoor->pFaceExtras[face->uFaceExtraID];
            /*if ( !render->pRenderD3D )
            {
            if ( !(face->uAttributes & FACE_TexMoveByDoor) )
            continue;
            v83 = (uint64_t)(door->vDirection.x * (int64_t)v70.x)
            >> 16; v85 = (uint64_t)(door->vDirection.y * (signed
            int64_t)v70.y) >> 16; v84 = (uint64_t)(door->vDirection.z *
            (int64_t)v70.z) >> 16; v29 = open_distance; v28->sTextureDeltaU =
            -((v83 + v85 + v84) * (int64_t)open_distance) >> 16; v85 = (unsigned
            int64_t)(door->vDirection.x * (int64_t)v67.x) >> 16; v83 =
            (uint64_t)(door->vDirection.y * (int64_t)v67.y) >>
            16; v84 = (uint64_t)(door->vDirection.z * (signed
            int64_t)v67.z) >> 16; v31 = (v85 + v83 + v84) * (int64_t)v29;
            v32 = v31 >> 16;
            v57 = -v32;
            v28->sTextureDeltaV = v57;
            v28->sTextureDeltaU += door->pDeltaUs[j];
            v28->sTextureDeltaV = v57 + door->pDeltaVs[j];
            continue;
            }*/
            v28->sTextureDeltaU = 0;
            v28->sTextureDeltaV = 0;
            v34 = &pIndoor->pVertices[face->pVertexIDs[0]];
            v35 = v34->z;
            v36 = v34->y;
            v82 = v34->x;
            v37 = v70.x * v82 + v70.y * v36 + v70.z * v35;
            v38 = v67.x * v82 + v67.y * v36 + v67.z * v35;
            v39 = v37 >> 16;
            *face->pVertexUIDs = v39;
            v40 = v38 >> 16;
            *face->pVertexVIDs = v40;
            v84 = v39;
            v82 = v40;
            for (uint j = 1; j < face->uNumVertices; ++j) {
                v43 = &pIndoor->pVertices[face->pVertexIDs[j]];
                v76 = ((int64_t)v70.z * v43->z + (int64_t)v70.x * v43->x +
                       (int64_t)v70.y * v43->y) >>
                      16;
                v77 = ((int64_t)v67.x * v43->x + (int64_t)v67.y * v43->y +
                       (int64_t)v43->z * v67.z) >>
                      16;
                if (v76 < v39) v39 = v76;
                if (v77 < v40) v40 = v77;
                if (v76 > v84) v84 = v76;
                if (v77 > v82) v82 = v77;
                face->pVertexUIDs[j] = v76;
                face->pVertexVIDs[j] = v77;
            }
            if (face->uAttributes & FACE_TexAlignLeft) {
                v28->sTextureDeltaU -= v39;
            } else {
                if (face->uAttributes & FACE_TexAlignRight) {
                    if (face->resource) {
                        // v28->sTextureDeltaU -= v84 +
                        // pBitmaps_LOD->pTextures[face->uBitmapID].uTextureWidth;
                        v28->sTextureDeltaU -=
                            v84 + ((Texture *)face->resource)->GetWidth();
                    }
                }
            }
            if (face->uAttributes & FACE_TexAlignDown) {
                v28->sTextureDeltaV -= v40;
            } else {
                if (face->uAttributes & FACE_TexAlignBottom) {
                    v28->sTextureDeltaV -=
                        v84 + ((Texture *)face->resource)->GetHeight();
                    // if (face->uBitmapID != -1)
                    //    v28->sTextureDeltaV -= v82 +
                    //    pBitmaps_LOD->GetTexture(face->uBitmapID)->uTextureHeight;
                }
            }
            if (face->uAttributes & FACE_TexMoveByDoor) {
                v84 = fixpoint_mul(door->vDirection.x, v70.x);
                v82 = fixpoint_mul(door->vDirection.y, v70.y);
                v83 = fixpoint_mul(door->vDirection.z, v70.z);
                v75 = v84 + v82 + v83;
                v82 = fixpoint_mul(v75, open_distance);
                v28->sTextureDeltaU = -v82;
                v84 = fixpoint_mul(door->vDirection.x, v67.x);
                v82 = fixpoint_mul(door->vDirection.y, v67.y);
                v83 = fixpoint_mul(door->vDirection.z, v67.z);
                v75 = v84 + v82 + v83;
                v32 = fixpoint_mul(v75, open_distance);
                v57 = -v32;
                v28->sTextureDeltaV = v57;
                v28->sTextureDeltaU += door->pDeltaUs[j];
                v28->sTextureDeltaV = v57 + door->pDeltaVs[j];
            }
        }
    }
}

//----- (0046F90C) --------------------------------------------------------
void UpdateActors_BLV() {
    if (engine->config->debug.NoActors.Get())
        return;

    for (Actor &actor : pActors) {
        if (actor.uAIState == Removed || actor.uAIState == Disabled || actor.uAIState == Summoned || actor.uMovementSpeed == 0)
            continue;

        unsigned int uFaceID;
        unsigned int uSectorID = actor.uSectorID;
        int floorZ = GetIndoorFloorZ(actor.vPosition, &uSectorID, &uFaceID);
        actor.uSectorID = uSectorID;

        if (uSectorID == 0 || floorZ <= -30000)
            continue;

        bool isFlying = actor.pMonsterInfo.uFlying;
        if (!actor.CanAct())
            isFlying = false;

        bool isAboveGround = false;
        if (actor.vPosition.z > floorZ + 1)
            isAboveGround = true;

        // make bloodsplat when the ground is hit
        if (!actor.donebloodsplat) {
            if (actor.uAIState == Dead || actor.uAIState == Dying) {
                if (actor.vPosition.z < floorZ + 30) { // 30 to provide small error / rounding factor
                    if (pMonsterStats->pInfos[actor.pMonsterInfo.uID].bBloodSplatOnDeath) {
                        if (engine->config->graphics.BloodSplats.Get()) {
                            float splatRadius = actor.uActorRadius * engine->config->graphics.BloodSplatsMultiplier.Get();
                            decal_builder->AddBloodsplat((float)actor.vPosition.x, (float)actor.vPosition.y, (float)(floorZ + 30), 1.0, 0.0, 0.0, splatRadius);
                        }
                        actor.donebloodsplat = true;
                    }
                }
            }
        }

        if (actor.uCurrentActionAnimation == ANIM_Walking) {  // actor is moving
            int moveSpeed = actor.uMovementSpeed;

            if (actor.pActorBuffs[ACTOR_BUFF_SLOWED].Active()) {
                if (actor.pActorBuffs[ACTOR_BUFF_SLOWED].uPower)
                    moveSpeed = actor.uMovementSpeed / actor.pActorBuffs[ACTOR_BUFF_SLOWED].uPower;
                else
                    moveSpeed = actor.uMovementSpeed / 2;
            }

            if (actor.uAIState == Pursuing || actor.uAIState == Fleeing)
                moveSpeed *= 2;

            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_WAIT)
                moveSpeed = moveSpeed * debug_turn_based_monster_movespeed_mul;

            if (moveSpeed > 1000)
                moveSpeed = 1000;

            actor.vVelocity.x = TrigLUT.Cos(actor.uYawAngle) * moveSpeed;
            actor.vVelocity.y = TrigLUT.Sin(actor.uYawAngle) * moveSpeed;
            if (isFlying)
                actor.vVelocity.z = TrigLUT.Sin(actor.uPitchAngle) * moveSpeed;
        } else {
            // actor is not moving
            // fixpoint(55000) = 0.83923339843, appears to be velocity decay.
            actor.vVelocity.x = fixpoint_mul(55000, actor.vVelocity.x);
            actor.vVelocity.y = fixpoint_mul(55000, actor.vVelocity.y);
            if (isFlying)
                actor.vVelocity.z = fixpoint_mul(55000, actor.vVelocity.z);
        }

        if (actor.vPosition.z <= floorZ) {
            actor.vPosition.z = floorZ + 1;
            if (pIndoor->pFaces[uFaceID].uPolygonType == POLYGON_Floor) {
                if (actor.vVelocity.z < 0)
                    actor.vVelocity.z = 0;
            } else {
                // fixpoint(45000) = 0.68664550781, no idea what the actual semantics here is.
                if (pIndoor->pFaces[uFaceID].pFacePlane_old.vNormal.z < 45000)
                    actor.vVelocity.z -= pEventTimer->uTimeElapsed * GetGravityStrength();
            }
        } else {
            if (isAboveGround && !isFlying)
                actor.vVelocity.z += -8 * pEventTimer->uTimeElapsed * GetGravityStrength();
        }

        if (actor.vVelocity.LengthSqr() >= 400) {
            ProcessActorCollisionsBLV(actor, isAboveGround, isFlying);
        } else {
            actor.vVelocity = Vec3s(0, 0, 0);
            if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_SKY) {
                if (actor.uAIState == Dead)
                    actor.uAIState = Removed;
            }
        }
    }
}

//----- (00460A78) --------------------------------------------------------
void PrepareToLoadBLV(bool bLoading) {
    unsigned int respawn_interval;  // ebx@1
    unsigned int map_id;            // eax@8
    MapInfo *map_info;              // edi@9
    int v4;                         // eax@11
    bool v28;                       // zf@81
    int v35;                        // [sp+3F8h] [bp-1Ch]@1
    bool v38;                        // [sp+404h] [bp-10h]@1
    int pDest;                      // [sp+40Ch] [bp-8h]@1

    respawn_interval = 0;
    pGameLoadingUI_ProgressBar->Reset(0x20u);
    bNoNPCHiring = false;
    pDest = 1;
    uCurrentlyLoadedLevelType = LEVEL_Indoor;
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
    if (_A750D8_player_speech_timer)
        _A750D8_player_speech_timer = 0;
    map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id) {
        map_info = &pMapStats->pInfos[map_id];
        respawn_interval = pMapStats->pInfos[map_id].uRespawnIntervalDays;
        v38 = GetAlertStatus();
    } else {
        map_info = nullptr;
    }
    dword_6BE13C_uCurrentlyLoadedLocationID = map_id;

    pStationaryLightsStack->uNumLightsActive = 0;
    v4 = pIndoor->Load(pCurrentMapName, pParty->GetPlayingTime().GetDays() + 1,
                       respawn_interval, (char *)&pDest) - 1;
    if (v4 == 0) Error("Unable to open %s", pCurrentMapName.c_str());
    if (v4 == 1) Error("File %s is not a BLV File", pCurrentMapName.c_str()); // TODO(captainurist): these checks never trigger.
    if (v4 == 2) Error("Attempt to open new level before clearing old");
    if (v4 == 3) Error("Out of memory loading indoor level");
    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)) {
        Actor::InitializeActors();
        SpriteObject::InitializeSpriteObjects();
    }
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN;
    if (!map_id)
        pDest = 0;

    if (pDest == 1) {
        for (uint i = 0; i < pIndoor->pSpawnPoints.size(); ++i) {
            auto spawn = &pIndoor->pSpawnPoints[i];
            if (spawn->uKind == OBJECT_Actor)
                SpawnEncounter(map_info, spawn, 0, 0, 0);
            else
                map_info->SpawnRandomTreasure(spawn);
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

    v35 = 0;
    for (uint i = 0; i < pLevelDecorations.size(); ++i) {
        pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);

        const DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);

        if (decoration->uSoundID && _6807E0_num_decorations_with_sounds_6807B8 < 9) {
            // pSoundList->LoadSound(decoration->uSoundID, 0);
            _6807B8_level_decorations_ids[_6807E0_num_decorations_with_sounds_6807B8++] = i;
        }

        if (!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            if (!decoration->DontDraw()) {
                if (decoration->uLightRadius) {
                    unsigned char r = 255, g = 255, b = 255;
                    if (/*render->pRenderD3D*/ true &&
                        render->config->graphics.ColoredLights.Get()) {
                        r = decoration->uColoredLightRed;
                        g = decoration->uColoredLightGreen;
                        b = decoration->uColoredLightBlue;
                    }
                    pStationaryLightsStack->AddLight(
                            pLevelDecorations[i].vPosition.ToFloat() +
                            Vec3f(0, 0, decoration->uDecorationHeight),
                        decoration->uLightRadius, r, g, b, _4E94D0_light_type);
                }
            }
        }

        if (!pLevelDecorations[i].uEventID) {
            if (pLevelDecorations[i].IsInteractive()) {
                if (v35 < 124) {
                    pLevelDecorations[i]._idx_in_stru123 = v35 + 75;
                    if (!stru_5E4C90_MapPersistVars._decor_events[v35])
                        pLevelDecorations[i].uFlags |=
                            LEVEL_DECORATION_INVISIBLE;
                    v35++;
                }
            }
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].uObjectDescID) {
            if (pSpriteObjects[i].containing_item.uItemID != ITEM_NULL) {
                if (pSpriteObjects[i].containing_item.uItemID != ITEM_POTION_BOTTLE &&
                    pItemTable->pItems[pSpriteObjects[i].containing_item.uItemID].uEquipType == EQUIP_POTION &&
                    !pSpriteObjects[i].containing_item.uEnchantmentType)
                    pSpriteObjects[i].containing_item.uEnchantmentType = grng->Random(15) + 5;
                pItemTable->SetSpecialBonus(&pSpriteObjects[i].containing_item);
            }
        }
    }

    // INDOOR initialize actors
    v38 = false;

    for (uint i = 0; i < pActors.size(); ++i) {
        if (pActors[i].uAttributes & ACTOR_UNKNOW7) {
            if (!map_id) {
                pActors[i].pMonsterInfo.field_3E = 19;
                pActors[i].uAttributes |= ACTOR_UNKNOW11;
                continue;
            }
            v28 = !v38;
        } else {
            v28 = v38;
        }

        if (!v28) {
            pActors[i].PrepareSprites(0);
            pActors[i].pMonsterInfo.uHostilityType = MonsterInfo::Hostility_Friendly;
            if (pActors[i].pMonsterInfo.field_3E != 11 &&
                pActors[i].pMonsterInfo.field_3E != 19 &&
                (!pActors[i].sCurrentHP || !pActors[i].pMonsterInfo.uHP)) {
                pActors[i].pMonsterInfo.field_3E = 5;
                pActors[i].UpdateAnimation();
            }
        } else {
            pActors[i].pMonsterInfo.field_3E = 19;
            pActors[i].uAttributes |= ACTOR_UNKNOW11;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    // Party to start position
    Actor this_;
    this_.pMonsterInfo.uID = 45;
    this_.PrepareSprites(0);
    if (!bLoading) {
        pParty->sRotationY = 0;
        pParty->sRotationZ = 0;
        pParty->vPosition.z = 0;
        pParty->vPosition.y = 0;
        pParty->vPosition.x = 0;
        pParty->uFallStartZ = 0;
        pParty->uFallSpeed = 0;
        TeleportToStartingPoint(uLevel_StartingPointType);
        pBLVRenderParams->Reset();
    }
    viewparams->_443365();
    PlayLevelMusic();

    // Active character speaks.
    if (!bLoading && pDest) {
        int id = pParty->getRandomActiveCharacterId();

        if (id != -1) {
            _A750D8_player_speech_timer = 256;
            PlayerSpeechID = SPEECH_EnterDungeon;
            uSpeakingCharacter = id;
        }
    }
}

//----- (0046CEC3) --------------------------------------------------------
int BLV_GetFloorLevel(const Vec3i &pos, unsigned int uSectorID, unsigned int *pFaceID) {
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

        if (!pFloor->Contains(pos, MODEL_INDOOR, engine->config->gameplay.FloorChecksEps.Get(), FACE_XY_PLANE))
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
            z_calc = pFloor->zCalc.Calculate(pos.x, pos.y);
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

            if(!portal->Contains(pos, MODEL_INDOOR, engine->config->gameplay.FloorChecksEps.Get(), FACE_XY_PLANE))
                continue;

            blv_floor_z[FacesFound] = -29000;
            blv_floor_id[FacesFound] = pSector->pPortals[i];
            FacesFound++;
        }
    }

    // one face found
    if (FacesFound == 1) {
        *pFaceID = blv_floor_id[0];
        if (blv_floor_z[0] <= -29000) __debugbreak();
        return blv_floor_z[0];
    }

    // no face found - probably wrong sector supplied
    if (!FacesFound) {
        if (engine->config->debug.VerboseLogging.Get())
            logger->Warning("Floorlvl fail: {} {} {}", pos.x, pos.y, pos.z);

        *pFaceID = -1;
        return -30000;
    }

    // multiple faces found - pick nearest
    int result = blv_floor_z[0];
    *pFaceID = blv_floor_id[0];
    for (uint i = 1; i < FacesFound; ++i) {
        int v38 = blv_floor_z[i];

        if (abs(pos.z - v38) <= abs(pos.z - result)) {
            result = blv_floor_z[i];
            if (blv_floor_z[i] <= -29000) __debugbreak();
            *pFaceID = blv_floor_id[i];
        }
    }

    if (result <= -29000) __debugbreak();

    return result;
}

//----- (0043FA33) --------------------------------------------------------
void IndoorLocation::PrepareDecorationsRenderList_BLV(unsigned int uDecorationID, unsigned int uSectorID) {
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
        particle.uDiffuse = colorTable.OrangeyRed.C32();
        particle.x = (double)pLevelDecorations[uDecorationID].vPosition.x;
        particle.y = (double)pLevelDecorations[uDecorationID].vPosition.y;
        particle.z = (double)pLevelDecorations[uDecorationID].vPosition.z;
        particle.r = 0.0;
        particle.g = 0.0;
        particle.b = 0.0;
        particle.particle_size = 1.0;
        particle.timeToLive = vrng->Random(0x80) + 128; // was rand() & 0x80
        particle.texture = spell_fx_renderer->effpar01;
        particle_engine->AddParticle(&particle);
        return;
    }

    if (decoration->uFlags & DECORATION_DESC_DONT_DRAW) {
        return;
    }

    v8 = pLevelDecorations[uDecorationID].field_10_y_rot +
         ((signed int)TrigLUT.uIntegerPi >> 3) -
         TrigLUT.Atan2(pLevelDecorations[uDecorationID].vPosition.x -
                                pCamera3D->vCameraPos.x,
                            pLevelDecorations[uDecorationID].vPosition.y -
                                pCamera3D->vCameraPos.y);
    v9 = ((signed int)(TrigLUT.uIntegerPi + v8) >> 8) & 7;
    int v37 = pBLVRenderParams->field_0_timer_;
    if (pParty->bTurnBasedModeOn) v37 = pMiscTimer->uTotalGameTimeElapsed;
    v10 = abs(pLevelDecorations[uDecorationID].vPosition.x +
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
        if (2 * abs(view_x) >= abs(view_y)) {
            int projected_x = 0;
            int projected_y = 0;
            pCamera3D->Project(view_x, view_y, view_z, &projected_x, &projected_y);

            float billb_scale = v11->scale * pCamera3D->ViewPlaneDist_X / view_x;

            int screen_space_half_width = static_cast<int>(billb_scale * v11->hw_sprites[(int64_t)v9]->uBufferWidth / 2.0f);
            int screen_space_height = static_cast<int>(billb_scale * v11->hw_sprites[(int64_t)v9]->uBufferHeight);

            if (projected_x + screen_space_half_width >= (signed int)pViewport->uViewportTL_X &&
                projected_x - screen_space_half_width <= (signed int)pViewport->uViewportBR_X) {
                if (projected_y >= pViewport->uViewportTL_Y && (projected_y - screen_space_height) <= pViewport->uViewportBR_Y) {
                    assert(uNumBillboardsToDraw < 500);
                    ++uNumBillboardsToDraw;
                    ++uNumDecorationsDrawnThisFrame;

                    pBillboardRenderList[uNumBillboardsToDraw - 1].hwsprite =
                        v11->hw_sprites[v9];

                    if (v11->hw_sprites[v9]->texture->GetHeight() == 0 || v11->hw_sprites[v9]->texture->GetWidth() == 0)
                        __debugbreak();

                    pBillboardRenderList[uNumBillboardsToDraw - 1].uPaletteIndex = v11->GetPaletteIndex();
                    pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID =
                        uSectorID;

                    pBillboardRenderList[uNumBillboardsToDraw - 1].fov_x =
                        pCamera3D->ViewPlaneDist_X;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].fov_y =
                        pCamera3D->ViewPlaneDist_Y;
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
                        PID(OBJECT_Decoration, uDecorationID);

                    pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor = 0;
                    pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = v11;
                }
            }
        }
    }
}

//----- (00407A1C) --------------------------------------------------------
bool Check_LineOfSight(const Vec3i &target, const Vec3i &from) {  // target from - true on clear
    int AngleToTarget = TrigLUT.Atan2(from.x - target.x, from.y - target.y);
    bool LOS_Obscurred = 0;
    bool LOS_Obscurred2 = 0;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        Vec3i targetmod{};
        Vec3i frommod{};

        // offset 32 to side and check LOS
        Vec3i::Rotate(32, TrigLUT.uIntegerHalfPi + AngleToTarget, 0, target, &targetmod.x, &targetmod.y, &targetmod.z);
        Vec3i::Rotate(32, TrigLUT.uIntegerHalfPi + AngleToTarget, 0, from, &frommod.x, &frommod.y, &frommod.z);
        LOS_Obscurred2 = Check_LOS_Obscurred_Indoors(targetmod, frommod);

        // offset other side and repeat check
        Vec3i::Rotate(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0, target, &targetmod.x, &targetmod.y, &targetmod.z);
        Vec3i::Rotate(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0, from, &frommod.x, &frommod.y, &frommod.z);
        LOS_Obscurred = Check_LOS_Obscurred_Indoors(targetmod, frommod);
    } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        // TODO(pskelton): Need to add check against terrain
        Vec3i targetmod{};
        Vec3i frommod{};

        // offset 32 to side and check LOS
        Vec3i::Rotate(32, TrigLUT.uIntegerHalfPi + AngleToTarget, 0, target, &targetmod.x, &targetmod.y, &targetmod.z);
        Vec3i::Rotate(32, TrigLUT.uIntegerHalfPi + AngleToTarget, 0, from, &frommod.x, &frommod.y, &frommod.z);
        LOS_Obscurred2 = Check_LOS_Obscurred_Outdoors_Bmodels(targetmod, frommod);

        // offset other side and repeat check
        Vec3i::Rotate(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0, target, &targetmod.x, &targetmod.y, &targetmod.z);
        Vec3i::Rotate(32, AngleToTarget - TrigLUT.uIntegerHalfPi, 0, from, &frommod.x, &frommod.y, &frommod.z);
        LOS_Obscurred = Check_LOS_Obscurred_Outdoors_Bmodels(targetmod, frommod);
    }

    bool result{ !LOS_Obscurred2 || !LOS_Obscurred };
    return result;  // true if LOS clear
}

bool Check_LOS_Obscurred_Indoors(const Vec3i &target, const Vec3i &from) {  // true if obscurred
    int dist_x = from.x - target.x;
    int dist_y = from.y - target.y;
    int dist_z = from.z - target.z;

    int distance = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
    int fp_normalisation = 65536;
    if (distance) fp_normalisation = 65536 / distance;

    int fp_dist_x_normed = dist_x * fp_normalisation;
    int fp_dist_y_normed = dist_y * fp_normalisation;
    int fp_dist_z_normed = dist_z * fp_normalisation;

    int max_x = std::max(from.x, target.x);
    int min_x = std::min(from.x, target.x);

    int max_y = std::max(from.y, target.y);
    int min_y = std::min(from.y, target.y);

    int max_z = std::max(from.z, target.z);
    int min_z = std::min(from.z, target.z);

    for (int sectargetrflip = 0; sectargetrflip < 2; sectargetrflip++) {
        int SectargetrID = 0;
        if (sectargetrflip)
            SectargetrID = pIndoor->GetSector(target.x, target.y, target.z);
        else
            SectargetrID = pIndoor->GetSector(from.x, from.y, from.z);

        // loop over sectargetr faces
        for (int FaceLoop = 0; FaceLoop < pIndoor->pSectors[SectargetrID].uNumFaces; ++FaceLoop) {
            BLVFace* face = &pIndoor->pFaces[pIndoor->pSectors[SectargetrID].pFaceIDs[FaceLoop]];

            // dot product
            int x_dot = fixpoint_mul(fp_dist_x_normed, face->pFacePlane_old.vNormal.x);
            int y_dot = fixpoint_mul(fp_dist_y_normed, face->pFacePlane_old.vNormal.y);
            int z_dot = fixpoint_mul(fp_dist_z_normed, face->pFacePlane_old.vNormal.z);
            int sumdot = x_dot + y_dot + z_dot;
            bool FaceIsParallel = (sumdot == 0);

            // skip further checks
            if (face->Portal() || min_x > face->pBounding.x2 ||
                max_x < face->pBounding.x1 || min_y > face->pBounding.y2 ||
                max_y < face->pBounding.y1 || min_z > face->pBounding.z2 ||
                max_z < face->pBounding.z1 || FaceIsParallel)
                continue;

            int NegFacePlaceDist = -face->pFacePlane_old.SignedDistanceToAsFixpoint(target.x, target.y, target.z);
            // are we on same side of plane
            if (sumdot <= 0) {
                if (NegFacePlaceDist > 0)
                    continue;
            } else {
                if (NegFacePlaceDist < 0)
                    continue;
            }

            int EpsilonCheck = abs(NegFacePlaceDist) >> 14;
            if (EpsilonCheck <= abs(sumdot)) {
                int IntersectionDist = fixpoint_div(NegFacePlaceDist, sumdot);
                // less than zero means intersection is behind target point
                if (IntersectionDist >= 0) {
                    Vec3i pos = Vec3i(
                        target.x + ((signed int)(fixpoint_mul(IntersectionDist, fp_dist_x_normed) + 0x8000) >> 16),
                        target.y + ((signed int)(fixpoint_mul(IntersectionDist, fp_dist_y_normed) + 0x8000) >> 16),
                        target.z + ((signed int)(fixpoint_mul(IntersectionDist, fp_dist_z_normed) + 0x8000) >> 16));
                    if (face->Contains(pos, MODEL_INDOOR)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Check_LOS_Obscurred_Outdoors_Bmodels(const Vec3i& target, const Vec3i& from) {  // true is obscurred
    int dist_x = from.x - target.x;
    int dist_y = from.y - target.y;
    int dist_z = from.z - target.z;

    int distance = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
    int fp_normalisation = 65536;
    if (distance) fp_normalisation = 65536 / distance;

    // normalising
    int fp_dist_x_normed = dist_x * fp_normalisation;
    int fp_dist_y_normed = dist_y * fp_normalisation;
    int fp_dist_z_normed = dist_z * fp_normalisation;

    int max_x = std::max(from.x, target.x);
    int min_x = std::min(from.x, target.x);

    int max_y = std::max(from.y, target.y);
    int min_y = std::min(from.y, target.y);

    int max_z = std::max(from.z, target.z);
    int min_z = std::min(from.z, target.z);

    for (BSPModel& model : pOutdoor->pBModels) {
        if (CalcDistPointToLine(target.x, target.y, from.x, from.y, model.vPosition.x, model.vPosition.y) <= model.sBoundingRadius + 128) {
            for (ODMFace& face : model.pFaces) {
                // dot product
                int x_dot = fixpoint_mul(fp_dist_x_normed, face.pFacePlaneOLD.vNormal.x);
                int y_dot = fixpoint_mul(fp_dist_z_normed, face.pFacePlaneOLD.vNormal.y);
                int z_dot = fixpoint_mul(fp_dist_y_normed, face.pFacePlaneOLD.vNormal.z);
                int sumdot = x_dot + y_dot + z_dot;
                bool FaceIsParallel = (sumdot == 0);

                // bounds check
                if (min_x > face.pBoundingBox.x2 ||
                    max_x < face.pBoundingBox.x1 ||
                    min_y > face.pBoundingBox.y2 ||
                    max_y < face.pBoundingBox.y1 ||
                    min_z > face.pBoundingBox.z2 ||
                    max_z < face.pBoundingBox.z1 || FaceIsParallel)
                    continue;

                // point target plane distacne
                int NegFacePlaceDist = -face.pFacePlaneOLD.SignedDistanceToAsFixpoint(target.x, target.y, target.z);

                // are we on same side of plane
                if (sumdot <= 0) {
                    // angle obtuse - is target underneath plane
                    if (NegFacePlaceDist > 0)
                        continue;  // can never hit
                } else {
                    // angle acute - is target above plane
                    if (NegFacePlaceDist < 0)
                        continue;  // can never hit
                }

                int EpsilonCheck = abs(NegFacePlaceDist) >> 14;
                if (EpsilonCheck <= abs(sumdot)) {
                    // calc how far along line interesction is
                    int IntersectionDist = fixpoint_div(NegFacePlaceDist, sumdot);
                    // less than zero means intersection is behind target point
                    if (IntersectionDist >= 0) {
                        Vec3i pos = Vec3i(
                            target.x + ((signed int)(fixpoint_mul(IntersectionDist, fp_dist_x_normed) + 0x8000) >> 16),
                            target.y + ((signed int)(fixpoint_mul(IntersectionDist, fp_dist_y_normed) + 0x8000) >> 16),
                            target.z + ((signed int)(fixpoint_mul(IntersectionDist, fp_dist_z_normed) + 0x8000) >> 16));
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
char DoInteractionWithTopmostZObject(int pid) {
    auto id = PID_ID(pid);
    auto type = PID_TYPE(pid);

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
            if (pActors[id].uAIState == Dying || pActors[id].uAIState == Summoned)
                return 1;
            if (pActors[id].uAIState == Dead) {
                pActors[id].LootActor();
            } else {
                extern bool CanInteractWithActor(unsigned int id);
                extern void InteractWithActor(unsigned int id);
                if (CanInteractWithActor(id))
                    InteractWithActor(id);
            }
            break;

        case OBJECT_Decoration:
            extern void DecorationInteraction(unsigned int id, unsigned int pid);
            DecorationInteraction(id, pid);
            break;

        default:
            logger->Warning("Warning: Invalid ID reached!");
            return 1;

        case OBJECT_Face:
            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                int bmodel_id = pid >> 9;
                int face_id = id & 0x3F;

                if (bmodel_id >= pOutdoor->pBModels.size()) {
                    return 1;
                }
                if (pOutdoor->pBModels[bmodel_id].pFaces[face_id].uAttributes & FACE_HAS_EVENT ||
                    pOutdoor->pBModels[bmodel_id].pFaces[face_id].sCogTriggeredID == 0)
                    return 1;
                EventProcessor((int16_t)pOutdoor->pBModels[bmodel_id].pFaces[face_id].sCogTriggeredID,
                               pid, 1);
            } else {
                if (!(pIndoor->pFaces[id].uAttributes & FACE_CLICKABLE)) {
                    GameUI_StatusBar_NothingHere();
                    return 1;
                }
                if (pIndoor->pFaces[id].uAttributes & FACE_HAS_EVENT ||
                    !pIndoor->pFaceExtras[pIndoor->pFaces[id].uFaceExtraID].uEventID)
                    return 1;
                if (current_screen_type != CURRENT_SCREEN::SCREEN_BRANCHLESS_NPC_DIALOG)
                    EventProcessor((int16_t)pIndoor->pFaceExtras[pIndoor->pFaces[id].uFaceExtraID].uEventID,
                                   pid, 1);
            }
            return 0;
            break;
    }
    return 0;
}
//----- (0046BDF1) --------------------------------------------------------
void BLV_UpdateUserInputAndOther() {
    UpdateObjects();
    BLV_ProcessPartyActions();
    UpdateActors_BLV();
    BLV_UpdateDoors();
    check_event_triggers();
}

//----- (00472866) --------------------------------------------------------
void BLV_ProcessPartyActions() {  // could this be combined with odm process actions?
    unsigned int uFaceEvent = 0;

    bool party_running_flag = false;
    bool party_walking_flag = false;
    bool hovering = false;
    bool not_high_fall = false;
    bool on_water = false;
    bool bFeatherFall;

    unsigned int uSectorID = pBLVRenderParams->uPartySectorID;
    unsigned int uFaceID = -1;
    int party_z = pParty->vPosition.z;
    int floor_z = GetIndoorFloorZ(pParty->vPosition + Vec3i(0, 0, 40), &uSectorID, &uFaceID);

    if (pParty->bFlying)  // disable flight
        pParty->bFlying = false;

    if (floor_z == -30000 || uFaceID == -1) {
        floor_z = GetApproximateIndoorFloorZ(pParty->vPosition + Vec3i(0, 0, 40), &uSectorID, &uFaceID);
        if (floor_z == -30000 || uFaceID == -1) {
            __debugbreak();  // level built with errors
            pParty->vPosition = blv_prev_party_pos;
            pParty->uFallStartZ = blv_prev_party_pos.z;
            return;
        }
    }

    blv_prev_party_pos = pParty->vPosition;

    int fall_start;
    if (pParty->FeatherFallActive() || pParty->WearsItemAnywhere(ITEM_ARTIFACT_LADYS_ESCORT)) {
        fall_start = floor_z;
        bFeatherFall = true;
        pParty->uFallStartZ = floor_z;
    } else {
        bFeatherFall = false;
        fall_start = pParty->uFallStartZ;
    }

    if (fall_start - party_z > 512 && !bFeatherFall && party_z <= floor_z + 1) {  // fall damage
        if (pParty->uFlags & PARTY_FLAGS_1_LANDING) {
            __debugbreak(); // why land in indoor?
            pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
        } else {
            for (uint i = 0; i < 4; ++i) {  // receive falling damage
                if (pParty->pPlayers[i].HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) ||
                    pParty->pPlayers[i].WearsItem(ITEM_ARTIFACT_HERMES_SANDALS, ITEM_SLOT_BOOTS))
                    continue;

                pParty->pPlayers[i].ReceiveDamage(
                    (pParty->uFallStartZ - party_z) * (0.1f * pParty->pPlayers[i].GetMaxHealth()) / 256, DMGT_PHISYCAL);

                // TODO: this can become negative, and there is an assert for that in SetRecoveryTime.
                pParty->pPlayers[i].SetRecoveryTime(
                    (20 - pParty->pPlayers[i].GetParameterBonus(pParty->pPlayers[i].GetActualEndurance())) *
                    debug_non_combat_recovery_mul * flt_debugrecmod3);
            }
        }
    }

    if (party_z > floor_z + 1)
        hovering = true;

    if (party_z - floor_z <= 32) {
        pParty->uFallStartZ = party_z;
        not_high_fall = true;
    }

    // update timer for walking sounds
    if (engine->config->settings.WalkSound.Get() && pParty->walk_sound_timer > 0)
        pParty->walk_sound_timer = std::max(0, pParty->walk_sound_timer - static_cast<int>(pEventTimer->uTimeElapsed));

    // party is below floor level?
    if (party_z <= floor_z + 1) {
        party_z = floor_z + 1;
        pParty->uFallStartZ = floor_z + 1;

        // not hovering & stepped onto a new face => activate potential pressure plate,
        // TODO: but why is this condition under "below floor level" if above?
        if (!hovering && pParty->floor_face_pid != uFaceID) {
            if (pIndoor->pFaces[uFaceID].uAttributes & FACE_PRESSURE_PLATE)
                uFaceEvent = pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID].uEventID;
        }
    }
    if (!hovering)
        pParty->floor_face_pid = uFaceID;

    // party is on water?
    if (pIndoor->pFaces[uFaceID].Fluid())
        on_water = true;

    // Party angle in XY plane.
    int angle = pParty->sRotationZ;

    // Vertical party angle (basically azimuthal angle in polar coordinates).
    int vertical_angle = pParty->sRotationY;

    // Calculate rotation in ticks (1024 ticks per 180 degree).
    int rotation =
        (static_cast<int64_t>(pEventTimer->dt_fixpoint) * pParty->y_rotation_speed * TrigLUT.uIntegerPi / 180) >> 16;

    // If party movement delta is lower then this number then the party remains stationary.
    int64_t elapsed_time_bounded = std::min(pEventTimer->uTimeElapsed, 10000);
    int min_party_move_delta_sqr = 400 * elapsed_time_bounded * elapsed_time_bounded / 8;

    int party_dy = 0;
    int party_dx = 0;
    while (pPartyActionQueue->uNumActions) {
        switch (pPartyActionQueue->Next()) {
            case PARTY_TurnLeft:
                if (engine->config->settings.TurnSpeed.Get() > 0)
                    angle = TrigLUT.uDoublePiMask & (angle + (int)engine->config->settings.TurnSpeed.Get());
                else
                    angle = TrigLUT.uDoublePiMask & (angle + static_cast<int>(rotation * fTurnSpeedMultiplier));
                break;
            case PARTY_TurnRight:
                if (engine->config->settings.TurnSpeed.Get() > 0)
                    angle = TrigLUT.uDoublePiMask & (angle - (int)engine->config->settings.TurnSpeed.Get());
                else
                    angle = TrigLUT.uDoublePiMask & (angle - static_cast<int>(rotation * fTurnSpeedMultiplier));
                break;

            case PARTY_FastTurnLeft:
                if (engine->config->settings.TurnSpeed.Get() > 0)
                    angle = TrigLUT.uDoublePiMask & (angle + (int)engine->config->settings.TurnSpeed.Get());
                else
                    angle = TrigLUT.uDoublePiMask & (angle + static_cast<int>(2.0f * rotation * fTurnSpeedMultiplier));
                break;

            case PARTY_FastTurnRight:
                if (engine->config->settings.TurnSpeed.Get() > 0)
                    angle = TrigLUT.uDoublePiMask & (angle - (int)engine->config->settings.TurnSpeed.Get());
                else
                    angle = TrigLUT.uDoublePiMask & (angle - static_cast<int>(2.0f * rotation * fTurnSpeedMultiplier));
                break;

            case PARTY_StrafeLeft:
                party_dx -= TrigLUT.Sin(angle) * pParty->uWalkSpeed * fWalkSpeedMultiplier / 2;
                party_dy += TrigLUT.Cos(angle) * pParty->uWalkSpeed * fWalkSpeedMultiplier / 2;
                party_walking_flag = true;
                break;

            case PARTY_StrafeRight:
                party_dy -= TrigLUT.Cos(angle) * pParty->uWalkSpeed * fWalkSpeedMultiplier / 2;
                party_dx += TrigLUT.Sin(angle) * pParty->uWalkSpeed * fWalkSpeedMultiplier / 2;
                party_walking_flag = true;
                break;

            case PARTY_WalkForward:
                party_dx += TrigLUT.Cos(angle) * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                party_dy += TrigLUT.Sin(angle) * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                party_walking_flag = true;
                break;

            case PARTY_WalkBackward:
                party_dx -= TrigLUT.Cos(angle) * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                party_dy -= TrigLUT.Sin(angle) * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                party_walking_flag = true;
                break;

            case PARTY_RunForward:
                party_dx += TrigLUT.Cos(angle) * 2 * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                party_dy += TrigLUT.Sin(angle) * 2 * pParty->uWalkSpeed * fWalkSpeedMultiplier;
                party_running_flag = true;
                break;

            case PARTY_RunBackward:
                party_dx -= TrigLUT.Cos(angle) * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                party_dy -= TrigLUT.Sin(angle) * pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier;
                party_running_flag = true;
                break;

            case PARTY_LookUp:
                vertical_angle += engine->config->settings.VerticalTurnSpeed.Get();
                if (vertical_angle > 128)
                    vertical_angle = 128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_LookUp, 0);
                break;

            case PARTY_LookDown:
                vertical_angle -= engine->config->settings.VerticalTurnSpeed.Get();
                if (vertical_angle < -128)
                    vertical_angle = -128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_LookDown, 0);
                break;

            case PARTY_CenterView:
                vertical_angle = 0;
                break;

            case PARTY_Jump:
                if ((!hovering || party_z <= floor_z + 6 && pParty->uFallSpeed <= 0) && pParty->jump_strength) {
                    hovering = true;
                    pParty->uFallSpeed += pParty->jump_strength * 96;
                }
                break;
            default:
                break;
        }
    }

    if (party_dx * party_dx + party_dy * party_dy < min_party_move_delta_sqr) {
        party_dy = 0;
        party_dx = 0;
    }

    pParty->sRotationZ = angle;
    pParty->sRotationY = vertical_angle;

    if (hovering) {
        pParty->uFallSpeed += -2 * pEventTimer->uTimeElapsed * GetGravityStrength();
        if (pParty->uFallSpeed <= 0) {
            if (pParty->uFallSpeed < -500) {
                for (uint pl = 1; pl <= 4; pl++) {
                    if (!pPlayers[pl]->HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) &&
                        !pPlayers[pl]->WearsItem(ITEM_ARTIFACT_HERMES_SANDALS, ITEM_SLOT_BOOTS))  // was 8
                        pPlayers[pl]->PlayEmotion(CHARACTER_EXPRESSION_SCARED, 0);
                }
            }
        } else {
            pParty->uFallStartZ = party_z;
        }
    } else {
        if (pIndoor->pFaces[uFaceID].pFacePlane_old.vNormal.z < 0x8000) {
            pParty->uFallSpeed -= pEventTimer->uTimeElapsed * GetGravityStrength();
            pParty->uFallStartZ = party_z;
        } else {
            if (!(pParty->uFlags & PARTY_FLAGS_1_LANDING))
                pParty->uFallSpeed = 0;
            pParty->uFallStartZ = party_z;
        }
    }

    int new_party_x = pParty->vPosition.x;
    int new_party_y = pParty->vPosition.y;
    int new_party_z = party_z;

    collision_state.ignored_face_id = -1;
    collision_state.total_move_distance = 0;
    collision_state.radius_lo = pParty->radius;
    collision_state.radius_hi = pParty->radius / 2;
    collision_state.check_hi = true;
    for (uint i = 0; i < 100; i++) {
        new_party_z = party_z;
        collision_state.position_hi.x = new_party_x;
        collision_state.position_hi.y = new_party_y;
        collision_state.position_hi.z = (pParty->uPartyHeight - 32.0f) + party_z + 1.0f;

        collision_state.position_lo.x = new_party_x;
        collision_state.position_lo.y = new_party_y;
        collision_state.position_lo.z = collision_state.radius_lo + party_z + 1.0f;

        collision_state.velocity.x = party_dx;
        collision_state.velocity.y = party_dy;
        collision_state.velocity.z = pParty->uFallSpeed;

        collision_state.uSectorID = uSectorID;
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

        Vec3i adjusted_pos;
        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            adjusted_pos.x = collision_state.new_position_lo.x;
            adjusted_pos.y = collision_state.new_position_lo.y;
            adjusted_pos.z = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
        } else {
            adjusted_pos.x = new_party_x + collision_state.adjusted_move_distance * collision_state.direction.x;
            adjusted_pos.y = new_party_y + collision_state.adjusted_move_distance * collision_state.direction.y;
            adjusted_pos.z = new_party_z + collision_state.adjusted_move_distance * collision_state.direction.z;
        }
        int adjusted_floor_z = GetIndoorFloorZ(adjusted_pos + Vec3i(0, 0, 40), &collision_state.uSectorID, &uFaceID);
        if (adjusted_floor_z == -30000 || adjusted_floor_z - new_party_z > 128)
            return; // TODO: whaaa?

        if (collision_state.adjusted_move_distance >= collision_state.move_distance) {
            new_party_x = collision_state.new_position_lo.x;
            new_party_y = collision_state.new_position_lo.y;
            new_party_z = collision_state.new_position_lo.z - collision_state.radius_lo - 1;
            break; // And we're done with collisions.
        }

        collision_state.total_move_distance += collision_state.adjusted_move_distance;

        new_party_x += collision_state.adjusted_move_distance * collision_state.direction.x;
        new_party_y += collision_state.adjusted_move_distance * collision_state.direction.y;
        unsigned long long new_party_z_tmp = new_party_z +
            collision_state.adjusted_move_distance * collision_state.direction.z;

        if (PID_TYPE(collision_state.pid) == OBJECT_Actor) {
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset(); // Break invisibility when running into a monster.
        } else if (PID_TYPE(collision_state.pid) == OBJECT_Decoration) {
            // Bounce back from a decoration & do another round of collision checks.
            // This way the party can "slide" along & past a decoration.
            int angle = TrigLUT.Atan2(
                new_party_x - pLevelDecorations[PID_ID(collision_state.pid)].vPosition.x,
                new_party_y - pLevelDecorations[PID_ID(collision_state.pid)].vPosition.y);
            int len = integer_sqrt(party_dx * party_dx + party_dy * party_dy);
            party_dx = TrigLUT.Cos(angle) * len;
            party_dy = TrigLUT.Sin(angle) * len;
        } else if (PID_TYPE(collision_state.pid) == OBJECT_Face) {
            BLVFace *pFace = &pIndoor->pFaces[PID_ID(collision_state.pid)];
            if (pFace->uPolygonType == POLYGON_Floor) {
                if (pParty->uFallSpeed < 0)
                    pParty->uFallSpeed = 0;
                new_party_z_tmp = pIndoor->pVertices[*pFace->pVertexIDs].z + 1;
                if (pParty->uFallStartZ - new_party_z_tmp < 512)
                    pParty->uFallStartZ = new_party_z_tmp;
                if (party_dx * party_dx + party_dy * party_dy < min_party_move_delta_sqr) {
                    party_dy = 0;
                    party_dx = 0;
                }
                if (pParty->floor_face_pid != PID_ID(collision_state.pid) && pFace->Pressure_Plate())
                    uFaceEvent = pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
            } else { // Not floor
                int speed_dot_normal = abs(
                    party_dx * pFace->pFacePlane_old.vNormal.x +
                    party_dy * pFace->pFacePlane_old.vNormal.y +
                    pParty->uFallSpeed * pFace->pFacePlane_old.vNormal.z) >> 16;

                if ((collision_state.speed / 8) > speed_dot_normal)
                    speed_dot_normal = collision_state.speed / 8;

                party_dx += fixpoint_mul(speed_dot_normal, pFace->pFacePlane_old.vNormal.x);
                party_dy += fixpoint_mul(speed_dot_normal, pFace->pFacePlane_old.vNormal.y);
                pParty->uFallSpeed += fixpoint_mul(speed_dot_normal, pFace->pFacePlane_old.vNormal.z);

                if (pFace->uPolygonType != POLYGON_InBetweenFloorAndWall) { // wall / ceiling
                    int distance_to_face =
                        pFace->pFacePlane_old.SignedDistanceTo(new_party_x, new_party_y, new_party_z_tmp) -
                        collision_state.radius_lo;
                    if (distance_to_face < 0) {
                        // We're too close to the face, push back.
                        new_party_x += fixpoint_mul(-distance_to_face, pFace->pFacePlane_old.vNormal.x);
                        new_party_y += fixpoint_mul(-distance_to_face, pFace->pFacePlane_old.vNormal.y);
                        new_party_z_tmp += fixpoint_mul(-distance_to_face, pFace->pFacePlane_old.vNormal.z);
                    }
                    if (pParty->floor_face_pid != PID_ID(collision_state.pid) && pFace->Pressure_Plate())
                        uFaceEvent = pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
                } else { // between floor & wall
                    if (party_dx * party_dx + party_dy * party_dy >= min_party_move_delta_sqr) {
                        if (pParty->floor_face_pid != PID_ID(collision_state.pid) && pFace->Pressure_Plate())
                            uFaceEvent = pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
                    } else {
                        party_dx = 0;
                        party_dy = 0;
                        pParty->uFallSpeed = 0;
                    }
                }
            }
        }
        party_dx = fixpoint_mul(58500, party_dx);  // 58500 is roughly 0.89
        party_dy = fixpoint_mul(58500, party_dy);
        pParty->uFallSpeed = fixpoint_mul(58500, pParty->uFallSpeed);
    }

    //  //Воспроизведение звуков ходьбы/бега-------------------------
    uint pX_ = abs(pParty->vPosition.x - new_party_x);
    uint pY_ = abs(pParty->vPosition.y - new_party_y);
    uint pZ_ = abs(pParty->vPosition.z - new_party_z);
    if (engine->config->settings.WalkSound.Get() && pParty->walk_sound_timer <= 0) {
        pAudioPlayer->StopAll(804);  // stop sound
        if (party_running_flag && (!hovering || not_high_fall)) {  // Бег и (не прыжок или не высокое падение )
            if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) >= 16) {
                if (on_water)
                    pAudioPlayer->PlaySound(SOUND_RunWaterIndoor, 804, 1, -1, 0, 0);
                else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_CARPET)  //по ковру
                    pAudioPlayer->PlaySound(SOUND_RunCarpet, PID_INVALID /*804*/, 1, -1, 0, 0);
                else
                    pAudioPlayer->PlaySound(SOUND_RunWood, PID_INVALID /*804*/, 1, -1, 0, 0);
                pParty->walk_sound_timer = 96;  // 64
            }
        } else if (party_walking_flag && (!hovering || not_high_fall)) {  // Ходьба и (не прыжок или не
                                                    // высокое падение)
            if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) >= 8) {
                if (on_water)
                    pAudioPlayer->PlaySound(SOUND_WalkWaterIndoor, 804, 1, -1, 0, 0);
                else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_CARPET)  //по ковру
                    pAudioPlayer->PlaySound(SOUND_WalkCarpet, PID_INVALID /*804*/, 1, -1, 0, 0);
                else
                    pAudioPlayer->PlaySound(SOUND_WalkWood, PID_INVALID /*804*/, 1, -1, 0, 0);
                pParty->walk_sound_timer = 144;  // 64
            }
        }
    }
    if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) < 8)  //отключить  звук ходьбы при остановке
        pAudioPlayer->StopAll(804);
    //-------------------------------------------------------------

    if (!hovering || not_high_fall)
        pParty->SetAirborne(false);
    else
        pParty->SetAirborne(true);

    pParty->uFlags &= ~PARTY_FLAGS_1_BURNING;
    pParty->vPosition.x = new_party_x;
    pParty->vPosition.y = new_party_y;
    pParty->vPosition.z = new_party_z;
    // pParty->uFallSpeed = v89;

    if (!hovering && pIndoor->pFaces[uFaceID].uAttributes & FACE_IsLava)
        pParty->uFlags |= PARTY_FLAGS_1_BURNING;

    if (uFaceEvent)
        EventProcessor(uFaceEvent, 0, 1);
}

//----- (00449A49) --------------------------------------------------------
void Door_switch_animation(unsigned int uDoorID, int a2) {
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
    result = integer_sqrt(abs(x2 - x1) * abs(x2 - x1) + abs(y2 - y1) * abs(y2 - y1));

    // orthogonal projection from line to point
    if (result)
        result = abs(((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / result);

    return result;
}

//----- (00450DA3) --------------------------------------------------------
bool GetAlertStatus() {
    int result;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        result = pOutdoor->ddm.field_C_alert;
    else
        result = uCurrentlyLoadedLevelType == LEVEL_Outdoor ? pIndoor->dlv.field_C_alert : 0;

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

    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        int dist_y;
        int dist_x;
        bool not_in_model = false;
        bool bInWater = false;
        int modelPID = 0;

        // 100 attempts to make a usuable spawn point
        for (; loop_cnt < 100; ++loop_cnt) {
            // random x,y at distance from party
            dist_from_party = grng->Random(1024) + 512;
            angle_from_party = (grng->Random(TrigLUT.uIntegerDoublePi) * 2 * pi) / TrigLUT.uIntegerDoublePi;
            enc_spawn_point.vPosition.x = pParty->vPosition.x + cos(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.y = pParty->vPosition.y + sin(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.z = pParty->vPosition.z;
            enc_spawn_point.uKind = OBJECT_Actor;
            enc_spawn_point.uMonsterIndex = enc_index;

            // get proposed floor level
            enc_spawn_point.vPosition.z = ODM_GetFloorLevel(enc_spawn_point.vPosition, 0, &bInWater, &modelPID, 0);

            // check spawn point is not in a model
            for (BSPModel &model : pOutdoor->pBModels) {
                dist_y = abs(enc_spawn_point.vPosition.y - model.vBoundingCenter.y);
                dist_x = abs(enc_spawn_point.vPosition.x - model.vBoundingCenter.x);
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
    } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        int party_sectorID = pBLVRenderParams->uPartySectorID;
        int mon_sectorID;
        int indoor_floor_level;
        unsigned int uFaceID;

        // 100 attempts to make a usuable spawn point
        for (loop_cnt = 0; loop_cnt < 100; ++loop_cnt) {
            // random x,y at distance from party
            dist_from_party = grng->Random(512) + 256;
            angle_from_party = (grng->Random(TrigLUT.uIntegerDoublePi) * 2 * pi) / TrigLUT.uIntegerDoublePi;
            enc_spawn_point.vPosition.x = pParty->vPosition.x + cos(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.y = pParty->vPosition.y + sin(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.z = pParty->vPosition.z;
            enc_spawn_point.uKind = OBJECT_Actor;
            enc_spawn_point.uMonsterIndex = enc_index;

            // get proposed sector
            mon_sectorID = pIndoor->GetSector(enc_spawn_point.vPosition.x, enc_spawn_point.vPosition.y, pParty->vPosition.z);
            if (mon_sectorID == party_sectorID) {
                // check proposed floor level
                indoor_floor_level = BLV_GetFloorLevel(enc_spawn_point.vPosition, mon_sectorID, &uFaceID);
                enc_spawn_point.vPosition.z = indoor_floor_level;
                if (indoor_floor_level != -30000) {
                    // break if spanwn point is okay
                    if (abs(indoor_floor_level - pParty->vPosition.z) <= 1024) break;
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
    pItemTable->GenerateItem(trs_level, trs_type, &a1.containing_item);
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

//----- (0049B04D) --------------------------------------------------------
void stru154::GetFacePlaneAndClassify(ODMFace *a2, const std::vector<Vec3i> &a3) {
    Vec3f OutPlaneNorm;
    float OutPlaneDist;

    OutPlaneNorm.x = 0.0;
    OutPlaneNorm.y = 0.0;
    OutPlaneNorm.z = 0.0;
    GetFacePlane(a2, a3, &OutPlaneNorm, &OutPlaneDist);

    if (fabsf(a2->pFacePlane.vNormal.z) < 1e-6f)
        polygonType = POLYGON_VerticalWall;
    else if (fabsf(a2->pFacePlane.vNormal.x) < 1e-6f &&
             fabsf(a2->pFacePlane.vNormal.y) < 1e-6f)
        polygonType = POLYGON_Floor;
    else
        polygonType = POLYGON_InBetweenFloorAndWall;

    face_plane.vNormal.x = OutPlaneNorm.x;
    face_plane.vNormal.y = OutPlaneNorm.y;
    face_plane.vNormal.z = OutPlaneNorm.z;
    face_plane.dist = OutPlaneDist;
}

//----- (0049B0C9) --------------------------------------------------------
void stru154::ClassifyPolygon(Vec3f *pNormal, float dist) {
    if (fabsf(pNormal->z) < 1e-6f)
        polygonType = POLYGON_VerticalWall;
    else if (fabsf(pNormal->x) < 1e-6f && fabsf(pNormal->y) < 1e-6f)
        polygonType = POLYGON_Floor;
    else
        polygonType = POLYGON_InBetweenFloorAndWall;

    face_plane.vNormal.x = pNormal->x;
    face_plane.vNormal.y = pNormal->y;
    face_plane.vNormal.z = pNormal->z;
    face_plane.dist = dist;
}

//----- (0049B13D) --------------------------------------------------------
void stru154::GetFacePlane(ODMFace *pFace, const std::vector<Vec3i> &pVertices,
                           Vec3f *pOutNormal, float *pOutDist) {
    Vec3f FirstPairVec;
    Vec3f SecPairVec;
    Vec3f CrossProd;

    if (pFace->uNumVertices >= 2) {
        for (int i = 0; i < pFace->uNumVertices - 2; i++) {
            FirstPairVec = (pVertices[pFace->pVertexIDs[i + 1]] - pVertices[pFace->pVertexIDs[i]]).ToFloat();
            SecPairVec = (pVertices[pFace->pVertexIDs[i + 2]] - pVertices[pFace->pVertexIDs[i + 1]]).ToFloat();

            CrossProd = Cross(FirstPairVec, SecPairVec);

            if (CrossProd.x != 0.0 || CrossProd.y != 0.0 || CrossProd.z != 0.0) {
                CrossProd.Normalize();
                *pOutNormal = CrossProd;
                *pOutDist = -Dot(pVertices[pFace->pVertexIDs[i]].ToFloat(), CrossProd);
                return;
            }
        }
    }

    // only one/two vert?
    __debugbreak();
    pOutNormal->x = pFace->pFacePlane.vNormal.x;
    pOutNormal->y = pFace->pFacePlane.vNormal.y;
    pOutNormal->z = pFace->pFacePlane.vNormal.z;
    *pOutDist = pFace->pFacePlane.dist;
}

//----- (0043F515) --------------------------------------------------------
void FindBillboardsLightLevels_BLV() {
    for (uint i = 0; i < uNumBillboardsToDraw; ++i) {
        if (pBillboardRenderList[i].field_1E & 2 ||
            uCurrentlyLoadedLevelType == LEVEL_Indoor &&
                !pBillboardRenderList[i].uIndoorSectorID)
            pBillboardRenderList[i].dimming_level = 0;
        else
            pBillboardRenderList[i].dimming_level =
                _43F55F_get_billboard_light_level(&pBillboardRenderList[i], -1);
    }
}

int GetIndoorFloorZ(const Vec3i &pos, unsigned int *pSectorID, unsigned int *pFaceID) {
    if (*pSectorID != 0) {
        int result = BLV_GetFloorLevel(pos, *pSectorID, pFaceID);
        if (result != -30000 && result <= pos.z + 50)
            return result;
    }

    *pSectorID = pIndoor->GetSector(pos);
    if (*pSectorID == 0) {
        *pFaceID = -1;
        return -30000;
    }

    return BLV_GetFloorLevel(pos, *pSectorID, pFaceID);
}

//----- (0047272C) --------------------------------------------------------
int GetApproximateIndoorFloorZ(const Vec3i &pos, unsigned int *pSectorID, unsigned int *pFaceID) {
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

