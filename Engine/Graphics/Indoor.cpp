#include "Engine/Graphics/Indoor.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/Lights.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/stru10.h"
#include "Engine/Graphics/stru9.h"
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
#include "Engine/SpellFxRenderer.h"
#include "Engine/stru123.h"
#include "Engine/stru367.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/ZlibWrapper.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"


IndoorLocation *pIndoor = new IndoorLocation;
BLVRenderParams *pBLVRenderParams = new BLVRenderParams;

LEVEL_TYPE uCurrentlyLoadedLevelType = LEVEL_null;

LightsData Lights;
stru337_unused _DLV_header_unused;
BspRenderer_PortalViewportData _PortalViewportData_unused;
BspRenderer *pBspRenderer = new BspRenderer;
stru141_actor_collision_object _actor_collision_struct;
// std::array<stru352, 480> stru_F83B80;

unsigned __int16 pDoorSoundIDsByLocationID[78] = {
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
    300, 300, 300, 404, 302, 306, 308, 304, 308, 302, 400, 302, 300,
    308, 308, 306, 308, 308, 304, 300, 404, 406, 300, 400, 406, 404,
    306, 302, 408, 304, 300, 300, 300, 300, 300, 300, 300, 300, 300,
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 404, 304,
    400, 300, 300, 404, 304, 400, 300, 300, 404, 304, 400, 300, 300};

// possibly all locations which should have special tranfer message??
std::array<const char *, 11> _4E6BDC_loc_names = {
    "mdt12.blv", "d18.blv",   "mdt14.blv", "d37.blv",
    "mdk01.blv", "mdt01.blv", "mdr01.blv", "mdt10.blv",
    "mdt09.blv", "mdt15.blv", "mdt11.blv"};

bool BLVFace::Deserialize(BLVFace_MM7 *data) {
    memcpy(&this->pFacePlane, &data->pFacePlane, sizeof(this->pFacePlane));
    memcpy(&this->pFacePlane_old, &data->pFacePlane_old,
           sizeof(this->pFacePlane_old));
    this->zCalc1 = data->zCalc1;
    this->zCalc2 = data->zCalc2;
    this->zCalc3 = data->zCalc3;
    this->uAttributes = data->uAttributes;
    this->pVertexIDs = (uint16_t *)data->pVertexIDs;
    this->pXInterceptDisplacements = (int16_t *)data->pXInterceptDisplacements;
    this->pYInterceptDisplacements = (int16_t *)data->pYInterceptDisplacements;
    this->pZInterceptDisplacements = (int16_t *)data->pZInterceptDisplacements;
    this->pVertexUIDs = (int16_t *)data->pVertexUIDs;
    this->pVertexVIDs = (int16_t *)data->pVertexVIDs;
    this->uFaceExtraID = data->uFaceExtraID;
    // unsigned __int16  uBitmapID;
    this->uSectorID = data->uSectorID;
    this->uBackSectorID = data->uBackSectorID;
    memcpy(&this->pBounding, &data->pBounding, sizeof(this->pBounding));
    this->uPolygonType = (PolygonType)data->uPolygonType;
    this->uNumVertices = data->uNumVertices;
    this->field_5E = data->field_5E;
    this->field_5F = data->field_5F;

    return true;
}

//----- (0043F39E) --------------------------------------------------------
void PrepareDrawLists_BLV() {
    int TorchLightPower;           // eax@4
    // unsigned int v7;  // ebx@8
    BLVSector *v8;    // esi@8

    pBLVRenderParams->Reset();
    pMobileLightsStack->uNumLightsActive = 0;
    // uNumMobileLightsApplied = 0;
    uNumDecorationsDrawnThisFrame = 0;
    uNumSpritesDrawnThisFrame = 0;
    uNumBillboardsToDraw = 0;

    if (!_4D864C_force_sw_render_rules || !engine->config->TorchlightEffect()) {  // lightspot around party
        TorchLightPower = 800;
        if (pParty->TorchlightActive()) {
            // max is 800 * torchlight
            // min is 800
            int MinTorch = TorchLightPower;
            int MaxTorch = TorchLightPower * pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;

            // TorchLightPower *= pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;  // 2,3,4
            int ran = rand();
            int mod = ((ran - (RAND_MAX * .4)) / 200);
            TorchLightPower = (pParty->TorchLightLastIntensity + mod);

            // clamp
            if (TorchLightPower < MinTorch)
                TorchLightPower = MinTorch;
            if (TorchLightPower > MaxTorch)
                TorchLightPower = MaxTorch;
        }

        pParty->TorchLightLastIntensity = TorchLightPower;

        //       double nexLightIntensity(lastIntensity)
        //   return clamp(0, 1, lastIntensity + (rand() - .3) / 100)

        pMobileLightsStack->AddLight(
            pIndoorCameraD3D->vPartyPos.x, pIndoorCameraD3D->vPartyPos.y,
            pIndoorCameraD3D->vPartyPos.z, pBLVRenderParams->uPartySectorID, TorchLightPower,
            floorf(pParty->flt_TorchlightColorR + 0.5f),
            floorf(pParty->flt_TorchlightColorG + 0.5f),
            floorf(pParty->flt_TorchlightColorB + 0.5f), _4E94D0_light_type);
    }

    // PrepareBspRenderList_BLV();
    pIndoor->PrepareItemsRenderList_BLV();
    pIndoor->PrepareActorRenderList_BLV();

    // for (uint i = 0; i < pBspRenderer->uNumVisibleNotEmptySectors; ++i) {
    //     v7 = pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i];
    //     v8 = &pIndoor->pSectors[pBspRenderer->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[i]];

    for (int i = 1; i < pIndoor->uNumSectors; i++) {
        v8 = &pIndoor->pSectors[i];
        for (uint j = 0; j < v8->uNumDecorations; ++j)
            pIndoor->PrepareDecorationsRenderList_BLV(v8->pDecorationIDs[j], i);
    }
        // }

    FindBillboardsLightLevels_BLV();
    // engine->PrepareBloodsplats();
}

//----- (004407D9) --------------------------------------------------------
void BLVRenderParams::Reset() {
    this->field_0_timer_ = pEventTimer->uTotalGameTimeElapsed;

    pIndoorCameraD3D->debug_flags = 0;
    if (viewparams->draw_sw_outlines)
        pIndoorCameraD3D->debug_flags |= BLV_RENDER_DRAW_SW_OUTLINES;
    if (viewparams->draw_d3d_outlines)
        pIndoorCameraD3D->debug_flags |= BLV_RENDER_DRAW_D3D_OUTLINES;

    // v2 = a2;
    // this->field_0_timer_ = a2->field_0_timer;
    // this->uFlags = a2->uFlags;
    // this->vPartyPos.x = a2->vPosition.x;
    // this->vPartyPos.y = a2->vPosition.y;
    // this->vPartyPos.z = a2->vPosition.z;
    // v4 = this->vPartyPos.z;
    // v5 = this->vPartyPos.y;
    // this->sPartyRotY = a2->sRotationZ;
    // v6 = this->vPartyPos.x;
    // this->sPartyRotX = a2->sRotationX;
    int v7 = pIndoor->GetSector(pIndoorCameraD3D->vPartyPos.x,
                            pIndoorCameraD3D->vPartyPos.y,
                            pIndoorCameraD3D->vPartyPos.z);
    this->uPartySectorID = v7;
    if (!v7) {
        __debugbreak();  // shouldnt happen, please provide savegame
        /*v8 = this->vPartyPos.z;
        this->vPartyPos.x = pParty->vPosition.x;
        v9 = pParty->vPosition.y;
        v10 = this->vPartyPos.x;
        this->vPartyPos.y = pParty->vPosition.y;*/
        // this->uPartySectorID = pIndoor->GetSector(
        //    pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);
    }
    // if ( render->pRenderD3D )
    {
        this->fov = pViewport->field_of_view;

        this->uViewportX = pViewport->uScreen_TL_X;
        this->uViewportY = pViewport->uScreen_TL_Y;
        this->uViewportZ = pViewport->uScreen_BR_X;
        this->uViewportW = pViewport->uScreen_BR_Y;

        this->uViewportWidth = uViewportZ - uViewportX + 1;
        this->uViewportHeight = uViewportW - uViewportY + 1;
        this->uViewportCenterX = (uViewportZ + uViewportX) / 2;
        this->uViewportCenterY = (uViewportY + uViewportW) / 2;
    }


    this->uTargetWidth = window->GetWidth();
    this->uTargetHeight = window->GetHeight();
    this->pTargetZBuffer = render->pActiveZBuffer;
    this->field_8C = 0;
    this->field_84 = 0;
    this->uNumFacesRenderedThisFrame = 0;
    this->field_88 = 0;
    pBLVRenderParams->field_90 = 64;
    pBLVRenderParams->field_94 = 6;
}

//----- (00440B44) --------------------------------------------------------
void IndoorLocation::DrawIndoorFaces(bool bD3D) {
    // sector list checks until triangle batching is done

    int sectorcount = 0;
    int sectorslist[50];

    sectorslist[0] = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z) + 1024;
    if ((sectorslist[0] & 1023) == 0) __debugbreak();
    sectorcount++;

    // loop through all faces
    for (int test = 0; test < pIndoor->uNumFaces; test++) {
        // only want portals
        if (!pIndoor->pFaces[test].Portal()) continue;

        // loop through our sector list
        for (int check = 0; check < sectorcount; check++) {
            // grab sector and level from list
            int whichsector = sectorslist[check] & 1023;
            int nodelevel = sectorslist[check] >> 10;

            // only from first and second level
            if (nodelevel == 3) continue;

            // check agsint current portal
            if (pIndoor->pFaces[test].uSectorID == whichsector) {
                if (pIndoor->pFaces[test].uBackSectorID) {
                    // add backsector to list
                    int toadd = pIndoor->pFaces[test].uBackSectorID;

                    // cehck to add is not already on the list
                    bool onlist = false;
                    for (int listcheck = 0; listcheck < sectorcount; listcheck++) {
                        if ((sectorslist[listcheck] & 1023) == toadd) {
                            onlist = true;
                            break;
                        }
                    }

                    // add onto list with level masked
                    if (onlist == false) {
                        sectorslist[sectorcount] = (toadd + (1024*(nodelevel+1)));
                        sectorcount++;

                        test = 0;

                        break;
                    }
                }
            } else if (pIndoor->pFaces[test].uBackSectorID == whichsector) {
                if (pIndoor->pFaces[test].uSectorID) {
                    // add front sector to list

                    int toadd2 = pIndoor->pFaces[test].uSectorID;

                    // cehck to add is not already on the list
                    bool onlist2 = false;
                    for (int listcheck = 0; listcheck < sectorcount; listcheck++) {
                        if ((sectorslist[listcheck] & 1023) == toadd2) {
                            onlist2 = true;
                            break;
                        }
                    }

                    if (onlist2 == false) {
                        sectorslist[sectorcount] = (toadd2 + (1024 * (nodelevel + 1)));
                        sectorcount++;

                        test = 0;

                        break;
                    }
                }
            }
        }
    }



    // if (bD3D) {
        // pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);

        for (int test = 0; test < pIndoor->uNumFaces; test++) {
            for (int check = 0; check < sectorcount; check++) {
                if (pIndoor->pFaces[test].uSectorID == (sectorslist[check] & 1023)) {
                    IndoorLocation::ExecDraw_d3d(test, nullptr, 4, nullptr);
                }
            }
        }
        // pFace = &pIndoor->pFaces[uFaceID];

        return;

    //    for (uint i = 0; i < pBspRenderer->num_faces; ++i) {
    //        if (pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID]
    //                .viewing_portal_id == -1)
    //            IndoorLocation::ExecDraw_d3d(pBspRenderer->faces[i].uFaceID,
    //                                         nullptr, 4, nullptr);
    //        else
    //            IndoorLocation::ExecDraw_d3d(
    //                pBspRenderer->faces[i].uFaceID,
    //                pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID]
    //                    .std__vector_0007AC,
    //                4,
    //                pBspRenderer->nodes[pBspRenderer->faces[i].uNodeID]
    //                    .pPortalBounding);
    //    }
    //} else {
    //    for (uint j = 0; j < pBspRenderer->num_faces; ++j) {
    //        __debugbreak();  // no SW
    //        // pBLVRenderParams->field_7C =
    //        // &pBspRenderer->nodes[pBspRenderer->faces[j].uNodeID].PortalScreenData;
    //        // IndoorLocation::ExecDraw_sw(pBspRenderer->faces[j].uFaceID);
    //    }
    //}
}



//----- (00441BD4) --------------------------------------------------------
void IndoorLocation::Draw() {
    // int v0; // eax@1
    // IndoorLocation_drawstru _this; // [sp+0h] [bp-4Ch]@5
    //  int v2; // [sp+44h] [bp-8h]@5
    //  int v3; // [sp+48h] [bp-4h]@5

    /*_this.uFlags = 0;
    if (viewparams->draw_sw_outlines)
      _this.uFlags |= BLV_RENDER_DRAW_SW_OUTLINES;
    if (viewparams->draw_d3d_outlines)
      _this.uFlags |= BLV_RENDER_DRAW_D3D_OUTLINES;

    _this.uFlags |= BLV_RENDER_DRAW_SW_OUTLINES;
    _this.uFlags |= BLV_RENDER_DRAW_D3D_OUTLINES;

    _this.field_0_timer = pEventTimer->uTotalGameTimeElapsed;
    //_this.fov_deg = 65;
    _this.uViewportX = pViewport->uScreen_TL_X;
    _this.uViewportY = pViewport->uScreen_TL_Y;
    _this.uViewportZ = pViewport->uScreen_BR_X;
    _this.uViewportW = pViewport->uScreen_BR_Y;
    _this.field_3C = pViewport->field_30;

    _this.uTargetWidth = 640;
    _this.uTargetHeight = 480;
    _this.pTargetZ = render->pActiveZBuffer;*/

    // sub_440BED(&_this); -- inlined
    //{



    PrepareDrawLists_BLV();
    if (pBLVRenderParams->uPartySectorID)
        DrawIndoorFaces(true /*render->pRenderD3D != 0*/);
    render->DrawBillboardList_BLV();
    //}

    pParty->uFlags &= ~PARTY_FLAGS_1_0002;
    engine->DrawParticles();
    trail_particle_generator.UpdateParticles();
}

//----- (004C0EF2) --------------------------------------------------------
void BLVFace::FromODM(ODMFace *face) {
    this->pFacePlane_old.vNormal.x = face->pFacePlane.vNormal.x;
    this->pFacePlane_old.vNormal.y = face->pFacePlane.vNormal.y;
    this->pFacePlane_old.vNormal.z = face->pFacePlane.vNormal.z;
    this->pFacePlane_old.dist = face->pFacePlane.dist;
    this->pFacePlane.vNormal.x = (double)(face->pFacePlane.vNormal.x) / 65536.0;
    this->pFacePlane.vNormal.y = (double)(face->pFacePlane.vNormal.y) / 65536.0;
    this->pFacePlane.vNormal.z = (double)(face->pFacePlane.vNormal.z) / 65536.0;
    this->pFacePlane.dist = (double)(face->pFacePlane.dist) / 65536.0;
    this->uAttributes = face->uAttributes;
    this->pBounding.x1 = face->pBoundingBox.x1;
    this->pBounding.y1 = face->pBoundingBox.y1;
    this->pBounding.z1 = face->pBoundingBox.z1;
    this->pBounding.x2 = face->pBoundingBox.x2;
    this->pBounding.y2 = face->pBoundingBox.y2;
    this->pBounding.z2 = face->pBoundingBox.z2;
    this->zCalc1 = face->zCalc1;
    this->zCalc2 = face->zCalc2;
    this->zCalc3 = face->zCalc3;
    this->pXInterceptDisplacements = face->pXInterceptDisplacements;
    this->pYInterceptDisplacements = face->pYInterceptDisplacements;
    this->pZInterceptDisplacements = face->pZInterceptDisplacements;
    this->uPolygonType = (PolygonType)face->uPolygonType;
    this->uNumVertices = face->uNumVertices;
    this->resource = face->resource;
    this->pVertexIDs = face->pVertexIDs;
}

//----- (004B0A25) --------------------------------------------------------
void IndoorLocation::ExecDraw_d3d(unsigned int uFaceID,
                                  IndoorCameraD3D_Vec4 *pVertices,
                                  unsigned int uNumVertices,
                                  RenderVertexSoft *pPortalBounding) {
    int ColourMask;  // ebx@25
    // IDirect3DTexture2 *v27; // eax@42
    unsigned int uNumVerticesa;  // [sp+24h] [bp-4h]@17
    int LightLevel;                     // [sp+34h] [bp+Ch]@25

    if (uFaceID >= pIndoor->uNumFaces)
        return;

    static RenderVertexSoft static_vertices_buff_in[64];  // buff in
    static RenderVertexSoft static_vertices_calc_out[64];  // buff out - calc portal shape

    static stru154 FacePlaneHolder;  // idb


    BLVFace *pFace = &pIndoor->pFaces[uFaceID];

    if (pFace->Portal()) {
        // pIndoorCameraD3D->DebugDrawPortal(pFace);
        return;
    }

    if (pFace->uNumVertices < 3) return;

    if (pFace->Invisible()) {
        return;
    }

    ++pBLVRenderParams->uNumFacesRenderedThisFrame;

    int xd = pParty->vPosition.x - pIndoor->pVertices[pFace->pVertexIDs[0]].x;
    int yd = pParty->vPosition.y - pIndoor->pVertices[pFace->pVertexIDs[0]].y;
    int zd = pParty->vPosition.z - pIndoor->pVertices[pFace->pVertexIDs[0]].z;

    int dist = sqrt(xd * xd + yd * yd + zd * zd);

    // check against adjacent sectors and any vertex
    if (/*(pBLVRenderParams->uPartySectorID == pFace->uSectorID || pBLVRenderParams->uPartySectorID == pFace->uBackSectorID) && */(dist < 2000)) {
        pFace->uAttributes |= FACE_RENDERED;
    } else {
        // return;
    }

    if (!pFace->GetTexture()) {
        return;
    }

    if (!pIndoorCameraD3D->IsCulled(pFace)) {
        uNumVerticesa = pFace->uNumVertices;

        // copy to buff in
        for (uint i = 0; i < pFace->uNumVertices; ++i) {
            static_vertices_buff_in[i].vWorldPosition.x =
                pIndoor->pVertices[pFace->pVertexIDs[i]].x;
            static_vertices_buff_in[i].vWorldPosition.y =
                pIndoor->pVertices[pFace->pVertexIDs[i]].y;
            static_vertices_buff_in[i].vWorldPosition.z =
                pIndoor->pVertices[pFace->pVertexIDs[i]].z;
            static_vertices_buff_in[i].u = (signed short)pFace->pVertexUIDs[i];
            static_vertices_buff_in[i].v = (signed short)pFace->pVertexVIDs[i];
        }

        // 498377 always true - appears to be anothe function to clip vertices to portal planes??
        if (!pVertices || true/*(engine->pStru9Instance->_498377(pPortalBounding, 4, pVertices, static_vertices_buff_in, &uNumVerticesa), uNumVerticesa)*/) {
            if (pIndoorCameraD3D->CullFaceToFrustum(  // clips vertices to the frustum planes
                    static_vertices_buff_in, &uNumVerticesa,
                    static_vertices_calc_out,
                    pIndoorCameraD3D->FrustumPlanes, 4,
                    false, 0) != 1 || uNumVerticesa) {
                LightLevel = HEXRAYS_SHIWORD(Lights.uCurrentAmbientLightLevel);
                ColourMask =
                    (248 -
                     (HEXRAYS_SHIWORD(Lights.uCurrentAmbientLightLevel) << 3)) |
                    (((248 - (HEXRAYS_SHIWORD(Lights.uCurrentAmbientLightLevel)
                              << 3)) |
                      ((248 - (HEXRAYS_SHIWORD(Lights.uCurrentAmbientLightLevel)
                               << 3))
                       << 8))
                     << 8);

                FaceFlowTextureOffset(uFaceID);

                lightmap_builder->ApplyLights_IndoorFace(uFaceID);
                decal_builder->ApplyBloodsplatDecals_IndoorFace(uFaceID);

                pIndoorCameraD3D->ViewTransfrom_OffsetUV(static_vertices_calc_out, uNumVerticesa, array_507D30, &Lights);

                pIndoorCameraD3D->Project(array_507D30, uNumVerticesa, 0);

                lightmap_builder->StationaryLightsCount = 0;
                if (Lights.uNumLightsApplied > 0 || decal_builder->uNumDecals > 0) {
                    FacePlaneHolder.face_plane.vNormal.x =
                        pFace->pFacePlane.vNormal.x;
                    FacePlaneHolder.polygonType = pFace->uPolygonType;
                    FacePlaneHolder.face_plane.vNormal.y =
                        pFace->pFacePlane.vNormal.y;
                    FacePlaneHolder.face_plane.vNormal.z =
                        pFace->pFacePlane.vNormal.z;
                    FacePlaneHolder.face_plane.dist = pFace->pFacePlane.dist;
                }

                if (Lights.uNumLightsApplied > 0 && !pFace->Indoor_sky())  // for torchlight(для света факелов)
                    lightmap_builder->ApplyLights(&Lights, &FacePlaneHolder, uNumVerticesa, array_507D30, /*pVertices*/0, 0);

                if (decal_builder->uNumDecals > 0)  // blood draw
                    decal_builder->ApplyDecals(LightLevel, 1, &FacePlaneHolder,
                                               uNumVerticesa, array_507D30,
                                               pVertices, 0, pFace->uSectorID);

                Texture *face_texture = pFace->GetTexture();
                if (pFace->Fluid()) {
                    // if (pFace->uBitmapID == render->hd_water_tile_id)
                    //    v27 =
                    //    pBitmaps_LOD->pHardwareTextures[render->pHDWaterBitmapIDs[render->hd_water_current_frame]];
                    // else
                    {
                        face_texture = (Texture *)pFace->resource;
                        // auto v24 = GetTickCount() / 4;
                        // auto v25 = v24 - TrigLUT->uIntegerHalfPi;
                        uint eightSeconds = OS_GetTime() % 8000;
                        float angle = (eightSeconds / 8000.0f) * 2 * 3.1415f;

                        // animte lava back and forth
                        for (uint i = 0; i < uNumVerticesa; ++i)
                            // array_507D30[i].v +=
                            // (double)(pBitmaps_LOD->pTextures[pFace->uBitmapID].uHeightMinus1
                            // & (unsigned int)(TrigLUT->SinCos(v25) >> 8));
                            array_507D30[i].v +=
                                (face_texture->GetHeight() - 1) * cosf(angle);
                    }
                } else if (pFace->IsTextureFrameTable()) {
                    face_texture = pTextureFrameTable->GetFrameTexture(
                            (int64_t)pFace->resource, pBLVRenderParams->field_0_timer_);
                } else {
                    ColourMask = 0xFF808080;
                    // v27 = pBitmaps_LOD->pHardwareTextures[pFace->uBitmapID];
                }

                if (pFace->Indoor_sky()) {
                    render->DrawIndoorSky(uNumVerticesa, uFaceID);
                } else {
                    // if (!(pFace->uAttributes & FACE_OUTLINED)) {
                        // __debugbreak();
                        render->DrawIndoorPolygon(uNumVerticesa, pFace, PID(OBJECT_BModel, uFaceID), ColourMask, 0);
                    // }
                    // }
                }
                return;
            }
        }
    }
}

//----- (004B0E07) --------------------------------------------------------
unsigned int FaceFlowTextureOffset(unsigned int uFaceID) {  // time texture offset
    Lights.pDeltaUV[0] = pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID].sTextureDeltaU;
    Lights.pDeltaUV[1] = pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID].sTextureDeltaV;

    unsigned int offset = OS_GetTime() >> 3;

    if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FLOW_DIAGONAL) {
        Lights.pDeltaUV[1] -= offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetHeight() - 1);
    } else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FLOW_VERTICAL) {
        Lights.pDeltaUV[1] += offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetHeight() - 1);
    }

    if (pIndoor->pFaces[uFaceID].uAttributes & FACE_FLOW_HORIZONTAL) {
        Lights.pDeltaUV[0] -= offset & (((Texture *)pIndoor->pFaces[uFaceID].resource)->GetWidth() - 1);
    } else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_DONT_CACHE_TEXTURE) {
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

void BLVFace::SetTexture(const String &filename) {
    if (this->IsTextureFrameTable()) {
        this->resource =
            (void *)pTextureFrameTable->FindTextureByName(filename.c_str());
        if (this->resource != (void *)-1) {
            return;
        }

        this->ToggleIsTextureFrameTable();
    }

    this->resource = assets->GetBitmap(filename);
}

//----- (00498B15) --------------------------------------------------------
void IndoorLocation::Release() {
    free(this->ptr_0002B4_doors_ddata);
    this->ptr_0002B4_doors_ddata = NULL;

    free(this->ptr_0002B0_sector_rdata);
    this->ptr_0002B0_sector_rdata = NULL;

    free(this->ptr_0002B8_sector_lrdata);
    this->ptr_0002B8_sector_lrdata = NULL;

    free(this->pLFaces);
    this->pLFaces = NULL;

    free(this->pSpawnPoints);
    this->pSpawnPoints = NULL;

    this->uNumSectors = 0;
    this->uNumFaces = 0;
    this->uNumVertices = 0;
    this->uNumNodes = 0;
    this->uNumDoors = 0;
    this->uNumLights = 0;

    free(this->pVertices);
    this->pVertices = NULL;

    //  pfaces alloc by new during load
    //  free(this->pFaces);
    delete[] this->pFaces;
    this->pFaces = NULL;

    free(this->pFaceExtras);
    this->pFaceExtras = NULL;

    free(this->pSectors);
    this->pSectors = NULL;

    free(this->pLights);
    this->pLights = NULL;

    free(this->pDoors);
    this->pDoors = NULL;

    free(this->pNodes);
    this->pNodes = NULL;

    free(this->pMapOutlines);
    this->pMapOutlines = NULL;

    this->bLoaded = 0;
}

//----- (00498C45) --------------------------------------------------------
bool IndoorLocation::Alloc() {
    pVertices = (Vec3_short_ *)malloc(15000 * sizeof(Vec3_short_));  // 0x15F90u

    //  pfaces alloc by new during load
    //  pFaces = (BLVFace *)malloc(10000 * sizeof(BLVFace));             // 0xEA600u

    pFaceExtras = (BLVFaceExtra *)malloc(5000 * sizeof(BLVFaceExtra));     // 0x2BF20u
    pSectors = (BLVSector *)malloc(512 * sizeof(BLVSector));     // 0xE800u
    pLights = (BLVLightMM7 *)malloc(400 * sizeof(BLVLightMM7));  // 0x1900u
    pDoors = (BLVDoor *)malloc(200 * sizeof(BLVDoor));           // 0x3E80u
    pNodes = (BSPNode *)malloc(5000 * sizeof(BSPNode));          // 0x9C40u
    pMapOutlines = (BLVMapOutlines *)malloc(sizeof(BLVMapOutlines));  // 0x14824u
    if (pVertices /*&& pFaces*/ && pFaceExtras && pSectors && pLights && pDoors &&
        pNodes && pMapOutlines) {
        memset(pVertices, 0, 15000 * sizeof(Vec3_short_));
        //  memset(pFaces, 0, 10000 * sizeof(BLVFace));
        memset(pFaceExtras, 0, 5000 * sizeof(BLVFaceExtra));
        memset(pSectors, 0, 512 * sizeof(BLVSector));
        memset(pLights, 0, 400 * sizeof(BLVLightMM7));
        memset(pDoors, 0, 200 * sizeof(BLVDoor));
        memset(pNodes, 0, 5000 * sizeof(BSPNode));
        memset(pMapOutlines, 0, sizeof(BLVMapOutlines));
        return true;
    } else {
        __debugbreak();
        return false;
    }
}

//----- (00444810) --------------------------------------------------------
unsigned int IndoorLocation::GetLocationIndex(const char *Str1) {
    for (uint i = 0; i < 11; ++i)
        if (!_stricmp(Str1, _4E6BDC_loc_names[i])) return i + 1;
    return 0;
}

//----- (004488F7) --------------------------------------------------------
void IndoorLocation::ToggleLight(signed int sLightID, unsigned int bToggle) {
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor &&
        (sLightID <= pIndoor->uNumLights - 1) && (sLightID >= 0)) {
        if (bToggle)
            pIndoor->pLights[sLightID].uAtributes &= 0xFFFFFFF7;
        else
            pIndoor->pLights[sLightID].uAtributes |= 8;
        pParty->uFlags |= PARTY_FLAGS_1_0002;
    }
}

//----- (00498E0A) --------------------------------------------------------
bool IndoorLocation::Load(const String &filename, int num_days_played,
                          int respawn_interval_days, char *pDest) {
    decal_builder->Reset(0);

    _6807E0_num_decorations_with_sounds_6807B8 = 0;

    if (bLoaded) {
        log->Warning("BLV is already loaded");
        return true;
    }

    auto blv_filename = String(filename);
    blv_filename.replace(blv_filename.length() - 4, 4, ".blv");

    this->filename = String(filename);
    if (!pGames_LOD->DoesContainerExist(blv_filename)) {
        Error("Unable to find %s in Games.LOD", blv_filename.c_str());
    }

    Release();
    if (!Alloc())
        return false;

    size_t blv_size = 0;
    void *rawData = pGames_LOD->LoadCompressed(blv_filename, &blv_size);
    char *pData = (char*)rawData;

    bLoaded = true;

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&blv, pData, sizeof(BLVHeader));
    pData += sizeof(BLVHeader);
    memcpy(&uNumVertices, pData, 4);
    pData += 4;
    memcpy(pVertices, pData, uNumVertices * sizeof(Vec3_short_));

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumFaces, pData += uNumVertices * sizeof(Vec3_short_), 4);
    pData += 4;

    pGameLoadingUI_ProgressBar->Progress();

    // memcpy(pFaces, pData, uNumFaces * sizeof(BLVFace));
    auto face_data = (BLVFace_MM7 *)pData;
    pFaces = new BLVFace[uNumFaces];
    for (unsigned int i = 0; i < uNumFaces; ++i) {
        pFaces[i].Deserialize(face_data);
        face_data++;
    }

    pLFaces = (unsigned __int16 *)malloc(blv.uFaces_fdata_Size);
    memcpy(pLFaces, pData += uNumFaces * sizeof(BLVFace_MM7), blv.uFaces_fdata_Size);

    for (uint i = 0, j = 0; i < uNumFaces; ++i) {
        BLVFace *pFace = &pFaces[i];

        pFace->pVertexIDs = &pLFaces[j];

        j += pFace->uNumVertices + 1;
        pFace->pXInterceptDisplacements = (short *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pYInterceptDisplacements = (short *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pZInterceptDisplacements = (short *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pVertexUIDs = (__int16 *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
        pFace->pVertexVIDs = (__int16 *)(&pLFaces[j]);

        j += pFace->uNumVertices + 1;
    }

    pGameLoadingUI_ProgressBar->Progress();

    pData += blv.uFaces_fdata_Size;

    for (uint i = 0; i < uNumFaces; ++i) {
        BLVFace *pFace = &pFaces[i];

        char pTexName[16];
        strncpy(pTexName, pData, 10);
        pData += 10;

        pFace->SetTexture(String(pTexName));
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumFaceExtras, pData, 4);
    memcpy(pFaceExtras, pData += 4, uNumFaceExtras * sizeof(BLVFaceExtra));
    pData += uNumFaceExtras * sizeof(BLVFaceExtra);

    pGameLoadingUI_ProgressBar->Progress();

    // v108 = (char *)v107 + 36 * uNumFaceExtras;
    // v245 = 0;
    // *(int *)((char *)&uSourceLen + 1) = 0;
    for (uint i = 0; i < uNumFaceExtras; ++i) {
        char pTexName[32];
        strncpy(pTexName, pData, 10);
        pData += 10;

        if (!strcmp(pTexName, ""))
            pFaceExtras[i].uAdditionalBitmapID = -1;
        else
            pFaceExtras[i].uAdditionalBitmapID =
                pBitmaps_LOD->LoadTexture(pTexName);
    }

    for (uint i = 0; i < uNumFaces; ++i) {
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

    memcpy(&uNumSectors, pData, 4);

    // memcpy(pSectors, pData + 4, uNumSectors * sizeof(BLVSector));

    BLVSector_MM7 *tmp_sector = (BLVSector_MM7 *)malloc(sizeof(BLVSector_MM7));
    for (int i = 0; i < uNumSectors; ++i) {
        memcpy(tmp_sector, pData + 4 + i * sizeof(BLVSector_MM7), sizeof(BLVSector_MM7));
        tmp_sector->Deserialize(&pSectors[i]);
    }
    free(tmp_sector);

    pData += 4 + uNumSectors * sizeof(BLVSector_MM7);

    pGameLoadingUI_ProgressBar->Progress();

    ptr_0002B0_sector_rdata =
        (unsigned short *)malloc(blv.uSector_rdata_Size);  //, "L.RData");
    memcpy(ptr_0002B0_sector_rdata, pData, blv.uSector_rdata_Size);
    pData += blv.uSector_rdata_Size;

    for (uint i = 0, j = 0; i < uNumSectors; ++i) {
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

    ptr_0002B8_sector_lrdata =
        (unsigned __int16 *)malloc(blv.uSector_lrdata_Size);  //, "L.RLData");
    memcpy(ptr_0002B8_sector_lrdata, pData, blv.uSector_lrdata_Size);
    pData += blv.uSector_lrdata_Size;

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0, j = 0; i < uNumSectors; ++i) {
        pSectors[i].pLights = ptr_0002B8_sector_lrdata + j;
        j += pSectors[i].uNumLights;
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumDoors, pData, 4);
    pData += 4;

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumLevelDecorations, pData, 4);
    memcpy(pLevelDecorations.data(), pData + 4,
           uNumLevelDecorations * sizeof(LevelDecoration));
    pData += 4 + uNumLevelDecorations * sizeof(LevelDecoration);

    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        pLevelDecorations[i].uDecorationDescID =
            pDecorationList->GetDecorIdByName(pData);

        pData += 32;
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumLights, pData, 4);
    memcpy(pLights, pData + 4, uNumLights * sizeof(BLVLightMM7));
    pData += 4 + uNumLights * sizeof(BLVLightMM7);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumNodes, pData, 4);
    memcpy(pNodes, pData + 4, uNumNodes * sizeof(BSPNode));
    pData += 4 + uNumNodes * sizeof(BSPNode);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumSpawnPoints, pData, 4);
    pSpawnPoints = (SpawnPointMM7 *)malloc(uNumSpawnPoints * sizeof(SpawnPointMM7));
    memcpy(pSpawnPoints, pData + 4, uNumSpawnPoints * sizeof(SpawnPointMM7));
    pData += 4 + uNumSpawnPoints * sizeof(SpawnPointMM7);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&pMapOutlines->uNumOutlines, pData, 4);
    memcpy(pMapOutlines->pOutlines, pData + 4, pMapOutlines->uNumOutlines * sizeof(BLVMapOutline));
    free(rawData);

    String dlv_filename = String(filename);
    dlv_filename.replace(dlv_filename.length() - 4, 4, ".dlv");

    bool bResetSpawn = false;
    size_t dlv_size = 0;
    rawData = pNew_LOD->LoadCompressed(dlv_filename, &dlv_size);
    if (rawData != nullptr) {
        pData = (char*)rawData;
        memcpy(&dlv, pData, sizeof(DDM_DLV_Header));
        pData += sizeof(DDM_DLV_Header);
    } else {
        bResetSpawn = true;
    }

    if (dlv.uNumFacesInBModels > 0) {
        if (dlv.uNumDecorations > 0) {
            if (dlv.uNumFacesInBModels != uNumFaces ||
                dlv.uNumDecorations != uNumLevelDecorations)
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

    char SavedOutlines[875];
    if (bResetSpawn || (bRespawnLocation || !dlv.uLastRepawnDay)) {
        if (bResetSpawn) {
            memset(SavedOutlines, 0, 875);
        } else if (bRespawnLocation || !dlv.uLastRepawnDay) {
            memcpy(SavedOutlines, pData, 875);
        }

        dlv.uLastRepawnDay = num_days_played;
        if (!bResetSpawn) ++dlv.uNumRespawns;
        *(int *)pDest = 1;

        pData = (char*)pGames_LOD->LoadCompressed(dlv_filename);
        pData += sizeof(DDM_DLV_Header);
    } else {
        *(int*)pDest = 0;
    }

    memcpy(_visible_outlines, pData, 875);
    pData += 875;

    if (*(int *)pDest) memcpy(_visible_outlines, SavedOutlines, 875);

    for (uint i = 0; i < pMapOutlines->uNumOutlines; ++i) {
        BLVMapOutline *pVertex = &pMapOutlines->pOutlines[i];
        if ((unsigned __int8)(1 << (7 - i % 8)) & _visible_outlines[i / 8])
            pVertex->uFlags |= 1;
    }

    for (uint i = 0; i < uNumFaces; ++i) {
        BLVFace *pFace = &pFaces[i];
        BLVFaceExtra *pFaceExtra = &pFaceExtras[pFace->uFaceExtraID];

        memcpy(&pFace->uAttributes, pData, 4);
        pData += 4;

        if (pFaceExtra->uEventID) {
            if (pFaceExtra->HasEventHint())
                pFace->uAttributes |= FACE_HAS_EVENT;
            else
                pFace->uAttributes &= ~FACE_HAS_EVENT;
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        memcpy(&pLevelDecorations[i].uFlags, pData, 2);
        pData += 2;
    }

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumActors, pData, 4);

    // memcpy(&pActors, pData + 4, uNumActors * sizeof(Actor));
    // pData += 4 + uNumActors * sizeof(Actor);
    Actor_MM7* tmp_actor = (Actor_MM7*)malloc(sizeof(Actor_MM7));

    for (int i = 0; i < uNumActors; ++i) {
        memcpy(tmp_actor, pData + 4 + i * sizeof(Actor_MM7), sizeof(Actor_MM7));
        tmp_actor->Deserialize(&pActors[i]);
    }
    free(tmp_actor);

    pData += 4 + uNumActors * sizeof(Actor_MM7);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&uNumSpriteObjects, pData, 4);
    memcpy(pSpriteObjects.data(), pData + 4,
           uNumSpriteObjects * sizeof(SpriteObject));
    pData += 4 + uNumSpriteObjects * sizeof(SpriteObject);

    pGameLoadingUI_ProgressBar->Progress();

    for (uint i = 0; i < uNumSpriteObjects; ++i) {
        if (pSpriteObjects[i].containing_item.uItemID && !(pSpriteObjects[i].uAttributes & 0x0100)) {
            pSpriteObjects[i].uType = (SPRITE_OBJECT_TYPE)pItemsTable->pItems[pSpriteObjects[i].containing_item.uItemID].uSpriteID;
            pSpriteObjects[i].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[i].uType);
        }
    }

    pGameLoadingUI_ProgressBar->Progress();

    pData = ChestsDeserialize(pData);

    pGameLoadingUI_ProgressBar->Progress();
    pGameLoadingUI_ProgressBar->Progress();

    memcpy(pDoors, pData, 0x3E80);
    pData += 0x3E80;

    // v201 = (const char *)blv.uDoors_ddata_Size;
    // v200 = (size_t)ptr_0002B4_doors_ddata;
    // v170 = malloc(ptr_0002B4_doors_ddata, blv.uDoors_ddata_Size, "L.DData");
    // v171 = blv.uDoors_ddata_Size;
    ptr_0002B4_doors_ddata = (uint16_t*)malloc(blv.uDoors_ddata_Size);  //, "L.DData");
    if (ptr_0002B4_doors_ddata == nullptr) {
        log->Warning("Malloc error");
        Error("Malloc");  // is this recoverable
    }

    memcpy(ptr_0002B4_doors_ddata, pData, blv.uDoors_ddata_Size);
    pData += blv.uDoors_ddata_Size;

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
        j += pDoor->field_48;

        pDoor->pDeltaUs = (short *)(&ptr_0002B4_doors_ddata[j]);
        j += pDoor->uNumFaces;

        pDoor->pDeltaVs = (short *)(&ptr_0002B4_doors_ddata[j]);
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

    memcpy(&stru_5E4C90_MapPersistVars, pData, 0xC8);
    pData += 0xC8;

    pGameLoadingUI_ProgressBar->Progress();

    memcpy(&stru1, pData, 0x38u);
    pData += 0x38;

    free(rawData);

    return 0;
}

int IndoorLocation::GetSector(int sX, int sY, int sZ) {
    // sx = input world XYZ s

    if (uCurrentlyLoadedLevelType != LEVEL_Indoor) return 0;
    if (uNumSectors < 2) {
        __debugbreak();
        return 0;
    }

     // holds faces the coords are above
    int FoundFaceStore[5] = { 0 };
    int NumFoundFaceStore = 0;

    // loop through sectors
    for (uint i = 1; i < uNumSectors; ++i) {
        if (NumFoundFaceStore >= 5) break;

        BLVSector *pSector = &pSectors[i];

        if ((pSector->pBounding.x1 - 5) > sX || (pSector->pBounding.x2 + 5) < sX ||
            (pSector->pBounding.y1 - 5) > sY || (pSector->pBounding.y2 + 5) < sY ||
            (pSector->pBounding.z1 - 64) > sZ || (pSector->pBounding.z2 + 64) < sZ)
            continue;  // outside sector bounding

        // logger->Warning("Sector[%u]", i);
        int FloorsAndPortals = pSector->uNumFloors + pSector->uNumPortals;

        // nothing in secotr to check against so skip
        if (!FloorsAndPortals) continue;

        // loop over check faces
        for (uint z = 0; z < FloorsAndPortals; ++z) {
            uint uFaceID;
            if (z < pSector->uNumFloors)
                uFaceID = pSector->pFloors[z];
            else
                uFaceID = pSector->pPortals[z - pSector->uNumFloors];

            BLVFace *pFace = &pFaces[uFaceID];
            if (pFace->uPolygonType != POLYGON_Floor &&
                pFace->uPolygonType != POLYGON_InBetweenFloorAndWall)
                continue;

            // classic crossing multiplication point in poly algorithm
            float pointy = sY;
            float pointx = sX;
            float pointsy[50] = {0};
            float pointsx[50] = {0};

            // use intercept displacements to create a slightly larger poly - this covers the gap in badly stitched triangles
            for (uint j = 0; j < pFace->uNumVertices; ++j) {
                pointsx[2 * j] =
                    pFace->pXInterceptDisplacements[j] + pIndoor->pVertices[pFace->pVertexIDs[j]].x;
                pointsx[2 * j + 1] =
                    pFace->pXInterceptDisplacements[j] + pIndoor->pVertices[pFace->pVertexIDs[j + 1]].x;
                pointsy[2 * j] =
                    pFace->pYInterceptDisplacements[j] + pIndoor->pVertices[pFace->pVertexIDs[j]].y;
                pointsy[2 * j + 1] =
                    pFace->pYInterceptDisplacements[j] + pIndoor->pVertices[pFace->pVertexIDs[j + 1]].y;
            }

            int ij, hj, nvert = pFace->uNumVertices * 2;
            bool c = false;

            for (ij = 0, hj = nvert - 1; ij < nvert; hj = ij++) {
                if (((pointsy[ij] > pointy) != (pointsy[hj] > pointy)) &&
                    (pointx < (pointsx[hj] - pointsx[ij]) * (pointy - pointsy[ij]) / (pointsy[hj] - pointsy[ij]) + pointsx[ij])
                    )
                    c = !c;
            }

            // add found faces into store
            if (pFace->uNumVertices && c) FoundFaceStore[NumFoundFaceStore++] = uFaceID;
            if (NumFoundFaceStore >= 5) break;
        }
    }

    // only one face found
    if (NumFoundFaceStore == 1) return this->pFaces[FoundFaceStore[0]].uSectorID;

    // No face found - outside of level
    if (!NumFoundFaceStore) {
        logger->Warning("Sector fail");
        return 0;
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
                CalcZDist = abs(sZ -
                    ((fixpoint_mul(this->pFaces[FoundFaceStore[s]].zCalc1, (sX << 16)) +
                      fixpoint_mul(this->pFaces[FoundFaceStore[s]].zCalc2, (sY << 16)) +
                      this->pFaces[FoundFaceStore[s]].zCalc3 + 0x8000) >> 16));
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
void BLVFace::_get_normals(Vec3_int_ *a2, Vec3_int_ *a3) {
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
            Vec3_float_ a1;
            a1.x = (double)-this->pFacePlane_old.vNormal.y;
            a1.y = (double)this->pFacePlane_old.vNormal.x;
            a1.z = 0.0;
            a1.Normalize();

            a2->x = (signed __int64)(a1.x * 65536.0);
            a2->y = (signed __int64)(a1.y * 65536.0);
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
    if (this->uAttributes & FACE_UNKNOW3) {
        a2->x = -a2->x;
        a2->y = -a2->y;
        a2->z = -a2->z;
    }
    if (this->uAttributes & FACE_UNKNOW4) {
        a3->x = -a3->x;
        a3->y = -a3->y;
        a3->z = -a3->z;
    }
    return;
}

bool BLVFaceExtra::HasEventHint() {
    int event_index = 0;
    if ((uLevelEVT_NumEvents - 1) <= 0) {
        return false;
    }
    while (pLevelEVT_Index[event_index].uEventID != this->uEventID) {
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
    BLVFace *face;       // ebx@24
    Vec3_short_ *v17;    // esi@24
    int v18;             // eax@24
    int v19;             // edx@24
    signed int v20;      // eax@24
    int v24;             // esi@25
    int v25;             // eax@25
    signed __int64 v27;  // qtt@27
    BLVFaceExtra *v28;   // esi@32
    int v32;             // eax@34
    Vec3_short_ *v34;    // eax@35
    int v35;             // ecx@35
    int v36;             // edx@35
    signed int v37;      // eax@35
    signed int v38;      // edx@35
    int v39;             // eax@35
    int v40;             // edx@35
    Vec3_short_ *v43;    // edi@36
    int v57;             // eax@58
    Vec3_int_ v67;
    Vec3_int_ v70;
    int v73;               // [sp+20h] [bp-44h]@24
    int v75;               // [sp+28h] [bp-3Ch]@36
    int v76;               // [sp+2Ch] [bp-38h]@36
    int v77;               // [sp+30h] [bp-34h]@36
    int v82;               // [sp+44h] [bp-20h]@35
    int v83;               // [sp+48h] [bp-1Ch]@34
    int v84;               // [sp+4Ch] [bp-18h]@34
         int v88;               // [sp+5Ch] [bp-8h]@18
    int v89;               // [sp+60h] [bp-4h]@6

    SoundID eDoorSoundID = (SoundID)pDoorSoundIDsByLocationID[dword_6BE13C_uCurrentlyLoadedLocationID];

    // loop over all doors
    for (uint i = 0; i < pIndoor->uNumDoors; ++i) {
        BLVDoor *door = &pIndoor->pDoors[i];

        // door not moving currently
        if (door->uState == BLVDoor::Closed || door->uState == BLVDoor::Open) {
            door->uAttributes &= 0xFFFFFFFDu;  // ~0x2
            continue;
        }

        door->uTimeSinceTriggered += pEventTimer->uTimeElapsed;
        if (door->uState == BLVDoor::Opening) {
            v89 = (signed int)(door->uTimeSinceTriggered * door->uCloseSpeed) / 128;
            if (v89 >= door->uMoveLength) {
                v89 = door->uMoveLength;
                door->uState = BLVDoor::Open;
                if (!(door->uAttributes & FACE_UNKNOW5) && door->uNumVertices != 0)
                    pAudioPlayer->PlaySound((SoundID)((int)eDoorSoundID + 1), PID(OBJECT_BLVDoor, i), 0, -1, 0, 0);
                // goto LABEL_18;
            } else if (!(door->uAttributes & FACE_UNKNOW5) && door->uNumVertices) {
                pAudioPlayer->PlaySound(eDoorSoundID, PID(OBJECT_BLVDoor, i), 1, -1, 0, 0);
            }
        } else {  // door closing
            signed int v5 =
                (signed int)(door->uTimeSinceTriggered * door->uOpenSpeed) / 128;
            if (v5 >= door->uMoveLength) {
                v89 = 0;
                door->uState = BLVDoor::Closed;
                if (!(door->uAttributes & FACE_UNKNOW5) &&
                    door->uNumVertices != 0)
                    pAudioPlayer->PlaySound((SoundID)((int)eDoorSoundID + 1),
                                            PID(OBJECT_BLVDoor, i), 0, -1, 0, 0);
                // goto LABEL_18;
            } else {
                v89 = door->uMoveLength - v5;
                if (!(door->uAttributes & FACE_UNKNOW5) && door->uNumVertices)
                    pAudioPlayer->PlaySound(eDoorSoundID,
                                            PID(OBJECT_BLVDoor, i), 1, -1, 0, 0);
            }
        }

        // adjust verts to how open the door is
        for (uint j = 0; j < door->uNumVertices; ++j) {
            pIndoor->pVertices[door->pVertexIDs[j]].x =
                fixpoint_mul(door->vDirection.x, v89) + door->pXOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].y =
                fixpoint_mul(door->vDirection.y, v89) + door->pYOffsets[j];
            pIndoor->pVertices[door->pVertexIDs[j]].z =
                fixpoint_mul(door->vDirection.z, v89) + door->pZOffsets[j];
        }


        for (v88 = 0; v88 < door->uNumFaces; ++v88) {
            face = &pIndoor->pFaces[door->pFaceIDs[v88]];
            v17 = &pIndoor->pVertices[face->pVertexIDs[0]];
            v18 = face->pFacePlane_old.vNormal.y;
            v73 = *(int *)&v17->x;
            v19 = face->pFacePlane_old.vNormal.z;
            v20 = -(v19 * (int)v17->z +
                    (signed __int16)v73 * face->pFacePlane_old.vNormal.x +
                    HEXRAYS_SHIWORD(v73) * v18);
            face->pFacePlane_old.dist = v20;
            face->pFacePlane.dist =
                -((double)v17->z * face->pFacePlane.vNormal.z +
                  (double)v17->y * face->pFacePlane.vNormal.y +
                  (double)v17->x * face->pFacePlane.vNormal.x);
            if (v19) {
                v24 = abs(v20 >> 15);
                v25 = abs(face->pFacePlane_old.vNormal.z);
                if (v24 > v25)
                    Error(
                        "Door Error\ndoor id: %i\nfacet no: %i\n\nOverflow "
                        "dividing facet->d [%i] by facet->nz [%i]",
                        door->uDoorID, door->pFaceIDs[v88],
                        face->pFacePlane_old.dist,
                        face->pFacePlane_old.vNormal.z);
                HEXRAYS_LODWORD(v27) = face->pFacePlane_old.dist << 16;
                HEXRAYS_HIDWORD(v27) = face->pFacePlane_old.dist >> 16;
                face->zCalc3 = -v27 / face->pFacePlane_old.vNormal.z;
            }
            // if ( face->uAttributes & FACE_TEXTURE_FLOW || render->pRenderD3D
            // )
            face->_get_normals(&v70, &v67);
            v28 = &pIndoor->pFaceExtras[face->uFaceExtraID];
            /*if ( !render->pRenderD3D )
            {
            if ( !(face->uAttributes & FACE_TEXTURE_FLOW) )
            continue;
            v83 = (unsigned __int64)(door->vDirection.x * (signed __int64)v70.x)
            >> 16; v85 = (unsigned __int64)(door->vDirection.y * (signed
            __int64)v70.y) >> 16; v84 = (unsigned __int64)(door->vDirection.z *
            (signed __int64)v70.z) >> 16; v29 = v89; v28->sTextureDeltaU =
            -((v83 + v85 + v84) * (signed __int64)v89) >> 16; v85 = (unsigned
            __int64)(door->vDirection.x * (signed __int64)v67.x) >> 16; v83 =
            (unsigned __int64)(door->vDirection.y * (signed __int64)v67.y) >>
            16; v84 = (unsigned __int64)(door->vDirection.z * (signed
            __int64)v67.z) >> 16; v31 = (v85 + v83 + v84) * (signed __int64)v29;
            v32 = v31 >> 16;
            v57 = -v32;
            v28->sTextureDeltaV = v57;
            v28->sTextureDeltaU += door->pDeltaUs[v88];
            v28->sTextureDeltaV = v57 + door->pDeltaVs[v88];
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
                v76 = ((__int64)v70.z * v43->z + (__int64)v70.x * v43->x +
                       (__int64)v70.y * v43->y) >>
                      16;
                v77 = ((__int64)v67.x * v43->x + (__int64)v67.y * v43->y +
                       (__int64)v43->z * v67.z) >>
                      16;
                if (v76 < v39) v39 = v76;
                if (v77 < v40) v40 = v77;
                if (v76 > v84) v84 = v76;
                if (v77 > v82) v82 = v77;
                face->pVertexUIDs[j] = v76;
                face->pVertexVIDs[j] = v77;
            }
            if (face->uAttributes & 0x00001000) {
                v28->sTextureDeltaU -= v39;
            } else {
                if (face->uAttributes & 0x8000) {
                    if (face->resource) {
                        // v28->sTextureDeltaU -= v84 +
                        // pBitmaps_LOD->pTextures[face->uBitmapID].uTextureWidth;
                        v28->sTextureDeltaU -=
                            v84 + ((Texture *)face->resource)->GetWidth();
                    }
                }
            }
            if (face->uAttributes & FACE_UNKNOW6) {
                v28->sTextureDeltaV -= v40;
            } else {
                if (face->uAttributes & FACE_INDOOR_DOOR) {
                    v28->sTextureDeltaV -=
                        v84 + ((Texture *)face->resource)->GetHeight();
                    // if (face->uBitmapID != -1)
                    //    v28->sTextureDeltaV -= v82 +
                    //    pBitmaps_LOD->GetTexture(face->uBitmapID)->uTextureHeight;
                }
            }
            if (face->uAttributes & FACE_TEXTURE_FLOW) {
                v84 = fixpoint_mul(door->vDirection.x, v70.x);
                v82 = fixpoint_mul(door->vDirection.y, v70.y);
                v83 = fixpoint_mul(door->vDirection.z, v70.z);
                v75 = v84 + v82 + v83;
                v82 = fixpoint_mul(v75, v89);
                v28->sTextureDeltaU = -v82;
                v84 = fixpoint_mul(door->vDirection.x, v67.x);
                v82 = fixpoint_mul(door->vDirection.y, v67.y);
                v83 = fixpoint_mul(door->vDirection.z, v67.z);
                v75 = v84 + v82 + v83;
                v32 = fixpoint_mul(v75, v89);
                v57 = -v32;
                v28->sTextureDeltaV = v57;
                v28->sTextureDeltaU += door->pDeltaUs[v88];
                v28->sTextureDeltaV = v57 + door->pDeltaVs[v88];
            }
        }
    }
}

//----- (0046F90C) --------------------------------------------------------
void UpdateActors_BLV() {
    int v2;                  // edi@6
    int v3;                  // eax@6
    int v4;                  // eax@8
    __int16 v5;              // ax@11
    signed int v6;           // ebx@14
    signed __int64 v10;      // qax@18
    int v22;                 // edi@46
    unsigned int v24;        // eax@51
    int v27;                 // ST08_4@54
    int v28;                 // edi@54
    int v29;                 // eax@54
    int v30;                 // ecx@62
    int v31;                 // ebx@62
    int v32;                 // eax@62
    int v33;                 // eax@64
    int v37;          // ebx@85
    int v44;                 // ecx@96
    int v45;                 // edi@101
    AIDirection v52;         // [sp+0h] [bp-60h]@75
    AIDirection v53;         // [sp+1Ch] [bp-44h]@116
    unsigned int uSectorID;  // [sp+3Ch] [bp-24h]@6
    int v56;                 // [sp+40h] [bp-20h]@6
    unsigned int _this;      // [sp+44h] [bp-1Ch]@51
    int v58;                 // [sp+48h] [bp-18h]@51
    int v59;                 // [sp+4Ch] [bp-14h]@8
    unsigned int uFaceID;    // [sp+50h] [bp-10h]@6
    int v61;                 // [sp+54h] [bp-Ch]@14
    int v62;                 // [sp+58h] [bp-8h]@6
    unsigned int actor_id;   // [sp+5Ch] [bp-4h]@1

    if (engine->config->no_actors)
        return;  // uNumActors = 0;

    for (actor_id = 0; actor_id < uNumActors; actor_id++) {
        if (pActors[actor_id].uAIState == Removed ||
            pActors[actor_id].uAIState == Disabled ||
            pActors[actor_id].uAIState == Summoned ||
            !pActors[actor_id].uMovementSpeed)
            continue;
        uSectorID = pActors[actor_id].uSectorID;
        v2 = collide_against_floor(
            pActors[actor_id].vPosition.x, pActors[actor_id].vPosition.y,
            pActors[actor_id].vPosition.z, &uSectorID, &uFaceID);
        pActors[actor_id].uSectorID = uSectorID;
        v3 = pActors[actor_id].pMonsterInfo.uFlying;
        v56 = v2;
        v62 = v3;
        if (!pActors[actor_id].CanAct()) v62 = 0;
        v4 = pActors[actor_id].vPosition.z;
        v59 = 0;
        if (pActors[actor_id].vPosition.z > v2 + 1) v59 = 1;
        if (v2 <= -30000) {
            v5 = pIndoor->GetSector(pActors[actor_id].vPosition.x,
                                    pActors[actor_id].vPosition.y, v4);
            pActors[actor_id].uSectorID = v5;
            v56 = BLV_GetFloorLevel(
                pActors[actor_id].vPosition.x, pActors[actor_id].vPosition.y,
                pActors[actor_id].vPosition.z, v5, &uFaceID);
            if (!v5 || v56 == -30000) continue;
        }
        if (pActors[actor_id].uCurrentActionAnimation ==
            ANIM_Walking) {  //монстр двигается
            v6 = pActors[actor_id].uMovementSpeed;
            if (pActors[actor_id].pActorBuffs[ACTOR_BUFF_SLOWED].Active()) {
                if (pActors[actor_id].pActorBuffs[ACTOR_BUFF_SLOWED].uPower)
                    HEXRAYS_LODWORD(v10) = pActors[actor_id].uMovementSpeed /
                                           (unsigned __int16)pActors[actor_id]
                                               .pActorBuffs[ACTOR_BUFF_SLOWED]
                                               .uPower;
                else
                    v10 = (signed __int64)((double)pActors[actor_id]
                                               .uMovementSpeed *
                                           0.5);
                v6 = v10;
            }
            if (pActors[actor_id].uAIState == Pursuing ||
                pActors[actor_id].uAIState == Fleeing)
                v6 *= 2;
            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_WAIT)
                v6 = (signed __int64)((double)v6 *
                                      flt_6BE3AC_debug_recmod1_x_1_6);
            if (v6 > 1000) v6 = 1000;
            pActors[actor_id].vVelocity.x =
                fixpoint_mul(TrigLUT->Cos(pActors[actor_id].uYawAngle), v6);
            pActors[actor_id].vVelocity.y =
                fixpoint_mul(TrigLUT->Sin(pActors[actor_id].uYawAngle), v6);
            if (v62)
                pActors[actor_id].vVelocity.z = fixpoint_mul(
                    TrigLUT->Sin(pActors[actor_id].uPitchAngle), v6);
        } else {  // actor is not moving(актор не двигается)
            pActors[actor_id].vVelocity.x =
                fixpoint_mul(55000, pActors[actor_id].vVelocity.x);
            pActors[actor_id].vVelocity.y =
                fixpoint_mul(55000, pActors[actor_id].vVelocity.y);
            if (v62)
                pActors[actor_id].vVelocity.z =
                    fixpoint_mul(55000, pActors[actor_id].vVelocity.z);
        }
        if (pActors[actor_id].vPosition.z <= v56) {
            pActors[actor_id].vPosition.z = v56 + 1;
            if (pIndoor->pFaces[uFaceID].uPolygonType == 3) {
                if (pActors[actor_id].vVelocity.z < 0)
                    pActors[actor_id].vVelocity.z = 0;
            } else {
                if (pIndoor->pFaces[uFaceID].pFacePlane_old.vNormal.z < 45000)
                    pActors[actor_id].vVelocity.z -=
                        (short)pEventTimer->uTimeElapsed * GetGravityStrength();
            }
        } else {
            if (v59 && !v62)
                pActors[actor_id].vVelocity.z +=
                    -8 * (short)pEventTimer->uTimeElapsed *
                    GetGravityStrength();
        }
        if (pActors[actor_id].vVelocity.x * pActors[actor_id].vVelocity.x +
                pActors[actor_id].vVelocity.y * pActors[actor_id].vVelocity.y +
                pActors[actor_id].vVelocity.z * pActors[actor_id].vVelocity.z >=
            400) {
            _actor_collision_struct.field_84 = -1;
            _actor_collision_struct.field_70 = 0;
            _actor_collision_struct.field_0 = 1;
            _actor_collision_struct.field_8_radius = pActors[actor_id].uActorRadius;
            _actor_collision_struct.prolly_normal_d = pActors[actor_id].uActorRadius;
            _actor_collision_struct.height = pActors[actor_id].uActorHeight;
            v22 = 0;
            for (uSectorID = 0; uSectorID < 100; uSectorID++) {
                _actor_collision_struct.position.x = pActors[actor_id].vPosition.x;
                _actor_collision_struct.normal.x = _actor_collision_struct.position.x;
                _actor_collision_struct.position.y = pActors[actor_id].vPosition.y;
                _actor_collision_struct.normal.y = _actor_collision_struct.position.y;
                _actor_collision_struct.normal.z = pActors[actor_id].vPosition.z +
                                       pActors[actor_id].uActorRadius + 1;
                _actor_collision_struct.position.z = pActors[actor_id].vPosition.z -
                                         pActors[actor_id].uActorRadius +
                                         _actor_collision_struct.height - 1;
                if (_actor_collision_struct.position.z < _actor_collision_struct.normal.z)
                    _actor_collision_struct.position.z = pActors[actor_id].vPosition.z +
                                             pActors[actor_id].uActorRadius + 1;
                _actor_collision_struct.velocity.x = pActors[actor_id].vVelocity.x;
                _actor_collision_struct.velocity.y = pActors[actor_id].vVelocity.y;
                _actor_collision_struct.velocity.z = pActors[actor_id].vVelocity.z;
                _actor_collision_struct.uSectorID = pActors[actor_id].uSectorID;
                if (!_actor_collision_struct.CalcMovementExtents(v22)) {
                    v58 = 0;
                    v24 = 8 * actor_id;
                    HEXRAYS_LOBYTE(v24) = PID(OBJECT_Actor, actor_id);
                    for (v61 = 0; v61 < 100; ++v61) {
                        _46E44E_collide_against_faces_and_portals(1);
                        _46E0B2_collide_against_decorations();
                        _46EF01_collision_chech_player(0);
                        _46ED8A_collide_against_sprite_objects(v24);
                        for (uint j = 0; j < ai_arrays_size; j++) {
                            if (ai_near_actors_ids[j] != actor_id) {
                                v27 = abs(
                                    pActors[ai_near_actors_ids[j]].vPosition.z -
                                    pActors[actor_id].vPosition.z);
                                v28 = abs(
                                    pActors[ai_near_actors_ids[j]].vPosition.y -
                                    pActors[actor_id].vPosition.y);
                                v29 = abs(
                                    pActors[ai_near_actors_ids[j]].vPosition.x -
                                    pActors[actor_id].vPosition.x);
                                if (int_get_vector_length(v29, v28, v27) >=
                                        pActors[actor_id].uActorRadius +
                                            (signed int)
                                                pActors[ai_near_actors_ids[j]]
                                                    .uActorRadius &&
                                    Actor::_46DF1A_collide_against_actor(
                                        ai_near_actors_ids[j], 40))
                                    ++v58;
                            }
                        }
                        if (_46F04E_collide_against_portals()) break;
                    }
                    v56 = v58 > 1;
                    if (_actor_collision_struct.field_7C >= _actor_collision_struct.field_6C) {
                        v30 = _actor_collision_struct.normal2.x;
                        v31 = _actor_collision_struct.normal2.y;
                        v32 = _actor_collision_struct.normal2.z -
                              _actor_collision_struct.prolly_normal_d - 1;
                    } else {
                        v30 = pActors[actor_id].vPosition.x +
                              fixpoint_mul(_actor_collision_struct.field_7C,
                                           _actor_collision_struct.direction.x);
                        v31 = pActors[actor_id].vPosition.y +
                              fixpoint_mul(_actor_collision_struct.field_7C,
                                           _actor_collision_struct.direction.y);
                        v32 = pActors[actor_id].vPosition.z +
                              fixpoint_mul(_actor_collision_struct.field_7C,
                                           _actor_collision_struct.direction.z);
                    }
                    v33 = collide_against_floor(
                        v30, v31, v32, &_actor_collision_struct.uSectorID, &uFaceID);
                    if (pIndoor->pFaces[uFaceID].uAttributes &
                            FACE_INDOOR_SKY &&
                        pActors[actor_id].uAIState == Dead) {
                        pActors[actor_id].uAIState = Removed;
                        continue;
                    }
                    if (v59 || v62 ||
                        !(pIndoor->pFaces[uFaceID].uAttributes &
                          FACE_INDOOR_SKY)) {
                        if (v33 == -30000) continue;
                        if (pActors[actor_id].uCurrentActionAnimation != 1 ||
                            v33 >= pActors[actor_id].vPosition.z - 100 || v59 ||
                            v62) {
                            if (_actor_collision_struct.field_7C < _actor_collision_struct.field_6C) {
                                pActors[actor_id].vPosition.x +=
                                    fixpoint_mul(_actor_collision_struct.field_7C,
                                                 _actor_collision_struct.direction.x);
                                pActors[actor_id].vPosition.y +=
                                    fixpoint_mul(_actor_collision_struct.field_7C,
                                                 _actor_collision_struct.direction.y);
                                pActors[actor_id].vPosition.z +=
                                    fixpoint_mul(_actor_collision_struct.field_7C,
                                                 _actor_collision_struct.direction.z);
                                pActors[actor_id].uSectorID =
                                    (short)_actor_collision_struct.uSectorID;
                                _actor_collision_struct.field_70 += _actor_collision_struct.field_7C;
                                v37 = PID_ID(_actor_collision_struct.pid);
                                if (PID_TYPE(_actor_collision_struct.pid) == OBJECT_Actor) {
                                    if (pParty->bTurnBasedModeOn &&
                                        (pTurnEngine->turn_stage == TE_ATTACK ||
                                         pTurnEngine->turn_stage == TE_MOVEMENT)) {
                                        pActors[actor_id].vVelocity.x =
                                            fixpoint_mul(58500, pActors[actor_id].vVelocity.x);
                                        pActors[actor_id].vVelocity.y =
                                            fixpoint_mul(58500, pActors[actor_id].vVelocity.y);
                                        pActors[actor_id].vVelocity.z =
                                            fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                        v22 = 0;
                                        continue;
                                    }
                                    if (pActors[actor_id]
                                            .pMonsterInfo.uHostilityType) {
                                        if (!v56) {
                                            Actor::AI_Flee(actor_id,
                                                           _actor_collision_struct.pid, v22,
                                                           (AIDirection *)v22);
                                            pActors[actor_id]
                                                .vVelocity.x = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.x);
                                            pActors[actor_id]
                                                .vVelocity.y = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.y);
                                            pActors[actor_id]
                                                .vVelocity.z = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                            v22 = 0;
                                            continue;
                                        }
                                    } else {
                                        if (!v56) {
                                            if (!pActors[v37]
                                                     .pMonsterInfo
                                                     .uHostilityType) {
                                                Actor::AI_FaceObject(
                                                    actor_id, _actor_collision_struct.pid,
                                                    v22, (AIDirection *)v22);
                                                pActors[actor_id].vVelocity.x =
                                                    fixpoint_mul(
                                                        58500,
                                                        pActors[actor_id]
                                                            .vVelocity.x);
                                                pActors[actor_id].vVelocity.y =
                                                    fixpoint_mul(
                                                        58500,
                                                        pActors[actor_id]
                                                            .vVelocity.y);
                                                pActors[actor_id].vVelocity.z =
                                                    fixpoint_mul(
                                                        58500,
                                                        pActors[actor_id]
                                                            .vVelocity.z);
                                                v22 = 0;
                                                continue;
                                            }
                                            Actor::AI_Flee(actor_id,
                                                           _actor_collision_struct.pid, v22,
                                                           (AIDirection *)v22);
                                            pActors[actor_id]
                                                .vVelocity.x = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.x);
                                            pActors[actor_id]
                                                .vVelocity.y = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.y);
                                            pActors[actor_id]
                                                .vVelocity.z = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                            v22 = 0;
                                            continue;
                                        }
                                    }
                                    Actor::AI_StandOrBored(actor_id, 4, v22,
                                                           &v53);
                                    pActors[actor_id].vVelocity.x =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.x);
                                    pActors[actor_id].vVelocity.y =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.y);
                                    pActors[actor_id].vVelocity.z =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.z);
                                    v22 = 0;
                                    continue;
                                }
                                if (PID_TYPE(_actor_collision_struct.pid) ==
                                    OBJECT_Player) {
                                    if (pActors[actor_id].GetActorsRelation(
                                            0)) {
                                        // v51 =
                                        // __OFSUB__(HIDWORD(pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].uExpireTime),
                                        // v22); v49 =
                                        // HIDWORD(pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].uExpireTime)
                                        // == v22; v50 =
                                        // HIDWORD(pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].uExpireTime)
                                        // - v22 < 0;
                                        pActors[actor_id].vVelocity.y = 0;
                                        pActors[actor_id].vVelocity.x = 0;
                                        if (pParty
                                                ->pPartyBuffs
                                                    [PARTY_BUFF_INVISIBILITY]
                                                .Active()) {
                                            pParty
                                                ->pPartyBuffs
                                                    [PARTY_BUFF_INVISIBILITY]
                                                .Reset();
                                        }

                                        viewparams->bRedrawGameUI = 1;
                                        pActors[actor_id].vVelocity.x =
                                            fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.x);
                                        pActors[actor_id].vVelocity.y =
                                            fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.y);
                                        pActors[actor_id].vVelocity.z =
                                            fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                        continue;
                                    }
                                    Actor::AI_FaceObject(actor_id,
                                                         _actor_collision_struct.pid, v22,
                                                         (AIDirection *)v22);
                                    pActors[actor_id].vVelocity.x =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.x);
                                    pActors[actor_id].vVelocity.y =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.y);
                                    pActors[actor_id].vVelocity.z =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.z);
                                    v22 = 0;
                                    continue;
                                }
                                if (PID_TYPE(_actor_collision_struct.pid) ==
                                    OBJECT_Decoration) {
                                    _this = integer_sqrt(
                                        pActors[actor_id].vVelocity.x *
                                            pActors[actor_id].vVelocity.x +
                                        pActors[actor_id].vVelocity.y *
                                            pActors[actor_id].vVelocity.y);
                                    v45 = TrigLUT->Atan2(
                                        pActors[actor_id].vPosition.x -
                                            pLevelDecorations[v37].vPosition.x,
                                        pActors[actor_id].vPosition.y -
                                            pLevelDecorations[v37].vPosition.y);
                                    pActors[actor_id].vVelocity.x =
                                        fixpoint_mul(TrigLUT->Cos(v45),
                                                     _this);
                                    pActors[actor_id].vVelocity.y =
                                        fixpoint_mul(TrigLUT->Sin(v45),
                                                     _this);
                                    pActors[actor_id].vVelocity.x =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.x);
                                    pActors[actor_id].vVelocity.y =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.y);
                                    pActors[actor_id].vVelocity.z =
                                        fixpoint_mul(
                                            58500,
                                            pActors[actor_id].vVelocity.z);
                                    v22 = 0;
                                    continue;
                                }
                                if (PID_TYPE(_actor_collision_struct.pid) ==
                                    OBJECT_BModel) {
                                    _actor_collision_struct.field_84 = _actor_collision_struct.pid >> 3;
                                    if (pIndoor->pFaces[v37].uPolygonType ==
                                        3) {
                                        pActors[actor_id].vVelocity.z = 0;
                                        pActors[actor_id].vPosition.z =
                                            pIndoor
                                                ->pVertices[*pIndoor
                                                                 ->pFaces[v37]
                                                                 .pVertexIDs]
                                                .z +
                                            1;
                                        if (pActors[actor_id].vVelocity.x *
                                                    pActors[actor_id]
                                                        .vVelocity.x +
                                                pActors[actor_id].vVelocity.y *
                                                    pActors[actor_id]
                                                        .vVelocity.y <
                                            400) {
                                            pActors[actor_id].vVelocity.y = 0;
                                            pActors[actor_id].vVelocity.x = 0;
                                            pActors[actor_id]
                                                .vVelocity.x = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.x);
                                            pActors[actor_id]
                                                .vVelocity.y = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.y);
                                            pActors[actor_id]
                                                .vVelocity.z = fixpoint_mul(
                                                58500,
                                                pActors[actor_id].vVelocity.z);
                                            v22 = 0;
                                            continue;
                                        }
                                    } else {
                                        v61 = abs(pIndoor->pFaces[v37]
                                                          .pFacePlane_old
                                                          .vNormal.x *
                                                      pActors[actor_id]
                                                          .vVelocity.x +
                                                  pIndoor->pFaces[v37]
                                                          .pFacePlane_old
                                                          .vNormal.y *
                                                      pActors[actor_id]
                                                          .vVelocity.y +
                                                  pIndoor->pFaces[v37]
                                                          .pFacePlane_old
                                                          .vNormal.z *
                                                      pActors[actor_id]
                                                          .vVelocity.z) >>
                                              16;
                                        if ((_actor_collision_struct.speed >> 3) > v61)
                                            v61 = _actor_collision_struct.speed >> 3;
                                        pActors[actor_id].vVelocity.x +=
                                            fixpoint_mul(
                                                v61,
                                                pIndoor->pFaces[v37]
                                                    .pFacePlane_old.vNormal.x);
                                        pActors[actor_id].vVelocity.y +=
                                            fixpoint_mul(
                                                v61,
                                                pIndoor->pFaces[v37]
                                                    .pFacePlane_old.vNormal.y);
                                        pActors[actor_id].vVelocity.z +=
                                            fixpoint_mul(
                                                v61,
                                                pIndoor->pFaces[v37]
                                                    .pFacePlane_old.vNormal.z);
                                        if (pIndoor->pFaces[v37].uPolygonType !=
                                                4 &&
                                            pIndoor->pFaces[v37].uPolygonType !=
                                                3) {
                                            v44 = _actor_collision_struct.prolly_normal_d -
                                                  ((pIndoor->pFaces[v37]
                                                        .pFacePlane_old.dist +
                                                    pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.z *
                                                        pActors[actor_id]
                                                            .vPosition.z +
                                                    pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.y *
                                                        pActors[actor_id]
                                                            .vPosition.y +
                                                    pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.x *
                                                        pActors[actor_id]
                                                            .vPosition.x) >>
                                                   16);
                                            if (v44 > 0) {
                                                pActors[actor_id].vPosition.x +=
                                                    fixpoint_mul(
                                                        v44,
                                                        pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.x);
                                                pActors[actor_id].vPosition.y +=
                                                    fixpoint_mul(
                                                        v44,
                                                        pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.y);
                                                pActors[actor_id].vPosition.z +=
                                                    fixpoint_mul(
                                                        v44,
                                                        pIndoor->pFaces[v37]
                                                            .pFacePlane_old
                                                            .vNormal.z);
                                            }
                                            pActors[actor_id]
                                                .uYawAngle = TrigLUT->Atan2(
                                                pActors[actor_id].vVelocity.x,
                                                pActors[actor_id].vVelocity.y);
                                        }
                                    }
                                    if (pIndoor->pFaces[v37].uAttributes &
                                        FACE_UNKNOW1)
                                        EventProcessor(
                                            pIndoor
                                                ->pFaceExtras[pIndoor
                                                                  ->pFaces[v37]
                                                                  .uFaceExtraID]
                                                .uEventID,
                                            0, 1);
                                }
                                pActors[actor_id].vVelocity.x = fixpoint_mul(
                                    58500, pActors[actor_id].vVelocity.x);
                                pActors[actor_id].vVelocity.y = fixpoint_mul(
                                    58500, pActors[actor_id].vVelocity.y);
                                pActors[actor_id].vVelocity.z = fixpoint_mul(
                                    58500, pActors[actor_id].vVelocity.z);
                                v22 = 0;
                                continue;
                            } else {
                                pActors[actor_id].vPosition.x =
                                    (short)_actor_collision_struct.normal2.x;
                                pActors[actor_id].vPosition.y =
                                    (short)_actor_collision_struct.normal2.y;
                                pActors[actor_id].vPosition.z =
                                    (short)_actor_collision_struct.normal2.z -
                                    (short)_actor_collision_struct.prolly_normal_d - 1;
                                pActors[actor_id].uSectorID =
                                    (short)_actor_collision_struct.uSectorID;
                                // goto LABEL_123;
                                break;
                            }

                        } else if (pActors[actor_id].vPosition.x & 1) {
                            pActors[actor_id].uYawAngle += 100;
                        } else {
                            pActors[actor_id].uYawAngle -= 100;
                        }
                    } else {
                        if (pParty->bTurnBasedModeOn &&
                            (pTurnEngine->turn_stage == TE_ATTACK ||
                             pTurnEngine->turn_stage == TE_MOVEMENT))
                            continue;
                        if (!pActors[actor_id].pMonsterInfo.uHostilityType ||
                            v56 != v22) {
                            Actor::AI_StandOrBored(actor_id, 4, v22, &v52);
                            continue;
                        }
                    }
                }
            }
        } else {
            pActors[actor_id].vVelocity.z = 0;
            pActors[actor_id].vVelocity.y = 0;
            pActors[actor_id].vVelocity.x = 0;
            if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_SKY) {
                if (pActors[actor_id].uAIState == Dead)
                    pActors[actor_id].uAIState = Removed;
            }
        }
        // LABEL_123:
    }
}

//----- (00460A78) --------------------------------------------------------
void PrepareToLoadBLV(unsigned int bLoading) {
    unsigned int respawn_interval;  // ebx@1
    unsigned int map_id;            // eax@8
    MapInfo *map_info;              // edi@9
    int v4;                         // eax@11
    char v28;                       // zf@81
    signed int v30;                 // edi@94
    int v34[4];                     // [sp+3E8h] [bp-2Ch]@96
    int v35;                        // [sp+3F8h] [bp-1Ch]@1
    int v38;                        // [sp+404h] [bp-10h]@1
    int pDest;                      // [sp+40Ch] [bp-8h]@1

    respawn_interval = 0;
    pGameLoadingUI_ProgressBar->Reset(0x20u);
    bNoNPCHiring = false;
    pDest = 1;
    uCurrentlyLoadedLevelType = LEVEL_Indoor;

    engine->SetUnderwater(
        Is_out15odm_underwater());

    if ((pCurrentMapName == "out15.odm") || (pCurrentMapName == "d23.blv")) {
        bNoNPCHiring = true;
    }
    pPaletteManager->pPalette_tintColor[0] = 0;
    pPaletteManager->pPalette_tintColor[1] = 0;
    pPaletteManager->pPalette_tintColor[2] = 0;
    pPaletteManager->RecalculateAll();
    if (_A750D8_player_speech_timer) _A750D8_player_speech_timer = 0;
    map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id) {
        map_info = &pMapStats->pInfos[map_id];
        respawn_interval = pMapStats->pInfos[map_id].uRespawnIntervalDays;
        v38 = GetAlertStatus();
    } else {
        map_info = (MapInfo *)bLoading;
    }
    dword_6BE13C_uCurrentlyLoadedLocationID = map_id;

    pStationaryLightsStack->uNumLightsActive = 0;
    v4 = pIndoor->Load(pCurrentMapName, pParty->GetPlayingTime().GetDays() + 1,
                       respawn_interval, (char *)&pDest) -
         1;
    if (!v4) Error("Unable to open %s", pCurrentMapName.c_str());

    if (v4 == 1) Error("File %s is not a BLV File", pCurrentMapName.c_str());

    if (v4 == 2) Error("Attempt to open new level before clearing old");
    if (v4 == 3) Error("Out of memory loading indoor level");
    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN)) {
        Actor::InitializeActors();
        SpriteObject::InitializeSpriteObjects();
    }
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN;
    if (!map_id) pDest = 0;
    if (pDest == 1) {
        for (uint i = 0; i < pIndoor->uNumSpawnPoints; ++i) {
            auto spawn = pIndoor->pSpawnPoints + i;
            if (spawn->IsMonsterSpawn())
                SpawnEncounter(map_info, spawn, 0, 0, 0);
            else
                map_info->SpawnRandomTreasure(spawn);
        }
        RespawnGlobalDecorations();
    }

    for (uint i = 0; i < pIndoor->uNumDoors; ++i) {
        if (pIndoor->pDoors[i].uAttributes & 0x01) {
            pIndoor->pDoors[i].uState = BLVDoor::Opening;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = 2;
        }

        if (pIndoor->pDoors[i].uState == BLVDoor::Closed) {
            pIndoor->pDoors[i].uState = BLVDoor::Closing;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = 2;
        } else if (pIndoor->pDoors[i].uState == BLVDoor::Open) {
            pIndoor->pDoors[i].uState = BLVDoor::Opening;
            pIndoor->pDoors[i].uTimeSinceTriggered = 15360;
            pIndoor->pDoors[i].uAttributes = 2;
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
    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);

        DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[i].uDecorationDescID);

        if (decoration->uSoundID && _6807E0_num_decorations_with_sounds_6807B8 < 9) {
            // pSoundList->LoadSound(decoration->uSoundID, 0);
            _6807B8_level_decorations_ids[_6807E0_num_decorations_with_sounds_6807B8++] = i;
        }

        if (!(pLevelDecorations[i].uFlags & LEVEL_DECORATION_INVISIBLE)) {
            if (!decoration->DontDraw()) {
                if (decoration->uLightRadius) {
                    unsigned char r = 255, g = 255, b = 255;
                    if (/*render->pRenderD3D*/ true &&
                        render->config->is_using_colored_lights) {
                        r = decoration->uColoredLightRed;
                        g = decoration->uColoredLightGreen;
                        b = decoration->uColoredLightBlue;
                    }
                    pStationaryLightsStack->AddLight(
                        pLevelDecorations[i].vPosition.x,
                        pLevelDecorations[i].vPosition.y,
                        pLevelDecorations[i].vPosition.z +
                            decoration->uDecorationHeight,
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

    for (uint i = 0; i < uNumSpriteObjects; ++i) {
        if (pSpriteObjects[i].uObjectDescID) {
            if (pSpriteObjects[i].containing_item.uItemID) {
                if (pSpriteObjects[i].containing_item.uItemID != 220 &&
                    pItemsTable
                            ->pItems[pSpriteObjects[i].containing_item.uItemID]
                            .uEquipType == EQUIP_POTION &&
                    !pSpriteObjects[i].containing_item.uEnchantmentType)
                    pSpriteObjects[i].containing_item.uEnchantmentType =
                        rand() % 15 + 5;
                pItemsTable->SetSpecialBonus(
                    &pSpriteObjects[i].containing_item);
            }
        }
    }

    // INDOOR initialize actors
    v38 = 0;

    for (uint i = 0; i < uNumActors; ++i) {
        if (pActors[i].uAttributes & ACTOR_UNKNOW7) {
            if (!map_id) {
                pActors[i].pMonsterInfo.field_3E = 19;
                pActors[i].uAttributes |= ACTOR_UNKNOW11;
                continue;
            }
            v28 = v38 == 0;
        } else {
            v28 = v38 == 1;
        }

        if (!v28) {
            pActors[i].PrepareSprites(0);
            pActors[i].pMonsterInfo.uHostilityType =
                MonsterInfo::Hostility_Friendly;
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
        pParty->sRotationX = 0;
        pParty->sRotationZ = 0;
        pParty->vPosition.z = 0;
        pParty->vPosition.y = 0;
        pParty->vPosition.x = 0;
        pParty->uFallStartY = 0;
        pParty->uFallSpeed = 0;
        TeleportToStartingPoint(uLevel_StartingPointType);
    }
    viewparams->_443365();
    PlayLevelMusic();
    if (!bLoading) {
        v30 = 0;
        for (uint pl_id = 1; pl_id <= 4; ++pl_id) {
            if (pPlayers[pl_id]->CanAct()) v34[v30++] = pl_id;
        }
        if (v30) {
            if (pDest) {
                _A750D8_player_speech_timer = 256;
                PlayerSpeechID = SPEECH_46;
                uSpeakingCharacter = v34[rand() % v30];
            }
        }
    }
}

//----- (0046CEC3) --------------------------------------------------------
int BLV_GetFloorLevel(int x, int y, int z, unsigned int uSectorID, unsigned int *pFaceID) {
    // gets the floor position at XYZ - if wrong secor is supplied, will return -30000

    // stores faces and floor levels
    int FacesFound = 0;
    static int blv_floor_id[5] = { 0 };
    static int blv_floor_level[5] = { 0 };

    int z_calc;

    // vert store for point in poly checks
    static float floor_vert_Y[104];
    static float floor_vert_X[104];

    BLVSector *pSector = &pIndoor->pSectors[uSectorID];

    // loop over all floor faces
    for (uint i = 0; i < pSector->uNumFloors; ++i) {
        BLVFace *pFloor = &pIndoor->pFaces[pSector->pFloors[i]];
        if (pFloor->Ethereal()) continue;
        if (!pFloor->uNumVertices) continue;

        if (x <= pFloor->pBounding.x2 && x >= pFloor->pBounding.x1 &&
            y <= pFloor->pBounding.y2 && y >= pFloor->pBounding.y1) {
            for (uint j = 0; j < pFloor->uNumVertices; ++j) {
                floor_vert_X[2 * j] =
                    pFloor->pXInterceptDisplacements[j] + pIndoor->pVertices[pFloor->pVertexIDs[j]].x;
                floor_vert_X[2 * j + 1] =
                    pFloor->pXInterceptDisplacements[j] + pIndoor->pVertices[pFloor->pVertexIDs[j + 1]].x;
                floor_vert_Y[2 * j] =
                    pFloor->pYInterceptDisplacements[j] + pIndoor->pVertices[pFloor->pVertexIDs[j]].y;
                floor_vert_Y[2 * j + 1] =
                    pFloor->pYInterceptDisplacements[j] + pIndoor->pVertices[pFloor->pVertexIDs[j + 1]].y;
            }

            int ti, hj, nvert = (2 * pFloor->uNumVertices);
            bool c = false;

            for (ti = 0, hj = nvert - 1; ti < nvert; hj = ti++) {
                if (((floor_vert_Y[ti] > y) != (floor_vert_Y[hj] > y)) &&
                    (x < (floor_vert_X[hj] - floor_vert_X[ti]) * (y - floor_vert_Y[ti]) / (floor_vert_Y[hj] - floor_vert_Y[ti]) + floor_vert_X[ti])
                    )
                    c = !c;
            }

            if (c) {
                if (pFloor->uPolygonType == POLYGON_Floor || pFloor->uPolygonType == POLYGON_Ceiling) {
                    z_calc = pIndoor->pVertices[pFloor->pVertexIDs[0]].z;
                } else {
                    z_calc = fixpoint_mul(pFloor->zCalc1, x) +
                        fixpoint_mul(pFloor->zCalc2, y) +
                        ((pFloor->zCalc3 + 0x8000) >> 16);
                }

                blv_floor_level[FacesFound] = z_calc;
                blv_floor_id[FacesFound] = pSector->pFloors[i];
                FacesFound++;
            }
            if (FacesFound >= 5) break;
        }
    }


    // as above but for sector portal faces
    if (pSector->field_0 & 8) {
        for (uint i = 0; i < pSector->uNumPortals; ++i) {
            if (FacesFound >= 5) break;
            BLVFace *portal = &pIndoor->pFaces[pSector->pPortals[i]];
            if (portal->uPolygonType != POLYGON_Floor) continue;

            if (!portal->uNumVertices) continue;

            if (x <= portal->pBounding.x2 && x >= portal->pBounding.x1 &&
                y <= portal->pBounding.y2 && y >= portal->pBounding.y1) {
                for (uint j = 0; j < portal->uNumVertices; ++j) {
                    floor_vert_X[2 * j] =
                        portal->pXInterceptDisplacements[j] + pIndoor->pVertices[portal->pVertexIDs[j]].x;
                    floor_vert_X[2 * j + 1] =
                        portal->pXInterceptDisplacements[j + 1] + pIndoor->pVertices[portal->pVertexIDs[j + 1]].x;
                    floor_vert_Y[2 * j] =
                        portal->pYInterceptDisplacements[j] + pIndoor->pVertices[portal->pVertexIDs[j]].y;
                    floor_vert_Y[2 * j + 1] =
                        portal->pYInterceptDisplacements[j + 1] + pIndoor->pVertices[portal->pVertexIDs[j + 1]].y;
                }

                int ti, hj, nvert = (2 * portal->uNumVertices);
                bool c = false;

                for (ti = 0, hj = nvert - 1; ti < nvert; hj = ti++) {
                    if (((floor_vert_Y[ti] > y) != (floor_vert_Y[hj] > y)) &&
                        (x < (floor_vert_X[hj] - floor_vert_X[ti]) * (y - floor_vert_Y[ti]) / (floor_vert_Y[hj] - floor_vert_Y[ti]) + floor_vert_X[ti])
                        )
                        c = !c;
                }

                if (c) {
                    if (FacesFound >= 5) break;
                    blv_floor_level[FacesFound] = -29000;
                    blv_floor_id[FacesFound] = pSector->pPortals[i];
                    FacesFound++;
                }
            }
        }
    }

    // one face found
    if (FacesFound == 1) {
        *pFaceID = blv_floor_id[0];
        if (blv_floor_level[0] <= -29000) __debugbreak();
        return blv_floor_level[0];
    }

    // no face found - probably wrong sector supplied
    if (!FacesFound) {
        logger->Warning("Floorlvl fail");
        return -30000;
    }

    // multiple faces found - pick nearest
    int result = blv_floor_level[0];
    *pFaceID = blv_floor_id[0];
    for (uint i = 1; i < FacesFound; ++i) {
        int v38 = blv_floor_level[i];

        if (abs(z - v38) <= abs(z - result)) {
            result = blv_floor_level[i];
            if (blv_floor_level[i] <= -29000) __debugbreak();
            *pFaceID = blv_floor_id[i];
        }
    }

    if (result <= -29000) __debugbreak();

    return result;
}

//----- (0043FDED) --------------------------------------------------------
void IndoorLocation::PrepareActorRenderList_BLV() {  // combines this with outdoorlocation ??
    unsigned int v4;  // eax@5
    int v6;           // esi@5
    int v8;           // eax@10
    SpriteFrame *v9;  // eax@16
    // int v12;          // ecx@28
    __int16 v41;      // [sp+3Ch] [bp-18h]@18
    // int z; // [sp+48h] [bp-Ch]@32
    // signed int y; // [sp+4Ch] [bp-8h]@32
    // int x; // [sp+50h] [bp-4h]@32

    for (uint i = 0; i < uNumActors; ++i) {
        if (pActors[i].uAIState == Removed || pActors[i].uAIState == Disabled)
            continue;

        v4 = TrigLUT->Atan2(
            pActors[i].vPosition.x - pIndoorCameraD3D->vPartyPos.x,
            pActors[i].vPosition.y - pIndoorCameraD3D->vPartyPos.y);
        v6 = ((signed int)(pActors[i].uYawAngle +
                           ((signed int)TrigLUT->uIntegerPi >> 3) - v4 +
                           TrigLUT->uIntegerPi) >>
              8) &
             7;
        v8 = pActors[i].uCurrentActionTime;
        if (pParty->bTurnBasedModeOn) {
            if (pActors[i].uCurrentActionAnimation == 1)
                v8 = i * 32 + pMiscTimer->uTotalGameTimeElapsed;
        } else {
            if (pActors[i].uCurrentActionAnimation == 1)
                v8 = i * 32 + pBLVRenderParams->field_0_timer_;
        }
        if (pActors[i].pActorBuffs[ACTOR_BUFF_STONED].Active() ||
            pActors[i].pActorBuffs[ACTOR_BUFF_PARALYZED].Active())
            v8 = 0;

        if (pActors[i].uAIState == Resurrected)
            v9 = pSpriteFrameTable->GetFrameBy_x(
                pActors[i].pSpriteIDs[pActors[i].uCurrentActionAnimation], v8);
        else
            v9 = pSpriteFrameTable->GetFrame(
                pActors[i].pSpriteIDs[pActors[i].uCurrentActionAnimation], v8);

        if (v9->icon_name == "null") continue;

        v41 = 0;
        if (v9->uFlags & 2) v41 = 2;
        if (v9->uFlags & 0x40000) v41 |= 0x40;
        if (v9->uFlags & 0x20000) v41 |= 0x80;
        if ((256 << v6) & v9->uFlags) v41 |= 4;
        if (v9->uGlowRadius) {
            pMobileLightsStack->AddLight(
                pActors[i].vPosition.x, pActors[i].vPosition.y,
                pActors[i].vPosition.z, pActors[i].uSectorID, v9->uGlowRadius,
                0xFFu, 0xFFu, 0xFFu, _4E94D3_light_type);
        }

        // for (v12 = 0; v12 < pBspRenderer->uNumVisibleNotEmptySectors; ++v12) {
        //    if (pBspRenderer
        //            ->pVisibleSectorIDs_toDrawDecorsActorsEtcFrom[v12] ==
        //        pActors[i].uSectorID || true) {
                int view_x = 0;
                int view_y = 0;
                int view_z = 0;
                bool visible = pIndoorCameraD3D->ViewClip(
                    pActors[i].vPosition.x, pActors[i].vPosition.y,
                    pActors[i].vPosition.z, &view_x, &view_y, &view_z);
                if (visible) {
                    if (abs(view_x) >= abs(view_y)) {
                        int projected_x = 0;
                        int projected_y = 0;
                        pIndoorCameraD3D->Project(view_x, view_y, view_z,
                                                  &projected_x, &projected_y);

                        if (uNumBillboardsToDraw >= 500) break;
                        ++uNumBillboardsToDraw;
                        ++uNumSpritesDrawnThisFrame;

                        pActors[i].uAttributes |= ACTOR_VISIBLE;
                        pBillboardRenderList[uNumBillboardsToDraw - 1]
                            .hwsprite = v9->hw_sprites[v6];

                        // error catching
                        if (v9->hw_sprites[v6]->texture->GetHeight() == 0 || v9->hw_sprites[v6]->texture->GetWidth() == 0)
                            __debugbreak();

                        pBillboardRenderList[uNumBillboardsToDraw - 1]
                            .uPalette = v9->uPaletteIndex;
                        pBillboardRenderList[uNumBillboardsToDraw - 1] .uIndoorSectorID = pActors[i].uSectorID;

                        pBillboardRenderList[uNumBillboardsToDraw - 1].fov_x = pIndoorCameraD3D->fov_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].fov_y = pIndoorCameraD3D->fov_y;

                        float _v18_over_x = v9->scale * floorf(pIndoorCameraD3D->fov_x + 0.5f) / (view_x);
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x =  _v18_over_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = _v18_over_x;

                        if (pActors[i].pActorBuffs[ACTOR_BUFF_MASS_DISTORTION].Active()) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = spell_fx_renderer->_4A806F_get_mass_distortion_value(&pActors[i]) *
                                pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y;
                        } else if (pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].Active() &&
                                   pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].uPower > 0) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y =
                                    1.0f / pActors[i].pActorBuffs[ACTOR_BUFF_SHRINK].uPower *
                                    pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y;
                        }

                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_x = pActors[i].vPosition.x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_y = pActors[i].vPosition.y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_z = pActors[i].vPosition.z;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_x = projected_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_y = projected_y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_z = view_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].object_pid = PID(OBJECT_Actor, i);
                        pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = v41 & 0xFF;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = v9;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor = pMonsterList
                                ->pMonsters[pActors[i].pMonsterInfo.uID - 1].sTintColor;

                        if (pActors[i].pActorBuffs[ACTOR_BUFF_STONED].Active()) {
                            pBillboardRenderList[uNumBillboardsToDraw - 1]
                                .field_1E |= 0x100;
                        }
                    }
                }
           // }
        //}
    }
}

void IndoorLocation::PrepareItemsRenderList_BLV() {
    unsigned int v6;     // eax@12

    for (uint i = 0; i < uNumSpriteObjects; ++i) {
        if (pSpriteObjects[i].uObjectDescID) {
            if (pSpriteObjects[i].HasSprite()) {
                if ((pSpriteObjects[i].uType < 1000 ||
                     pSpriteObjects[i].uType >= 10000) &&
                        (pSpriteObjects[i].uType < 500 ||
                         pSpriteObjects[i].uType >= 600) &&
                        (pSpriteObjects[i].uType < 811 ||
                         pSpriteObjects[i].uType >= 815) ||
                    spell_fx_renderer->RenderAsSprite(&pSpriteObjects[i])) {
                    SpriteFrame *v4 = pSpriteObjects[i].GetSpriteFrame();
                    int a6 = v4->uGlowRadius * pSpriteObjects[i].field_22_glow_radius_multiplier;
                    v6 = TrigLUT->Atan2(pSpriteObjects[i].vPosition.x - pIndoorCameraD3D->vPartyPos.x,
                                            pSpriteObjects[i].vPosition.y - pIndoorCameraD3D->vPartyPos.y);
                    int v7 = pSpriteObjects[i].uFacing;
                    int v9 = ((int)(TrigLUT->uIntegerPi + ((int)TrigLUT->uIntegerPi >> 3) + v7 - v6) >> 8) & 7;

                    pBillboardRenderList[uNumBillboardsToDraw].hwsprite = v4->hw_sprites[v9];
                    // error catching
                    if (v4->hw_sprites[v9]->texture->GetHeight() == 0 || v4->hw_sprites[v9]->texture->GetWidth() == 0)
                        __debugbreak();

                    // centre sprite on coords
                    int modz = pSpriteObjects[i].vPosition.z;
                    if (v4->uFlags & 0x20)
                       modz -= (int)((v4->scale, v4->hw_sprites[v9]->uBufferHeight) / 2);

                    int16_t v34 = 0;
                    if (v4->uFlags & 2) v34 = 2;
                    if (v4->uFlags & 0x40000) v34 |= 0x40;
                    if (v4->uFlags & 0x20000) v34 |= 0x80;
                    // v11 = (int *)(256 << v9);
                    if ((256 << v9) & v4->uFlags) v34 |= 4;
                    if (a6) {
                        pMobileLightsStack->AddLight(
                            pSpriteObjects[i].vPosition.x,
                            pSpriteObjects[i].vPosition.y,
                            pSpriteObjects[i].vPosition.z,
                            pSpriteObjects[i].uSectorID, a6,
                            pSpriteObjects[i].GetParticleTrailColorR(),
                            pSpriteObjects[i].GetParticleTrailColorG(),
                            pSpriteObjects[i].GetParticleTrailColorB(),
                            _4E94D3_light_type);
                    }

                    int view_x = 0;
                    int view_y = 0;
                    int view_z = 0;

                    bool visible = pIndoorCameraD3D->ViewClip(pSpriteObjects[i].vPosition.x,
                                                              pSpriteObjects[i].vPosition.y,
                                                              modz,
                                                              &view_x, &view_y, &view_z);

                    view_x -= 0.005;


                    if (visible) {
                        int projected_x = 0;
                        int projected_y = 0;
                        pIndoorCameraD3D->Project(view_x, view_y, view_z, &projected_x, &projected_y);

                        assert(uNumBillboardsToDraw < 499);
                        ++uNumBillboardsToDraw;
                        ++uNumSpritesDrawnThisFrame;

                        pSpriteObjects[i].uAttributes |= 1;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].uPalette = v4->uPaletteIndex;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID = pSpriteObjects[i].uSectorID;
                        // if ( render->pRenderD3D )
                        {
                            pBillboardRenderList[uNumBillboardsToDraw - 1].fov_x = pIndoorCameraD3D->fov_x;
                            pBillboardRenderList[uNumBillboardsToDraw - 1].fov_y = pIndoorCameraD3D->fov_y;
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x =
                                v4->scale * (int)floorf(pIndoorCameraD3D->fov_x + 0.5f) / view_x;
                            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y =
                                v4->scale * (int)floorf(pIndoorCameraD3D->fov_x + 0.5f) / view_x;
                        }

                        pBillboardRenderList[uNumBillboardsToDraw - 1].field_1E = v34;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_x = pSpriteObjects[i].vPosition.x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_y = pSpriteObjects[i].vPosition.y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].world_z = pSpriteObjects[i].vPosition.z;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_x = projected_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_y = projected_y;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].sTintColor = 0;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].pSpriteFrame = v4;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].screen_space_z = view_x;
                        pBillboardRenderList[uNumBillboardsToDraw - 1].object_pid = PID(OBJECT_Item, i);
                    }
                }
            }
        }
    }
}

void IndoorLocation::PrepareDecorationsRenderList_BLV(unsigned int uDecorationID, unsigned int uSectorID) {
    unsigned int v8;       // edi@5
    int v9;                // edi@5
    int v10;               // eax@7
    SpriteFrame *v11;      // eax@7
    Particle_sw particle;  // [sp+Ch] [bp-A0h]@3
    int v30;               // [sp+8Ch] [bp-20h]@7

    if (pLevelDecorations[uDecorationID].uFlags & LEVEL_DECORATION_INVISIBLE)
        return;

    DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[uDecorationID].uDecorationDescID);

    if (decoration->uFlags & DECORATION_DESC_EMITS_FIRE) {
        memset(&particle, 0, sizeof(Particle_sw));  // fire,  like at the Pit's tavern
        particle.type =
            ParticleType_Bitmap | ParticleType_Rotating | ParticleType_8;
        particle.uDiffuse = 0xFF3C1E;
        particle.x = (double)pLevelDecorations[uDecorationID].vPosition.x;
        particle.y = (double)pLevelDecorations[uDecorationID].vPosition.y;
        particle.z = (double)pLevelDecorations[uDecorationID].vPosition.z;
        particle.r = 0.0;
        particle.g = 0.0;
        particle.b = 0.0;
        particle.particle_size = 1.0;
        particle.timeToLive = (rand() & 0x80) + 128;
        particle.texture = spell_fx_renderer->effpar01;
        particle_engine->AddParticle(&particle);
        return;
    }

    if (decoration->uFlags & DECORATION_DESC_DONT_DRAW) {
        return;
    }

    v8 = pLevelDecorations[uDecorationID].field_10_y_rot +
         ((signed int)TrigLUT->uIntegerPi >> 3) -
         TrigLUT->Atan2(pLevelDecorations[uDecorationID].vPosition.x -
                                pIndoorCameraD3D->vPartyPos.x,
                            pLevelDecorations[uDecorationID].vPosition.y -
                                pIndoorCameraD3D->vPartyPos.y);
    v9 = ((signed int)(TrigLUT->uIntegerPi + v8) >> 8) & 7;
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
        pIndoorCameraD3D->ViewClip(pLevelDecorations[uDecorationID].vPosition.x,
                                   pLevelDecorations[uDecorationID].vPosition.y,
                                   pLevelDecorations[uDecorationID].vPosition.z,
                                   &view_x, &view_y, &view_z);

    if (visible) {
        if (abs(view_x) >= abs(view_y)) {
            int projected_x = 0;
            int projected_y = 0;
            pIndoorCameraD3D->Project(view_x, view_y, view_z, &projected_x,
                                      &projected_y);

            assert(uNumBillboardsToDraw < 500);
            ++uNumBillboardsToDraw;
            ++uNumDecorationsDrawnThisFrame;

            pBillboardRenderList[uNumBillboardsToDraw - 1].hwsprite =
                v11->hw_sprites[v9];

            if (v11->hw_sprites[v9]->texture->GetHeight() == 0 || v11->hw_sprites[v9]->texture->GetWidth() == 0)
                __debugbreak();

            pBillboardRenderList[uNumBillboardsToDraw - 1].uPalette =
                v11->uPaletteIndex;
            pBillboardRenderList[uNumBillboardsToDraw - 1].uIndoorSectorID =
                uSectorID;

            pBillboardRenderList[uNumBillboardsToDraw - 1].fov_x =
                pIndoorCameraD3D->fov_x;
            pBillboardRenderList[uNumBillboardsToDraw - 1].fov_y =
                pIndoorCameraD3D->fov_y;
            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_x = v11->scale * (int)floorf(pIndoorCameraD3D->fov_x + 0.5f) / view_x;
            pBillboardRenderList[uNumBillboardsToDraw - 1].screenspace_projection_factor_y = v11->scale * (int)floorf(pIndoorCameraD3D->fov_y + 0.5f) / view_x;
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

bool Check_LineOfSight(int target_x, int target_y, int target_z, Vec3_int_ Pos_From) {  // target xyz from position v true on clear
    int dist_y;       // edi@2
    int dist_z;       // ebx@2
    int dist_x;       // esi@2
    int v9;           // ecx@2
    int v10;          // eax@2
    int rayznorm;          // eax@4
    int v17;          // ST34_4@25
    int v18;          // ST38_4@25
    int v19;          // eax@25
    char FaceIsParallel;         // zf@25
    int dot_ray2;          // ebx@25
    int v23;          // edi@26
    int v24;          // ST34_4@30
    int v32;          // ecx@37
    int v33;          // eax@37
    int v35;          // eax@39
    int v40;          // ebx@60
    int v42;          // edi@61
    int v49;          // ecx@73
    int v50;          // eax@73
    // int v51;          // edx@75
    int Y_VecDist;          // ecx@75
    int Z_VecDist;          // eax@75
    int SectorID;          // eax@90
    BLVFace *face;    // esi@91
    int X_NormMag;          // ST34_4@98
    int Y_NormMag;          // ST30_4@98
    int Z_NormMag;          // eax@98
    int v66;          // ebx@98
    int ShiftDotDist;          // edi@99
    int v69;          // ST2C_4@103
    int v77;          // ecx@111
    int v78;          // eax@111
    // int v79;          // edx@113
    int v80;          // ecx@113
    int v81;          // eax@113
    int v87;          // ecx@128
    int v91;          // ebx@136
    int v93;          // edi@137

    int v107;         // [sp+10h] [bp-6Ch]@98
    int v108;         // [sp+10h] [bp-6Ch]@104
    int dot_ray;         // [sp+18h] [bp-64h]@25
    int v110;         // [sp+18h] [bp-64h]@31
    // int LOS_Obscurred;         // [sp+20h] [bp-5Ch]@1
    // int LOS_Obscurred2;         // [sp+24h] [bp-58h]@1
    int Min_x;         // [sp+34h] [bp-48h]@75
    int v120;         // [sp+34h] [bp-48h]@113
    int rayynorm;         // [sp+38h] [bp-44h]@4
    int v122;         // [sp+38h] [bp-44h]@39
    int Max_x;         // [sp+38h] [bp-44h]@76
    int v124;         // [sp+38h] [bp-44h]@114
    int rayxnorm;         // [sp+3Ch] [bp-40h]@4
    int v126;         // [sp+3Ch] [bp-40h]@39
    int Min_y;         // [sp+3Ch] [bp-40h]@77
    int v128;         // [sp+3Ch] [bp-40h]@115
    int zmax;         // [sp+40h] [bp-3Ch]@11
    int v130;         // [sp+40h] [bp-3Ch]@46
    int Max_y;         // [sp+40h] [bp-3Ch]@78
    int v132;         // [sp+40h] [bp-3Ch]@116
    int zmin;         // [sp+44h] [bp-38h]@10
    int v134;         // [sp+44h] [bp-38h]@45
    int Min_z;         // [sp+44h] [bp-38h]@81
    int v136;         // [sp+44h] [bp-38h]@119
    int ymax;         // [sp+48h] [bp-34h]@7
    int v138;         // [sp+48h] [bp-34h]@42
    int Max_z;         // [sp+48h] [bp-34h]@82
    int v140;         // [sp+48h] [bp-34h]@120
    int ymin;         // [sp+4Ch] [bp-30h]@6
    int v142;         // [sp+4Ch] [bp-30h]@41
    int X_VecDist;         // [sp+4Ch] [bp-30h]@75
    int v144;         // [sp+4Ch] [bp-30h]@113
    int xmax;         // [sp+50h] [bp-2Ch]@5
    int v146;         // [sp+50h] [bp-2Ch]@40
    int xmin;         // [sp+54h] [bp-28h]@4
    int v150;         // [sp+54h] [bp-28h]@39
    // int FaceLoop;      // [sp+58h] [bp-24h]@90
    // int TargetFromFlip;          // [sp+5Ch] [bp-20h]@83
    int a5c;          // [sp+5Ch] [bp-20h]@121
    int v162;         // [sp+60h] [bp-1Ch]@128
    int ShiftedFromz;         // [sp+64h] [bp-18h]@2
    int ShiftedFromX;         // [sp+68h] [bp-14h]@2
    int ShiftedFromY;         // [sp+6Ch] [bp-10h]@2
    int ShiftedTargetZ;           // [sp+70h] [bp-Ch]@2
    int ShiftedTargetX;           // [sp+74h] [bp-8h]@2
    int ShiftedTargetY;           // [sp+78h] [bp-4h]@2
                      // 8bytes unused
    int ya;           // [sp+84h] [bp+8h]@60
    int yb;           // [sp+84h] [bp+8h]@136
    int ve;           // [sp+88h] [bp+Ch]@60
    int va;           // [sp+88h] [bp+Ch]@60
    int vb;           // [sp+88h] [bp+Ch]@66
    int vf;           // [sp+88h] [bp+Ch]@136
    int vc;           // [sp+88h] [bp+Ch]@136
    int vd;           // [sp+88h] [bp+Ch]@142
    int v_4;          // [sp+8Ch] [bp+10h]@60
    int v_4a;         // [sp+8Ch] [bp+10h]@65
    int v_4b;         // [sp+8Ch] [bp+10h]@136
    int v_4c;         // [sp+8Ch] [bp+10h]@141

     // __debugbreak(); // срабатывает при стрельбе огненным шаром
    // triggered by fireball

     unsigned int AngleToTarget = TrigLUT->Atan2(Pos_From.x - target_x, Pos_From.y - target_y);

    bool LOS_Obscurred = 0;
    bool LOS_Obscurred2 = 0;

    Vec3_int_ TargetVec;
    TargetVec.z = target_z;
    TargetVec.x = target_x;
    TargetVec.y = target_y;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        // offset 32 to side and check LOS
        Vec3_int_::Rotate(32, TrigLUT->uIntegerHalfPi + AngleToTarget, 0, TargetVec, &ShiftedTargetX, &ShiftedTargetY, &ShiftedTargetZ);
        Vec3_int_::Rotate(32, TrigLUT->uIntegerHalfPi + AngleToTarget, 0, Pos_From, &ShiftedFromX, &ShiftedFromY, &ShiftedFromz);
        dist_y = ShiftedFromY - ShiftedTargetY;
        dist_z = ShiftedFromz - ShiftedTargetZ;
        dist_x = ShiftedFromX - ShiftedTargetX;
        v49 = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
        v50 = 65536;
        if (v49) v50 = 65536 / v49;
        // v51 = ShiftedFromX;

        X_VecDist = dist_x * v50;
        Y_VecDist = dist_y * v50;
        Z_VecDist = dist_z * v50;

        Max_x = std::max(ShiftedFromX, ShiftedTargetX);
        Min_x = std::min(ShiftedFromX, ShiftedTargetX);

        Max_y = std::max(ShiftedFromY, ShiftedTargetY);
        Min_y = std::min(ShiftedFromY, ShiftedTargetY);

        Max_z = std::max(ShiftedFromz, ShiftedTargetZ);
        Min_z = std::min(ShiftedFromz, ShiftedTargetZ);

        for (int TargetFromFlip = 0; TargetFromFlip < 2; TargetFromFlip++) {
            if (TargetFromFlip)
                SectorID = pIndoor->GetSector(ShiftedTargetX, ShiftedTargetY, ShiftedTargetZ);
            else
                SectorID = pIndoor->GetSector(ShiftedFromX, ShiftedFromY, ShiftedFromz);

            // loop over sector faces
            for (int FaceLoop = 0; FaceLoop < pIndoor->pSectors[SectorID].uNumFaces; ++FaceLoop) {
                face = &pIndoor->pFaces[pIndoor->pSectors[SectorID].pFaceIDs[FaceLoop]];

                X_NormMag = fixpoint_mul(X_VecDist, face->pFacePlane_old.vNormal.x);
                Y_NormMag = fixpoint_mul(Z_VecDist, face->pFacePlane_old.vNormal.z);
                Z_NormMag = fixpoint_mul(Y_VecDist, face->pFacePlane_old.vNormal.y);

                // dot product
                FaceIsParallel = (X_NormMag + Y_NormMag + Z_NormMag) == 0;
                v66 = X_NormMag + Y_NormMag + Z_NormMag;
                v107 = X_NormMag + Y_NormMag + Z_NormMag;

                // skip further checks
                if (face->Portal() || Min_x > face->pBounding.x2 ||
                    Max_x < face->pBounding.x1 || Min_y > face->pBounding.y2 ||
                    Max_y < face->pBounding.y1 || Min_z > face->pBounding.z2 ||
                    Max_z < face->pBounding.z1 || FaceIsParallel)
                    continue;

                ShiftDotDist = -(face->pFacePlane_old.dist +
                        ShiftedTargetX * face->pFacePlane_old.vNormal.x +
                        ShiftedTargetY * face->pFacePlane_old.vNormal.y +
                        ShiftedTargetZ * face->pFacePlane_old.vNormal.z);

                if (v66 <= 0) {
                    if (face->pFacePlane_old.dist +
                            ShiftedTargetX * face->pFacePlane_old.vNormal.x +
                            ShiftedTargetY * face->pFacePlane_old.vNormal.y +
                            ShiftedTargetZ * face->pFacePlane_old.vNormal.z < 0)
                        continue;
                } else {
                    if (face->pFacePlane_old.dist +
                            ShiftedTargetX * face->pFacePlane_old.vNormal.x +
                            ShiftedTargetY * face->pFacePlane_old.vNormal.y +
                            ShiftedTargetZ * face->pFacePlane_old.vNormal.z > 0)
                        continue;
                }

                v69 = abs(-(face->pFacePlane_old.dist +
                            ShiftedTargetX * face->pFacePlane_old.vNormal.x +
                            ShiftedTargetY * face->pFacePlane_old.vNormal.y +
                            ShiftedTargetZ * face->pFacePlane_old.vNormal.z)) >> 14;

                if (v69 <= abs(v66)) {
                    v108 = fixpoint_div(ShiftDotDist, v107);
                    if (v108 >= 0) {
                        if (PointInPolyIndoor(ShiftedTargetX + ((signed int)(fixpoint_mul(v108, X_VecDist) + 0x8000) >> 16),
                                        ShiftedTargetY + ((signed int)(fixpoint_mul(v108, Y_VecDist) + 0x8000) >> 16),
                                        ShiftedTargetZ + ((signed int)(fixpoint_mul(v108, Z_VecDist) + 0x8000) >> 16),
                                        face)) {
                            LOS_Obscurred2 = 1;
                            break;
                        }
                    }
                }
            }
        }

        // offset other side and repeat check
        Vec3_int_::Rotate(32, AngleToTarget - TrigLUT->uIntegerHalfPi, 0, TargetVec, &ShiftedTargetX, &ShiftedTargetY, &ShiftedTargetZ);
        Vec3_int_::Rotate(32, AngleToTarget - TrigLUT->uIntegerHalfPi, 0, Pos_From, &ShiftedFromX, &ShiftedFromY, &ShiftedFromz);
        dist_y = ShiftedFromY - ShiftedTargetY;
        dist_z = ShiftedFromz - ShiftedTargetZ;
        dist_x = ShiftedFromX - ShiftedTargetX;
        v77 = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
        v78 = 65536;
        if (v77) v78 = 65536 / v77;
        // v79 = ShiftedFromX;
        v144 = dist_x * v78;
        v80 = dist_y * v78;
        v81 = dist_z * v78;

        v120 = std::max(ShiftedFromX, ShiftedTargetX);
        v124 = std::min(ShiftedFromX, ShiftedTargetX);

        v132 = std::max(ShiftedFromY, ShiftedTargetY);
        v128 = std::min(ShiftedFromY, ShiftedTargetY);

        v140 = std::max(ShiftedFromz, ShiftedTargetZ);
        v136 = std::min(ShiftedFromz, ShiftedTargetZ);

        for (a5c = 0; a5c < 2; a5c++) {
            if (LOS_Obscurred) return !LOS_Obscurred2 || !LOS_Obscurred;
            if (a5c) {
                v87 = pIndoor->GetSector(ShiftedTargetX, ShiftedTargetY, ShiftedTargetZ);
            } else {
                v87 = pIndoor->GetSector(ShiftedFromX, ShiftedFromY, ShiftedFromz);
            }
            for (v162 = 0; v162 < pIndoor->pSectors[v87].uNumFaces; v162++) {
                face = &pIndoor->pFaces[pIndoor->pSectors[v87].pFaceIDs[v162]];
                yb = fixpoint_mul(v144, face->pFacePlane_old.vNormal.x);
                v_4b = fixpoint_mul(v80, face->pFacePlane_old.vNormal.y);
                vf = fixpoint_mul(v81, face->pFacePlane_old.vNormal.z);
                FaceIsParallel = yb + vf + v_4b == 0;
                v91 = yb + vf + v_4b;
                vc = yb + vf + v_4b;
                if (face->Portal() || v120 > face->pBounding.x2 ||
                    v124 < face->pBounding.x1 || v128 > face->pBounding.y2 ||
                    v132 < face->pBounding.y1 || v136 > face->pBounding.z2 ||
                    v140 < face->pBounding.z1 || FaceIsParallel)
                    continue;
                v93 = -(face->pFacePlane_old.dist +
                        ShiftedTargetX * face->pFacePlane_old.vNormal.x +
                        ShiftedTargetY * face->pFacePlane_old.vNormal.y +
                        ShiftedTargetZ * face->pFacePlane_old.vNormal.z);
                if (v91 <= 0) {
                    if (face->pFacePlane_old.dist +
                            ShiftedTargetX * face->pFacePlane_old.vNormal.x +
                            ShiftedTargetY * face->pFacePlane_old.vNormal.y +
                            ShiftedTargetZ * face->pFacePlane_old.vNormal.z <
                        0)
                        continue;
                } else {
                    if (face->pFacePlane_old.dist +
                            ShiftedTargetX * face->pFacePlane_old.vNormal.x +
                            ShiftedTargetY * face->pFacePlane_old.vNormal.y +
                            ShiftedTargetZ * face->pFacePlane_old.vNormal.z >
                        0)
                        continue;
                }
                v_4c = abs(-(face->pFacePlane_old.dist +
                             ShiftedTargetX * face->pFacePlane_old.vNormal.x +
                             ShiftedTargetY * face->pFacePlane_old.vNormal.y +
                             ShiftedTargetZ * face->pFacePlane_old.vNormal.z)) >>
                       14;
                if (v_4c <= abs(v91)) {
                    vd = fixpoint_div(v93, vc);
                    if (vd >= 0) {
                        if (PointInPolyIndoor(
                                ShiftedTargetX + ((signed int)(fixpoint_mul(vd, v144) +
                                                   0x8000) >>
                                      16),
                                ShiftedTargetY + ((signed int)(fixpoint_mul(vd, v80) +
                                                   0x8000) >>
                                      16),
                                ShiftedTargetZ + ((signed int)(fixpoint_mul(vd, v81) +
                                                   0x8000) >>
                                      16),
                                face)) {
                            LOS_Obscurred = 1;
                            break;
                        }
                    }
                }
            }
        }


    // outdooor
    } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        // offset 32 to side and check LOS
        Vec3_int_::Rotate(32, TrigLUT->uIntegerHalfPi + AngleToTarget, 0, TargetVec, &ShiftedTargetX, &ShiftedTargetY, &ShiftedTargetZ);
        Vec3_int_::Rotate(32, TrigLUT->uIntegerHalfPi + AngleToTarget, 0, Pos_From, &ShiftedFromX, &ShiftedFromY, &ShiftedFromz);
        dist_y = ShiftedFromY - ShiftedTargetY;
        dist_z = ShiftedFromz - ShiftedTargetZ;
        dist_x = ShiftedFromX - ShiftedTargetX;

        v9 = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
        v10 = 65536;
        if (v9) v10 = 65536 / v9;

        // normalising
        rayxnorm = dist_x * v10;
        rayznorm = dist_z * v10;
        rayynorm = dist_y * v10;

        xmax = std::max(ShiftedFromX, ShiftedTargetX);
        xmin = std::min(ShiftedFromX, ShiftedTargetX);

        ymax = std::max(ShiftedFromY, ShiftedTargetY);
        ymin = std::min(ShiftedFromY, ShiftedTargetY);

        zmax = std::max(ShiftedFromz, ShiftedTargetZ);
        zmin = std::min(ShiftedFromz, ShiftedTargetZ);

        for (BSPModel &model : pOutdoor->pBModels) {
            if (CalcDistPointToLine(ShiftedTargetX, ShiftedTargetY, ShiftedFromX, ShiftedFromY, model.vPosition.x, model.vPosition.y) <= model.sBoundingRadius + 128) {
                for (ODMFace &face : model.pFaces) {
                    v17 = fixpoint_mul(rayxnorm, face.pFacePlane.vNormal.x);
                    v18 = fixpoint_mul(rayynorm, face.pFacePlane.vNormal.y);
                    v19 = fixpoint_mul(rayznorm, face.pFacePlane.vNormal.z);

                    FaceIsParallel = v17 + v18 + v19 == 0;  // dot product implies face normal is perpendicular - face is parallel to LOS

                    dot_ray2 = v17 + v18 + v19;
                    dot_ray = v17 + v18 + v19;

                    // bounds check
                    if (xmin > face.pBoundingBox.x2 ||
                        xmax < face.pBoundingBox.x1 ||
                        ymin > face.pBoundingBox.y2 ||
                        ymax < face.pBoundingBox.y1 ||
                        zmin > face.pBoundingBox.z2 ||
                        zmax < face.pBoundingBox.z1 || FaceIsParallel)
                        continue;

                    // point to plane distacne
                    v23 = -(face.pFacePlane.dist +
                            ShiftedTargetX * face.pFacePlane.vNormal.x +
                            ShiftedTargetY * face.pFacePlane.vNormal.y +
                            ShiftedTargetZ * face.pFacePlane.vNormal.z);

                    // are we on same side of plane
                    if (dot_ray2 <= 0) {
                        // angle obtuse - is target underneath plane
                        if (face.pFacePlane.dist +
                                ShiftedTargetX * face.pFacePlane.vNormal.x +
                                ShiftedTargetY * face.pFacePlane.vNormal.y +
                                ShiftedTargetZ * face.pFacePlane.vNormal.z <
                            0)
                            continue;  // can never hit
                    } else {
                        // angle acute - is target above plane
                        if (face.pFacePlane.dist +
                                ShiftedTargetX * face.pFacePlane.vNormal.x +
                                ShiftedTargetY * face.pFacePlane.vNormal.y +
                                ShiftedTargetZ * face.pFacePlane.vNormal.z >
                            0)
                            continue;  // can never hit
                    }

                    v24 = abs(-(face.pFacePlane.dist +
                                ShiftedTargetX * face.pFacePlane.vNormal.x +
                                ShiftedTargetY * face.pFacePlane.vNormal.y +
                                ShiftedTargetZ * face.pFacePlane.vNormal.z)) >>
                          14;

                    // maybe some sort of epsilon check?
                    if (v24 <= abs(dot_ray2)) {
                        // calc how far along line interesction is
                        v110 = fixpoint_div(v23, dot_ray);

                        // less than zero means intersection is behind target point
                        if (v110 >= 0) {
                            if (PointInPolyOutdoor(
                                    ShiftedTargetX + ((signed int)(fixpoint_mul(v110, rayxnorm) + 0x8000) >> 16),
                                    ShiftedTargetY + ((signed int)(fixpoint_mul(v110, rayynorm) + 0x8000) >> 16),
                                    ShiftedTargetZ + ((signed int)(fixpoint_mul(v110, rayznorm) + 0x8000) >> 16),
                                    &face, &model.pVertices)) {
                                LOS_Obscurred2 = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // offset 32 to other side and check LOS
        Vec3_int_::Rotate(32, AngleToTarget - TrigLUT->uIntegerHalfPi, 0, TargetVec, &ShiftedTargetX, &ShiftedTargetY, &ShiftedTargetZ);
        Vec3_int_::Rotate(32, AngleToTarget - TrigLUT->uIntegerHalfPi, 0, Pos_From, &ShiftedFromX, &ShiftedFromY, &ShiftedFromz);
        dist_y = ShiftedFromY - ShiftedTargetY;
        dist_z = ShiftedFromz - ShiftedTargetZ;
        dist_x = ShiftedFromX - ShiftedTargetX;
        v32 = integer_sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);
        v33 = 65536;
        if (v32) v33 = 65536 / v32;
        v126 = dist_x * v33;
        v35 = dist_z * v33;
        v122 = dist_y * v33;

        v146 = std::max(ShiftedFromX, ShiftedTargetX);
        v150 = std::min(ShiftedFromX, ShiftedTargetX);

        v138 = std::max(ShiftedFromY, ShiftedTargetY);
        v142 = std::min(ShiftedFromY, ShiftedTargetY);

        v130 = std::max(ShiftedFromz, ShiftedTargetZ);
        v134 = std::min(ShiftedFromz, ShiftedTargetZ);

        for (BSPModel &model : pOutdoor->pBModels) {
            if (CalcDistPointToLine(ShiftedTargetX, ShiftedTargetY, ShiftedFromX, ShiftedFromY, model.vPosition.x,
                           model.vPosition.y) <= model.sBoundingRadius + 128) {
                for (ODMFace &face : model.pFaces) {
                    ya = fixpoint_mul(v126, face.pFacePlane.vNormal.x);
                    ve = fixpoint_mul(v122, face.pFacePlane.vNormal.y);
                    v_4 = fixpoint_mul(v35, face.pFacePlane.vNormal.z);
                    FaceIsParallel = ya + ve + v_4 == 0;
                    v40 = ya + ve + v_4;
                    va = ya + ve + v_4;
                    if (v150 > face.pBoundingBox.x2 ||
                        v146 < face.pBoundingBox.x1 ||
                        v142 > face.pBoundingBox.y2 ||
                        v138 < face.pBoundingBox.y1 ||
                        v134 > face.pBoundingBox.z2 ||
                        v130 < face.pBoundingBox.z1 || FaceIsParallel)
                        continue;
                    v42 = -(face.pFacePlane.dist +
                            ShiftedTargetX * face.pFacePlane.vNormal.x +
                            ShiftedTargetY * face.pFacePlane.vNormal.y +
                            ShiftedTargetZ * face.pFacePlane.vNormal.z);
                    if (v40 <= 0) {
                        if (face.pFacePlane.dist +
                                ShiftedTargetX * face.pFacePlane.vNormal.x +
                                ShiftedTargetY * face.pFacePlane.vNormal.y +
                                ShiftedTargetZ * face.pFacePlane.vNormal.z <
                            0)
                            continue;
                    } else {
                        if (face.pFacePlane.dist +
                                ShiftedTargetX * face.pFacePlane.vNormal.x +
                                ShiftedTargetY * face.pFacePlane.vNormal.y +
                                ShiftedTargetZ * face.pFacePlane.vNormal.z >
                            0)
                            continue;
                    }
                    v_4a = abs(-(face.pFacePlane.dist +
                                 ShiftedTargetX * face.pFacePlane.vNormal.x +
                                 ShiftedTargetY * face.pFacePlane.vNormal.y +
                                 ShiftedTargetZ * face.pFacePlane.vNormal.z)) >>
                           14;
                    if (v_4a <= abs(v40)) {
                        // LODWORD(v43) = v42 << 16;
                        // HIDWORD(v43) = v42 >> 16;
                        // vb = v43 / va;
                        vb = fixpoint_div(v42, va);
                        if (vb >= 0) {
                            if (PointInPolyOutdoor(ShiftedTargetX + ((int)(fixpoint_mul(vb, v126) +
                                                       0x8000) >>
                                                 16),
                                           ShiftedTargetY + ((int)(fixpoint_mul(vb, v122) +
                                                       0x8000) >>
                                                 16),
                                           ShiftedTargetZ + ((int)(fixpoint_mul(vb, v35) +
                                                       0x8000) >>
                                                 16),
                                           &face, &model.pVertices)) {
                                LOS_Obscurred = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return (!LOS_Obscurred2 || !LOS_Obscurred);  // true if LOS clear
}

char DoInteractionWithTopmostZObject(int pid) {
    auto id = PID_ID(pid);
    auto type = PID_TYPE(pid);

    switch (type) {
        case OBJECT_Item: {  // take the item
            if (pSpriteObjects[id].IsUnpickable() || id >= MAX_SPRITE_OBJECTS || !pSpriteObjects[id].uObjectDescID) {
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
                extern bool ActorInteraction(unsigned int id);
                ActorInteraction(id);
            }
            break;

        case OBJECT_Decoration:
            extern void DecorationInteraction(unsigned int id, unsigned int pid);
            DecorationInteraction(id, pid);
            break;

        default:
            logger->Warning("Warning: Invalid ID reached!");
            return 1;

        case OBJECT_BModel:
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
    int v1;                   // ebx@1
    int v2;                   // edi@1
    double v10;               // st7@27
    int new_party_z;          // esi@96
    int v38;                  // eax@96
    int v39;                  // ecx@106
    int v40;                  // eax@106
    int v42;                  // eax@120
    BLVFace *pFace;           // esi@126
    int v46;                  // ecx@133
    int v52;                  // eax@140
    int v54;                  // ebx@146
    unsigned int uFaceEvent;  // [sp+14h] [bp-4Ch]@1
    bool party_running_flag;  // [sp+1Ch] [bp-44h]@1
    bool bFeatherFall;        // [sp+24h] [bp-3Ch]@15
    unsigned int uSectorID;   // [sp+28h] [bp-38h]@1
    bool party_walking_flag;  // [sp+2Ch] [bp-34h]@1
    unsigned int uFaceID;     // [sp+30h] [bp-30h]@1
    int v80;                  // [sp+34h] [bp-2Ch]@1
    int v82;                  // [sp+3Ch] [bp-24h]@47
    int _view_angle;          // [sp+40h] [bp-20h]@47
    bool hovering;            // [sp+44h] [bp-1Ch]@1
    int new_party_y;          // [sp+48h] [bp-18h]@1
    int new_party_x;          // [sp+4Ch] [bp-14h]@1
    int party_z;              // [sp+50h] [bp-10h]@1
    int angle;                // [sp+5Ch] [bp-4h]@47

    uFaceEvent = 0;
    // v89 = pParty->uFallSpeed;
    v1 = 0;
    v2 = 0;
    new_party_x = pParty->vPosition.x;
    new_party_y = pParty->vPosition.y;
    party_z = pParty->vPosition.z;
    uSectorID = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                   pParty->vPosition.z);
    party_running_flag = false;
    party_walking_flag = false;
    hovering = false;

    uFaceID = -1;
    int floor_level =
        collide_against_floor(new_party_x, new_party_y, party_z + 40,
                              &uSectorID, &uFaceID);  //получить высоту пола

    if (pParty->bFlying) {  // отключить полёт
        pParty->bFlying = false;
        if (pParty->FlyActive())
            pOtherOverlayList
                ->pOverlays[pParty->pPartyBuffs[PARTY_BUFF_FLY].uOverlayID - 1]
                .field_E |= 1;
    }

    if (floor_level == -30000 || uFaceID == -1) {
        floor_level = collide_against_floor_approximate(
            new_party_x, new_party_y, party_z + 40, &uSectorID, &uFaceID);
        if (floor_level == -30000 || uFaceID == -1) {
            __debugbreak();  // level built with errors
            pParty->vPosition.x = blv_prev_party_x;
            pParty->vPosition.y = blv_prev_party_z;
            pParty->vPosition.z = blv_prev_party_y;
            pParty->uFallStartY = blv_prev_party_y;
            return;
        }
    }

    blv_prev_party_x = pParty->vPosition.x;
    blv_prev_party_z = pParty->vPosition.y;
    blv_prev_party_y = pParty->vPosition.z;
    if (!pParty->bTurnBasedModeOn) {
        static int dword_720CDC = 0;

        int v67 = OS_GetTime() / 500;
        if (dword_720CDC != v67) {
            dword_4F8580[3 * dword_4F8580[1]] = pParty->vPosition.x;
            dword_4F8580[3 * dword_4F8580[2]] = pParty->vPosition.y;
            dword_4F8580[3 * dword_4F8580[3]] = pParty->vPosition.z;
            if (dword_4F8580[0] > 60) dword_4F8580[0] = 1;

            dword_720CDC = v67;
        }
    }

    int fall_start;
    /*
    if (!pParty->FeatherFallActive())// не активно падение пера
    {
      bFeatherFall = false;
      if (!pParty->pPlayers[0].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) &&
    // grants feather fall
          !pParty->pPlayers[1].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) &&
          !pParty->pPlayers[2].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) &&
          !pParty->pPlayers[3].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT))
      {
        fall_start = pParty->uFallStartY;
      }
      else// was missing
      {
          fall_start = floor_level;
          bFeatherFall = true;
          pParty->uFallStartY = floor_level;
      }
    }
    else// активно падение пера
    {
      fall_start = floor_level;
      bFeatherFall = true;
      pParty->uFallStartY = floor_level;
    }

    Reworked condition below
    */
    if (pParty->FeatherFallActive() ||
        pParty->pPlayers[0].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) ||
        pParty->pPlayers[1].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) ||
        pParty->pPlayers[2].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT) ||
        pParty->pPlayers[3].WearsItemAnyWhere(ITEM_ARTIFACT_LADYS_ESCORT)) {
        fall_start = floor_level;
        bFeatherFall = true;
        pParty->uFallStartY = floor_level;
    } else {
        bFeatherFall = false;
        fall_start = pParty->uFallStartY;
    }

    if (fall_start - party_z > 512 && !bFeatherFall &&
        party_z <= floor_level + 1) {  // повреждение от падения с высоты
        assert(~pParty->uFlags & PARTY_FLAGS_1_LANDING);  // why land in indoor?
        if (pParty->uFlags & PARTY_FLAGS_1_LANDING) {
            pParty->uFlags &= ~PARTY_FLAGS_1_LANDING;
        } else {
            for (uint i = 0; i < 4; ++i) {  // receive falling damage
                if (!pParty->pPlayers[i].HasEnchantedItemEquipped(72) &&
                    !pParty->pPlayers[i].WearsItem(ITEM_ARTIFACT_HERMES_SANDALS,
                        EQUIP_BOOTS)) {
                    pParty->pPlayers[i].ReceiveDamage(
                        (pParty->uFallStartY - party_z) *
                        (0.1f * pParty->pPlayers[i].GetMaxHealth()) / 256,
                        DMGT_PHISYCAL);
                    v10 = (double)(20 - pParty->pPlayers[i].GetParameterBonus(
                        pParty->pPlayers[i]
                        .GetActualEndurance())) *
                        flt_6BE3A4_debug_recmod1 * 2.133333333333333;
                    pParty->pPlayers[i].SetRecoveryTime((signed __int64)v10);
                }
            }
        }
    }

    if (party_z > floor_level + 1) hovering = true;

    bool not_high_fall = false;

    if (party_z - floor_level <= 32) {
        pParty->uFallStartY = party_z;
        not_high_fall = true;
    }

    if (!engine->config->NoWalkSound() && pParty->walk_sound_timer) {  //таймеры для звуков передвижения
        if (pParty->walk_sound_timer > pEventTimer->uTimeElapsed)
            pParty->walk_sound_timer -= pEventTimer->uTimeElapsed;
        else
            pParty->walk_sound_timer = 0;
    }

    if (party_z <= floor_level + 1) {  // группа ниже уровня пола
        party_z = floor_level + 1;
        pParty->uFallStartY = floor_level + 1;

        if (!hovering && pParty->floor_face_pid != uFaceID) {  // не парящие и
            if (pIndoor->pFaces[uFaceID].uAttributes & FACE_PRESSURE_PLATE)
                uFaceEvent =
                    pIndoor->pFaceExtras[pIndoor->pFaces[uFaceID].uFaceExtraID]
                        .uEventID;
        }
    }
    if (!hovering) pParty->floor_face_pid = uFaceID;

    bool on_water = false;
    if (pIndoor->pFaces[uFaceID].Fluid())  // на воде
        on_water = true;

    // v81 = pParty->uWalkSpeed;
    angle = pParty->sRotationZ;
    _view_angle = pParty->sRotationX;
    v82 =
        (unsigned __int64)(pEventTimer->dt_in_some_format *
                           (signed __int64)((signed int)(pParty
                                                             ->y_rotation_speed *
                                                         TrigLUT
                                                             ->uIntegerPi) /
                                            180)) >>
        16;
    while (pPartyActionQueue->uNumActions) {
        switch (pPartyActionQueue->Next()) {
            case PARTY_TurnLeft:
                if (engine->config->turn_speed > 0)
                    angle = TrigLUT->uDoublePiMask & (angle + engine->config->turn_speed);
                else
                    angle = TrigLUT->uDoublePiMask & (angle + (int)(v82 * fTurnSpeedMultiplier));
                break;
            case PARTY_TurnRight:
                if (engine->config->turn_speed > 0)
                    angle = TrigLUT->uDoublePiMask & (angle - engine->config->turn_speed);
                else
                    angle = TrigLUT->uDoublePiMask & (angle - (int)(v82 * fTurnSpeedMultiplier));
                break;

            case PARTY_FastTurnLeft:
                if (engine->config->turn_speed > 0)
                    angle = TrigLUT->uDoublePiMask & (angle + engine->config->turn_speed);
                else
                    angle = TrigLUT->uDoublePiMask & (angle + (int)(2.0f * fTurnSpeedMultiplier * (double)v82));
                break;

            case PARTY_FastTurnRight:
                if (engine->config->turn_speed > 0)
                    angle = TrigLUT->uDoublePiMask & (angle - engine->config->turn_speed);
                else
                    angle = TrigLUT->uDoublePiMask & (angle - (int)(2.0f * fTurnSpeedMultiplier * (double)v82));
                break;

            case PARTY_StrafeLeft:
                v2 -= fixpoint_mul(TrigLUT->Sin(angle), pParty->uWalkSpeed * fWalkSpeedMultiplier / 2);
                v1 += fixpoint_mul(TrigLUT->Cos(angle), pParty->uWalkSpeed * fWalkSpeedMultiplier / 2);
                party_walking_flag = true;
                break;
            case PARTY_StrafeRight:
                v2 += fixpoint_mul(TrigLUT->Sin(angle), pParty->uWalkSpeed * fWalkSpeedMultiplier / 2);
                v1 -= fixpoint_mul(TrigLUT->Cos(angle), pParty->uWalkSpeed * fWalkSpeedMultiplier / 2);
                party_walking_flag = true;
                break;
            case PARTY_WalkForward:
                v2 += fixpoint_mul(TrigLUT->Cos(angle), /*2 **/ pParty->uWalkSpeed * fWalkSpeedMultiplier);
                v1 += fixpoint_mul(TrigLUT->Sin(angle), /*2 **/ pParty->uWalkSpeed * fWalkSpeedMultiplier);
                party_walking_flag = true;
                break;
            case PARTY_WalkBackward:
                v2 -= fixpoint_mul(TrigLUT->Cos(angle), pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier);
                v1 -= fixpoint_mul(TrigLUT->Sin(angle), pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier);
                party_walking_flag = true;
                break;
            case PARTY_RunForward:  //Бег вперёд
                v2 += fixpoint_mul(TrigLUT->Cos(angle), /*5*/2 * pParty->uWalkSpeed * fWalkSpeedMultiplier);
                v1 += fixpoint_mul(TrigLUT->Sin(angle), /*5*/2 * pParty->uWalkSpeed * fWalkSpeedMultiplier);
                party_running_flag = true;
                break;
            case PARTY_RunBackward:
                v2 -= fixpoint_mul(TrigLUT->Cos(angle), pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier);
                v1 -= fixpoint_mul(TrigLUT->Sin(angle), pParty->uWalkSpeed * fBackwardWalkSpeedMultiplier);
                party_running_flag = true;
                break;
            case PARTY_LookDown:
                _view_angle += (signed __int64)(flt_6BE150_look_up_down_dangle * 25.0);
                if (_view_angle > 128)
                    _view_angle = 128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_63, 0);
                break;
            case PARTY_LookUp:
                _view_angle += (signed __int64)(flt_6BE150_look_up_down_dangle * -25.0);
                if (_view_angle < -128)
                    _view_angle = -128;
                if (uActiveCharacter)
                    pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)SPEECH_64, 0);
                break;
            case PARTY_CenterView:
                _view_angle = 0;
                break;
            case PARTY_Jump:
                if ((!hovering ||
                     party_z <= floor_level + 6 && pParty->uFallSpeed <= 0) &&
                    pParty->field_24) {
                    hovering = true;
                    pParty->uFallSpeed += pParty->field_24 * 96;
                }
                break;
            default:
                break;
        }
    }
    pParty->sRotationZ = angle;
    pParty->sRotationX = _view_angle;
    if (hovering) {  // парящие
        pParty->uFallSpeed += -2 * pEventTimer->uTimeElapsed *
                              GetGravityStrength();  // расчёт скорости падения
        if (hovering && pParty->uFallSpeed <= 0) {
            if (pParty->uFallSpeed < -500 && !pParty->bFlying) {
                for (uint pl = 1; pl <= 4; pl++) {
                    if (!pPlayers[pl]->HasEnchantedItemEquipped(72) &&
                        !pPlayers[pl]->WearsItem(ITEM_ARTIFACT_HERMES_SANDALS,
                                                 EQUIP_BOOTS))  // was 8
                        pPlayers[pl]->PlayEmotion(CHARACTER_EXPRESSION_SCARED,
                                                  0);
                }
            }
        } else {
            pParty->uFallStartY = party_z;
        }
    } else {  // не парящие
        if (pIndoor->pFaces[uFaceID].pFacePlane_old.vNormal.z < 0x8000) {
            pParty->uFallSpeed -=
                pEventTimer->uTimeElapsed * GetGravityStrength();
            pParty->uFallStartY = party_z;
        } else {
            if (!(pParty->uFlags & PARTY_FLAGS_1_LANDING))
                pParty->uFallSpeed = 0;
            pParty->uFallStartY = party_z;
        }
    }
    if (v2 * v2 + v1 * v1 < 400) {
        v1 = 0;
        v2 = 0;
    }

    _actor_collision_struct.field_84 = -1;
    _actor_collision_struct.field_70 = 0;
    _actor_collision_struct.prolly_normal_d = pParty->field_14_radius;
    _actor_collision_struct.field_8_radius = pParty->field_14_radius / 2;
    _actor_collision_struct.field_0 = 1;
    _actor_collision_struct.height = pParty->uPartyHeight - 32;
    for (uint i = 0; i < 100; i++) {
        new_party_z = party_z;
        _actor_collision_struct.position.x = new_party_x;
        _actor_collision_struct.position.y = new_party_y;
        _actor_collision_struct.position.z = _actor_collision_struct.height + party_z + 1;

        _actor_collision_struct.normal.x = new_party_x;
        _actor_collision_struct.normal.y = new_party_y;
        _actor_collision_struct.normal.z = _actor_collision_struct.prolly_normal_d + party_z + 1;

        _actor_collision_struct.velocity.x = v2;
        _actor_collision_struct.velocity.y = v1;
        _actor_collision_struct.velocity.z = pParty->uFallSpeed;

        _actor_collision_struct.uSectorID = uSectorID;
        v38 = 0;
        if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
            v38 = 13312;
        }
        if (_actor_collision_struct.CalcMovementExtents(v38)) break;
        for (uint j = 0; j < 100; ++j) {
            _46E44E_collide_against_faces_and_portals(1);
            _46E0B2_collide_against_decorations();  //столкновения с декором
            for (v80 = 0; v80 < (signed int)uNumActors; ++v80)
                Actor::_46DF1A_collide_against_actor(
                    v80, 0);  //столкновения с монстрами
            if (_46F04E_collide_against_portals())  //столкновения с порталами
                break;
        }
        if (_actor_collision_struct.field_7C >= _actor_collision_struct.field_6C) {
            v39 = _actor_collision_struct.normal2.x;
            uSectorID = _actor_collision_struct.normal2.y;
            v40 = _actor_collision_struct.normal2.z - _actor_collision_struct.prolly_normal_d - 1;
        } else {
            v39 = new_party_x +
                  fixpoint_mul(_actor_collision_struct.field_7C, _actor_collision_struct.direction.x);
            uSectorID = new_party_y + fixpoint_mul(_actor_collision_struct.field_7C,
                                                   _actor_collision_struct.direction.y);
            v40 = new_party_z +
                  fixpoint_mul(_actor_collision_struct.field_7C, _actor_collision_struct.direction.z);
        }
        v42 = collide_against_floor(v39, uSectorID, v40 + 40,
                                    &_actor_collision_struct.uSectorID, &uFaceID);
        if (v42 == -30000 || v42 - new_party_z > 128) return;
        if (_actor_collision_struct.field_7C >= _actor_collision_struct.field_6C) {  // ???
            new_party_x = _actor_collision_struct.normal2.x;
            new_party_y = _actor_collision_struct.normal2.y;
            new_party_z =
                _actor_collision_struct.normal2.z - _actor_collision_struct.prolly_normal_d - 1;
            break;
        }
        new_party_x +=
            fixpoint_mul(_actor_collision_struct.field_7C, _actor_collision_struct.direction.x);
        new_party_y +=
            fixpoint_mul(_actor_collision_struct.field_7C, _actor_collision_struct.direction.y);
        uSectorID = _actor_collision_struct.uSectorID;
        _actor_collision_struct.field_70 += _actor_collision_struct.field_7C;
        unsigned long long v87 =
            new_party_z +
            fixpoint_mul(_actor_collision_struct.field_7C, _actor_collision_struct.direction.z);
        if (PID_TYPE(_actor_collision_struct.pid) ==
            OBJECT_Actor) {  // invis break on actor collision    /    при
                             // столкновении с монстром
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active()) {
                pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Reset();
            }
            viewparams->bRedrawGameUI = true;
        } else if (PID_TYPE(_actor_collision_struct.pid) ==
                   OBJECT_Decoration) {  // decoration collision   /   при
                                         // столкновении с декорацией
            v54 = TrigLUT->Atan2(
                new_party_x -
                    pLevelDecorations[_actor_collision_struct.pid >> 3].vPosition.x,
                new_party_y -
                    pLevelDecorations[_actor_collision_struct.pid >> 3].vPosition.y);
            v2 = fixpoint_mul(TrigLUT->Cos(v54),
                              integer_sqrt(v2 * v2 + v1 * v1));
            v1 = fixpoint_mul(TrigLUT->Sin(v54),
                              integer_sqrt(v2 * v2 + v1 * v1));
        } else if (PID_TYPE(_actor_collision_struct.pid) ==
                   OBJECT_BModel) {  // при столкновении с bmodel
            pFace = &pIndoor->pFaces[(signed int)_actor_collision_struct.pid >> 3];
            if (pFace->uPolygonType == POLYGON_Floor) {  // если bmodel - пол
                if (pParty->uFallSpeed < 0) pParty->uFallSpeed = 0;
                v87 = pIndoor->pVertices[*pFace->pVertexIDs].z + 1;
                if (pParty->uFallStartY - v87 < 512) pParty->uFallStartY = v87;
                if (v2 * v2 + v1 * v1 < 400) {
                    v1 = 0;
                    v2 = 0;
                }
                if (pParty->floor_face_pid != PID_ID(_actor_collision_struct.pid) &&
                    pFace->Pressure_Plate())
                    uFaceEvent =
                        pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
            } else {  // если не пол
                v46 = pParty->uFallSpeed * pFace->pFacePlane_old.vNormal.z;
                if (pFace->uPolygonType !=
                    POLYGON_InBetweenFloorAndWall) {  // полез на холм
                    v80 = abs(v1 * pFace->pFacePlane_old.vNormal.y + v46 +
                              v2 * pFace->pFacePlane_old.vNormal.x) >>
                          16;
                    if ((_actor_collision_struct.speed >> 3) > v80)
                        v80 = _actor_collision_struct.speed >> 3;
                    v2 += fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.x);
                    v1 += fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.y);
                    pParty->uFallSpeed +=
                        fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.z);
                    // v80 = pFace->pFacePlane_old.vNormal.y;
                    v52 = _actor_collision_struct.prolly_normal_d -
                          ((pFace->pFacePlane_old.dist +
                            v87 * pFace->pFacePlane_old.vNormal.z +
                            new_party_y * pFace->pFacePlane_old.vNormal.y +
                            new_party_x * pFace->pFacePlane_old.vNormal.x) >>
                           16);
                    if (v52 > 0) {
                        new_party_x +=
                            fixpoint_mul(v52, pFace->pFacePlane_old.vNormal.x);
                        new_party_y +=
                            fixpoint_mul(v52, pFace->pFacePlane_old.vNormal.y);
                        v87 +=
                            fixpoint_mul(v52, pFace->pFacePlane_old.vNormal.z);
                    }
                    if (pParty->floor_face_pid != PID_ID(_actor_collision_struct.pid) &&
                        pFace->Pressure_Plate())
                        uFaceEvent =
                            pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
                }
                if (pFace->uPolygonType == POLYGON_InBetweenFloorAndWall) {
                    v80 = abs(v1 * pFace->pFacePlane_old.vNormal.y + v46 +
                              v2 * pFace->pFacePlane_old.vNormal.x) >>
                          16;
                    if ((_actor_collision_struct.speed >> 3) > v80)
                        v80 = _actor_collision_struct.speed >> 3;
                    v2 += fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.x);
                    v1 += fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.y);
                    pParty->uFallSpeed +=
                        fixpoint_mul(v80, pFace->pFacePlane_old.vNormal.z);
                    if (v2 * v2 + v1 * v1 >= 400) {
                        if (pParty->floor_face_pid != PID_ID(_actor_collision_struct.pid) &&
                            pFace->Pressure_Plate())
                            uFaceEvent =
                                pIndoor->pFaceExtras[pFace->uFaceExtraID]
                                    .uEventID;
                    } else {
                        v2 = 0;
                        v1 = 0;
                        pParty->uFallSpeed = 0;
                    }
                }
            }
        }
        v2 = fixpoint_mul(58500, v2);
        v1 = fixpoint_mul(58500, v1);
        pParty->uFallSpeed = fixpoint_mul(58500, pParty->uFallSpeed);
    }

    //  //Воспроизведение звуков ходьбы/бега-------------------------
    uint pX_ = abs(pParty->vPosition.x - new_party_x);
    uint pY_ = abs(pParty->vPosition.y - new_party_y);
    uint pZ_ = abs(pParty->vPosition.z - new_party_z);
    if (!engine->config->NoWalkSound() && pParty->walk_sound_timer <= 0) {
        pAudioPlayer->StopAll(804);  // stop sound
        if (party_running_flag &&
            (!hovering || not_high_fall)) {  // Бег и (не прыжок или не высокое падение )
            if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) >= 16) {
                if (on_water)
                    pAudioPlayer->PlaySound(SOUND_RunWaterIndoor, 804, 1, -1, 0, 0);
                else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_CARPET)  //по ковру
                    pAudioPlayer->PlaySound(SOUND_RunCarpet, -1 /*804*/, 1, -1, 0, 0);
                else
                    pAudioPlayer->PlaySound(SOUND_RunWood, -1 /*804*/, 1, -1, 0, 0);
                pParty->walk_sound_timer = 96;  // 64
            }
        } else if (party_walking_flag &&
                   (!hovering || not_high_fall)) {  // Ходьба и (не прыжок или не
                                                    // высокое падение)
            if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) >= 8) {
                if (on_water)
                    pAudioPlayer->PlaySound(SOUND_WalkWaterIndoor, 804, 1, -1, 0, 0);
                else if (pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_CARPET)  //по ковру
                    pAudioPlayer->PlaySound(SOUND_WalkCarpet, -1 /*804*/, 1, -1, 0, 0);
                else
                    pAudioPlayer->PlaySound(SOUND_WalkWood, -1 /*804*/, 1, -1, 0, 0);
                pParty->walk_sound_timer = 144;  // 64
            }
        }
    }
    if (integer_sqrt(pX_ * pX_ + pY_ * pY_ + pZ_ * pZ_) <
        8)  //отключить  звук ходьбы при остановке
        pAudioPlayer->StopAll(804);
    //-------------------------------------------------------------
    if (!hovering || !not_high_fall)
        pParty->uFlags &= ~PARTY_FLAGS_1_FALLING;
    else
        pParty->uFlags |= PARTY_FLAGS_1_FALLING;
    pParty->uFlags &= ~PARTY_FLAGS_1_BURNING;
    pParty->vPosition.x = new_party_x;
    pParty->vPosition.z = new_party_z;
    pParty->vPosition.y = new_party_y;
    // pParty->uFallSpeed = v89;
    if (!hovering && pIndoor->pFaces[uFaceID].uAttributes & FACE_INDOOR_LAVA)
        pParty->uFlags |= PARTY_FLAGS_1_BURNING;  // 0x200
    if (uFaceEvent) EventProcessor(uFaceEvent, 0, 1);
}

//----- (00449A49) --------------------------------------------------------
void Door_switch_animation(unsigned int uDoorID, int a2) {
    int old_state;       // eax@1
    signed int door_id;  // esi@2

    if (!pIndoor->pDoors) return;
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
        if (pIndoor->pDoors[door_id].uState) {
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

int GetAlertStatus() {
    int result;

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        result = pOutdoor->ddm.field_C_alert;
    else
        result = uCurrentlyLoadedLevelType == LEVEL_Outdoor ? pIndoor->dlv.field_C_alert : 0;

    return result;
}

int SpawnEncounterMonsters(MapInfo *map_info, int enc_index) {
    // creates random spawn point for encounter
    bool failed_point = false;
    float angle_from_party;
    int dist_from_party;
    SpawnPointMM7 enc_spawn_point;
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
        for (loop_cnt; loop_cnt < 100; ++loop_cnt) {
            // random x,y at distance from party
            dist_from_party = rand() % 1024 + 512;
            angle_from_party = ((rand() % (signed int)TrigLUT->uIntegerDoublePi) * 2 * pi) / TrigLUT->uIntegerDoublePi;
            enc_spawn_point.vPosition.x = pParty->vPosition.x + cos(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.y = pParty->vPosition.y + sin(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.z = pParty->vPosition.z;
            enc_spawn_point.uIndex = enc_index;

            // get proposed floor level
            enc_spawn_point.vPosition.z = ODM_GetFloorLevel(
                enc_spawn_point.vPosition.x, enc_spawn_point.vPosition.y, pParty->vPosition.z,
                0, &bInWater, &modelPID, 0);

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
        int party_sectorID = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);
        int mon_sectorID;
        int indoor_floor_level;
        unsigned int uFaceID;

        // 100 attempts to make a usuable spawn point
        for (loop_cnt = 0; loop_cnt < 100; ++loop_cnt) {
            // random x,y at distance from party
            dist_from_party = rand() % 512 + 256;
            angle_from_party = ((rand() % (signed int)TrigLUT->uIntegerDoublePi) * 2 * pi) / TrigLUT->uIntegerDoublePi;
            enc_spawn_point.vPosition.x = pParty->vPosition.x + cos(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.y = pParty->vPosition.y + sin(angle_from_party) * dist_from_party;
            enc_spawn_point.vPosition.z = pParty->vPosition.z;
            enc_spawn_point.uIndex = enc_index;

            // get proposed sector
            mon_sectorID = pIndoor->GetSector(enc_spawn_point.vPosition.x, enc_spawn_point.vPosition.y, pParty->vPosition.z);
            if (mon_sectorID == party_sectorID) {
                // check proposed floor level
                indoor_floor_level = BLV_GetFloorLevel(enc_spawn_point.vPosition.x, enc_spawn_point.vPosition.y,
                                        enc_spawn_point.vPosition.z, mon_sectorID, &uFaceID);
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

int DropTreasureAt(int trs_level, int trs_type, int x, int y, int z, uint16_t facing) {
    SpriteObject a1;
    pItemsTable->GenerateItem(trs_level, trs_type, &a1.containing_item);
    a1.uType = (SPRITE_OBJECT_TYPE)pItemsTable->pItems[a1.containing_item.uItemID].uSpriteID;
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

bool PointInPolyIndoor(int x, int y, int z, BLVFace *face) {
    // check if point is inside polygon - LOS check indoors

    // store for projected coords
    int ProjCoord_X;
    int ProjCoord_Y;
    std::array<int, 52> ProjCoords_Ys;
    std::array<int, 52> ProjCoords_Xs;

    // project/flatten polygon onto primary plane
    if (face->uAttributes & FACE_XY_PLANE) {
        ProjCoord_X = x;
        ProjCoord_Y = y;
        for (int i = 0; i < face->uNumVertices; i++) {
            ProjCoords_Xs[i] = pIndoor->pVertices[face->pVertexIDs[i]].x;
            ProjCoords_Ys[i] = pIndoor->pVertices[face->pVertexIDs[i]].y;
        }
    } else {
        ProjCoord_Y = z;
        if (face->uAttributes & FACE_XZ_PLANE) {
            ProjCoord_X = x;
            for (int i = 0; i < face->uNumVertices; i++) {
                ProjCoords_Xs[i] = pIndoor->pVertices[face->pVertexIDs[i]].x;
                ProjCoords_Ys[i] = pIndoor->pVertices[face->pVertexIDs[i]].z;
            }
        } else {
            ProjCoord_X = y;
            for (int i = 0; i < face->uNumVertices; i++) {
                ProjCoords_Xs[i] = pIndoor->pVertices[face->pVertexIDs[i]].y;
                ProjCoords_Ys[i] = pIndoor->pVertices[face->pVertexIDs[i]].z;
            }
        }
    }

    // copy first vert to last
    ProjCoords_Xs[face->uNumVertices] = ProjCoords_Xs[0];
    ProjCoords_Ys[face->uNumVertices] = ProjCoords_Ys[0];

    float LineSlope;
    float LinePoint;
    int InsideLine = 0;

    // loop over verts
    for (int i = 0; i < face->uNumVertices && InsideLine < 2; i++) {
        // XOR check if Y is within vert y edges
        if ((ProjCoords_Ys[i] >= ProjCoord_Y) ^ (ProjCoords_Ys[i + 1] >= ProjCoord_Y)) {
            // elimate by x
            if (!(ProjCoords_Xs[i + 1] >= ProjCoord_X && ProjCoords_Xs[i] < ProjCoord_X)) {
                // check within x edges
                if ((ProjCoords_Xs[i + 1] < ProjCoord_X && ProjCoords_Xs[i] >= ProjCoord_X)) {
                   ++InsideLine;
                } else {
                    // slope of line connecting verts
                    LineSlope = (float)(ProjCoords_Xs[i + 1] - ProjCoords_Xs[i]) / (float)(ProjCoords_Ys[i + 1] - ProjCoords_Ys[i]);
                    // calc x coords given y coords
                    LinePoint = ProjCoords_Xs[i] + (LineSlope * (ProjCoord_Y - ProjCoords_Ys[i]) + 0.5);
                    // test against actual x to left
                    if (LinePoint >= ProjCoord_X) ++InsideLine;
                }
            }
        }
    }

    // intersection must be unique
    if (InsideLine != 1) return false;
    return true;
}

bool PointInPolyOutdoor(int x, int y, int z, ODMFace *face, BSPVertexBuffer *verts) {
    // check if point is inside polygon - LOS check outdoors

    // store for projected coords
    int ProjCoord_X;
    int ProjCoord_Y;
    std::array<int, 52> ProjCoords_Ys;
    std::array<int, 52> ProjCoords_Xs;

    // project/flatten polygon onto primary plane
    if (face->uAttributes & FACE_XY_PLANE) {
        ProjCoord_X = x;
        ProjCoord_Y = y;
        for (int i = 0; i < face->uNumVertices; i++) {
            ProjCoords_Xs[i + 1] = verts->pVertices[face->pVertexIDs[i]].x;
            ProjCoords_Ys[i + 1] = verts->pVertices[face->pVertexIDs[i]].y;
        }
    } else {
        ProjCoord_Y = z;
        if (face->uAttributes & FACE_XY_PLANE) {
            ProjCoord_X = x;
            for (int i = 0; i < face->uNumVertices; i++) {
                ProjCoords_Xs[i + 1] = verts->pVertices[face->pVertexIDs[i]].x;
                ProjCoords_Ys[i + 1] = verts->pVertices[face->pVertexIDs[i]].z;
            }
        } else {
            ProjCoord_X = y;
            for (int i = 0; i < face->uNumVertices; i++) {
                ProjCoords_Xs[i + 1] = verts->pVertices[face->pVertexIDs[i]].y;
                ProjCoords_Ys[i + 1] = verts->pVertices[face->pVertexIDs[i]].z;
            }
        }
    }

    // copy first vert to last
    ProjCoords_Xs[face->uNumVertices] = ProjCoords_Xs[0];
    ProjCoords_Ys[face->uNumVertices] = ProjCoords_Ys[0];

    float LineSlope;
    float LinePoint;
    int InsideLine = 0;

    // loop over verts
    for (int i = 0; i < face->uNumVertices && InsideLine < 2; i++) {
        // XOR check if Y is within vert y edges
        if ((ProjCoords_Ys[i] >= ProjCoord_Y) ^ (ProjCoords_Ys[i + 1] >= ProjCoord_Y)) {
            // elimate by x
            if (!(ProjCoords_Xs[i + 1] >= ProjCoord_X && ProjCoords_Xs[i] < ProjCoord_X)) {
                // check within x edges
                if ((ProjCoords_Xs[i + 1] < ProjCoord_X && ProjCoords_Xs[i] >= ProjCoord_X)) {
                    ++InsideLine;
                } else {
                    // slope of line connecting verts
                    LineSlope = (float)(ProjCoords_Xs[i + 1] - ProjCoords_Xs[i]) / (float)(ProjCoords_Ys[i + 1] - ProjCoords_Ys[i]);
                    // calc x coords given y coords
                    LinePoint = ProjCoords_Xs[i] + (LineSlope * (ProjCoord_Y - ProjCoords_Ys[i]) + 0.5);
                    // test against actual x to left
                    if (LinePoint >= ProjCoord_X) ++InsideLine;
                }
            }
        }
    }

    // intersection must be unique
    if (InsideLine != 1) return false;
    return true;
}

//----- (0049B04D) --------------------------------------------------------
void stru154::GetFacePlaneAndClassify(ODMFace *a2, BSPVertexBuffer *a3) {
    Vec3_float_ OutPlaneNorm;
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
void stru154::ClassifyPolygon(Vec3_float_ *pNormal, float dist) {
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
void stru154::GetFacePlane(ODMFace *pFace, BSPVertexBuffer *pVertices,
                           Vec3_float_ *pOutNormal, float *pOutDist) {
    Vec3_float_ FirstPairVec;
    Vec3_float_ SecPairVec;
    Vec3_float_ *CPReturn;
    Vec3_float_ CrossProd;

    if (pFace->uNumVertices >= 2) {
        for (int i = 0; i < pFace->uNumVertices - 2; i++) {
            FirstPairVec.x = pVertices->pVertices[pFace->pVertexIDs[i + 1]].x -
                   pVertices->pVertices[pFace->pVertexIDs[i]].x;
            FirstPairVec.y = pVertices->pVertices[pFace->pVertexIDs[i + 1]].y -
                   pVertices->pVertices[pFace->pVertexIDs[i]].y;
            FirstPairVec.z = pVertices->pVertices[pFace->pVertexIDs[i + 1]].z -
                   pVertices->pVertices[pFace->pVertexIDs[i]].z;

            SecPairVec.x = pVertices->pVertices[pFace->pVertexIDs[i + 2]].x -
                  pVertices->pVertices[pFace->pVertexIDs[i + 1]].x;
            SecPairVec.y = pVertices->pVertices[pFace->pVertexIDs[i + 2]].y -
                  pVertices->pVertices[pFace->pVertexIDs[i + 1]].y;
            SecPairVec.z = pVertices->pVertices[pFace->pVertexIDs[i + 2]].z -
                  pVertices->pVertices[pFace->pVertexIDs[i + 1]].z;

            CPReturn = Vec3_float_::Cross(&FirstPairVec, &CrossProd, SecPairVec.x, SecPairVec.y, SecPairVec.z);

            if (CrossProd.x != 0.0 || CrossProd.y != 0.0 || CrossProd.z != 0.0) {
                CrossProd.Normalize();

                pOutNormal->x = CrossProd.x;
                pOutNormal->y = CrossProd.y;
                pOutNormal->z = CrossProd.z;

                *pOutDist = -(pVertices->pVertices[pFace->pVertexIDs[i]].x * CrossProd.x +
                      pVertices->pVertices[pFace->pVertexIDs[i]].y * CrossProd.y +
                      pVertices->pVertices[pFace->pVertexIDs[i]].z * CrossProd.z);
                return;
            }
        }
    }

    // only one/two vert?
    __debugbreak();
    pOutNormal->x = (double)(pFace->pFacePlane.vNormal.x & 0xFFFF) / 65535.0f +
                    (double)(pFace->pFacePlane.vNormal.x >> 16);
    pOutNormal->y = (double)(pFace->pFacePlane.vNormal.y & 0xFFFF) / 65535.0f +
                    (double)(pFace->pFacePlane.vNormal.y >> 16);
    pOutNormal->z = (double)(pFace->pFacePlane.vNormal.z & 0xFFFF) / 65535.0f +
                    (double)(pFace->pFacePlane.vNormal.z >> 16);
    *pOutDist = (double)(pFace->pFacePlane.dist & 0xFFFF) / 65535.0f +
                (double)(pFace->pFacePlane.dist >> 16);
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

//----- (0047272C) --------------------------------------------------------
int collide_against_floor_approximate(int x, int y, int z,
                                      unsigned int *pSectorID,
                                      unsigned int *pFaceID) {
    int result;  // eax@1

    *pSectorID = pIndoor->GetSector(x - 2, y, z + 40);
    result = collide_against_floor(x - 2, y, z + 40, pSectorID, pFaceID);
    if (result == -30000 || !*pSectorID) {
        *pSectorID = pIndoor->GetSector(x + 2, y, z + 40);
        result = collide_against_floor(x + 2, y, z + 40, pSectorID, pFaceID);
        if (result == -30000 || !*pSectorID) {
            *pSectorID = pIndoor->GetSector(x, y - 2, z + 40);
            result =
                collide_against_floor(x, y - 2, z + 40, pSectorID, pFaceID);
            if (result == -30000 || !*pSectorID) {
                *pSectorID = pIndoor->GetSector(x, y + 2, z + 40);
                result =
                    collide_against_floor(x, y + 2, z + 40, pSectorID, pFaceID);
                if (result == -30000 || !*pSectorID) {
                    *pSectorID = pIndoor->GetSector(x, y, z + 140);
                    result = collide_against_floor(x, y, z + 140, pSectorID,
                                                   pFaceID);
                }
            }
        }
    }
    return result;
}

//----- (0047050A) --------------------------------------------------------
int stru141_actor_collision_object::CalcMovementExtents(int dt) {  // true if no movement
    int result = 1;
    int v17;            // eax@5
    int v18;            // eax@7
    int v21;            // eax@9
    int v22;            // eax@11

    int speed = 1 | integer_sqrt(this->velocity.z * this->velocity.z +
                                 this->velocity.y * this->velocity.y +
                                 this->velocity.x * this->velocity.x);

    this->direction.x = 65536 / speed * this->velocity.x;
    this->direction.y = 65536 / speed * this->velocity.y;
    this->direction.z = 65536 / speed * this->velocity.z;

    this->speed = speed;
    this->inv_speed = 65536 / speed;

    int timedelta = dt;
    if (!timedelta) timedelta = pEventTimer->dt_in_some_format;

    // v8 = fixpoint_mul(timedelta, speed) - this->field_70; // speed * dt - something
    this->field_6C = fixpoint_mul(timedelta, speed) - this->field_70;
    if (this->field_6C > 0) {
        this->field_4C = fixpoint_mul(this->field_6C, this->direction.x) + this->normal.x;
        this->normal2.x = fixpoint_mul(this->field_6C, this->direction.x) + this->normal.x;

        this->field_50 = fixpoint_mul(this->field_6C, this->direction.y) + this->normal.y;
        this->normal2.y = fixpoint_mul(this->field_6C, this->direction.y) + this->normal.y;

        this->normal2.z = fixpoint_mul(this->field_6C, this->direction.z) + this->normal.z;
        this->field_54 = fixpoint_mul(this->field_6C, this->direction.z) + this->position.z;


        v17 = this->normal.x;
        if (v17 >= this->normal2.x) v17 = this->normal2.x;
        this->sMaxX = v17 - this->prolly_normal_d;
        v18 = this->prolly_normal_d + this->normal.x;
        if (this->normal.x <= this->normal2.x)
            v18 = this->prolly_normal_d + this->normal2.x;
        // v19 = this->normal.y;
        // v20 = this->normal2.y;
        this->sMinX = v18;


        v21 = this->normal.y;
        if (v21 >= this->normal2.y) v21 = this->normal2.y;
        this->sMaxY = v21 - this->prolly_normal_d;
        v22 = this->prolly_normal_d + this->normal.y;
        if (this->normal.y <= this->normal2.y)
            v22 = this->normal2.y + this->prolly_normal_d;
        // v23 = this->normal2.z;
        this->sMinY = v22;


        // v24 = this->normal.z;
        if (this->normal.z >= this->normal2.z)
            this->sMaxZ = this->normal2.z - this->prolly_normal_d;
        else
            this->sMaxZ = this->normal.z - this->prolly_normal_d;
        // this->sMaxZ = v25;
        // v26 = this->field_8_radius;
        if (this->position.z <=
            this->position.z + fixpoint_mul(this->field_6C, this->direction.z))
            this->sMinZ = (this->position.z +
                           fixpoint_mul(this->field_6C, this->direction.z)) +
                          this->field_8_radius;
        else
            this->sMinZ = this->position.z + this->field_8_radius;



        this->pid = 0;
        this->field_80 = -1;
        this->field_88 = -1;
        // this->sMinZ = v27;
        this->field_7C = 0xFFFFFFu;
        result = 0;
    }
    return result;
}
